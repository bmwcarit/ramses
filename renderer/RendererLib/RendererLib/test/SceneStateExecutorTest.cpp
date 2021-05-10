//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "renderer_common_gmock_header.h"
#include "RendererLib/SceneStateExecutor.h"
#include "RendererLib/RendererLogContext.h"
#include "RendererLib/RendererScenes.h"
#include "RendererLib/SceneExpirationMonitor.h"
#include "RendererEventCollector.h"
#include "ComponentMocks.h"
#include "RendererMock.h"
#include "RendererSceneEventSenderMock.h"
#include "Utils/ThreadLocalLog.h"

namespace ramses_internal
{
    using namespace testing;
    class ASceneStateExecutor : public testing::Test
    {
    protected:
        ASceneStateExecutor()
            : senderID(1000)
            , displayHandle(1u)
            , rendererScenes(rendererEventCollector)
            , expirationMonitor(rendererScenes, rendererEventCollector)
            , renderer(displayHandle, rendererScenes, rendererEventCollector, expirationMonitor, rendererStatistics)
            , sceneStateExecutor(renderer, rendererSceneSender, rendererEventCollector)
        {
            // caller is expected to have a display prefix for logs
            ThreadLocalLog::SetPrefix(1);
        }

        virtual void TearDown() override
        {
            // make sure all test cases dispatch all collected events
            RendererEventVector resultEvents;
            rendererEventCollector.appendAndConsumePendingEvents(resultEvents, resultEvents);
            EXPECT_TRUE(resultEvents.empty());
            InternalSceneStateEvents sceneEvts;
            rendererEventCollector.dispatchInternalSceneStateEvents(sceneEvts);
            EXPECT_TRUE(sceneEvts.empty());
        }

        void expectRendererEvents(const std::vector<ERendererEventType>& expectedEvents, SceneId sId = sceneId)
        {
            InternalSceneStateEvents sceneEvts;
            rendererEventCollector.dispatchInternalSceneStateEvents(sceneEvts);
            ASSERT_EQ(expectedEvents.size(), sceneEvts.size());
            for (size_t i = 0; i < sceneEvts.size(); ++i)
            {
                EXPECT_EQ(expectedEvents[i], sceneEvts[i].type) << "internal scene state event #" << i;
                EXPECT_EQ(sId, sceneEvts[i].sceneId) << "scene control event #" << i;
            }
        }

        void expectNoRendererEvent()
        {
            expectRendererEvents({});
        }

        void expectRendererEvent(ERendererEventType eventType, SceneId sId = sceneId)
        {
            expectRendererEvents({ eventType }, sId);
        }

        void publishScene()
        {
            sceneStateExecutor.setPublished(sceneId, EScenePublicationMode_LocalAndRemote);
            expectRendererEvent(ERendererEventType::ScenePublished);
            EXPECT_EQ(ESceneState::Published, sceneStateExecutor.getSceneState(sceneId));
        }

        void subscribeScene()
        {
            EXPECT_CALL(rendererSceneSender, sendSubscribeScene(sceneId));
            sceneStateExecutor.setSubscriptionRequested(sceneId);
            EXPECT_EQ(ESceneState::SubscriptionRequested, sceneStateExecutor.getSceneState(sceneId));
        }

        void receiveScene()
        {
            sceneStateExecutor.setSubscriptionPending(sceneId);
            expectNoRendererEvent();
            EXPECT_EQ(ESceneState::SubscriptionPending, sceneStateExecutor.getSceneState(sceneId));
        }

        void receiveFlush()
        {
            sceneStateExecutor.setSubscribed(sceneId);
            expectRendererEvent(ERendererEventType::SceneSubscribed);
            EXPECT_EQ(ESceneState::Subscribed, sceneStateExecutor.getSceneState(sceneId));
        }

        void requestMapScene()
        {
            sceneStateExecutor.setMapRequested(sceneId);
            expectNoRendererEvent();
            EXPECT_EQ(ESceneState::MapRequested, sceneStateExecutor.getSceneState(sceneId));
        }

        void setSceneMappingAndUploading()
        {
            sceneStateExecutor.setMappingAndUploading(sceneId);
            expectNoRendererEvent();
            EXPECT_EQ(ESceneState::MappingAndUploading, sceneStateExecutor.getSceneState(sceneId));
        }

        void setSceneMapped()
        {
            sceneStateExecutor.setMapped(sceneId);
            expectRendererEvent(ERendererEventType::SceneMapped);
            EXPECT_EQ(ESceneState::Mapped, sceneStateExecutor.getSceneState(sceneId));
        }

        void setSceneRenderedRequested()
        {
            sceneStateExecutor.setRenderedRequested(sceneId);
            expectNoRendererEvent();
            EXPECT_EQ(ESceneState::RenderRequested, sceneStateExecutor.getSceneState(sceneId));
        }

        void setSceneRendered()
        {
            sceneStateExecutor.setRendered(sceneId);
            expectRendererEvent(ERendererEventType::SceneShown);
            EXPECT_EQ(ESceneState::Rendered, sceneStateExecutor.getSceneState(sceneId));
        }

        void setSceneHidden()
        {
            sceneStateExecutor.setHidden(sceneId);
            expectRendererEvent(ERendererEventType::SceneHidden);
            EXPECT_EQ(ESceneState::Mapped, sceneStateExecutor.getSceneState(sceneId));
        }

        void createDisplay()
        {
            ASSERT_FALSE(renderer.hasDisplayController());
            renderer.createDisplayContext({});
        }

        void destroyDisplay()
        {
            ASSERT_TRUE(renderer.hasDisplayController());
            renderer.destroyDisplayContext();
        }

        const Guid senderID;
        static constexpr SceneId sceneId{ 123u };

        const DisplayHandle displayHandle;

        RendererEventCollector rendererEventCollector;
        RendererScenes rendererScenes;
        SceneExpirationMonitor expirationMonitor;
        RendererStatistics rendererStatistics;
        NiceMock<RendererMockWithNiceMockDisplay> renderer;
        StrictMock<RendererSceneEventSenderMock> rendererSceneSender;
        SceneStateExecutor sceneStateExecutor;
    };

    constexpr SceneId ASceneStateExecutor::sceneId;

    TEST_F(ASceneStateExecutor, canPublishUnknownScene)
    {
        const SceneId sceneIdForUnknownScene(1u);
        EXPECT_TRUE(sceneStateExecutor.checkIfCanBePublished(sceneIdForUnknownScene));
    }

    TEST_F(ASceneStateExecutor, canNotPublishAlreadyPublishedScene)
    {
        publishScene();
        EXPECT_FALSE(sceneStateExecutor.checkIfCanBePublished(sceneId));
    }

    TEST_F(ASceneStateExecutor, canUnpublishAlreadyPublishedScene)
    {
        publishScene();
        EXPECT_TRUE(sceneStateExecutor.checkIfCanBeUnpublished(sceneId));
    }

    TEST_F(ASceneStateExecutor, canUnpublishSceneRequestedForSubscription)
    {
        publishScene();
        subscribeScene();
        EXPECT_TRUE(sceneStateExecutor.checkIfCanBeUnpublished(sceneId));
    }

    TEST_F(ASceneStateExecutor, canUnpublishSubscriptionPendingScene)
    {
        publishScene();
        subscribeScene();
        receiveScene();
        EXPECT_TRUE(sceneStateExecutor.checkIfCanBeUnpublished(sceneId));
    }

    TEST_F(ASceneStateExecutor, canUnpublishSubscribedScene)
    {
        publishScene();
        subscribeScene();
        receiveScene();
        receiveFlush();
        EXPECT_TRUE(sceneStateExecutor.checkIfCanBeUnpublished(sceneId));
    }

    TEST_F(ASceneStateExecutor, canUnpublishMapRequestedScene)
    {
        publishScene();
        subscribeScene();
        receiveScene();
        receiveFlush();
        createDisplay();
        requestMapScene();
        EXPECT_TRUE(sceneStateExecutor.checkIfCanBeUnpublished(sceneId));
        destroyDisplay();
    }

    TEST_F(ASceneStateExecutor, canUnpublishMappingAndUploadingScene)
    {
        publishScene();
        subscribeScene();
        receiveScene();
        receiveFlush();
        createDisplay();
        requestMapScene();
        setSceneMappingAndUploading();
        EXPECT_TRUE(sceneStateExecutor.checkIfCanBeUnpublished(sceneId));
        destroyDisplay();
    }

    TEST_F(ASceneStateExecutor, canUnpublishMappedScene)
    {
        publishScene();
        subscribeScene();
        receiveScene();
        receiveFlush();
        createDisplay();
        requestMapScene();
        setSceneMappingAndUploading();
        setSceneMapped();
        EXPECT_TRUE(sceneStateExecutor.checkIfCanBeUnpublished(sceneId));
        destroyDisplay();
    }

    TEST_F(ASceneStateExecutor, canUnpublishRenderedRequestedScene)
    {
        publishScene();
        subscribeScene();
        receiveScene();
        receiveFlush();
        createDisplay();
        requestMapScene();
        setSceneMappingAndUploading();
        setSceneMapped();
        setSceneRenderedRequested();
        EXPECT_TRUE(sceneStateExecutor.checkIfCanBeUnpublished(sceneId));
        destroyDisplay();
    }

    TEST_F(ASceneStateExecutor, canUnpublishRenderedScene)
    {
        publishScene();
        subscribeScene();
        receiveScene();
        receiveFlush();
        createDisplay();
        requestMapScene();
        setSceneMappingAndUploading();
        setSceneMapped();
        setSceneRenderedRequested();
        setSceneRendered();
        EXPECT_TRUE(sceneStateExecutor.checkIfCanBeUnpublished(sceneId));
        destroyDisplay();
    }

    TEST_F(ASceneStateExecutor, canNotUnpublishUnknownScene)
    {
        const SceneId sceneIdForUnknownScene(1u);
        EXPECT_FALSE(sceneStateExecutor.checkIfCanBeUnpublished(sceneIdForUnknownScene));
        expectNoRendererEvent();
    }

    TEST_F(ASceneStateExecutor, canRequestSubscriptionForPublishedScene)
    {
        publishScene();
        EXPECT_TRUE(sceneStateExecutor.checkIfCanBeSubscriptionRequested(sceneId));
        expectNoRendererEvent();
    }

    TEST_F(ASceneStateExecutor, canNotRequestSubscriptionForUnknownScene)
    {
        const SceneId sceneIdForUnknownScene(1u);
        EXPECT_FALSE(sceneStateExecutor.checkIfCanBeSubscriptionRequested(sceneIdForUnknownScene));
        expectRendererEvent(ERendererEventType::SceneSubscribeFailed, sceneIdForUnknownScene);
    }

    TEST_F(ASceneStateExecutor, canNotRequestSubscriptionForSceneAlreadyRequestedForSubscription)
    {
        publishScene();
        subscribeScene();
        EXPECT_FALSE(sceneStateExecutor.checkIfCanBeSubscriptionRequested(sceneId));
        expectRendererEvent(ERendererEventType::SceneSubscribeFailed);
    }

    TEST_F(ASceneStateExecutor, canNotRequestSubscriptionForSubscriptionPendingScene)
    {
        publishScene();
        subscribeScene();
        receiveScene();
        EXPECT_FALSE(sceneStateExecutor.checkIfCanBeSubscriptionRequested(sceneId));
        expectRendererEvent(ERendererEventType::SceneSubscribeFailed);
    }

    TEST_F(ASceneStateExecutor, canNotRequestSubscriptionForAlreadySubscribedScene)
    {
        publishScene();
        subscribeScene();
        receiveScene();
        receiveFlush();
        EXPECT_FALSE(sceneStateExecutor.checkIfCanBeSubscriptionRequested(sceneId));
        expectRendererEvent(ERendererEventType::SceneSubscribeFailed);
    }

    TEST_F(ASceneStateExecutor, canNotRequestSubscriptionForMapRequestedScene)
    {
        publishScene();
        subscribeScene();
        receiveScene();
        receiveFlush();
        createDisplay();
        requestMapScene();
        EXPECT_FALSE(sceneStateExecutor.checkIfCanBeSubscriptionRequested(sceneId));
        expectRendererEvent(ERendererEventType::SceneSubscribeFailed);
        destroyDisplay();
    }

    TEST_F(ASceneStateExecutor, canNotRequestSubscriptionForMappedScene)
    {
        publishScene();
        subscribeScene();
        receiveScene();
        receiveFlush();
        createDisplay();
        requestMapScene();
        setSceneMappingAndUploading();
        setSceneMapped();
        EXPECT_FALSE(sceneStateExecutor.checkIfCanBeSubscriptionRequested(sceneId));
        expectRendererEvent(ERendererEventType::SceneSubscribeFailed);
        destroyDisplay();
    }

    TEST_F(ASceneStateExecutor, canNotRequestSubscriptionForRenderedRequestedScene)
    {
        publishScene();
        subscribeScene();
        receiveScene();
        receiveFlush();
        createDisplay();
        requestMapScene();
        setSceneMappingAndUploading();
        setSceneMapped();
        setSceneRenderedRequested();
        EXPECT_FALSE(sceneStateExecutor.checkIfCanBeSubscriptionRequested(sceneId));
        expectRendererEvent(ERendererEventType::SceneSubscribeFailed);
        destroyDisplay();
    }

    TEST_F(ASceneStateExecutor, canNotRequestSubscriptionForRenderedScene)
    {
        publishScene();
        subscribeScene();
        receiveScene();
        receiveFlush();
        createDisplay();
        requestMapScene();
        setSceneMappingAndUploading();
        setSceneMapped();
        setSceneRenderedRequested();
        setSceneRendered();
        EXPECT_FALSE(sceneStateExecutor.checkIfCanBeSubscriptionRequested(sceneId));
        expectRendererEvent(ERendererEventType::SceneSubscribeFailed);
        destroyDisplay();
    }

    TEST_F(ASceneStateExecutor, canUnsubscribeAlreadySubscribedScene)
    {
        publishScene();
        subscribeScene();
        receiveScene();
        receiveFlush();
        EXPECT_TRUE(sceneStateExecutor.checkIfCanBeUnsubscribed(sceneId));
        expectNoRendererEvent();
    }

    TEST_F(ASceneStateExecutor, canUnsubscribeSceneRequestedForSubscription)
    {
        publishScene();
        subscribeScene();
        EXPECT_TRUE(sceneStateExecutor.checkIfCanBeUnsubscribed(sceneId));
        expectNoRendererEvent();
    }

    TEST_F(ASceneStateExecutor, canUnsubscribeReceivedSceneWithSubscriptionPending)
    {
        publishScene();
        subscribeScene();
        receiveScene();
        EXPECT_TRUE(sceneStateExecutor.checkIfCanBeUnsubscribed(sceneId));
        expectNoRendererEvent();
    }

    TEST_F(ASceneStateExecutor, canNotUnsubscribeUnknownScene)
    {
        const SceneId sceneIdForUnknownScene(1u);
        EXPECT_FALSE(sceneStateExecutor.checkIfCanBeUnsubscribed(sceneIdForUnknownScene));
        expectRendererEvent(ERendererEventType::SceneUnsubscribeFailed, sceneIdForUnknownScene);
    }

    TEST_F(ASceneStateExecutor, canNotUnsubscribePublishedScene)
    {
        publishScene();
        EXPECT_FALSE(sceneStateExecutor.checkIfCanBeUnsubscribed(sceneId));
        expectRendererEvent(ERendererEventType::SceneUnsubscribeFailed);
    }

    TEST_F(ASceneStateExecutor, canNotUnsubscribeMapRequestedScene)
    {
        publishScene();
        subscribeScene();
        receiveScene();
        receiveFlush();
        createDisplay();
        requestMapScene();
        EXPECT_FALSE(sceneStateExecutor.checkIfCanBeUnsubscribed(sceneId));
        expectRendererEvent(ERendererEventType::SceneUnsubscribeFailed);
        destroyDisplay();
    }

    TEST_F(ASceneStateExecutor, canNotUnsubscribeMappingAndUploadingScene)
    {
        publishScene();
        subscribeScene();
        receiveScene();
        receiveFlush();
        createDisplay();
        requestMapScene();
        setSceneMappingAndUploading();
        EXPECT_FALSE(sceneStateExecutor.checkIfCanBeUnsubscribed(sceneId));
        expectRendererEvent(ERendererEventType::SceneUnsubscribeFailed);
        destroyDisplay();
    }

    TEST_F(ASceneStateExecutor, canNotUnsubscribeMappedScene)
    {
        publishScene();
        subscribeScene();
        receiveScene();
        receiveFlush();
        createDisplay();
        requestMapScene();
        setSceneMappingAndUploading();
        setSceneMapped();
        EXPECT_FALSE(sceneStateExecutor.checkIfCanBeUnsubscribed(sceneId));
        expectRendererEvent(ERendererEventType::SceneUnsubscribeFailed);
        destroyDisplay();
    }

    TEST_F(ASceneStateExecutor, canNotUnsubscribeRenderedRequestedScene)
    {
        publishScene();
        subscribeScene();
        receiveScene();
        receiveFlush();
        createDisplay();
        requestMapScene();
        setSceneMappingAndUploading();
        setSceneMapped();
        setSceneRenderedRequested();
        EXPECT_FALSE(sceneStateExecutor.checkIfCanBeUnsubscribed(sceneId));
        expectRendererEvent(ERendererEventType::SceneUnsubscribeFailed);
        destroyDisplay();
    }

    TEST_F(ASceneStateExecutor, canNotUnsubscribeRenderedScene)
    {
        publishScene();
        subscribeScene();
        receiveScene();
        receiveFlush();
        createDisplay();
        requestMapScene();
        setSceneMappingAndUploading();
        setSceneMapped();
        setSceneRenderedRequested();
        setSceneRendered();
        EXPECT_FALSE(sceneStateExecutor.checkIfCanBeUnsubscribed(sceneId));
        expectRendererEvent(ERendererEventType::SceneUnsubscribeFailed);
        destroyDisplay();
    }

    TEST_F(ASceneStateExecutor, canRequestMappingForSubscribedSceneOnExistingDisplay)
    {
        createDisplay();
        publishScene();
        subscribeScene();
        receiveScene();
        receiveFlush();
        EXPECT_TRUE(sceneStateExecutor.checkIfCanBeMapRequested(sceneId));
        expectNoRendererEvent();
        destroyDisplay();
    }

    TEST_F(ASceneStateExecutor, canMapRequestSceneOnExistingDisplay)
    {
        createDisplay();
        publishScene();
        subscribeScene();
        receiveScene();
        receiveFlush();
        EXPECT_TRUE(sceneStateExecutor.checkIfCanBeMapRequested(sceneId));
        requestMapScene();
        expectNoRendererEvent();
        destroyDisplay();
    }

    TEST_F(ASceneStateExecutor, canNotMapSubscribedSceneOnNonExistingDisplay)
    {
        publishScene();
        subscribeScene();
        receiveScene();
        receiveFlush();
        EXPECT_FALSE(sceneStateExecutor.checkIfCanBeMapRequested(sceneId));
        expectRendererEvent(ERendererEventType::SceneMapFailed);
    }

    TEST_F(ASceneStateExecutor, canNotMapAlreadyMappedScene)
    {
        createDisplay();
        publishScene();
        subscribeScene();
        receiveScene();
        receiveFlush();
        requestMapScene();
        setSceneMappingAndUploading();
        setSceneMapped();
        EXPECT_FALSE(sceneStateExecutor.checkIfCanBeMapped(sceneId));
        expectRendererEvent(ERendererEventType::SceneMapFailed);
        destroyDisplay();
    }

    TEST_F(ASceneStateExecutor, canNotMapPublishedScene)
    {
        createDisplay();
        publishScene();
        EXPECT_FALSE(sceneStateExecutor.checkIfCanBeMapRequested(sceneId));
        expectRendererEvent(ERendererEventType::SceneMapFailed);
        destroyDisplay();
    }

    TEST_F(ASceneStateExecutor, canNotMapSubscriptionRequestedScene)
    {
        createDisplay();
        publishScene();
        subscribeScene();
        EXPECT_FALSE(sceneStateExecutor.checkIfCanBeMapRequested(sceneId));
        expectRendererEvent(ERendererEventType::SceneMapFailed);
        destroyDisplay();
    }

    TEST_F(ASceneStateExecutor, canNotMapReceivedScene)
    {
        createDisplay();
        publishScene();
        subscribeScene();
        EXPECT_FALSE(sceneStateExecutor.checkIfCanBeMapRequested(sceneId));
        expectRendererEvent(ERendererEventType::SceneMapFailed);
        destroyDisplay();
    }

    TEST_F(ASceneStateExecutor, canNotMapUnknownScene)
    {
        createDisplay();
        const SceneId sceneIdForUnknownScene(1u);
        EXPECT_FALSE(sceneStateExecutor.checkIfCanBeMapRequested(sceneIdForUnknownScene));
        expectRendererEvent(ERendererEventType::SceneMapFailed, sceneIdForUnknownScene);
        destroyDisplay();
    }

    TEST_F(ASceneStateExecutor, canNotMapASceneWhichIsMapRequested)
    {
        createDisplay();
        publishScene();
        subscribeScene();
        receiveScene();
        receiveFlush();
        EXPECT_TRUE(sceneStateExecutor.checkIfCanBeMapRequested(sceneId));
        requestMapScene();
        EXPECT_FALSE(sceneStateExecutor.checkIfCanBeMapRequested(sceneId));
        expectNoRendererEvent();
        destroyDisplay();
    }

    TEST_F(ASceneStateExecutor, canNotMapASceneWhichIsRenderedRequested)
    {
        createDisplay();
        publishScene();
        subscribeScene();
        receiveScene();
        receiveFlush();
        requestMapScene();
        setSceneMappingAndUploading();
        setSceneMapped();
        setSceneRenderedRequested();
        EXPECT_FALSE(sceneStateExecutor.checkIfCanBeMapped(sceneId));
        expectRendererEvent(ERendererEventType::SceneMapFailed);
        destroyDisplay();
    }

    TEST_F(ASceneStateExecutor, canNotMapASceneWhichIsRendered)
    {
        createDisplay();
        publishScene();
        subscribeScene();
        receiveScene();
        receiveFlush();
        requestMapScene();
        setSceneMappingAndUploading();
        setSceneMapped();
        setSceneRenderedRequested();
        setSceneRendered();
        EXPECT_FALSE(sceneStateExecutor.checkIfCanBeMapped(sceneId));
        expectRendererEvent(ERendererEventType::SceneMapFailed);
        destroyDisplay();
    }

    TEST_F(ASceneStateExecutor, canNotHidePublishedScene)
    {
        publishScene();
        EXPECT_FALSE(sceneStateExecutor.checkIfCanBeHidden(sceneId));
        expectRendererEvent(ERendererEventType::SceneHideFailed);
    }

    TEST_F(ASceneStateExecutor, canNotHideSubscriptionRequestedScene)
    {
        publishScene();
        subscribeScene();
        EXPECT_FALSE(sceneStateExecutor.checkIfCanBeHidden(sceneId));
        expectRendererEvent(ERendererEventType::SceneHideFailed);
    }

    TEST_F(ASceneStateExecutor, canNotHideSubscriptionPendingScene)
    {
        publishScene();
        subscribeScene();
        receiveScene();
        EXPECT_FALSE(sceneStateExecutor.checkIfCanBeHidden(sceneId));
        expectRendererEvent(ERendererEventType::SceneHideFailed);
    }

    TEST_F(ASceneStateExecutor, canNotHideSubscribedScene)
    {
        publishScene();
        subscribeScene();
        receiveScene();
        receiveFlush();
        EXPECT_FALSE(sceneStateExecutor.checkIfCanBeHidden(sceneId));
        expectRendererEvent(ERendererEventType::SceneHideFailed);
    }

    TEST_F(ASceneStateExecutor, canNotHideMapRequestedScene)
    {
        createDisplay();
        publishScene();
        subscribeScene();
        receiveScene();
        receiveFlush();
        requestMapScene();
        EXPECT_FALSE(sceneStateExecutor.checkIfCanBeHidden(sceneId));
        expectRendererEvent(ERendererEventType::SceneHideFailed);
        destroyDisplay();
    }

    TEST_F(ASceneStateExecutor, canNotHideMappingAndUploadingScene)
    {
        createDisplay();
        publishScene();
        subscribeScene();
        receiveScene();
        receiveFlush();
        requestMapScene();
        setSceneMappingAndUploading();
        EXPECT_FALSE(sceneStateExecutor.checkIfCanBeHidden(sceneId));
        expectRendererEvent(ERendererEventType::SceneHideFailed);
        destroyDisplay();
    }

    TEST_F(ASceneStateExecutor, canNotHideASceneWhichIsMapped)
    {
        createDisplay();
        publishScene();
        subscribeScene();
        receiveScene();
        receiveFlush();
        requestMapScene();
        setSceneMappingAndUploading();
        setSceneMapped();
        EXPECT_FALSE(sceneStateExecutor.checkIfCanBeHidden(sceneId));
        expectRendererEvent(ERendererEventType::SceneHideFailed);
        destroyDisplay();
    }

    TEST_F(ASceneStateExecutor, canUnmapMappedScene)
    {
        createDisplay();
        publishScene();
        subscribeScene();
        receiveScene();
        receiveFlush();
        requestMapScene();
        setSceneMappingAndUploading();
        setSceneMapped();
        EXPECT_TRUE(sceneStateExecutor.checkIfCanBeUnmapped(sceneId));
        expectNoRendererEvent();
        destroyDisplay();
    }

    TEST_F(ASceneStateExecutor, canNotUnmapUnknownScene)
    {
        createDisplay();
        const SceneId sceneIdForUnknownScene(1u);
        EXPECT_FALSE(sceneStateExecutor.checkIfCanBeUnmapped(sceneIdForUnknownScene));
        expectRendererEvent(ERendererEventType::SceneUnmapFailed, sceneIdForUnknownScene);
        destroyDisplay();
    }

    TEST_F(ASceneStateExecutor, canNotUnmapPublishedScene)
    {
        createDisplay();
        publishScene();
        EXPECT_FALSE(sceneStateExecutor.checkIfCanBeUnmapped(sceneId));
        expectRendererEvent(ERendererEventType::SceneUnmapFailed);
        destroyDisplay();
    }

    TEST_F(ASceneStateExecutor, canNotUnmapSceneRequestedForSubscription)
    {
        createDisplay();
        publishScene();
        subscribeScene();
        EXPECT_FALSE(sceneStateExecutor.checkIfCanBeUnmapped(sceneId));
        expectRendererEvent(ERendererEventType::SceneUnmapFailed);
        destroyDisplay();
    }

    TEST_F(ASceneStateExecutor, canNotUnmapSceneSubscriptionPending)
    {
        createDisplay();
        publishScene();
        subscribeScene();
        receiveScene();
        EXPECT_FALSE(sceneStateExecutor.checkIfCanBeUnmapped(sceneId));
        expectRendererEvent(ERendererEventType::SceneUnmapFailed);
        destroyDisplay();
    }

    TEST_F(ASceneStateExecutor, canNotUnmapSubscribedScene)
    {
        createDisplay();
        publishScene();
        subscribeScene();
        receiveScene();
        receiveFlush();
        EXPECT_FALSE(sceneStateExecutor.checkIfCanBeUnmapped(sceneId));
        expectRendererEvent(ERendererEventType::SceneUnmapFailed);
        destroyDisplay();
    }

    TEST_F(ASceneStateExecutor, canUnmapMapRequestedScene)
    {
        createDisplay();
        publishScene();
        subscribeScene();
        receiveScene();
        receiveFlush();
        requestMapScene();
        EXPECT_TRUE(sceneStateExecutor.checkIfCanBeUnmapped(sceneId));
        sceneStateExecutor.setUnmapped(sceneId);
        expectRendererEvent(ERendererEventType::SceneUnmapped);
        destroyDisplay();
    }

    TEST_F(ASceneStateExecutor, canUnmapMappingAndUploadingScene)
    {
        createDisplay();
        publishScene();
        subscribeScene();
        receiveScene();
        receiveFlush();
        requestMapScene();
        setSceneMappingAndUploading();
        EXPECT_TRUE(sceneStateExecutor.checkIfCanBeUnmapped(sceneId));
        sceneStateExecutor.setUnmapped(sceneId);
        expectRendererEvent(ERendererEventType::SceneUnmapped);
        destroyDisplay();
    }

    TEST_F(ASceneStateExecutor, canNotUnmapRenderedRequestedScene)
    {
        createDisplay();
        publishScene();
        subscribeScene();
        receiveScene();
        receiveFlush();
        requestMapScene();
        setSceneMappingAndUploading();
        setSceneMapped();
        setSceneRenderedRequested();
        EXPECT_FALSE(sceneStateExecutor.checkIfCanBeUnmapped(sceneId));
        expectRendererEvent(ERendererEventType::SceneUnmapFailed);
        destroyDisplay();
    }

    TEST_F(ASceneStateExecutor, canNotUnmapRenderedScene)
    {
        createDisplay();
        publishScene();
        subscribeScene();
        receiveScene();
        receiveFlush();
        requestMapScene();
        setSceneMappingAndUploading();
        setSceneMapped();
        setSceneRenderedRequested();
        setSceneRendered();
        EXPECT_FALSE(sceneStateExecutor.checkIfCanBeUnmapped(sceneId));
        expectRendererEvent(ERendererEventType::SceneUnmapFailed);
        destroyDisplay();
    }

    TEST_F(ASceneStateExecutor, canReceiveSceneRequestedForSubscription)
    {
        publishScene();
        subscribeScene();
        EXPECT_TRUE(sceneStateExecutor.checkIfCanBeSubscriptionPending(sceneId));
        expectNoRendererEvent();
    }

    TEST_F(ASceneStateExecutor, canNotReceiveUnknownScene)
    {
        const SceneId sceneIdForUnknownScene(1u);
        EXPECT_FALSE(sceneStateExecutor.checkIfCanBeSubscriptionPending(sceneIdForUnknownScene));
        expectNoRendererEvent();
    }

    TEST_F(ASceneStateExecutor, canNotReceivePublishedSceneNotRequestedForSubscription)
    {
        publishScene();
        EXPECT_FALSE(sceneStateExecutor.checkIfCanBeSubscriptionPending(sceneId));
        expectNoRendererEvent();
    }

    TEST_F(ASceneStateExecutor, canNotReceiveAlreadyReceivedScene)
    {
        publishScene();
        subscribeScene();
        receiveScene();
        EXPECT_FALSE(sceneStateExecutor.checkIfCanBeSubscriptionPending(sceneId));
        expectNoRendererEvent();
    }

    TEST_F(ASceneStateExecutor, canNotReceiveMapRequestedScene)
    {
        publishScene();
        subscribeScene();
        receiveScene();
        receiveFlush();
        createDisplay();
        requestMapScene();
        EXPECT_FALSE(sceneStateExecutor.checkIfCanBeSubscriptionPending(sceneId));
        expectNoRendererEvent();
        destroyDisplay();
    }

    TEST_F(ASceneStateExecutor, canNotReceiveMappingAndUploadingScene)
    {
        publishScene();
        subscribeScene();
        receiveScene();
        receiveFlush();
        createDisplay();
        requestMapScene();
        setSceneMappingAndUploading();
        EXPECT_FALSE(sceneStateExecutor.checkIfCanBeSubscriptionPending(sceneId));
        expectNoRendererEvent();
        destroyDisplay();
    }

    TEST_F(ASceneStateExecutor, canNotReceiveMappedScene)
    {
        publishScene();
        subscribeScene();
        receiveScene();
        receiveFlush();
        createDisplay();
        requestMapScene();
        setSceneMappingAndUploading();
        setSceneMapped();
        EXPECT_FALSE(sceneStateExecutor.checkIfCanBeSubscriptionPending(sceneId));
        expectNoRendererEvent();
        destroyDisplay();
    }

    TEST_F(ASceneStateExecutor, canNotReceiveFlushForPublishedScene)
    {
        publishScene();
        EXPECT_FALSE(sceneStateExecutor.checkIfCanBeSubscribed(sceneId));
        expectNoRendererEvent();
    }

    TEST_F(ASceneStateExecutor, canNotReceiveFlushForNotReceivedScene)
    {
        publishScene();
        subscribeScene();
        EXPECT_FALSE(sceneStateExecutor.checkIfCanBeSubscribed(sceneId));
        expectNoRendererEvent();
    }

    TEST_F(ASceneStateExecutor, publishesScene)
    {
        sceneStateExecutor.setPublished(sceneId, EScenePublicationMode_LocalAndRemote);
        expectRendererEvent(ERendererEventType::ScenePublished);
    }

    TEST_F(ASceneStateExecutor, canGetClientGuidOfKnownScene)
    {
        sceneStateExecutor.setPublished(sceneId, EScenePublicationMode_LocalAndRemote);
        expectRendererEvent(ERendererEventType::ScenePublished);
    }

    TEST_F(ASceneStateExecutor, requestsSubscriptionForPublishedScene)
    {
        publishScene();
        EXPECT_CALL(rendererSceneSender, sendSubscribeScene(sceneId));
        sceneStateExecutor.setSubscriptionRequested(sceneId);
        expectNoRendererEvent();
    }

    TEST_F(ASceneStateExecutor, receivesSubscribedScene)
    {
        publishScene();
        subscribeScene();
        sceneStateExecutor.setSubscriptionPending(sceneId);
        expectNoRendererEvent();
    }

    TEST_F(ASceneStateExecutor, receivesFlushForReceivedScene)
    {
        publishScene();
        subscribeScene();
        receiveScene();
        sceneStateExecutor.setSubscribed(sceneId);
        expectRendererEvent(ERendererEventType::SceneSubscribed);
    }

    TEST_F(ASceneStateExecutor, unpublishesPublishedScene)
    {
        publishScene();
        sceneStateExecutor.setUnpublished(sceneId);
        expectRendererEvent(ERendererEventType::SceneUnpublished);
        EXPECT_EQ(ESceneState::Unknown, sceneStateExecutor.getSceneState(sceneId));
    }

    TEST_F(ASceneStateExecutor, unpublishesSceneRequestedForSubscription)
    {
        publishScene();
        subscribeScene();
        EXPECT_CALL(rendererSceneSender, sendUnsubscribeScene(sceneId));
        sceneStateExecutor.setUnpublished(sceneId);
        expectRendererEvents({ ERendererEventType::SceneSubscribeFailed, ERendererEventType::SceneUnpublished });
        EXPECT_EQ(ESceneState::Unknown, sceneStateExecutor.getSceneState(sceneId));
    }

    TEST_F(ASceneStateExecutor, unpublishesReceivedScene)
    {
        publishScene();
        subscribeScene();
        receiveScene();
        EXPECT_CALL(rendererSceneSender, sendUnsubscribeScene(sceneId));
        sceneStateExecutor.setUnpublished(sceneId);
        // subscription still pending
        expectRendererEvents({ ERendererEventType::SceneSubscribeFailed, ERendererEventType::SceneUnpublished });
        EXPECT_EQ(ESceneState::Unknown, sceneStateExecutor.getSceneState(sceneId));
    }

    TEST_F(ASceneStateExecutor, unpublishesSubscribedScene)
    {
        publishScene();
        subscribeScene();
        receiveScene();
        receiveFlush();
        EXPECT_CALL(rendererSceneSender, sendUnsubscribeScene(sceneId));
        sceneStateExecutor.setUnpublished(sceneId);
        expectRendererEvents({ ERendererEventType::SceneUnsubscribedIndirect, ERendererEventType::SceneUnpublished });
        EXPECT_EQ(ESceneState::Unknown, sceneStateExecutor.getSceneState(sceneId));
    }

    TEST_F(ASceneStateExecutor, unpublishesMapRequestedScene)
    {
        createDisplay();

        publishScene();
        subscribeScene();
        receiveScene();
        receiveFlush();
        requestMapScene();
        EXPECT_CALL(rendererSceneSender, sendUnsubscribeScene(sceneId));
        sceneStateExecutor.setUnpublished(sceneId);
        expectRendererEvents({ ERendererEventType::SceneMapFailed, ERendererEventType::SceneUnsubscribedIndirect, ERendererEventType::SceneUnpublished });
        EXPECT_EQ(ESceneState::Unknown, sceneStateExecutor.getSceneState(sceneId));

        destroyDisplay();
    }

    TEST_F(ASceneStateExecutor, unpublishesMappingAndUploadingScene)
    {
        createDisplay();

        publishScene();
        subscribeScene();
        receiveScene();
        receiveFlush();
        requestMapScene();
        setSceneMappingAndUploading();
        EXPECT_CALL(rendererSceneSender, sendUnsubscribeScene(sceneId));
        sceneStateExecutor.setUnpublished(sceneId);
        expectRendererEvents({ ERendererEventType::SceneMapFailed, ERendererEventType::SceneUnsubscribedIndirect, ERendererEventType::SceneUnpublished });
        EXPECT_EQ(ESceneState::Unknown, sceneStateExecutor.getSceneState(sceneId));

        destroyDisplay();
    }

    TEST_F(ASceneStateExecutor, unpublishesMappedScene)
    {
        createDisplay();

        publishScene();
        subscribeScene();
        receiveScene();
        receiveFlush();
        requestMapScene();
        setSceneMappingAndUploading();
        setSceneMapped();
        EXPECT_CALL(rendererSceneSender, sendUnsubscribeScene(sceneId));
        sceneStateExecutor.setUnpublished(sceneId);
        expectRendererEvents({ ERendererEventType::SceneUnmappedIndirect, ERendererEventType::SceneUnsubscribedIndirect, ERendererEventType::SceneUnpublished });
        EXPECT_EQ(ESceneState::Unknown, sceneStateExecutor.getSceneState(sceneId));

        destroyDisplay();
    }

    TEST_F(ASceneStateExecutor, unpublishesRenderedRequestedScene)
    {
        createDisplay();

        publishScene();
        subscribeScene();
        receiveScene();
        receiveFlush();
        requestMapScene();
        setSceneMappingAndUploading();
        setSceneMapped();
        setSceneRenderedRequested();
        EXPECT_CALL(rendererSceneSender, sendUnsubscribeScene(sceneId));
        sceneStateExecutor.setUnpublished(sceneId);
        expectRendererEvents({ ERendererEventType::SceneShowFailed, ERendererEventType::SceneUnmappedIndirect, ERendererEventType::SceneUnsubscribedIndirect, ERendererEventType::SceneUnpublished });
        EXPECT_EQ(ESceneState::Unknown, sceneStateExecutor.getSceneState(sceneId));

        destroyDisplay();
    }

    TEST_F(ASceneStateExecutor, unpublishesRenderedScene)
    {
        createDisplay();

        publishScene();
        subscribeScene();
        receiveScene();
        receiveFlush();
        requestMapScene();
        setSceneMappingAndUploading();
        setSceneMapped();
        setSceneRenderedRequested();
        setSceneRendered();
        EXPECT_CALL(rendererSceneSender, sendUnsubscribeScene(sceneId));
        sceneStateExecutor.setUnpublished(sceneId);
        expectRendererEvents({ ERendererEventType::SceneHiddenIndirect, ERendererEventType::SceneUnmappedIndirect, ERendererEventType::SceneUnsubscribedIndirect, ERendererEventType::SceneUnpublished });
        EXPECT_EQ(ESceneState::Unknown, sceneStateExecutor.getSceneState(sceneId));

        destroyDisplay();
    }

    TEST_F(ASceneStateExecutor, unsubscribesSubscribedScene)
    {
        publishScene();
        subscribeScene();
        receiveScene();
        receiveFlush();
        EXPECT_CALL(rendererSceneSender, sendUnsubscribeScene(sceneId));
        sceneStateExecutor.setUnsubscribed(sceneId, false);
        expectRendererEvent(ERendererEventType::SceneUnsubscribed);
    }

    TEST_F(ASceneStateExecutor, unsubscribesSubscriptionRequestedScene)
    {
        publishScene();
        subscribeScene();
        EXPECT_CALL(rendererSceneSender, sendUnsubscribeScene(sceneId));
        sceneStateExecutor.setUnsubscribed(sceneId, false);
        expectRendererEvents({ ERendererEventType::SceneUnsubscribed });
    }

    TEST_F(ASceneStateExecutor, unsubscribesSubscriptionPendingScene)
    {
        publishScene();
        subscribeScene();
        receiveScene();
        EXPECT_CALL(rendererSceneSender, sendUnsubscribeScene(sceneId));
        sceneStateExecutor.setUnsubscribed(sceneId, false);
        expectRendererEvents({ ERendererEventType::SceneUnsubscribed });
    }

    TEST_F(ASceneStateExecutor, forceUnsubscribesPublishedScene)
    {
        publishScene();
        sceneStateExecutor.setUnsubscribed(sceneId, true);
        expectNoRendererEvent();
    }

    TEST_F(ASceneStateExecutor, forceUnsubscribesSubscriptionRequestedScene)
    {
        publishScene();
        subscribeScene();
        EXPECT_CALL(rendererSceneSender, sendUnsubscribeScene(sceneId));
        sceneStateExecutor.setUnsubscribed(sceneId, true);
        // only expect failed because user never got subscribed and user didn't cause fail
        expectRendererEvents({ ERendererEventType::SceneSubscribeFailed });
    }

    TEST_F(ASceneStateExecutor, forceUnsubscribesSubscriptionPendingScene)
    {
        publishScene();
        subscribeScene();
        receiveScene();
        EXPECT_CALL(rendererSceneSender, sendUnsubscribeScene(sceneId));
        sceneStateExecutor.setUnsubscribed(sceneId, true);
        // only expect failed because user never got subscribed and user didn't cause fail
        expectRendererEvents({ ERendererEventType::SceneSubscribeFailed });
    }

    TEST_F(ASceneStateExecutor, forceUnsubscribesSubscribedScene)
    {
        publishScene();
        subscribeScene();
        receiveScene();
        receiveFlush();
        EXPECT_CALL(rendererSceneSender, sendUnsubscribeScene(sceneId));
        sceneStateExecutor.setUnsubscribed(sceneId, true);
        expectRendererEvent(ERendererEventType::SceneUnsubscribedIndirect);
    }

    TEST_F(ASceneStateExecutor, forceUnsubscribesMapRequestedScene)
    {
        createDisplay();

        publishScene();
        subscribeScene();
        receiveScene();
        receiveFlush();
        requestMapScene();
        EXPECT_CALL(rendererSceneSender, sendUnsubscribeScene(sceneId));
        sceneStateExecutor.setUnsubscribed(sceneId, true);
        expectRendererEvents({ ERendererEventType::SceneMapFailed, ERendererEventType::SceneUnsubscribedIndirect });

        destroyDisplay();
    }

    TEST_F(ASceneStateExecutor, forceUnsubscribesMappingAndUploadingScene)
    {
        createDisplay();

        publishScene();
        subscribeScene();
        receiveScene();
        receiveFlush();
        requestMapScene();
        setSceneMappingAndUploading();
        EXPECT_CALL(rendererSceneSender, sendUnsubscribeScene(sceneId));
        sceneStateExecutor.setUnsubscribed(sceneId, true);
        expectRendererEvents({ ERendererEventType::SceneMapFailed, ERendererEventType::SceneUnsubscribedIndirect });

        destroyDisplay();
    }

    TEST_F(ASceneStateExecutor, forceUnsubscribesMappedScene)
    {
        createDisplay();

        publishScene();
        subscribeScene();
        receiveScene();
        receiveFlush();
        requestMapScene();
        setSceneMappingAndUploading();
        setSceneMapped();
        EXPECT_CALL(rendererSceneSender, sendUnsubscribeScene(sceneId));
        sceneStateExecutor.setUnsubscribed(sceneId, true);
        expectRendererEvents({ ERendererEventType::SceneUnmappedIndirect, ERendererEventType::SceneUnsubscribedIndirect });

        destroyDisplay();
    }

    TEST_F(ASceneStateExecutor, forceUnsubscribesRenderRequestedScene)
    {
        createDisplay();

        publishScene();
        subscribeScene();
        receiveScene();
        receiveFlush();
        requestMapScene();
        setSceneMappingAndUploading();
        setSceneMapped();
        setSceneRenderedRequested();
        EXPECT_CALL(rendererSceneSender, sendUnsubscribeScene(sceneId));
        sceneStateExecutor.setUnsubscribed(sceneId, true);
        expectRendererEvents({ ERendererEventType::SceneShowFailed, ERendererEventType::SceneUnmappedIndirect, ERendererEventType::SceneUnsubscribedIndirect });

        destroyDisplay();
    }

    TEST_F(ASceneStateExecutor, forceUnsubscribesRenderedScene)
    {
        createDisplay();

        publishScene();
        subscribeScene();
        receiveScene();
        receiveFlush();
        requestMapScene();
        setSceneMappingAndUploading();
        setSceneMapped();
        setSceneRenderedRequested();
        setSceneRendered();
        EXPECT_CALL(rendererSceneSender, sendUnsubscribeScene(sceneId));
        sceneStateExecutor.setUnsubscribed(sceneId, true);
        expectRendererEvents({ ERendererEventType::SceneHiddenIndirect, ERendererEventType::SceneUnmappedIndirect, ERendererEventType::SceneUnsubscribedIndirect });

        destroyDisplay();
    }

    TEST_F(ASceneStateExecutor, requestsMappingOfSubscribedScene)
    {
        createDisplay();

        publishScene();
        subscribeScene();
        receiveScene();
        receiveFlush();
        sceneStateExecutor.setMapRequested(sceneId);
        expectNoRendererEvent();

        destroyDisplay();
    }

    TEST_F(ASceneStateExecutor, mapsSubscribedScene)
    {
        createDisplay();

        publishScene();
        subscribeScene();
        receiveScene();
        receiveFlush();
        requestMapScene();
        setSceneMappingAndUploading();
        setSceneMapped();

        destroyDisplay();
    }

    TEST_F(ASceneStateExecutor, renderRequestsMappedScene)
    {
        createDisplay();

        publishScene();
        subscribeScene();
        receiveScene();
        receiveFlush();
        requestMapScene();
        setSceneMappingAndUploading();
        setSceneMapped();
        setSceneRenderedRequested();

        destroyDisplay();
    }

    TEST_F(ASceneStateExecutor, rendersRenderedRequestedScene)
    {
        createDisplay();

        publishScene();
        subscribeScene();
        receiveScene();
        receiveFlush();
        requestMapScene();
        setSceneMappingAndUploading();
        setSceneMapped();
        setSceneRenderedRequested();
        setSceneRendered();

        destroyDisplay();
    }

    TEST_F(ASceneStateExecutor, hidesRenderedRequestedScene)
    {
        createDisplay();

        publishScene();
        subscribeScene();
        receiveScene();
        receiveFlush();
        requestMapScene();
        setSceneMappingAndUploading();
        setSceneMapped();
        setSceneRenderedRequested();
        setSceneHidden();

        destroyDisplay();
    }

    TEST_F(ASceneStateExecutor, hidesRenderedScene)
    {
        createDisplay();

        publishScene();
        subscribeScene();
        receiveScene();
        receiveFlush();
        requestMapScene();
        setSceneMappingAndUploading();
        setSceneMapped();
        setSceneRenderedRequested();
        setSceneRendered();
        setSceneHidden();

        destroyDisplay();
    }

    TEST_F(ASceneStateExecutor, unmapsMappedScene)
    {
        createDisplay();

        publishScene();
        subscribeScene();
        receiveScene();
        receiveFlush();
        requestMapScene();
        setSceneMappingAndUploading();
        setSceneMapped();
        sceneStateExecutor.setUnmapped(sceneId);
        expectRendererEvent(ERendererEventType::SceneUnmapped);

        destroyDisplay();
    }

    TEST_F(ASceneStateExecutor, canRepublishUnpublishedScene)
    {
        publishScene();
        sceneStateExecutor.setUnpublished(sceneId);
        expectRendererEvent(ERendererEventType::SceneUnpublished);
        EXPECT_EQ(ESceneState::Unknown, sceneStateExecutor.getSceneState(sceneId));
        publishScene();
    }

    TEST_F(ASceneStateExecutor, canResubscribeUnsubscribedScene)
    {
        publishScene();
        subscribeScene();
        receiveScene();
        receiveFlush();
        EXPECT_CALL(rendererSceneSender, sendUnsubscribeScene(sceneId));
        sceneStateExecutor.setUnsubscribed(sceneId, false);
        expectRendererEvent(ERendererEventType::SceneUnsubscribed);
        subscribeScene();
    }

    TEST_F(ASceneStateExecutor, canResubscribeSceneUnsubscribedByError)
    {
        publishScene();
        subscribeScene();
        receiveScene();
        receiveFlush();
        EXPECT_CALL(rendererSceneSender, sendUnsubscribeScene(sceneId));
        sceneStateExecutor.setUnsubscribed(sceneId, true);
        expectRendererEvent(ERendererEventType::SceneUnsubscribedIndirect);
        subscribeScene();
    }

    TEST_F(ASceneStateExecutor, canRemapUnmappedScene)
    {
        createDisplay();

        publishScene();
        subscribeScene();
        receiveScene();
        receiveFlush();
        requestMapScene();
        setSceneMappingAndUploading();
        setSceneMapped();
        sceneStateExecutor.setUnmapped(sceneId);
        expectRendererEvent(ERendererEventType::SceneUnmapped);
        requestMapScene();
        setSceneMappingAndUploading();
        setSceneMapped();

        destroyDisplay();
    }

    TEST_F(ASceneStateExecutor, getsUnknownStateForUnknownScene)
    {
        EXPECT_EQ(ESceneState::Unknown, sceneStateExecutor.getSceneState(SceneId(0u)));
    }
}
