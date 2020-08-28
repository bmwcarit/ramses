//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gmock/gmock.h"
#include "RendererLib/SceneReferenceLogic.h"
#include "RendererLib/RendererScenes.h"
#include "RendererLib/RendererSceneUpdater.h"
#include "RendererLib/SceneExpirationMonitor.h"
#include "RendererLib/SceneStateExecutor.h"
#include "RendererLib/FrameTimer.h"
#include "RendererLib/ResourceUploader.h"
#include "Scene/SceneActionCollectionCreator.h"
#include "RendererSceneControlMock.h"
#include "RendererSceneControlLogicMock.h"
#include "RendererEventCollector.h"
#include "SceneAllocateHelper.h"
#include "PlatformFactoryMock.h"
#include "RendererMock.h"
#include "RendererSceneEventSenderMock.h"
#include "ResourceProviderMock.h"
#include "Components/SceneUpdate.h"

using namespace testing;

namespace ramses_internal
{
    // This is a component test consisting of full implementations (no mock) of RendererSceneUpdter, RendererSceneControlLogic and SceneReferenceLogic.
    // It is supposed to test only expectations affected by the interaction of those components, it is not supposed to replace each component's unit tests.
    // Main purpose of tests here is to make sure that scene reference state changes are requested/executed within same renderer update cycle with its trigger (flush or master scene state change)
    class ASceneReferenceLogicWithSceneUpdater : public Test
    {
    public:
        ASceneReferenceLogicWithSceneUpdater()
            : m_scenes(m_eventCollector)
            , m_expirationMonitor(m_scenes, m_eventCollector)
            , m_renderer(m_platformFactory, m_scenes, m_eventCollector, m_expirationMonitor, m_rendererStatistics)
            , m_sceneStateExecutor(m_renderer, m_sceneEventSenderFromSceneUpdater, m_eventCollector)
            , m_sceneUpdater(m_renderer, m_scenes, m_sceneStateExecutor, m_eventCollector, m_frameTimer, m_expirationMonitor)
            , m_sceneLogic(m_sceneUpdater)
            , m_sceneRefLogic(m_scenes, m_sceneLogic, m_sceneUpdater, m_sceneEventSenderFromSceneRefLogic)
        {
            m_sceneUpdater.setSceneReferenceLogicHandler(m_sceneRefLogic);
        }

        virtual void SetUp() override
        {
            // get master scene to available state
            m_sceneLogic.setSceneState(MasterSceneId, RendererSceneState::Available);
            publishScene(MasterSceneId);
            update();
            receiveScene(MasterSceneId);
            flushScene(MasterSceneId);

            ASSERT_EQ(ESceneState::Subscribed, m_sceneStateExecutor.getSceneState(MasterSceneId));
            update();
        }

        void publishScene(SceneId sceneId)
        {
            m_sceneUpdater.handleScenePublished(sceneId, EScenePublicationMode_LocalAndRemote);
        }

        void receiveScene(SceneId sceneId)
        {
            m_sceneUpdater.handleSceneReceived(SceneInfo{ sceneId });
            ASSERT_TRUE(m_scenes.hasScene(sceneId));
        }

        void flushScene(SceneId sceneId)
        {
            SceneUpdate sceneUpdate;
            SceneActionCollectionCreator creator(sceneUpdate.actions);
            creator.flush(1u, false);
            m_sceneUpdater.handleSceneActions(sceneId, std::move(sceneUpdate));
        }

        void flushSceneWithRefSceneStateRequest(SceneId sceneId, SceneReferenceHandle refSceneHandle, RendererSceneState refSceneState)
        {
            SceneUpdate sceneUpdate;
            SceneActionCollectionCreator creator(sceneUpdate.actions);
            creator.requestSceneReferenceState(refSceneHandle, refSceneState);
            creator.flush(1u, false);
            m_sceneUpdater.handleSceneActions(sceneId, std::move(sceneUpdate));
        }

        void processInternalEventsBySceneLogic()
        {
            InternalSceneStateEvents internalSceneEvents;
            m_eventCollector.dispatchInternalSceneStateEvents(internalSceneEvents);
            for (const auto& evt : internalSceneEvents)
                m_sceneLogic.processInternalEvent(evt);

            RendererSceneControlLogic::Events outSceneEvents;
            m_sceneLogic.consumeEvents(outSceneEvents);
            for (const auto& evt : outSceneEvents)
                if (evt.type == RendererSceneControlLogic::Event::Type::SceneStateChanged)
                    m_eventCollector.addSceneEvent(ERendererEventType_SceneStateChanged, evt.sceneId, evt.state);
        }

        void update()
        {
            m_renderer.getProfilerStatistics().markFrameFinished(std::chrono::microseconds{ 0u });
            processInternalEventsBySceneLogic();

            m_sceneUpdater.updateScenes();

            // collect and process events
            RendererEventVector rendEvents;
            RendererEventVector sceneEvents;
            m_eventCollector.appendAndConsumePendingEvents(rendEvents, sceneEvents);
            m_sceneRefLogic.extractAndSendSceneReferenceEvents(sceneEvents);
        }

    protected:
        RendererEventCollector m_eventCollector;
        RendererScenes m_scenes;
        NiceMock<PlatformFactoryNiceMock> m_platformFactory;
        SceneExpirationMonitor m_expirationMonitor;
        RendererStatistics m_rendererStatistics;
        NiceMock<RendererMockWithNiceMockDisplay> m_renderer;

        NiceMock<RendererSceneEventSenderMock> m_sceneEventSenderFromSceneUpdater;
        StrictMock<RendererSceneEventSenderMock> m_sceneEventSenderFromSceneRefLogic;
        SceneStateExecutor m_sceneStateExecutor;
        FrameTimer m_frameTimer;
        RendererSceneUpdater m_sceneUpdater;

        RendererSceneControlLogic m_sceneLogic;
        SceneReferenceLogic m_sceneRefLogic;

        static constexpr SceneId MasterSceneId{ 123 };
        static constexpr SceneId RefSceneId{ 125 };
        static constexpr SceneReferenceHandle RefSceneHandle{ 13 };
    };

    constexpr SceneId ASceneReferenceLogicWithSceneUpdater::MasterSceneId;
    constexpr SceneId ASceneReferenceLogicWithSceneUpdater::RefSceneId;
    constexpr SceneReferenceHandle ASceneReferenceLogicWithSceneUpdater::RefSceneHandle;

    TEST_F(ASceneReferenceLogicWithSceneUpdater, requestsReferencedSceneStateChangeAlwaysInSameUpdateLoopWithFlushApplied)
    {
        // create display so mapping can work
        NiceMock<ResourceProviderMock> resourceProvider;
        ResourceUploader resourceUploader{ m_renderer.getStatistics() };
        constexpr DisplayHandle display{ 0 };
        m_sceneUpdater.createDisplayContext({}, resourceProvider, resourceUploader, display);
        // set mapping and request master scene rendered so that ref scene is not limited by master scene state
        m_sceneLogic.setSceneMapping(MasterSceneId, display);
        m_sceneLogic.setSceneState(MasterSceneId, RendererSceneState::Rendered);

        m_sceneUpdater.handleScenePublished(RefSceneId, EScenePublicationMode_LocalAndRemote);

        auto& masterScene = m_scenes.getScene(MasterSceneId);
        const auto refSceneHandle = SceneAllocateHelper(masterScene).allocateSceneReference(RefSceneId);

        InSequence seq;

        // request available
        flushSceneWithRefSceneStateRequest(MasterSceneId, refSceneHandle, RendererSceneState::Available);
        update();
        EXPECT_EQ(ESceneState::SubscriptionRequested, m_sceneStateExecutor.getSceneState(RefSceneId));

        receiveScene(RefSceneId);
        flushScene(RefSceneId);
        EXPECT_EQ(ESceneState::Subscribed, m_sceneStateExecutor.getSceneState(RefSceneId));
        EXPECT_CALL(m_sceneEventSenderFromSceneRefLogic, sendSceneStateChanged(MasterSceneId, RefSceneId, RendererSceneState::Available));

        // request ready
        flushSceneWithRefSceneStateRequest(MasterSceneId, refSceneHandle, RendererSceneState::Ready);
        update();
        EXPECT_EQ(ESceneState::MappingAndUploading, m_sceneStateExecutor.getSceneState(RefSceneId));
        update();
        EXPECT_EQ(ESceneState::Mapped, m_sceneStateExecutor.getSceneState(RefSceneId));
        EXPECT_CALL(m_sceneEventSenderFromSceneRefLogic, sendSceneStateChanged(MasterSceneId, RefSceneId, RendererSceneState::Ready));

        // request rendered
        flushSceneWithRefSceneStateRequest(MasterSceneId, refSceneHandle, RendererSceneState::Rendered);
        update();
        EXPECT_EQ(ESceneState::Rendered, m_sceneStateExecutor.getSceneState(RefSceneId));

        // due to order of processing scene ref events are sent with one frame delay
        EXPECT_CALL(m_sceneEventSenderFromSceneRefLogic, sendSceneStateChanged(MasterSceneId, RefSceneId, RendererSceneState::Rendered));
        update();
    }

    TEST_F(ASceneReferenceLogicWithSceneUpdater, willShowMasterSceneWithItsRefSceneInSameUpdateLoop)
    {
        NiceMock<ResourceProviderMock> resourceProvider;
        ResourceUploader resourceUploader{ m_renderer.getStatistics() };

        constexpr DisplayHandle display{ 0 };
        m_sceneUpdater.createDisplayContext({}, resourceProvider, resourceUploader, display);

        // get master scene ready
        {
            m_sceneLogic.setSceneMapping(MasterSceneId, display);
            m_sceneLogic.setSceneState(MasterSceneId, RendererSceneState::Ready);
            update();
            update(); // 2 updates needed due to intermediate states
            EXPECT_EQ(ESceneState::Mapped, m_sceneStateExecutor.getSceneState(MasterSceneId));
        }

        auto& masterScene = m_scenes.getScene(MasterSceneId);
        const auto refSceneHandle = SceneAllocateHelper(masterScene).allocateSceneReference(RefSceneId);

        // request ref scene rendered which will bring it to ready (cannot have higher state than master scene)
        {
            InSequence seq;
            EXPECT_CALL(m_sceneEventSenderFromSceneRefLogic, sendSceneStateChanged(MasterSceneId, RefSceneId, RendererSceneState::Available));
            EXPECT_CALL(m_sceneEventSenderFromSceneRefLogic, sendSceneStateChanged(MasterSceneId, RefSceneId, RendererSceneState::Ready));

            masterScene.requestSceneReferenceState(refSceneHandle, RendererSceneState::Rendered);
            m_sceneUpdater.handleScenePublished(RefSceneId, EScenePublicationMode_LocalAndRemote);
            update();
            receiveScene(RefSceneId);
            flushScene(RefSceneId);
            EXPECT_EQ(ESceneState::Subscribed, m_sceneStateExecutor.getSceneState(RefSceneId));
            update();
            update();
            update(); // in case of ref scene 3 updates needed due to intermediate states and additional cycle delay due to scene ref logic processing internal events at end of each cycle
            EXPECT_EQ(ESceneState::Mapped, m_sceneStateExecutor.getSceneState(RefSceneId));
            // will stay in ready due to master scene not rendered yet
            update();
            EXPECT_EQ(ESceneState::Mapped, m_sceneStateExecutor.getSceneState(RefSceneId));
        }

        m_sceneLogic.setSceneState(MasterSceneId, RendererSceneState::Rendered);
        update();
        EXPECT_EQ(ESceneState::Rendered, m_sceneStateExecutor.getSceneState(MasterSceneId));
        EXPECT_EQ(ESceneState::Rendered, m_sceneStateExecutor.getSceneState(RefSceneId));

        // due to order of processing scene ref events are sent with one frame delay
        EXPECT_CALL(m_sceneEventSenderFromSceneRefLogic, sendSceneStateChanged(MasterSceneId, RefSceneId, RendererSceneState::Rendered));
        update();
    }

    TEST_F(ASceneReferenceLogicWithSceneUpdater, willHideMasterSceneWithItsRefSceneInSameUpdateLoop)
    {
        // create display so mapping can work
        NiceMock<ResourceProviderMock> resourceProvider;
        ResourceUploader resourceUploader{ m_renderer.getStatistics() };
        constexpr DisplayHandle display{ 0 };
        m_sceneUpdater.createDisplayContext({}, resourceProvider, resourceUploader, display);

        auto& masterScene = m_scenes.getScene(MasterSceneId);
        const auto refSceneHandle = SceneAllocateHelper(masterScene).allocateSceneReference(RefSceneId);

        // get both scenes to rendered
        {
            InSequence seq;
            EXPECT_CALL(m_sceneEventSenderFromSceneRefLogic, sendSceneStateChanged(MasterSceneId, RefSceneId, RendererSceneState::Available));
            EXPECT_CALL(m_sceneEventSenderFromSceneRefLogic, sendSceneStateChanged(MasterSceneId, RefSceneId, RendererSceneState::Ready));
            EXPECT_CALL(m_sceneEventSenderFromSceneRefLogic, sendSceneStateChanged(MasterSceneId, RefSceneId, RendererSceneState::Rendered));

            m_sceneLogic.setSceneMapping(MasterSceneId, display);
            m_sceneLogic.setSceneState(MasterSceneId, RendererSceneState::Rendered);

            masterScene.requestSceneReferenceState(refSceneHandle, RendererSceneState::Rendered);
            m_sceneUpdater.handleScenePublished(RefSceneId, EScenePublicationMode_LocalAndRemote);
            update();
            receiveScene(RefSceneId);
            flushScene(RefSceneId);
            EXPECT_EQ(ESceneState::Subscribed, m_sceneStateExecutor.getSceneState(RefSceneId));

            update();
            update();
            update();
            update();
            EXPECT_EQ(ESceneState::Rendered, m_sceneStateExecutor.getSceneState(MasterSceneId));
            EXPECT_EQ(ESceneState::Rendered, m_sceneStateExecutor.getSceneState(RefSceneId));
        }

        m_sceneLogic.setSceneState(MasterSceneId, RendererSceneState::Ready);
        update();
        EXPECT_EQ(ESceneState::Mapped, m_sceneStateExecutor.getSceneState(MasterSceneId));
        EXPECT_EQ(ESceneState::Mapped, m_sceneStateExecutor.getSceneState(RefSceneId));

        // due to order of processing scene ref events are sent with one frame delay
        EXPECT_CALL(m_sceneEventSenderFromSceneRefLogic, sendSceneStateChanged(MasterSceneId, RefSceneId, RendererSceneState::Ready));
        update();
    }

    TEST_F(ASceneReferenceLogicWithSceneUpdater, unpublishOfMasterSceneTriggersUnsubscribeOfItsRefSceneInSameUpdateLoop)
    {
        // create display so mapping can work
        NiceMock<ResourceProviderMock> resourceProvider;
        ResourceUploader resourceUploader{ m_renderer.getStatistics() };
        constexpr DisplayHandle display{ 0 };
        m_sceneUpdater.createDisplayContext({}, resourceProvider, resourceUploader, display);

        auto& masterScene = m_scenes.getScene(MasterSceneId);
        const auto refSceneHandle = SceneAllocateHelper(masterScene).allocateSceneReference(RefSceneId);

        // get both scenes to rendered
        {
            InSequence seq;
            EXPECT_CALL(m_sceneEventSenderFromSceneRefLogic, sendSceneStateChanged(MasterSceneId, RefSceneId, RendererSceneState::Available));
            EXPECT_CALL(m_sceneEventSenderFromSceneRefLogic, sendSceneStateChanged(MasterSceneId, RefSceneId, RendererSceneState::Ready));
            EXPECT_CALL(m_sceneEventSenderFromSceneRefLogic, sendSceneStateChanged(MasterSceneId, RefSceneId, RendererSceneState::Rendered));

            m_sceneLogic.setSceneMapping(MasterSceneId, display);
            m_sceneLogic.setSceneState(MasterSceneId, RendererSceneState::Rendered);

            masterScene.requestSceneReferenceState(refSceneHandle, RendererSceneState::Rendered);
            m_sceneUpdater.handleScenePublished(RefSceneId, EScenePublicationMode_LocalAndRemote);
            update();
            receiveScene(RefSceneId);
            flushScene(RefSceneId);
            EXPECT_EQ(ESceneState::Subscribed, m_sceneStateExecutor.getSceneState(RefSceneId));

            update();
            update();
            update();
            update();
            EXPECT_EQ(ESceneState::Rendered, m_sceneStateExecutor.getSceneState(MasterSceneId));
            EXPECT_EQ(ESceneState::Rendered, m_sceneStateExecutor.getSceneState(RefSceneId));
        }

        m_sceneUpdater.handleSceneUnpublished(MasterSceneId);
        update();
        EXPECT_FALSE(m_scenes.hasScene(MasterSceneId));
        // in same loop ref scene is requested to be unavailable...
        EXPECT_NE(ESceneState::Rendered, m_sceneStateExecutor.getSceneState(RefSceneId));
        // exact sequence of state changes wrt update loops is not important here, only order is
        InSequence seq;
        EXPECT_CALL(m_sceneEventSenderFromSceneRefLogic, sendSceneStateChanged(MasterSceneId, RefSceneId, RendererSceneState::Ready));
        EXPECT_CALL(m_sceneEventSenderFromSceneRefLogic, sendSceneStateChanged(MasterSceneId, RefSceneId, RendererSceneState::Available));
        EXPECT_CALL(m_sceneEventSenderFromSceneRefLogic, sendSceneStateChanged(MasterSceneId, RefSceneId, RendererSceneState::Unavailable));
        update();
        update();
        update();
        // ... and destroyed eventually
        EXPECT_FALSE(m_scenes.hasScene(RefSceneId));
    }
}
