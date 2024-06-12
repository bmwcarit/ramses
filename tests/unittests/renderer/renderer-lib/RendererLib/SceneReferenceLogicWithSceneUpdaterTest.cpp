//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gmock/gmock.h"
#include "internal/RendererLib/SceneReferenceLogic.h"
#include "internal/RendererLib/RendererScenes.h"
#include "internal/RendererLib/RendererSceneUpdater.h"
#include "internal/RendererLib/SceneExpirationMonitor.h"
#include "internal/RendererLib/SceneStateExecutor.h"
#include "internal/RendererLib/FrameTimer.h"
#include "internal/RendererLib/SceneReferenceOwnership.h"
#include "internal/SceneGraph/Scene/SceneActionCollectionCreator.h"
#include "RendererSceneStateControlMock.h"
#include "RendererSceneControlLogicMock.h"
#include "internal/RendererLib/RendererEventCollector.h"
#include "SceneAllocateHelper.h"
#include "PlatformMock.h"
#include "RendererMock.h"
#include "RendererSceneEventSenderMock.h"
#include "ResourceDeviceHandleAccessorMock.h"
#include "internal/Components/SceneUpdate.h"
#include "internal/Watchdog/ThreadAliveNotifierMock.h"

using namespace testing;

namespace ramses::internal
{
    // This is a component test consisting of full implementations (no mock) of RendererSceneUpdter, RendererSceneControlLogic and SceneReferenceLogic.
    // It is supposed to test only expectations affected by the interaction of those components, it is not supposed to replace each component's unit tests.
    // Main purpose of tests here is to make sure that scene reference state changes are requested/executed within same renderer update cycle with its trigger (flush or master scene state change)
    class ASceneReferenceLogicWithSceneUpdater : public Test
    {
    public:
        ASceneReferenceLogicWithSceneUpdater()
            : m_scenes(m_eventCollector)
            , m_expirationMonitor(m_scenes, m_eventCollector, m_rendererStatistics)
            , m_renderer(DisplayId, m_scenes, m_eventCollector, m_expirationMonitor, m_rendererStatistics)
            , m_sceneStateExecutor(m_renderer, m_sceneEventSenderFromSceneUpdater, m_eventCollector)
            , m_sceneUpdater(DisplayId, m_platform, m_renderer, m_scenes, m_sceneStateExecutor, m_eventCollector, m_frameTimer, m_expirationMonitor, m_notifier, EFeatureLevel_Latest)
            , m_sceneLogic(m_sceneUpdater)
            , m_sceneRefLogic(m_scenes, m_sceneLogic, m_sceneUpdater, m_sceneEventSenderFromSceneRefLogic, m_sceneRefOwnership)
        {
            m_sceneUpdater.setSceneReferenceLogicHandler(m_sceneRefLogic);
        }

        void SetUp() override
        {
            // get master scene to ready state
            m_sceneUpdater.createDisplayContext({}, nullptr);
            m_sceneLogic.setSceneState(MasterSceneId, RendererSceneState::Ready);
            publishScene(MasterSceneId);
            update();
            receiveScene(MasterSceneId);
            flushScene(MasterSceneId);
            update();
            update();

            ASSERT_EQ(ESceneState::Mapped, m_sceneStateExecutor.getSceneState(MasterSceneId));
            update();
        }

        void publishScene(SceneId sceneId)
        {
            m_sceneUpdater.handleScenePublished(sceneId, EScenePublicationMode::LocalAndRemote);
        }

        void receiveScene(SceneId sceneId)
        {
            m_sceneUpdater.handleSceneReceived(SceneInfo{ sceneId });
            ASSERT_TRUE(m_scenes.hasScene(sceneId));
        }

        void flushScene(SceneId sceneId)
        {
            SceneUpdate sceneUpdate;
            SceneActionCollectionCreator creator(sceneUpdate.actions, EFeatureLevel_Latest);
            sceneUpdate.flushInfos.flushCounter = 1u;
            sceneUpdate.flushInfos.containsValidInformation = true;
            m_sceneUpdater.handleSceneUpdate(sceneId, std::move(sceneUpdate));
        }

        void flushSceneWithRefSceneStateRequest(SceneId sceneId, SceneReferenceHandle refSceneHandle, RendererSceneState refSceneState)
        {
            SceneUpdate sceneUpdate;
            SceneActionCollectionCreator creator(sceneUpdate.actions, EFeatureLevel_Latest);
            creator.requestSceneReferenceState(refSceneHandle, refSceneState);
            sceneUpdate.flushInfos.flushCounter = 1u;
            sceneUpdate.flushInfos.containsValidInformation = true;
            m_sceneUpdater.handleSceneUpdate(sceneId, std::move(sceneUpdate));
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
                m_eventCollector.addSceneEvent(ERendererEventType::SceneStateChanged, evt.sceneId, evt.state);
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
        NiceMock<PlatformNiceMock> m_platform;
        SceneExpirationMonitor m_expirationMonitor;
        RendererStatistics m_rendererStatistics;
        NiceMock<RendererMockWithNiceMockDisplay> m_renderer;

        NiceMock<RendererSceneEventSenderMock> m_sceneEventSenderFromSceneUpdater;
        StrictMock<RendererSceneEventSenderMock> m_sceneEventSenderFromSceneRefLogic;
        SceneStateExecutor m_sceneStateExecutor;
        FrameTimer m_frameTimer;
        NiceMock<ThreadAliveNotifierMock> m_notifier;
        RendererSceneUpdater m_sceneUpdater;

        RendererSceneControlLogic m_sceneLogic;
        SceneReferenceOwnership m_sceneRefOwnership;
        SceneReferenceLogic m_sceneRefLogic;

        static constexpr SceneId MasterSceneId{ 123 };
        static constexpr SceneId RefSceneId{ 125 };
        static constexpr SceneReferenceHandle RefSceneHandle{ 13 };
        static constexpr DisplayHandle DisplayId{ 0u };
    };

    TEST_F(ASceneReferenceLogicWithSceneUpdater, requestsReferencedSceneStateChangeAlwaysInSameUpdateLoopWithFlushApplied)
    {
        m_sceneLogic.setSceneState(MasterSceneId, RendererSceneState::Rendered);

        m_sceneUpdater.handleScenePublished(RefSceneId, EScenePublicationMode::LocalAndRemote);

        auto& masterScene = m_scenes.getScene(MasterSceneId);
        const auto refSceneHandle = SceneAllocateHelper(masterScene).allocateSceneReference(RefSceneId);

        InSequence seq;

        // request available
        flushSceneWithRefSceneStateRequest(MasterSceneId, refSceneHandle, RendererSceneState::Ready);
        EXPECT_CALL(m_sceneEventSenderFromSceneRefLogic, sendSceneStateChanged(MasterSceneId, RefSceneId, RendererSceneState::Available));
        update();
        EXPECT_EQ(ESceneState::SubscriptionRequested, m_sceneStateExecutor.getSceneState(RefSceneId));

        receiveScene(RefSceneId);
        flushScene(RefSceneId);
        EXPECT_EQ(ESceneState::Subscribed, m_sceneStateExecutor.getSceneState(RefSceneId));
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
        auto& masterScene = m_scenes.getScene(MasterSceneId);
        const auto refSceneHandle = SceneAllocateHelper(masterScene).allocateSceneReference(RefSceneId);

        // request ref scene rendered which will bring it to ready (cannot have higher state than master scene)
        {
            InSequence seq;
            EXPECT_CALL(m_sceneEventSenderFromSceneRefLogic, sendSceneStateChanged(MasterSceneId, RefSceneId, RendererSceneState::Available));
            EXPECT_CALL(m_sceneEventSenderFromSceneRefLogic, sendSceneStateChanged(MasterSceneId, RefSceneId, RendererSceneState::Ready));

            masterScene.requestSceneReferenceState(refSceneHandle, RendererSceneState::Rendered);
            m_sceneUpdater.handleScenePublished(RefSceneId, EScenePublicationMode::LocalAndRemote);
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
        auto& masterScene = m_scenes.getScene(MasterSceneId);
        const auto refSceneHandle = SceneAllocateHelper(masterScene).allocateSceneReference(RefSceneId);

        // get both scenes to rendered
        {
            InSequence seq;
            EXPECT_CALL(m_sceneEventSenderFromSceneRefLogic, sendSceneStateChanged(MasterSceneId, RefSceneId, RendererSceneState::Available));
            EXPECT_CALL(m_sceneEventSenderFromSceneRefLogic, sendSceneStateChanged(MasterSceneId, RefSceneId, RendererSceneState::Ready));
            EXPECT_CALL(m_sceneEventSenderFromSceneRefLogic, sendSceneStateChanged(MasterSceneId, RefSceneId, RendererSceneState::Rendered));

            m_sceneLogic.setSceneState(MasterSceneId, RendererSceneState::Rendered);

            masterScene.requestSceneReferenceState(refSceneHandle, RendererSceneState::Rendered);
            m_sceneUpdater.handleScenePublished(RefSceneId, EScenePublicationMode::LocalAndRemote);
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
        auto& masterScene = m_scenes.getScene(MasterSceneId);
        const auto refSceneHandle = SceneAllocateHelper(masterScene).allocateSceneReference(RefSceneId);

        // get both scenes to rendered
        {
            InSequence seq;
            EXPECT_CALL(m_sceneEventSenderFromSceneRefLogic, sendSceneStateChanged(MasterSceneId, RefSceneId, RendererSceneState::Available));
            EXPECT_CALL(m_sceneEventSenderFromSceneRefLogic, sendSceneStateChanged(MasterSceneId, RefSceneId, RendererSceneState::Ready));
            EXPECT_CALL(m_sceneEventSenderFromSceneRefLogic, sendSceneStateChanged(MasterSceneId, RefSceneId, RendererSceneState::Rendered));

            m_sceneLogic.setSceneState(MasterSceneId, RendererSceneState::Rendered);

            masterScene.requestSceneReferenceState(refSceneHandle, RendererSceneState::Rendered);
            m_sceneUpdater.handleScenePublished(RefSceneId, EScenePublicationMode::LocalAndRemote);
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
        update();
        update();
        // ... and destroyed eventually
        EXPECT_FALSE(m_scenes.hasScene(RefSceneId));
    }
}
