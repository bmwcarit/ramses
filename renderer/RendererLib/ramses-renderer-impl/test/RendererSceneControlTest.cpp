//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>
#include "gmock/gmock.h"
#include "ramses-renderer-api/RamsesRenderer.h"
#include "ramses-renderer-api/RendererSceneControl.h"
#include "ramses-renderer-api/DisplayConfig.h"
#include "ramses-renderer-api/Types.h"
#include "ramses-renderer-api/IRendererSceneControlEventHandler.h"
#include "ramses-framework-api/RamsesFrameworkConfig.h"
#include "ramses-framework-api/RamsesFramework.h"
#include "ramses-renderer-api/IRendererEventHandler.h"

#include "RamsesRendererImpl.h"
#include "RendererSceneControlImpl.h"
#include "RendererSceneControlEventHandlerMock.h"
#include "RendererEventCollector.h"
#include "RendererLib/RendererCommands.h"
#include <thread>

using namespace testing;

namespace ramses
{
    class ARendererSceneControl : public ::testing::Test
    {
    protected:
        ARendererSceneControl()
            : m_renderer(*m_framework.createRenderer({}))
            , m_sceneControlAPI(*m_renderer.getSceneControlAPI())
            , m_pendingCommands(m_sceneControlAPI.impl.getPendingCommands())
            , m_eventsFromRenderer(m_renderer.impl.getRenderer().getEventCollector())
            , m_displayId(m_renderer.createDisplay(DisplayConfig{}))
        {
            m_renderer.setLoopMode(ELoopMode_UpdateOnly);
        }

        void expectCommand(uint32_t index, ramses_internal::ERendererCommand commandType)
        {
            ASSERT_LT(index, m_pendingCommands.getCommands().getTotalCommandCount());
            EXPECT_EQ(commandType, m_pendingCommands.getCommands().getCommandType(index));
        }

        void expectCommandCount(uint32_t count)
        {
            ASSERT_EQ(count, m_pendingCommands.getCommands().getTotalCommandCount());
        }

        void dispatchSceneControlEvents()
        {
            // do one loop to collect internal events to be dispatched
            m_renderer.doOneLoop();
            m_sceneControlAPI.dispatchEvents(m_eventHandler);
        }

    protected:
        RamsesFramework m_framework;
        RamsesRenderer& m_renderer;
        RendererSceneControl& m_sceneControlAPI;
        StrictMock<RendererSceneControlEventHandlerMock> m_eventHandler;
        const ramses_internal::RendererCommands& m_pendingCommands;
        ramses_internal::RendererEventCollector& m_eventsFromRenderer; // to simulate input from internal renderer
        const displayId_t m_displayId;
    };

    TEST_F(ARendererSceneControl, hasNoCommandsOnStartUp)
    {
        expectCommandCount(0u);
    }

    TEST_F(ARendererSceneControl, clearsAllPendingCommandsWhenCallingFlush)
    {
        EXPECT_EQ(StatusOK, m_sceneControlAPI.setSceneState(sceneId_t{ 1u }, RendererSceneState::Available));
        expectCommandCount(1u);
        m_sceneControlAPI.flush();
        expectCommandCount(0u);
    }

    TEST_F(ARendererSceneControl, createsCommandMakeSceneAvailable)
    {
        constexpr sceneId_t scene{ 1u };
        EXPECT_EQ(StatusOK, m_sceneControlAPI.setSceneState(scene, RendererSceneState::Available));
        expectCommandCount(1u);
        expectCommand(0u, ramses_internal::ERendererCommand_SetSceneState);

        const auto& cmd = m_pendingCommands.getCommands().getCommandData<ramses_internal::SceneStateCommand>(0);
        EXPECT_EQ(scene.getValue(), cmd.sceneId.getValue());
        EXPECT_EQ(ramses_internal::RendererSceneState::Available, cmd.state);
    }

    TEST_F(ARendererSceneControl, createsCommandMakeSceneRendered)
    {
        constexpr sceneId_t scene{ 1u };
        // has to set mapping before attempting to make scene rendered
        m_sceneControlAPI.setSceneMapping(scene, displayId_t{ 2 });
        m_sceneControlAPI.flush();
        EXPECT_EQ(StatusOK, m_sceneControlAPI.setSceneState(scene, RendererSceneState::Rendered));
        expectCommandCount(1u);
        expectCommand(0u, ramses_internal::ERendererCommand_SetSceneState);

        const auto& cmd = m_pendingCommands.getCommands().getCommandData<ramses_internal::SceneStateCommand>(0);
        EXPECT_EQ(scene.getValue(), cmd.sceneId.getValue());
        EXPECT_EQ(ramses_internal::RendererSceneState::Rendered, cmd.state);
    }

    TEST_F(ARendererSceneControl, createsCommandForDataLink)
    {
        constexpr sceneId_t scene1{ 1u };
        constexpr sceneId_t scene2{ 2u };
        constexpr dataProviderId_t providerId{ 3u };
        constexpr dataConsumerId_t consumerId{ 4u };

        EXPECT_EQ(StatusOK, m_sceneControlAPI.linkData(scene1, providerId, scene2, consumerId));
        expectCommandCount(1u);
        expectCommand(0u, ramses_internal::ERendererCommand_LinkSceneData);

        const auto& cmd = m_pendingCommands.getCommands().getCommandData<ramses_internal::DataLinkCommand>(0);
        EXPECT_EQ(scene1.getValue(), cmd.providerScene.getValue());
        EXPECT_EQ(providerId.getValue(), cmd.providerData.getValue());
        EXPECT_EQ(scene2.getValue(), cmd.consumerScene.getValue());
        EXPECT_EQ(consumerId.getValue(), cmd.consumerData.getValue());
    }

    TEST_F(ARendererSceneControl, createsComandForUnlinkData)
    {
        constexpr sceneId_t scene{ 1u };
        constexpr dataConsumerId_t consumerId{ 4u };

        m_sceneControlAPI.unlinkData(scene, consumerId);
        expectCommandCount(1u);
        expectCommand(0u, ramses_internal::ERendererCommand_UnlinkSceneData);

        const auto& cmd = m_pendingCommands.getCommands().getCommandData<ramses_internal::DataLinkCommand>(0);
        EXPECT_EQ(scene.getValue(), cmd.consumerScene.getValue());
        EXPECT_EQ(consumerId.getValue(), cmd.consumerData.getValue());
    }

    TEST_F(ARendererSceneControl, createsCommandForHandlePickEvent)
    {
        constexpr ramses::sceneId_t scene(0u);
        EXPECT_EQ(ramses::StatusOK, m_sceneControlAPI.handlePickEvent(scene, 1, 2));
        expectCommandCount(1u);
        expectCommand(0u, ramses_internal::ERendererCommand_PickEvent);
    }

    TEST_F(ARendererSceneControl, createsCommandForAssignSceneToOffscreenBuffer)
    {
        constexpr sceneId_t scene{ 1u };
        constexpr displayBufferId_t ob{ 5u };

        // must set mapping before assigning to display buffer
        EXPECT_EQ(StatusOK, m_sceneControlAPI.setSceneMapping(scene, m_displayId));

        EXPECT_EQ(StatusOK, m_sceneControlAPI.setSceneDisplayBufferAssignment(scene, ob, 11));
        expectCommandCount(2u);
        expectCommand(0u, ramses_internal::ERendererCommand_SetSceneMapping);
        expectCommand(1u, ramses_internal::ERendererCommand_SetSceneDisplayBufferAssignment);

        const auto& cmd = m_pendingCommands.getCommands().getCommandData<ramses_internal::SceneMappingCommand>(1);
        EXPECT_EQ(scene.getValue(), cmd.sceneId.getValue());
        EXPECT_EQ(ob.getValue(), cmd.offscreenBuffer.asMemoryHandle());
        EXPECT_EQ(11, cmd.sceneRenderOrder);
    }

    TEST_F(ARendererSceneControl, createsCommandForAssignSceneToFramebufferUsingInvalidDisplayBufferId)
    {
        constexpr sceneId_t scene{ 1u };

        // must set mapping before assigning to display buffer
        EXPECT_EQ(StatusOK, m_sceneControlAPI.setSceneMapping(scene, m_displayId));

        EXPECT_EQ(StatusOK, m_sceneControlAPI.setSceneDisplayBufferAssignment(scene, {}, 11));
        expectCommandCount(2u);
        expectCommand(0u, ramses_internal::ERendererCommand_SetSceneMapping);
        expectCommand(1u, ramses_internal::ERendererCommand_SetSceneDisplayBufferAssignment);

        const auto& cmd = m_pendingCommands.getCommands().getCommandData<ramses_internal::SceneMappingCommand>(1);
        EXPECT_EQ(scene.getValue(), cmd.sceneId.getValue());
        EXPECT_FALSE(cmd.offscreenBuffer.isValid());
        EXPECT_EQ(11, cmd.sceneRenderOrder);
    }

    TEST_F(ARendererSceneControl, failsToAssignSceneWithNoMappingInfo)
    {
        constexpr sceneId_t sceneWithoutMapping{ 11 };
        EXPECT_NE(StatusOK, m_sceneControlAPI.setSceneDisplayBufferAssignment(sceneWithoutMapping, m_renderer.getDisplayFramebuffer(m_displayId)));
        EXPECT_NE(StatusOK, m_sceneControlAPI.setSceneDisplayBufferAssignment(sceneWithoutMapping, displayBufferId_t{ 123 }));
        expectCommandCount(0u);
    }

    TEST_F(ARendererSceneControl, createsCommandForOffscreenBufferLink)
    {
        constexpr displayBufferId_t bufferId{ 1u };
        constexpr sceneId_t consumerScene{ 2u };
        constexpr dataConsumerId_t dataConsumer{ 3u };

        EXPECT_EQ(StatusOK, m_sceneControlAPI.linkOffscreenBuffer(bufferId, consumerScene, dataConsumer));
        expectCommandCount(1u);
        expectCommand(0u, ramses_internal::ERendererCommand_LinkBufferToSceneData);

        const auto& cmd = m_pendingCommands.getCommands().getCommandData<ramses_internal::DataLinkCommand>(0);
        EXPECT_EQ(bufferId.getValue(), cmd.providerBuffer.asMemoryHandle());
        EXPECT_EQ(consumerScene.getValue(), cmd.consumerScene.getValue());
        EXPECT_EQ(dataConsumer.getValue(), cmd.consumerData.getValue());
    }

    TEST_F(ARendererSceneControl, failsToMakeSceneReadyOrRenderedIfNoMappingSet)
    {
        const sceneId_t sceneWithNoMapping{ 1234 };
        EXPECT_NE(StatusOK, m_sceneControlAPI.setSceneState(sceneWithNoMapping, RendererSceneState::Ready));
        EXPECT_NE(StatusOK, m_sceneControlAPI.setSceneState(sceneWithNoMapping, RendererSceneState::Rendered));
    }

    TEST_F(ARendererSceneControl, failsToChangeMappingPropertiesIfSceneAlreadyReadyOrRendered)
    {
        constexpr sceneId_t scene{ 1u };
        constexpr ramses_internal::SceneId sceneInternal{ scene.getValue() };
        constexpr displayId_t display{ 2u };
        constexpr displayId_t otherDisplay{ 3u };

        InSequence seq;
        EXPECT_EQ(StatusOK, m_sceneControlAPI.setSceneMapping(scene, display));
        EXPECT_EQ(StatusOK, m_sceneControlAPI.setSceneState(scene, RendererSceneState::Ready));
        EXPECT_EQ(StatusOK, m_sceneControlAPI.flush());

        EXPECT_CALL(m_eventHandler, sceneStateChanged(scene, RendererSceneState::Available));
        m_eventsFromRenderer.addSceneEvent(ramses_internal::ERendererEventType_SceneStateChanged, sceneInternal, ramses_internal::RendererSceneState::Available);
        EXPECT_CALL(m_eventHandler, sceneStateChanged(scene, RendererSceneState::Ready));
        m_eventsFromRenderer.addSceneEvent(ramses_internal::ERendererEventType_SceneStateChanged, sceneInternal, ramses_internal::RendererSceneState::Ready);
        dispatchSceneControlEvents();

        // scene is now in ready state and will fail to change mapping
        EXPECT_NE(StatusOK, m_sceneControlAPI.setSceneMapping(scene, otherDisplay));

        EXPECT_EQ(StatusOK, m_sceneControlAPI.setSceneState(scene, RendererSceneState::Rendered));
        EXPECT_EQ(StatusOK, m_sceneControlAPI.flush());
        EXPECT_CALL(m_eventHandler, sceneStateChanged(scene, RendererSceneState::Rendered));
        m_eventsFromRenderer.addSceneEvent(ramses_internal::ERendererEventType_SceneStateChanged, sceneInternal, ramses_internal::RendererSceneState::Rendered);
        dispatchSceneControlEvents();

        // scene is now in rendered state and will fail to change mapping
        EXPECT_NE(StatusOK, m_sceneControlAPI.setSceneMapping(scene, otherDisplay));

        // to clear commands
        EXPECT_EQ(StatusOK, m_sceneControlAPI.flush());
    }

    TEST_F(ARendererSceneControl, failsToChangeMappingPropertiesIfSceneAlreadySetToReadyOrRendered)
    {
        constexpr sceneId_t scene{ 1u };
        constexpr displayId_t display{ 2u };
        constexpr displayId_t otherDisplay{ 3u };

        // set initial mapping
        EXPECT_EQ(StatusOK, m_sceneControlAPI.setSceneMapping(scene, display));

        EXPECT_EQ(StatusOK, m_sceneControlAPI.setSceneState(scene, RendererSceneState::Rendered));
        EXPECT_NE(StatusOK, m_sceneControlAPI.setSceneMapping(scene, otherDisplay));

        EXPECT_EQ(StatusOK, m_sceneControlAPI.setSceneState(scene, RendererSceneState::Ready));
        EXPECT_NE(StatusOK, m_sceneControlAPI.setSceneMapping(scene, otherDisplay));
    }

    TEST_F(ARendererSceneControl, canRunRendererInItsOwnThreadAndCallSceneAPIMethods)
    {
        class DisplayCreateEventHandler final : public RendererEventHandlerEmpty
        {
        public:
            explicit DisplayCreateEventHandler(RamsesRenderer& renderer)
                : m_renderer(renderer)
            {
            }

            virtual void displayCreated(displayId_t, ERendererEventResult) override
            {
                m_displayCreated = true;
            }

            void waitForDisplayCreated()
            {
                while (!m_displayCreated)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds{ 5 });
                    m_renderer.dispatchEvents(*this);
                }
            }

        private:
            RamsesRenderer& m_renderer;
            bool m_displayCreated = false;
        };

        EXPECT_EQ(StatusOK, m_renderer.startThread());

        // this is just to make sure the renderer is already running in thread
        DisplayCreateEventHandler rndHandler(m_renderer);
        m_renderer.createDisplay({});
        m_renderer.flush();
        rndHandler.waitForDisplayCreated();

        // most of these will fail but the purpose is to create and submit renderer commands for renderer running in another thread
        // thread sanitizer or other analyzer would catch race conditions when running this test
        constexpr sceneId_t scene{ 123u };
        m_sceneControlAPI.setSceneState(scene, RendererSceneState::Available);
        m_sceneControlAPI.linkData(scene, dataProviderId_t(1u), sceneId_t(2u), dataConsumerId_t(3u));
        m_sceneControlAPI.unlinkData(scene, dataConsumerId_t(1u));
        m_sceneControlAPI.handlePickEvent(scene, 1.f, 2.f);
        EXPECT_EQ(StatusOK, m_sceneControlAPI.flush());

        m_sceneControlAPI.setSceneDisplayBufferAssignment(scene, displayBufferId_t{ 12 }, 11);
        m_sceneControlAPI.linkOffscreenBuffer(displayBufferId_t{ 12 }, scene, dataConsumerId_t(0u));
        EXPECT_EQ(StatusOK, m_sceneControlAPI.flush());

        RendererSceneControlEventHandlerEmpty handler;
        EXPECT_EQ(StatusOK, m_sceneControlAPI.dispatchEvents(handler));

        EXPECT_EQ(StatusOK, m_renderer.stopThread());
    }

    TEST_F(ARendererSceneControl, dispatchesEventsForSceneStateChanges)
    {
        constexpr sceneId_t scene{ 1u };
        constexpr ramses_internal::SceneId sceneInternal{ scene.getValue() };
        constexpr displayId_t display{ 2u };

        InSequence seq;
        EXPECT_EQ(StatusOK, m_sceneControlAPI.setSceneMapping(scene, display));
        EXPECT_EQ(StatusOK, m_sceneControlAPI.setSceneState(scene, RendererSceneState::Rendered));
        EXPECT_EQ(StatusOK, m_sceneControlAPI.flush());

        // from published to rendered
        EXPECT_CALL(m_eventHandler, sceneStateChanged(scene, RendererSceneState::Available));
        m_eventsFromRenderer.addSceneEvent(ramses_internal::ERendererEventType_SceneStateChanged, sceneInternal, ramses_internal::RendererSceneState::Available);
        EXPECT_CALL(m_eventHandler, sceneStateChanged(scene, RendererSceneState::Ready));
        m_eventsFromRenderer.addSceneEvent(ramses_internal::ERendererEventType_SceneStateChanged, sceneInternal, ramses_internal::RendererSceneState::Ready);
        EXPECT_CALL(m_eventHandler, sceneStateChanged(scene, RendererSceneState::Rendered));
        m_eventsFromRenderer.addSceneEvent(ramses_internal::ERendererEventType_SceneStateChanged, sceneInternal, ramses_internal::RendererSceneState::Rendered);

        dispatchSceneControlEvents();

        // back to unavailable
        EXPECT_EQ(StatusOK, m_sceneControlAPI.setSceneState(scene, RendererSceneState::Available));
        EXPECT_EQ(StatusOK, m_sceneControlAPI.flush());
        EXPECT_CALL(m_eventHandler, sceneStateChanged(scene, RendererSceneState::Ready));
        m_eventsFromRenderer.addSceneEvent(ramses_internal::ERendererEventType_SceneStateChanged, sceneInternal, ramses_internal::RendererSceneState::Ready);
        EXPECT_CALL(m_eventHandler, sceneStateChanged(scene, RendererSceneState::Available));
        m_eventsFromRenderer.addSceneEvent(ramses_internal::ERendererEventType_SceneStateChanged, sceneInternal, ramses_internal::RendererSceneState::Available);

        dispatchSceneControlEvents();
    }

    TEST_F(ARendererSceneControl, dispatchesOffscreenBufferLinkedEventFromRenderer)
    {
        constexpr sceneId_t consumerScene{ 3 };
        constexpr ramses_internal::SceneId consumerSceneInternal{ consumerScene.getValue() };
        constexpr dataConsumerId_t consumerId{ 4 };
        constexpr ramses_internal::DataSlotId consumerIdInternal{ consumerId.getValue() };
        constexpr displayBufferId_t offscreenBufferId{ 5 };
        constexpr ramses_internal::OffscreenBufferHandle obInternal{ offscreenBufferId.getValue() };

        InSequence seq;
        EXPECT_CALL(m_eventHandler, offscreenBufferLinked(offscreenBufferId, consumerScene, consumerId, true));
        m_eventsFromRenderer.addBufferLinkEvent(ramses_internal::ERendererEventType_SceneDataBufferLinked, obInternal, consumerSceneInternal, consumerIdInternal);

        EXPECT_CALL(m_eventHandler, offscreenBufferLinked(offscreenBufferId, consumerScene, consumerId, false));
        m_eventsFromRenderer.addBufferLinkEvent(ramses_internal::ERendererEventType_SceneDataBufferLinkFailed, obInternal, consumerSceneInternal, consumerIdInternal);

        dispatchSceneControlEvents();
    }

    TEST_F(ARendererSceneControl, dispatchesDataLinkedEventFromRenderer)
    {
        constexpr sceneId_t providerScene{ 1 };
        constexpr ramses_internal::SceneId providerSceneInternal{ providerScene.getValue() };
        constexpr dataProviderId_t providerId{ 2 };
        constexpr ramses_internal::DataSlotId providerIdInternal{ providerId.getValue() };
        constexpr sceneId_t consumerScene{ 3 };
        constexpr ramses_internal::SceneId consumerSceneInternal{ consumerScene.getValue() };
        constexpr dataConsumerId_t consumerId{ 4 };
        constexpr ramses_internal::DataSlotId consumerIdInternal{ consumerId.getValue() };

        InSequence seq;
        EXPECT_CALL(m_eventHandler, dataLinked(providerScene, providerId, consumerScene, consumerId, true));
        m_eventsFromRenderer.addDataLinkEvent(ramses_internal::ERendererEventType_SceneDataLinked, providerSceneInternal, consumerSceneInternal, providerIdInternal, consumerIdInternal);

        EXPECT_CALL(m_eventHandler, dataLinked(providerScene, providerId, consumerScene, consumerId, false));
        m_eventsFromRenderer.addDataLinkEvent(ramses_internal::ERendererEventType_SceneDataLinkFailed, providerSceneInternal, consumerSceneInternal, providerIdInternal, consumerIdInternal);

        dispatchSceneControlEvents();
    }

    TEST_F(ARendererSceneControl, dispatchesDataUnlinkedEventFromRenderer)
    {
        constexpr sceneId_t consumerScene{ 3 };
        constexpr ramses_internal::SceneId consumerSceneInternal{ consumerScene.getValue() };
        constexpr dataConsumerId_t consumerId{ 4 };
        constexpr ramses_internal::DataSlotId consumerIdInternal{ consumerId.getValue() };

        InSequence seq;
        EXPECT_CALL(m_eventHandler, dataUnlinked(consumerScene, consumerId, true));
        m_eventsFromRenderer.addDataLinkEvent(ramses_internal::ERendererEventType_SceneDataUnlinked, {}, consumerSceneInternal, {}, consumerIdInternal);

        EXPECT_CALL(m_eventHandler, dataUnlinked(consumerScene, consumerId, false));
        m_eventsFromRenderer.addDataLinkEvent(ramses_internal::ERendererEventType_SceneDataUnlinkFailed, {}, consumerSceneInternal, {}, consumerIdInternal);

        // this event is ignored and will be removed
        m_eventsFromRenderer.addDataLinkEvent(ramses_internal::ERendererEventType_SceneDataUnlinkedAsResultOfClientSceneChange, {}, consumerSceneInternal, {}, consumerIdInternal);

        dispatchSceneControlEvents();
    }

    TEST_F(ARendererSceneControl, dispatchesHandlePickEventFromRenderer)
    {
        constexpr sceneId_t providerScene{ 3 };
        constexpr ramses_internal::SceneId providerSceneInternal{ providerScene.getValue() };
        constexpr  pickableObjectId_t pickable1{ 567u };
        constexpr  pickableObjectId_t pickable2{ 578u };
        constexpr ramses_internal::PickableObjectId pickable1Internal{pickable1.getValue()};
        constexpr ramses_internal::PickableObjectId pickable2Internal{pickable2.getValue()};
        constexpr std::array<pickableObjectId_t, 2> pickableObjects{ pickable1, pickable2 };

        EXPECT_CALL(m_eventHandler, objectsPicked(providerScene, _, static_cast<uint32_t>(pickableObjects.size()))).WillOnce(Invoke([&](auto, auto pickedObjects, auto pickedObjectsCount)
        {
            ASSERT_EQ(static_cast<uint32_t>(pickableObjects.size()), pickedObjectsCount);
            for (uint32_t i = 0u; i < pickedObjectsCount; ++i)
                EXPECT_EQ(pickableObjects[i], pickedObjects[i]);
        }));
        m_eventsFromRenderer.addPickedEvent(ramses_internal::ERendererEventType_ObjectsPicked, providerSceneInternal, {pickable1Internal, pickable2Internal});

        dispatchSceneControlEvents();
    }

    TEST_F(ARendererSceneControl, dispatchesNamedFlushEventFromRenderer)
    {
        constexpr sceneId_t scene{ 3 };
        constexpr sceneVersionTag_t tag{ 4 };

        EXPECT_CALL(m_eventHandler, sceneFlushed(scene, tag));
        m_eventsFromRenderer.addSceneFlushEvent(ramses_internal::ERendererEventType_SceneFlushed, ramses_internal::SceneId{ scene.getValue() }, ramses_internal::SceneVersionTag{ tag });

        dispatchSceneControlEvents();
    }

    TEST_F(ARendererSceneControl, dispatchesSceneExpirationEventFromRenderer)
    {
        constexpr sceneId_t scene{ 3 };

        InSequence seq;
        EXPECT_CALL(m_eventHandler, sceneExpirationMonitoringEnabled(scene));
        m_eventsFromRenderer.addSceneExpirationEvent(ramses_internal::ERendererEventType_SceneExpirationMonitoringEnabled, ramses_internal::SceneId{ scene.getValue() });

        EXPECT_CALL(m_eventHandler, sceneExpired(scene));
        m_eventsFromRenderer.addSceneExpirationEvent(ramses_internal::ERendererEventType_SceneExpired, ramses_internal::SceneId{ scene.getValue() });

        EXPECT_CALL(m_eventHandler, sceneRecoveredFromExpiration(scene));
        m_eventsFromRenderer.addSceneExpirationEvent(ramses_internal::ERendererEventType_SceneRecoveredFromExpiration, ramses_internal::SceneId{ scene.getValue() });

        EXPECT_CALL(m_eventHandler, sceneExpirationMonitoringDisabled(scene));
        m_eventsFromRenderer.addSceneExpirationEvent(ramses_internal::ERendererEventType_SceneExpirationMonitoringDisabled, ramses_internal::SceneId{ scene.getValue() });

        dispatchSceneControlEvents();
    }

    TEST_F(ARendererSceneControl, dispatchesStreamAvailabilityEventFromRenderer)
    {
        constexpr waylandIviSurfaceId_t stream{ 3 };

        InSequence seq;
        EXPECT_CALL(m_eventHandler, streamAvailabilityChanged(stream, true));
        m_eventsFromRenderer.addStreamSourceEvent(ramses_internal::ERendererEventType_StreamSurfaceAvailable, ramses_internal::WaylandIviSurfaceId{ stream.getValue() });

        EXPECT_CALL(m_eventHandler, streamAvailabilityChanged(stream, false));
        m_eventsFromRenderer.addStreamSourceEvent(ramses_internal::ERendererEventType_StreamSurfaceUnavailable, ramses_internal::WaylandIviSurfaceId{ stream.getValue() });

        dispatchSceneControlEvents();
    }

    TEST_F(ARendererSceneControl, dispatchesDataSlotEventsFromRenderer)
    {
        InSequence seq;
        EXPECT_CALL(m_eventHandler, dataProviderCreated(sceneId_t{ 123u }, dataProviderId_t{ 1u }));
        m_eventsFromRenderer.addDataLinkEvent(ramses_internal::ERendererEventType_SceneDataSlotProviderCreated, ramses_internal::SceneId{ 123u }, {}, ramses_internal::DataSlotId{ 1u }, {});

        EXPECT_CALL(m_eventHandler, dataProviderDestroyed(sceneId_t{ 124u }, dataProviderId_t{ 2u }));
        m_eventsFromRenderer.addDataLinkEvent(ramses_internal::ERendererEventType_SceneDataSlotProviderDestroyed, ramses_internal::SceneId{ 124u }, {}, ramses_internal::DataSlotId{ 2u }, {});

        EXPECT_CALL(m_eventHandler, dataConsumerCreated(sceneId_t{ 125u }, dataConsumerId_t{ 3u }));
        m_eventsFromRenderer.addDataLinkEvent(ramses_internal::ERendererEventType_SceneDataSlotConsumerCreated, {}, ramses_internal::SceneId{ 125u }, {}, ramses_internal::DataSlotId{ 3u });

        EXPECT_CALL(m_eventHandler, dataConsumerDestroyed(sceneId_t{ 126u }, dataConsumerId_t{ 4u }));
        m_eventsFromRenderer.addDataLinkEvent(ramses_internal::ERendererEventType_SceneDataSlotConsumerDestroyed, {}, ramses_internal::SceneId{ 126u }, {}, ramses_internal::DataSlotId{ 4u });

        dispatchSceneControlEvents();
    }

    TEST_F(ARendererSceneControl, canInvokeAPICommandsInEventHandler)
    {
        class RecursiveEventTestHandler final : public RendererSceneControlEventHandlerEmpty
        {
        public:
            RecursiveEventTestHandler(RendererSceneControl& sceneControl, ramses_internal::RendererEventCollector& collector, RamsesRenderer& renderer)
                : m_sceneControl(sceneControl)
                , m_eventsFromRenderer(collector)
                , m_renderer(renderer)
            {
                m_renderer.setLoopMode(ELoopMode_UpdateOnly);
            }

            virtual void sceneStateChanged(sceneId_t sceneId, RendererSceneState state) override
            {
                switch (state)
                {
                case RendererSceneState::Unavailable:
                    break;
                case RendererSceneState::Available:
                    EXPECT_EQ(StatusOK, m_sceneControl.setSceneMapping(sceneId, displayId_t{ 1 }));
                    EXPECT_EQ(StatusOK, m_sceneControl.setSceneState(sceneId, RendererSceneState::Ready));
                    EXPECT_EQ(StatusOK, m_sceneControl.flush());
                    m_eventsFromRenderer.addSceneEvent(ramses_internal::ERendererEventType_SceneStateChanged, ramses_internal::SceneId{ sceneId.getValue() }, ramses_internal::RendererSceneState::Ready);
                    break;
                case RendererSceneState::Ready:
                    EXPECT_EQ(StatusOK, m_sceneControl.setSceneState(sceneId, RendererSceneState::Rendered));
                    EXPECT_EQ(StatusOK, m_sceneControl.flush());
                    m_eventsFromRenderer.addSceneEvent(ramses_internal::ERendererEventType_SceneStateChanged, ramses_internal::SceneId{ sceneId.getValue() }, ramses_internal::RendererSceneState::Rendered);
                    break;
                case RendererSceneState::Rendered:
                    m_reachedRenderedState = true;
                    break;
                }
            }

            bool waitForRenderedStateReached()
            {
                int loopsLeft = 10; // avoid infinite loop if expectation not reached
                while (!m_reachedRenderedState && --loopsLeft >= 0)
                {
                    m_sceneControl.dispatchEvents(*this);
                    m_renderer.doOneLoop();
                }

                return m_reachedRenderedState;
            }

            RendererSceneControl& m_sceneControl;
            ramses_internal::RendererEventCollector& m_eventsFromRenderer;
            RamsesRenderer& m_renderer;
            bool m_reachedRenderedState = false;
        };

        // actual commands ending up at renderer will fail but that is not the purpose of this test
        // we simulate responses from renderer

        RecursiveEventTestHandler handler(m_sceneControlAPI, m_eventsFromRenderer, m_renderer);

        m_eventsFromRenderer.addSceneEvent(ramses_internal::ERendererEventType_SceneStateChanged, ramses_internal::SceneId{ 123 }, ramses_internal::RendererSceneState::Available);
        EXPECT_TRUE(handler.waitForRenderedStateReached());
    }
}
