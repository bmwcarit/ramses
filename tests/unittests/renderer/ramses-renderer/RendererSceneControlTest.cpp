//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>
#include "gmock/gmock.h"
#include "ramses/renderer/RamsesRenderer.h"
#include "ramses/renderer/RendererSceneControl.h"
#include "ramses/renderer/DisplayConfig.h"
#include "ramses/renderer/Types.h"
#include "ramses/renderer/IRendererSceneControlEventHandler.h"
#include "ramses/framework/RamsesFrameworkConfig.h"
#include "ramses/framework/RamsesFramework.h"
#include "ramses/renderer/IRendererEventHandler.h"

#include "impl/RamsesRendererImpl.h"
#include "impl/RendererSceneControlImpl.h"
#include "internal/RendererLib/RendererEventCollector.h"
#include "internal/RendererLib/RendererCommands.h"
#include "RendererSceneControlEventHandlerMock.h"
#include "RendererCommandVisitorMock.h"
#include "internal/Core/Utils/ThreadLocalLog.h"
#include "PlatformFactoryMock.h"
#include <thread>

using namespace testing;

namespace ramses::internal
{
    class ARendererSceneControl : public ::testing::Test
    {
    protected:
        ARendererSceneControl()
            : m_renderer(*m_framework.createRenderer({}))
            , m_sceneControlAPI(*m_renderer.getSceneControlAPI())
            , m_pendingCommands(m_sceneControlAPI.impl().getPendingCommands())
        {
            m_renderer.impl().getDisplayDispatcher().injectPlatformFactory(std::make_unique<PlatformFactoryNiceMock>());
            m_displayId = m_renderer.createDisplay(ramses::DisplayConfig{});

            ThreadLocalLog::SetPrefix(2);

            m_renderer.setLoopMode(ELoopMode::UpdateOnly);
        }

        void simulateSceneEventFromRenderer(ERendererEventType eventType, SceneId sceneId, RendererSceneState state)
        {
            RendererEvent event{ eventType };
            event.sceneId = sceneId;
            event.state = state;
            m_renderer.impl().getDisplayDispatcher().injectSceneControlEvent(std::move(event));
        }

        void submitEventsFromRenderer()
        {
            RendererEventVector rendererEvts;
            RendererEventVector sceneEvts;
            m_eventsFromRenderer.appendAndConsumePendingEvents(rendererEvts, sceneEvts);
            for (auto&& evt : rendererEvts)
                m_renderer.impl().getDisplayDispatcher().injectSceneControlEvent(std::move(evt));
            for (auto&& evt : sceneEvts)
                m_renderer.impl().getDisplayDispatcher().injectSceneControlEvent(std::move(evt));
        }

        void dispatchSceneControlEvents()
        {
            // do one loop to collect internal events to be dispatched
            m_renderer.doOneLoop();
            m_sceneControlAPI.dispatchEvents(m_eventHandler);
        }

    protected:
        RamsesFramework m_framework{ RamsesFrameworkConfig{EFeatureLevel_Latest} };
        RamsesRenderer& m_renderer;
        RendererSceneControl& m_sceneControlAPI;
        RendererEventCollector m_eventsFromRenderer; // to simulate events coming from internal renderer (alternative to mock)
        StrictMock<RendererSceneControlEventHandlerMock> m_eventHandler;
        const RendererCommands& m_pendingCommands;
        displayId_t m_displayId;
        StrictMock<RendererCommandVisitorMock> m_cmdVisitor;
    };

    TEST_F(ARendererSceneControl, hasNoCommandsOnStartUp)
    {
        m_cmdVisitor.visit(m_pendingCommands);
    }

    TEST_F(ARendererSceneControl, clearsAllPendingCommandsWhenCallingFlush)
    {
        EXPECT_TRUE(m_sceneControlAPI.setSceneState(sceneId_t{ 1u }, RendererSceneState::Available));
        m_sceneControlAPI.flush();
        m_cmdVisitor.visit(m_pendingCommands);
    }

    TEST_F(ARendererSceneControl, createsCommandMakeSceneAvailable)
    {
        constexpr sceneId_t scene{ 1u };
        EXPECT_TRUE(m_sceneControlAPI.setSceneState(scene, RendererSceneState::Available));
        EXPECT_CALL(m_cmdVisitor, setSceneState(SceneId{ scene.getValue() }, RendererSceneState::Available));
        m_cmdVisitor.visit(m_pendingCommands);
    }

    TEST_F(ARendererSceneControl, createsCommandMakeSceneRendered)
    {
        constexpr sceneId_t scene{ 1u };
        // has to set mapping before attempting to make scene rendered
        m_sceneControlAPI.setSceneMapping(scene, displayId_t{ 2 });
        m_sceneControlAPI.flush();
        EXPECT_TRUE(m_sceneControlAPI.setSceneState(scene, RendererSceneState::Rendered));
        EXPECT_CALL(m_cmdVisitor, setSceneState(SceneId{ scene.getValue() }, RendererSceneState::Rendered));
        m_cmdVisitor.visit(m_pendingCommands);
    }

    TEST_F(ARendererSceneControl, createsCommandForDataLink)
    {
        constexpr sceneId_t scene1{ 1u };
        constexpr sceneId_t scene2{ 2u };
        constexpr dataProviderId_t providerId{ 3u };
        constexpr dataConsumerId_t consumerId{ 4u };

        EXPECT_TRUE(m_sceneControlAPI.linkData(scene1, providerId, scene2, consumerId));
        EXPECT_CALL(m_cmdVisitor, handleSceneDataLinkRequest(
            SceneId{ scene1.getValue() },
            DataSlotId{ providerId.getValue() },
            SceneId{ scene2.getValue() },
            DataSlotId{ consumerId.getValue() }));
        m_cmdVisitor.visit(m_pendingCommands);
    }

    TEST_F(ARendererSceneControl, createsComandForUnlinkData)
    {
        constexpr sceneId_t scene{ 1u };
        constexpr dataConsumerId_t consumerId{ 4u };

        m_sceneControlAPI.unlinkData(scene, consumerId);
        EXPECT_CALL(m_cmdVisitor, handleDataUnlinkRequest(
            SceneId{ scene.getValue() },
            DataSlotId{ consumerId.getValue() }));
        m_cmdVisitor.visit(m_pendingCommands);
    }

    TEST_F(ARendererSceneControl, createsCommandForHandlePickEvent)
    {
        constexpr sceneId_t scene(0u);
        EXPECT_TRUE(m_sceneControlAPI.handlePickEvent(scene, 1, 2));
        EXPECT_CALL(m_cmdVisitor, handlePick(
            SceneId{ scene.getValue() },
            glm::vec2{ 1, 2 }));
        m_cmdVisitor.visit(m_pendingCommands);
    }

    TEST_F(ARendererSceneControl, createsCommandForAssignSceneToOffscreenBuffer)
    {
        constexpr sceneId_t scene{ 1u };
        constexpr displayBufferId_t ob{ 5u };

        // must set mapping before assigning to display buffer
        EXPECT_TRUE(m_sceneControlAPI.setSceneMapping(scene, m_displayId));
        EXPECT_TRUE(m_sceneControlAPI.setSceneDisplayBufferAssignment(scene, ob, 11));
        InSequence seq;
        EXPECT_CALL(m_cmdVisitor, setSceneMapping(SceneId{ scene.getValue() }, DisplayHandle{ m_displayId.getValue() }));
        EXPECT_CALL(m_cmdVisitor, setSceneDisplayBufferAssignment(SceneId{ scene.getValue() }, OffscreenBufferHandle{ ob.getValue() }, 11));
        m_cmdVisitor.visit(m_pendingCommands);
    }

    TEST_F(ARendererSceneControl, createsCommandForAssignSceneToFramebufferUsingInvalidDisplayBufferId)
    {
        constexpr sceneId_t scene{ 1u };

        // must set mapping before assigning to display buffer
        EXPECT_TRUE(m_sceneControlAPI.setSceneMapping(scene, m_displayId));
        EXPECT_TRUE(m_sceneControlAPI.setSceneDisplayBufferAssignment(scene, {}, 11));
        InSequence seq;
        EXPECT_CALL(m_cmdVisitor, setSceneMapping(SceneId{ scene.getValue() }, DisplayHandle{ m_displayId.getValue() }));
        EXPECT_CALL(m_cmdVisitor, setSceneDisplayBufferAssignment(SceneId{ scene.getValue() }, OffscreenBufferHandle{}, 11));
        m_cmdVisitor.visit(m_pendingCommands);
    }

    TEST_F(ARendererSceneControl, failsToAssignSceneWithNoMappingInfo)
    {
        constexpr sceneId_t sceneWithoutMapping{ 11 };
        EXPECT_FALSE(m_sceneControlAPI.setSceneDisplayBufferAssignment(sceneWithoutMapping, m_renderer.getDisplayFramebuffer(m_displayId)));
        EXPECT_FALSE(m_sceneControlAPI.setSceneDisplayBufferAssignment(sceneWithoutMapping, displayBufferId_t{ 123 }));
        m_cmdVisitor.visit(m_pendingCommands);
    }

    TEST_F(ARendererSceneControl, createsCommandForOffscreenBufferLink)
    {
        constexpr displayBufferId_t bufferId{ 1u };
        constexpr sceneId_t consumerScene{ 2u };
        constexpr dataConsumerId_t dataConsumer{ 3u };

        EXPECT_TRUE(m_sceneControlAPI.linkOffscreenBuffer(bufferId, consumerScene, dataConsumer));
        EXPECT_CALL(m_cmdVisitor, handleBufferToSceneDataLinkRequest(
            OffscreenBufferHandle{ bufferId.getValue() },
            SceneId{ consumerScene.getValue() },
            DataSlotId{ dataConsumer.getValue() }));
        m_cmdVisitor.visit(m_pendingCommands);
    }

    TEST_F(ARendererSceneControl, createsCommandForStreamBufferLink)
    {
        constexpr streamBufferId_t bufferId{ 1u };
        constexpr sceneId_t consumerScene{ 2u };
        constexpr dataConsumerId_t dataConsumer{ 3u };

        EXPECT_TRUE(m_sceneControlAPI.linkStreamBuffer(bufferId, consumerScene, dataConsumer));
        EXPECT_CALL(m_cmdVisitor, handleBufferToSceneDataLinkRequest(
            StreamBufferHandle{ bufferId.getValue() },
            SceneId{ consumerScene.getValue() },
            DataSlotId{ dataConsumer.getValue() }));
        m_cmdVisitor.visit(m_pendingCommands);
    }

    TEST_F(ARendererSceneControl, createsCommandForExternalBufferLink)
    {
        constexpr externalBufferId_t bufferId{ 1u };
        constexpr sceneId_t consumerScene{ 2u };
        constexpr dataConsumerId_t dataConsumer{ 3u };

        EXPECT_TRUE(m_sceneControlAPI.linkExternalBuffer(bufferId, consumerScene, dataConsumer));
        EXPECT_CALL(m_cmdVisitor, handleBufferToSceneDataLinkRequest(
            ExternalBufferHandle{ bufferId.getValue() },
            SceneId{ consumerScene.getValue() },
            DataSlotId{ dataConsumer.getValue() }));
        m_cmdVisitor.visit(m_pendingCommands);
    }

    TEST_F(ARendererSceneControl, failsToMakeSceneReadyOrRenderedIfNoMappingSet)
    {
        const sceneId_t sceneWithNoMapping{ 1234 };
        EXPECT_FALSE(m_sceneControlAPI.setSceneState(sceneWithNoMapping, RendererSceneState::Ready));
        EXPECT_FALSE(m_sceneControlAPI.setSceneState(sceneWithNoMapping, RendererSceneState::Rendered));
    }

    TEST_F(ARendererSceneControl, failsToChangeMappingPropertiesIfSceneAlreadyReadyOrRendered)
    {
        constexpr sceneId_t scene{ 1u };
        constexpr SceneId sceneInternal{ scene.getValue() };
        constexpr displayId_t display{ 2u };
        constexpr displayId_t otherDisplay{ 3u };

        InSequence seq;
        EXPECT_TRUE(m_sceneControlAPI.setSceneMapping(scene, display));
        EXPECT_TRUE(m_sceneControlAPI.setSceneState(scene, RendererSceneState::Ready));
        EXPECT_TRUE(m_sceneControlAPI.flush());

        EXPECT_CALL(m_eventHandler, sceneStateChanged(scene, RendererSceneState::Available));
        simulateSceneEventFromRenderer(ERendererEventType::SceneStateChanged, sceneInternal, RendererSceneState::Available);
        EXPECT_CALL(m_eventHandler, sceneStateChanged(scene, RendererSceneState::Ready));
        simulateSceneEventFromRenderer(ERendererEventType::SceneStateChanged, sceneInternal, RendererSceneState::Ready);
        dispatchSceneControlEvents();

        // scene is now in ready state and will fail to change mapping
        EXPECT_FALSE(m_sceneControlAPI.setSceneMapping(scene, otherDisplay));

        EXPECT_TRUE(m_sceneControlAPI.setSceneState(scene, RendererSceneState::Rendered));
        EXPECT_TRUE(m_sceneControlAPI.flush());
        EXPECT_CALL(m_eventHandler, sceneStateChanged(scene, RendererSceneState::Rendered));
        simulateSceneEventFromRenderer(ERendererEventType::SceneStateChanged, sceneInternal, RendererSceneState::Rendered);
        dispatchSceneControlEvents();

        // scene is now in rendered state and will fail to change mapping
        EXPECT_FALSE(m_sceneControlAPI.setSceneMapping(scene, otherDisplay));

        // to clear commands
        EXPECT_TRUE(m_sceneControlAPI.flush());
    }

    TEST_F(ARendererSceneControl, failsToChangeMappingPropertiesIfSceneAlreadySetToReadyOrRendered)
    {
        constexpr sceneId_t scene{ 1u };
        constexpr displayId_t display{ 2u };
        constexpr displayId_t otherDisplay{ 3u };

        // set initial mapping
        EXPECT_TRUE(m_sceneControlAPI.setSceneMapping(scene, display));

        EXPECT_TRUE(m_sceneControlAPI.setSceneState(scene, RendererSceneState::Rendered));
        EXPECT_FALSE(m_sceneControlAPI.setSceneMapping(scene, otherDisplay));

        EXPECT_TRUE(m_sceneControlAPI.setSceneState(scene, RendererSceneState::Ready));
        EXPECT_FALSE(m_sceneControlAPI.setSceneMapping(scene, otherDisplay));
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

            void displayCreated(displayId_t /*displayId*/, ERendererEventResult /*result*/) override
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

        EXPECT_TRUE(m_renderer.startThread());

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
        EXPECT_TRUE(m_sceneControlAPI.flush());

        m_sceneControlAPI.setSceneDisplayBufferAssignment(scene, displayBufferId_t{ 12 }, 11);
        m_sceneControlAPI.linkOffscreenBuffer(displayBufferId_t{ 12 }, scene, dataConsumerId_t(0u));
        EXPECT_TRUE(m_sceneControlAPI.flush());

        RendererSceneControlEventHandlerEmpty handler;
        EXPECT_TRUE(m_sceneControlAPI.dispatchEvents(handler));

        EXPECT_TRUE(m_renderer.stopThread());
    }

    TEST_F(ARendererSceneControl, dispatchesEventsForSceneStateChanges)
    {
        constexpr sceneId_t scene{ 1u };
        constexpr SceneId sceneInternal{ scene.getValue() };
        constexpr displayId_t display{ 2u };

        InSequence seq;
        EXPECT_TRUE(m_sceneControlAPI.setSceneMapping(scene, display));
        EXPECT_TRUE(m_sceneControlAPI.setSceneState(scene, RendererSceneState::Rendered));
        EXPECT_TRUE(m_sceneControlAPI.flush());

        // from published to rendered
        EXPECT_CALL(m_eventHandler, sceneStateChanged(scene, RendererSceneState::Available));
        simulateSceneEventFromRenderer(ERendererEventType::SceneStateChanged, sceneInternal, RendererSceneState::Available);
        EXPECT_CALL(m_eventHandler, sceneStateChanged(scene, RendererSceneState::Ready));
        simulateSceneEventFromRenderer(ERendererEventType::SceneStateChanged, sceneInternal, RendererSceneState::Ready);
        EXPECT_CALL(m_eventHandler, sceneStateChanged(scene, RendererSceneState::Rendered));
        simulateSceneEventFromRenderer(ERendererEventType::SceneStateChanged, sceneInternal, RendererSceneState::Rendered);

        dispatchSceneControlEvents();

        // back to unavailable
        EXPECT_TRUE(m_sceneControlAPI.setSceneState(scene, RendererSceneState::Available));
        EXPECT_TRUE(m_sceneControlAPI.flush());
        EXPECT_CALL(m_eventHandler, sceneStateChanged(scene, RendererSceneState::Ready));
        simulateSceneEventFromRenderer(ERendererEventType::SceneStateChanged, sceneInternal, RendererSceneState::Ready);
        EXPECT_CALL(m_eventHandler, sceneStateChanged(scene, RendererSceneState::Available));
        simulateSceneEventFromRenderer(ERendererEventType::SceneStateChanged, sceneInternal, RendererSceneState::Available);

        dispatchSceneControlEvents();
    }

    TEST_F(ARendererSceneControl, dispatchesOffscreenBufferLinkedEventFromRenderer)
    {
        constexpr sceneId_t consumerScene{ 3 };
        constexpr SceneId consumerSceneInternal{ consumerScene.getValue() };
        constexpr dataConsumerId_t consumerId{ 4 };
        constexpr DataSlotId consumerIdInternal{ consumerId.getValue() };
        constexpr displayBufferId_t offscreenBufferId{ 5 };
        constexpr OffscreenBufferHandle obInternal{ offscreenBufferId.getValue() };

        InSequence seq;
        EXPECT_CALL(m_eventHandler, offscreenBufferLinked(offscreenBufferId, consumerScene, consumerId, true));
        m_eventsFromRenderer.addBufferEvent(ERendererEventType::SceneDataBufferLinked, obInternal, consumerSceneInternal, consumerIdInternal);

        EXPECT_CALL(m_eventHandler, offscreenBufferLinked(offscreenBufferId, consumerScene, consumerId, false));
        m_eventsFromRenderer.addBufferEvent(ERendererEventType::SceneDataBufferLinkFailed, obInternal, consumerSceneInternal, consumerIdInternal);

        submitEventsFromRenderer();
        dispatchSceneControlEvents();
    }

    TEST_F(ARendererSceneControl, dispatchesExternalBufferLinkedEventFromRenderer)
    {
        constexpr sceneId_t consumerScene{ 3 };
        constexpr SceneId consumerSceneInternal{ consumerScene.getValue() };
        constexpr dataConsumerId_t consumerId{ 4 };
        constexpr DataSlotId consumerIdInternal{ consumerId.getValue() };
        constexpr externalBufferId_t bufferId{ 5 };
        constexpr ExternalBufferHandle ebInternal{ bufferId.getValue() };

        InSequence seq;
        EXPECT_CALL(m_eventHandler, externalBufferLinked(bufferId, consumerScene, consumerId, true));
        m_eventsFromRenderer.addBufferEvent(ERendererEventType::SceneDataBufferLinked, ebInternal, consumerSceneInternal, consumerIdInternal);

        EXPECT_CALL(m_eventHandler, externalBufferLinked(bufferId, consumerScene, consumerId, false));
        m_eventsFromRenderer.addBufferEvent(ERendererEventType::SceneDataBufferLinkFailed, ebInternal, consumerSceneInternal, consumerIdInternal);

        submitEventsFromRenderer();
        dispatchSceneControlEvents();
    }

    TEST_F(ARendererSceneControl, dispatchesStreamBufferLinkedEventFromRenderer)
    {
        constexpr sceneId_t consumerScene{ 3 };
        constexpr SceneId consumerSceneInternal{ consumerScene.getValue() };
        constexpr dataConsumerId_t consumerId{ 4 };
        constexpr DataSlotId consumerIdInternal{ consumerId.getValue() };
        constexpr streamBufferId_t bufferId{ 5 };
        constexpr StreamBufferHandle sbInternal{ bufferId.getValue() };

        InSequence seq;
        EXPECT_CALL(m_eventHandler, streamBufferLinked(bufferId, consumerScene, consumerId, true));
        m_eventsFromRenderer.addBufferEvent(ERendererEventType::SceneDataBufferLinked, sbInternal, consumerSceneInternal, consumerIdInternal);

        EXPECT_CALL(m_eventHandler, streamBufferLinked(bufferId, consumerScene, consumerId, false));
        m_eventsFromRenderer.addBufferEvent(ERendererEventType::SceneDataBufferLinkFailed, sbInternal, consumerSceneInternal, consumerIdInternal);

        submitEventsFromRenderer();
        dispatchSceneControlEvents();
    }

    TEST_F(ARendererSceneControl, dispatchesDataLinkedEventFromRenderer)
    {
        constexpr sceneId_t providerScene{ 1 };
        constexpr SceneId providerSceneInternal{ providerScene.getValue() };
        constexpr dataProviderId_t providerId{ 2 };
        constexpr DataSlotId providerIdInternal{ providerId.getValue() };
        constexpr sceneId_t consumerScene{ 3 };
        constexpr SceneId consumerSceneInternal{ consumerScene.getValue() };
        constexpr dataConsumerId_t consumerId{ 4 };
        constexpr DataSlotId consumerIdInternal{ consumerId.getValue() };

        InSequence seq;
        EXPECT_CALL(m_eventHandler, dataLinked(providerScene, providerId, consumerScene, consumerId, true));
        m_eventsFromRenderer.addDataLinkEvent(ERendererEventType::SceneDataLinked, providerSceneInternal, consumerSceneInternal, providerIdInternal, consumerIdInternal);

        EXPECT_CALL(m_eventHandler, dataLinked(providerScene, providerId, consumerScene, consumerId, false));
        m_eventsFromRenderer.addDataLinkEvent(ERendererEventType::SceneDataLinkFailed, providerSceneInternal, consumerSceneInternal, providerIdInternal, consumerIdInternal);

        submitEventsFromRenderer();
        dispatchSceneControlEvents();
    }

    TEST_F(ARendererSceneControl, dispatchesDataUnlinkedEventFromRenderer)
    {
        constexpr sceneId_t consumerScene{ 3 };
        constexpr SceneId consumerSceneInternal{ consumerScene.getValue() };
        constexpr dataConsumerId_t consumerId{ 4 };
        constexpr DataSlotId consumerIdInternal{ consumerId.getValue() };

        InSequence seq;
        EXPECT_CALL(m_eventHandler, dataUnlinked(consumerScene, consumerId, true));
        m_eventsFromRenderer.addDataLinkEvent(ERendererEventType::SceneDataUnlinked, {}, consumerSceneInternal, {}, consumerIdInternal);

        EXPECT_CALL(m_eventHandler, dataUnlinked(consumerScene, consumerId, false));
        m_eventsFromRenderer.addDataLinkEvent(ERendererEventType::SceneDataUnlinkFailed, {}, consumerSceneInternal, {}, consumerIdInternal);

        // this event is ignored and will be removed
        m_eventsFromRenderer.addDataLinkEvent(ERendererEventType::SceneDataUnlinkedAsResultOfClientSceneChange, {}, consumerSceneInternal, {}, consumerIdInternal);

        submitEventsFromRenderer();
        dispatchSceneControlEvents();
    }

    TEST_F(ARendererSceneControl, dispatchesHandlePickEventFromRenderer)
    {
        constexpr sceneId_t providerScene{ 3 };
        constexpr SceneId providerSceneInternal{ providerScene.getValue() };
        constexpr  pickableObjectId_t pickable1{ 567u };
        constexpr  pickableObjectId_t pickable2{ 578u };
        constexpr PickableObjectId pickable1Internal{pickable1.getValue()};
        constexpr PickableObjectId pickable2Internal{pickable2.getValue()};
        constexpr std::array<pickableObjectId_t, 2> pickableObjects{ pickable1, pickable2 };

        EXPECT_CALL(m_eventHandler, objectsPicked(providerScene, _, pickableObjects.size())).WillOnce(Invoke([&](auto /*unused*/, auto pickedObjects, auto pickedObjectsCount)
        {
            ASSERT_EQ(pickableObjects.size(), pickedObjectsCount);
            for (size_t i = 0u; i < pickedObjectsCount; ++i)
                EXPECT_EQ(pickableObjects[i], pickedObjects[i]);
        }));
        m_eventsFromRenderer.addPickedEvent(ERendererEventType::ObjectsPicked, providerSceneInternal, {pickable1Internal, pickable2Internal});

        submitEventsFromRenderer();
        dispatchSceneControlEvents();
    }

    TEST_F(ARendererSceneControl, dispatchesNamedFlushEventFromRenderer)
    {
        constexpr sceneId_t scene{ 3 };
        constexpr sceneVersionTag_t tag{ 4 };

        EXPECT_CALL(m_eventHandler, sceneFlushed(scene, tag));
        m_eventsFromRenderer.addSceneFlushEvent(ERendererEventType::SceneFlushed, SceneId{ scene.getValue() }, SceneVersionTag{ tag });

        submitEventsFromRenderer();
        dispatchSceneControlEvents();
    }

    TEST_F(ARendererSceneControl, dispatchesSceneExpirationEventFromRenderer)
    {
        constexpr sceneId_t scene{ 3 };

        InSequence seq;
        EXPECT_CALL(m_eventHandler, sceneExpirationMonitoringEnabled(scene));
        m_eventsFromRenderer.addSceneExpirationEvent(ERendererEventType::SceneExpirationMonitoringEnabled, SceneId{ scene.getValue() });

        EXPECT_CALL(m_eventHandler, sceneExpired(scene));
        m_eventsFromRenderer.addSceneExpirationEvent(ERendererEventType::SceneExpired, SceneId{ scene.getValue() });

        EXPECT_CALL(m_eventHandler, sceneRecoveredFromExpiration(scene));
        m_eventsFromRenderer.addSceneExpirationEvent(ERendererEventType::SceneRecoveredFromExpiration, SceneId{ scene.getValue() });

        EXPECT_CALL(m_eventHandler, sceneExpirationMonitoringDisabled(scene));
        m_eventsFromRenderer.addSceneExpirationEvent(ERendererEventType::SceneExpirationMonitoringDisabled, SceneId{ scene.getValue() });

        submitEventsFromRenderer();
        dispatchSceneControlEvents();
    }

    TEST_F(ARendererSceneControl, dispatchesStreamAvailabilityEventFromRenderer)
    {
        constexpr waylandIviSurfaceId_t stream{ 3 };

        InSequence seq;
        EXPECT_CALL(m_eventHandler, streamAvailabilityChanged(stream, true));
        m_eventsFromRenderer.addStreamSourceEvent(ERendererEventType::StreamSurfaceAvailable, WaylandIviSurfaceId{ stream.getValue() });

        EXPECT_CALL(m_eventHandler, streamAvailabilityChanged(stream, false));
        m_eventsFromRenderer.addStreamSourceEvent(ERendererEventType::StreamSurfaceUnavailable, WaylandIviSurfaceId{ stream.getValue() });

        submitEventsFromRenderer();
        dispatchSceneControlEvents();
    }

    TEST_F(ARendererSceneControl, dispatchesDataSlotEventsFromRenderer)
    {
        InSequence seq;
        EXPECT_CALL(m_eventHandler, dataProviderCreated(sceneId_t{ 123u }, dataProviderId_t{ 1u }));
        m_eventsFromRenderer.addDataLinkEvent(ERendererEventType::SceneDataSlotProviderCreated, SceneId{ 123u }, {}, DataSlotId{ 1u }, {});

        EXPECT_CALL(m_eventHandler, dataProviderDestroyed(sceneId_t{ 124u }, dataProviderId_t{ 2u }));
        m_eventsFromRenderer.addDataLinkEvent(ERendererEventType::SceneDataSlotProviderDestroyed, SceneId{ 124u }, {}, DataSlotId{ 2u }, {});

        EXPECT_CALL(m_eventHandler, dataConsumerCreated(sceneId_t{ 125u }, dataConsumerId_t{ 3u }));
        m_eventsFromRenderer.addDataLinkEvent(ERendererEventType::SceneDataSlotConsumerCreated, {}, SceneId{ 125u }, {}, DataSlotId{ 3u });

        EXPECT_CALL(m_eventHandler, dataConsumerDestroyed(sceneId_t{ 126u }, dataConsumerId_t{ 4u }));
        m_eventsFromRenderer.addDataLinkEvent(ERendererEventType::SceneDataSlotConsumerDestroyed, {}, SceneId{ 126u }, {}, DataSlotId{ 4u });

        submitEventsFromRenderer();
        dispatchSceneControlEvents();
    }

    TEST_F(ARendererSceneControl, canInvokeAPICommandsInEventHandler)
    {
        class RecursiveEventTestHandler final : public RendererSceneControlEventHandlerEmpty
        {
        public:
            RecursiveEventTestHandler(RendererSceneControl& sceneControl, RamsesRenderer& renderer)
                : m_sceneControl(sceneControl)
                , m_renderer(renderer)
            {
                m_renderer.setLoopMode(ELoopMode::UpdateOnly);
            }

            void sceneStateChanged(sceneId_t sceneId, RendererSceneState state) override
            {
                switch (state)
                {
                case RendererSceneState::Unavailable:
                    break;
                case RendererSceneState::Available:
                    EXPECT_TRUE(m_sceneControl.setSceneMapping(sceneId, displayId_t{ 1 }));
                    EXPECT_TRUE(m_sceneControl.setSceneState(sceneId, RendererSceneState::Ready));
                    EXPECT_TRUE(m_sceneControl.flush());
                    simulateSceneEventFromRenderer(ERendererEventType::SceneStateChanged, SceneId{ sceneId.getValue() }, RendererSceneState::Ready);
                    break;
                case RendererSceneState::Ready:
                    EXPECT_TRUE(m_sceneControl.setSceneState(sceneId, RendererSceneState::Rendered));
                    EXPECT_TRUE(m_sceneControl.flush());
                    simulateSceneEventFromRenderer(ERendererEventType::SceneStateChanged, SceneId{ sceneId.getValue() }, RendererSceneState::Rendered);
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

            void simulateSceneEventFromRenderer(ERendererEventType eventType, SceneId sceneId, RendererSceneState state)
            {
                RendererEvent event{ eventType };
                event.sceneId = sceneId;
                event.state = state;
                m_renderer.impl().getDisplayDispatcher().injectSceneControlEvent(std::move(event));
            }

            RendererSceneControl& m_sceneControl;
            RamsesRenderer& m_renderer;
            bool m_reachedRenderedState = false;
        };

        // actual commands ending up at renderer will fail but that is not the purpose of this test
        // we simulate responses from renderer

        RecursiveEventTestHandler handler(m_sceneControlAPI, m_renderer);

        simulateSceneEventFromRenderer(ERendererEventType::SceneStateChanged, SceneId{ 123 }, RendererSceneState::Available);
        EXPECT_TRUE(handler.waitForRenderedStateReached());
    }
}
