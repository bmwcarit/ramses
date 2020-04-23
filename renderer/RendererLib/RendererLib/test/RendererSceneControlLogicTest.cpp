//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gmock/gmock.h"
#include "RendererSceneControlMock.h"
#include "RendererLib/RendererSceneControlLogic.h"
#include "RendererLib/RendererCommandTypes.h"

using namespace testing;

namespace ramses_internal
{
    class ARendererSceneControlLogic : public Test
    {
    public:
        ARendererSceneControlLogic()
            : m_logic(m_logicOutput)
        {
        }

        virtual void SetUp() override
        {
            m_logic.setSceneMapping(sceneId, displayId);
            m_logic.setSceneDisplayBufferAssignment(sceneId, OffscreenBufferHandle{}, sceneRenderOrder);
        }

        virtual void TearDown() override
        {
            // expect nothing to dispatch
            expectEvents({});
        }

    protected:
        void expectEvents(const RendererSceneControlLogic::Events& expectedEvts)
        {
            RendererSceneControlLogic::Events events;
            m_logic.consumeEvents(events);
            ASSERT_EQ(expectedEvts.size(), events.size());

            for (size_t i = 0u; i < events.size(); ++i)
            {
                EXPECT_EQ(expectedEvts[i].type, events[i].type) << " Event #" << i;
                EXPECT_EQ(expectedEvts[i].sceneId, events[i].sceneId) << " Event #" << i;
                EXPECT_EQ(expectedEvts[i].state, events[i].state) << " Event #" << i;
            }
        }

        void expectPublishedEvent()
        {
            expectEvents({ { RendererSceneControlLogic::Event::Type::ScenePublished, sceneId, RendererSceneState::Unavailable } });
        }

        void publishAndExpectToGetToState(RendererSceneState state)
        {
            // output expectations
            if (state != RendererSceneState::Unavailable)
            {
                EXPECT_CALL(m_logicOutput, handleSceneSubscriptionRequest(sceneId));
                if (state != RendererSceneState::Available)
                {
                    EXPECT_CALL(m_logicOutput, handleSceneMappingRequest(sceneId, displayId));
                    EXPECT_CALL(m_logicOutput, handleSceneDisplayBufferAssignmentRequest(sceneId, OffscreenBufferHandle{}, sceneRenderOrder));
                    if (state != RendererSceneState::Ready)
                        EXPECT_CALL(m_logicOutput, handleSceneShowRequest(sceneId));
                }
            }

            // simulate response for each step
            m_logic.scenePublished(sceneId);
            if (state != RendererSceneState::Unavailable)
            {
                m_logic.sceneSubscribed(sceneId, RendererSceneControlLogic::EventResult::OK);
                if (state != RendererSceneState::Available)
                {
                    m_logic.sceneMapped(sceneId, RendererSceneControlLogic::EventResult::OK);
                    if (state != RendererSceneState::Ready)
                        m_logic.sceneShown(sceneId, RendererSceneControlLogic::EventResult::OK);
                }
            }

            // events expectations
            RendererSceneControlLogic::Events expectedEvents;
            expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::ScenePublished, sceneId, RendererSceneState::Unavailable });
            if (state != RendererSceneState::Unavailable)
            {
                expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::SceneStateChanged, sceneId, RendererSceneState::Available });
                if (state != RendererSceneState::Available)
                {
                    expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::SceneStateChanged, sceneId, RendererSceneState::Ready });
                    if (state != RendererSceneState::Ready)
                        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::SceneStateChanged, sceneId, RendererSceneState::Rendered });
                }
            }
            expectEvents(expectedEvents);
        }

        void unpublish(ERendererCommand lastSuccessfulCommand)
        {
            RendererSceneControlLogic::Events expectedEvents;
            // these are events that internal renderer would send on unpublish depending on current state
            switch (lastSuccessfulCommand)
            {
            case ERendererCommand_ShowScene:
                expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::SceneStateChanged, sceneId, RendererSceneState::Ready });
                m_logic.sceneHidden(sceneId, RendererSceneControlLogic::EventResult::Indirect);
                RFALLTHROUGH;
            case ERendererCommand_MapSceneToDisplay:
            case ERendererCommand_HideScene:
                expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::SceneStateChanged, sceneId, RendererSceneState::Available });
                m_logic.sceneUnmapped(sceneId, RendererSceneControlLogic::EventResult::Indirect);
                RFALLTHROUGH;
            case ERendererCommand_SubscribeScene:
            case ERendererCommand_UnmapSceneFromDisplays:
                expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::SceneStateChanged, sceneId, RendererSceneState::Unavailable });
                m_logic.sceneUnsubscribed(sceneId, RendererSceneControlLogic::EventResult::Indirect);
                RFALLTHROUGH;
            case ERendererCommand_PublishedScene:
            case ERendererCommand_UnsubscribeScene:
                m_logic.sceneUnpublished(sceneId);
                RFALLTHROUGH;
            case ERendererCommand_UnpublishedScene:
                break;
            default:
                assert(false && "invalid starting state");
            }
            expectEvents(expectedEvents);
        }

        void doAnotherFullCycleFromUnpublishToState(ERendererCommand lastSuccessfulCommand = ERendererCommand_ShowScene, RendererSceneState state = RendererSceneState::Rendered)
        {
            unpublish(lastSuccessfulCommand);
            publishAndExpectToGetToState(state);
        }

        void doAnotherFullCycleFromUnpublishToShownWithShowTriggered(ERendererCommand lastSuccessfulCommand = ERendererCommand_ShowScene)
        {
            unpublish(lastSuccessfulCommand);
            m_logic.setSceneState(sceneId, RendererSceneState::Rendered);
            publishAndExpectToGetToState(RendererSceneState::Rendered);
        }

    protected:
        StrictMock<RendererSceneControlMock>      m_logicOutput;
        RendererSceneControlLogic                 m_logic;

        const SceneId sceneId{ 33 };
        const DisplayHandle displayId{ 1u };
        const DisplayHandle otherDisplayId{ 3u };
        const OffscreenBufferHandle offscreenBufferId{ 19u };
        const int sceneRenderOrder{ -3 };
    };

    TEST_F(ARendererSceneControlLogic, willShowSceneWhenPublished)
    {
        m_logic.setSceneState(sceneId, RendererSceneState::Rendered);
        publishAndExpectToGetToState(RendererSceneState::Rendered);

        doAnotherFullCycleFromUnpublishToState();
        doAnotherFullCycleFromUnpublishToShownWithShowTriggered();
    }

    TEST_F(ARendererSceneControlLogic, willShowSceneAlreadyPublished)
    {
        m_logic.scenePublished(sceneId);

        EXPECT_CALL(m_logicOutput, handleSceneSubscriptionRequest(sceneId));
        m_logic.setSceneState(sceneId, RendererSceneState::Rendered);
        EXPECT_CALL(m_logicOutput, handleSceneMappingRequest(sceneId, displayId));
        m_logic.sceneSubscribed(sceneId, RendererSceneControlLogic::EventResult::OK);
        EXPECT_CALL(m_logicOutput, handleSceneDisplayBufferAssignmentRequest(sceneId, OffscreenBufferHandle{}, sceneRenderOrder));
        EXPECT_CALL(m_logicOutput, handleSceneShowRequest(sceneId));
        m_logic.sceneMapped(sceneId, RendererSceneControlLogic::EventResult::OK);
        m_logic.sceneShown(sceneId, RendererSceneControlLogic::EventResult::OK);

        RendererSceneControlLogic::Events expectedEvents;
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::ScenePublished, sceneId, RendererSceneState::Unavailable });
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::SceneStateChanged, sceneId, RendererSceneState::Available });
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::SceneStateChanged, sceneId, RendererSceneState::Ready });
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::SceneStateChanged, sceneId, RendererSceneState::Rendered });
        expectEvents(expectedEvents);

        doAnotherFullCycleFromUnpublishToState();
        doAnotherFullCycleFromUnpublishToShownWithShowTriggered();
    }

    TEST_F(ARendererSceneControlLogic, willRetryIfSubscribeFailed)
    {
        m_logic.scenePublished(sceneId);
        EXPECT_CALL(m_logicOutput, handleSceneSubscriptionRequest(sceneId));
        m_logic.setSceneState(sceneId, RendererSceneState::Rendered);

        EXPECT_CALL(m_logicOutput, handleSceneSubscriptionRequest(sceneId));
        m_logic.sceneSubscribed(sceneId, RendererSceneControlLogic::EventResult::Failed);

        EXPECT_CALL(m_logicOutput, handleSceneMappingRequest(sceneId, displayId));
        m_logic.sceneSubscribed(sceneId, RendererSceneControlLogic::EventResult::OK);
        EXPECT_CALL(m_logicOutput, handleSceneDisplayBufferAssignmentRequest(sceneId, OffscreenBufferHandle{}, sceneRenderOrder));
        EXPECT_CALL(m_logicOutput, handleSceneShowRequest(sceneId));
        m_logic.sceneMapped(sceneId, RendererSceneControlLogic::EventResult::OK);
        m_logic.sceneShown(sceneId, RendererSceneControlLogic::EventResult::OK);

        RendererSceneControlLogic::Events expectedEvents;
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::ScenePublished, sceneId, RendererSceneState::Unavailable });
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::SceneStateChanged, sceneId, RendererSceneState::Available });
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::SceneStateChanged, sceneId, RendererSceneState::Ready });
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::SceneStateChanged, sceneId, RendererSceneState::Rendered });
        expectEvents(expectedEvents);

        doAnotherFullCycleFromUnpublishToState();
        doAnotherFullCycleFromUnpublishToShownWithShowTriggered();
    }

    TEST_F(ARendererSceneControlLogic, willRetryIfMapFailed)
    {
        m_logic.scenePublished(sceneId);
        EXPECT_CALL(m_logicOutput, handleSceneSubscriptionRequest(sceneId));
        m_logic.setSceneState(sceneId, RendererSceneState::Rendered);
        EXPECT_CALL(m_logicOutput, handleSceneMappingRequest(sceneId, displayId));
        m_logic.sceneSubscribed(sceneId, RendererSceneControlLogic::EventResult::OK);

        EXPECT_CALL(m_logicOutput, handleSceneMappingRequest(sceneId, displayId));
        m_logic.sceneMapped(sceneId, RendererSceneControlLogic::EventResult::Failed);

        EXPECT_CALL(m_logicOutput, handleSceneDisplayBufferAssignmentRequest(sceneId, OffscreenBufferHandle{}, sceneRenderOrder));
        EXPECT_CALL(m_logicOutput, handleSceneShowRequest(sceneId));
        m_logic.sceneMapped(sceneId, RendererSceneControlLogic::EventResult::OK);
        m_logic.sceneShown(sceneId, RendererSceneControlLogic::EventResult::OK);

        RendererSceneControlLogic::Events expectedEvents;
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::ScenePublished, sceneId, RendererSceneState::Unavailable });
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::SceneStateChanged, sceneId, RendererSceneState::Available });
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::SceneStateChanged, sceneId, RendererSceneState::Ready });
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::SceneStateChanged, sceneId, RendererSceneState::Rendered });
        expectEvents(expectedEvents);

        doAnotherFullCycleFromUnpublishToState();
        doAnotherFullCycleFromUnpublishToShownWithShowTriggered();
    }

    TEST_F(ARendererSceneControlLogic, willRetryIfShowFailed)
    {
        m_logic.scenePublished(sceneId);
        EXPECT_CALL(m_logicOutput, handleSceneSubscriptionRequest(sceneId));
        m_logic.setSceneState(sceneId, RendererSceneState::Rendered);
        EXPECT_CALL(m_logicOutput, handleSceneMappingRequest(sceneId, displayId));
        m_logic.sceneSubscribed(sceneId, RendererSceneControlLogic::EventResult::OK);
        EXPECT_CALL(m_logicOutput, handleSceneDisplayBufferAssignmentRequest(sceneId, OffscreenBufferHandle{}, sceneRenderOrder));
        EXPECT_CALL(m_logicOutput, handleSceneShowRequest(sceneId));
        m_logic.sceneMapped(sceneId, RendererSceneControlLogic::EventResult::OK);

        EXPECT_CALL(m_logicOutput, handleSceneShowRequest(sceneId));
        m_logic.sceneShown(sceneId, RendererSceneControlLogic::EventResult::Failed);

        m_logic.sceneShown(sceneId, RendererSceneControlLogic::EventResult::OK);

        RendererSceneControlLogic::Events expectedEvents;
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::ScenePublished, sceneId, RendererSceneState::Unavailable });
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::SceneStateChanged, sceneId, RendererSceneState::Available });
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::SceneStateChanged, sceneId, RendererSceneState::Ready });
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::SceneStateChanged, sceneId, RendererSceneState::Rendered });
        expectEvents(expectedEvents);

        doAnotherFullCycleFromUnpublishToState();
        doAnotherFullCycleFromUnpublishToShownWithShowTriggered();
    }

    TEST_F(ARendererSceneControlLogic, willRetryIfHideFailed)
    {
        m_logic.scenePublished(sceneId);

        EXPECT_CALL(m_logicOutput, handleSceneSubscriptionRequest(sceneId));
        m_logic.setSceneState(sceneId, RendererSceneState::Rendered);
        EXPECT_CALL(m_logicOutput, handleSceneMappingRequest(sceneId, displayId));
        m_logic.sceneSubscribed(sceneId, RendererSceneControlLogic::EventResult::OK);
        EXPECT_CALL(m_logicOutput, handleSceneDisplayBufferAssignmentRequest(sceneId, OffscreenBufferHandle{}, sceneRenderOrder));
        EXPECT_CALL(m_logicOutput, handleSceneShowRequest(sceneId));
        m_logic.sceneMapped(sceneId, RendererSceneControlLogic::EventResult::OK);
        m_logic.sceneShown(sceneId, RendererSceneControlLogic::EventResult::OK);

        RendererSceneControlLogic::Events expectedEvents;
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::ScenePublished, sceneId, RendererSceneState::Unavailable });
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::SceneStateChanged, sceneId, RendererSceneState::Available });
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::SceneStateChanged, sceneId, RendererSceneState::Ready });
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::SceneStateChanged, sceneId, RendererSceneState::Rendered });
        expectEvents(expectedEvents);

        EXPECT_CALL(m_logicOutput, handleSceneHideRequest(sceneId));
        m_logic.setSceneState(sceneId, RendererSceneState::Ready);
        EXPECT_CALL(m_logicOutput, handleSceneHideRequest(sceneId));
        m_logic.sceneHidden(sceneId, RendererSceneControlLogic::EventResult::Failed);

        m_logic.sceneHidden(sceneId, RendererSceneControlLogic::EventResult::OK);
        expectEvents({ { RendererSceneControlLogic::Event::Type::SceneStateChanged, sceneId, RendererSceneState::Ready } });

        doAnotherFullCycleFromUnpublishToState(ERendererCommand_HideScene, RendererSceneState::Ready);
        doAnotherFullCycleFromUnpublishToShownWithShowTriggered(ERendererCommand_HideScene);
    }

    TEST_F(ARendererSceneControlLogic, willRetryIfUnmapFailed)
    {
        m_logic.scenePublished(sceneId);

        EXPECT_CALL(m_logicOutput, handleSceneSubscriptionRequest(sceneId));
        m_logic.setSceneState(sceneId, RendererSceneState::Rendered);
        EXPECT_CALL(m_logicOutput, handleSceneMappingRequest(sceneId, displayId));
        m_logic.sceneSubscribed(sceneId, RendererSceneControlLogic::EventResult::OK);
        EXPECT_CALL(m_logicOutput, handleSceneDisplayBufferAssignmentRequest(sceneId, OffscreenBufferHandle{}, sceneRenderOrder));
        EXPECT_CALL(m_logicOutput, handleSceneShowRequest(sceneId));
        m_logic.sceneMapped(sceneId, RendererSceneControlLogic::EventResult::OK);
        m_logic.sceneShown(sceneId, RendererSceneControlLogic::EventResult::OK);

        RendererSceneControlLogic::Events expectedEvents;
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::ScenePublished, sceneId, RendererSceneState::Unavailable });
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::SceneStateChanged, sceneId, RendererSceneState::Available });
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::SceneStateChanged, sceneId, RendererSceneState::Ready });
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::SceneStateChanged, sceneId, RendererSceneState::Rendered });
        expectEvents(expectedEvents);

        EXPECT_CALL(m_logicOutput, handleSceneHideRequest(sceneId));
        m_logic.setSceneState(sceneId, RendererSceneState::Available);
        EXPECT_CALL(m_logicOutput, handleSceneUnmappingRequest(sceneId));
        m_logic.sceneHidden(sceneId, RendererSceneControlLogic::EventResult::OK);
        EXPECT_CALL(m_logicOutput, handleSceneUnmappingRequest(sceneId));
        m_logic.sceneUnmapped(sceneId, RendererSceneControlLogic::EventResult::Failed);

        m_logic.sceneUnmapped(sceneId, RendererSceneControlLogic::EventResult::OK);

        expectedEvents.clear();
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::SceneStateChanged, sceneId, RendererSceneState::Ready });
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::SceneStateChanged, sceneId, RendererSceneState::Available });
        expectEvents(expectedEvents);

        doAnotherFullCycleFromUnpublishToState(ERendererCommand_UnmapSceneFromDisplays, RendererSceneState::Available);
        doAnotherFullCycleFromUnpublishToShownWithShowTriggered(ERendererCommand_UnmapSceneFromDisplays);
    }

    TEST_F(ARendererSceneControlLogic, willRetryIfUnsubscribeFailed)
    {
        m_logic.scenePublished(sceneId);

        EXPECT_CALL(m_logicOutput, handleSceneSubscriptionRequest(sceneId));
        m_logic.setSceneState(sceneId, RendererSceneState::Rendered);
        EXPECT_CALL(m_logicOutput, handleSceneMappingRequest(sceneId, displayId));
        m_logic.sceneSubscribed(sceneId, RendererSceneControlLogic::EventResult::OK);
        EXPECT_CALL(m_logicOutput, handleSceneDisplayBufferAssignmentRequest(sceneId, OffscreenBufferHandle{}, sceneRenderOrder));
        EXPECT_CALL(m_logicOutput, handleSceneShowRequest(sceneId));
        m_logic.sceneMapped(sceneId, RendererSceneControlLogic::EventResult::OK);
        m_logic.sceneShown(sceneId, RendererSceneControlLogic::EventResult::OK);

        RendererSceneControlLogic::Events expectedEvents;
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::ScenePublished, sceneId, RendererSceneState::Unavailable });
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::SceneStateChanged, sceneId, RendererSceneState::Available });
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::SceneStateChanged, sceneId, RendererSceneState::Ready });
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::SceneStateChanged, sceneId, RendererSceneState::Rendered });
        expectEvents(expectedEvents);

        EXPECT_CALL(m_logicOutput, handleSceneHideRequest(sceneId));
        m_logic.setSceneState(sceneId, RendererSceneState::Unavailable);
        EXPECT_CALL(m_logicOutput, handleSceneUnmappingRequest(sceneId));
        m_logic.sceneHidden(sceneId, RendererSceneControlLogic::EventResult::OK);
        EXPECT_CALL(m_logicOutput, handleSceneUnsubscriptionRequest(sceneId, false));
        m_logic.sceneUnmapped(sceneId, RendererSceneControlLogic::EventResult::OK);
        EXPECT_CALL(m_logicOutput, handleSceneUnsubscriptionRequest(sceneId, false));
        m_logic.sceneUnsubscribed(sceneId, RendererSceneControlLogic::EventResult::Failed);

        m_logic.sceneUnsubscribed(sceneId, RendererSceneControlLogic::EventResult::OK);

        expectedEvents.clear();
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::SceneStateChanged, sceneId, RendererSceneState::Ready });
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::SceneStateChanged, sceneId, RendererSceneState::Available });
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::SceneStateChanged, sceneId, RendererSceneState::Unavailable });
        expectEvents(expectedEvents);

        doAnotherFullCycleFromUnpublishToState(ERendererCommand_UnsubscribeScene, RendererSceneState::Unavailable);
        doAnotherFullCycleFromUnpublishToShownWithShowTriggered(ERendererCommand_UnsubscribeScene);
    }

    TEST_F(ARendererSceneControlLogic, hidesShownScene)
    {
        m_logic.setSceneState(sceneId, RendererSceneState::Rendered);
        publishAndExpectToGetToState(RendererSceneState::Rendered);

        EXPECT_CALL(m_logicOutput, handleSceneHideRequest(sceneId));
        m_logic.setSceneState(sceneId, RendererSceneState::Ready);
        m_logic.sceneHidden(sceneId, RendererSceneControlLogic::EventResult::OK);

        expectEvents({ { RendererSceneControlLogic::Event::Type::SceneStateChanged, sceneId, RendererSceneState::Ready } });

        doAnotherFullCycleFromUnpublishToState(ERendererCommand_HideScene, RendererSceneState::Ready);
        doAnotherFullCycleFromUnpublishToShownWithShowTriggered(ERendererCommand_HideScene);
    }

    TEST_F(ARendererSceneControlLogic, unmapsShownScene)
    {
        m_logic.setSceneState(sceneId, RendererSceneState::Rendered);
        publishAndExpectToGetToState(RendererSceneState::Rendered);

        EXPECT_CALL(m_logicOutput, handleSceneHideRequest(sceneId));
        m_logic.setSceneState(sceneId, RendererSceneState::Available);
        EXPECT_CALL(m_logicOutput, handleSceneUnmappingRequest(sceneId));
        m_logic.sceneHidden(sceneId, RendererSceneControlLogic::EventResult::OK);
        m_logic.sceneUnmapped(sceneId, RendererSceneControlLogic::EventResult::OK);

        RendererSceneControlLogic::Events expectedEvents;
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::SceneStateChanged, sceneId, RendererSceneState::Ready });
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::SceneStateChanged, sceneId, RendererSceneState::Available });
        expectEvents(expectedEvents);

        doAnotherFullCycleFromUnpublishToState(ERendererCommand_UnmapSceneFromDisplays, RendererSceneState::Available);
        doAnotherFullCycleFromUnpublishToShownWithShowTriggered(ERendererCommand_UnmapSceneFromDisplays);
    }

    TEST_F(ARendererSceneControlLogic, unmapsMappedScene)
    {
        m_logic.setSceneState(sceneId, RendererSceneState::Ready);
        publishAndExpectToGetToState(RendererSceneState::Ready);

        EXPECT_CALL(m_logicOutput, handleSceneUnmappingRequest(sceneId));
        m_logic.setSceneState(sceneId, RendererSceneState::Available);
        m_logic.sceneUnmapped(sceneId, RendererSceneControlLogic::EventResult::OK);

        expectEvents({ { RendererSceneControlLogic::Event::Type::SceneStateChanged, sceneId, RendererSceneState::Available } });

        doAnotherFullCycleFromUnpublishToState(ERendererCommand_UnmapSceneFromDisplays, RendererSceneState::Available);
        doAnotherFullCycleFromUnpublishToShownWithShowTriggered(ERendererCommand_UnmapSceneFromDisplays);
    }

    TEST_F(ARendererSceneControlLogic, unsubscribesShownScene)
    {
        m_logic.setSceneState(sceneId, RendererSceneState::Rendered);
        publishAndExpectToGetToState(RendererSceneState::Rendered);

        EXPECT_CALL(m_logicOutput, handleSceneHideRequest(sceneId));
        m_logic.setSceneState(sceneId, RendererSceneState::Unavailable);
        EXPECT_CALL(m_logicOutput, handleSceneUnmappingRequest(sceneId));
        m_logic.sceneHidden(sceneId, RendererSceneControlLogic::EventResult::OK);
        EXPECT_CALL(m_logicOutput, handleSceneUnsubscriptionRequest(sceneId, false));
        m_logic.sceneUnmapped(sceneId, RendererSceneControlLogic::EventResult::OK);
        m_logic.sceneUnsubscribed(sceneId, RendererSceneControlLogic::EventResult::OK);

        RendererSceneControlLogic::Events expectedEvents;
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::SceneStateChanged, sceneId, RendererSceneState::Ready });
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::SceneStateChanged, sceneId, RendererSceneState::Available });
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::SceneStateChanged, sceneId, RendererSceneState::Unavailable });
        expectEvents(expectedEvents);

        doAnotherFullCycleFromUnpublishToState(ERendererCommand_UnsubscribeScene, RendererSceneState::Unavailable);
        doAnotherFullCycleFromUnpublishToShownWithShowTriggered(ERendererCommand_UnsubscribeScene);
    }

    TEST_F(ARendererSceneControlLogic, unsubscribesMappedScene)
    {
        m_logic.setSceneState(sceneId, RendererSceneState::Ready);
        publishAndExpectToGetToState(RendererSceneState::Ready);

        EXPECT_CALL(m_logicOutput, handleSceneUnmappingRequest(sceneId));
        m_logic.setSceneState(sceneId, RendererSceneState::Unavailable);
        EXPECT_CALL(m_logicOutput, handleSceneUnsubscriptionRequest(sceneId, false));
        m_logic.sceneUnmapped(sceneId, RendererSceneControlLogic::EventResult::OK);
        m_logic.sceneUnsubscribed(sceneId, RendererSceneControlLogic::EventResult::OK);

        RendererSceneControlLogic::Events expectedEvents;
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::SceneStateChanged, sceneId, RendererSceneState::Available });
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::SceneStateChanged, sceneId, RendererSceneState::Unavailable });
        expectEvents(expectedEvents);

        doAnotherFullCycleFromUnpublishToState(ERendererCommand_UnsubscribeScene, RendererSceneState::Unavailable);
        doAnotherFullCycleFromUnpublishToShownWithShowTriggered(ERendererCommand_UnsubscribeScene);
    }

    TEST_F(ARendererSceneControlLogic, unsubscribesSubscribedScene)
    {
        m_logic.setSceneState(sceneId, RendererSceneState::Available);
        publishAndExpectToGetToState(RendererSceneState::Available);

        EXPECT_CALL(m_logicOutput, handleSceneUnsubscriptionRequest(sceneId, false));
        m_logic.setSceneState(sceneId, RendererSceneState::Unavailable);
        m_logic.sceneUnsubscribed(sceneId, RendererSceneControlLogic::EventResult::OK);

        RendererSceneControlLogic::Events expectedEvents;
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::SceneStateChanged, sceneId, RendererSceneState::Unavailable });
        expectEvents(expectedEvents);

        doAnotherFullCycleFromUnpublishToState(ERendererCommand_UnsubscribeScene, RendererSceneState::Unavailable);
        doAnotherFullCycleFromUnpublishToShownWithShowTriggered(ERendererCommand_UnsubscribeScene);
    }

    TEST_F(ARendererSceneControlLogic, canChangeMappingPropertiesWhenSceneAvailableAndNotSetToReadyOrRenderedYet)
    {
        m_logic.setSceneState(sceneId, RendererSceneState::Available);
        publishAndExpectToGetToState(RendererSceneState::Available);

        m_logic.setSceneMapping(sceneId, otherDisplayId);
        m_logic.setSceneDisplayBufferAssignment(sceneId, OffscreenBufferHandle{}, 666);

        // will map scene to other display
        EXPECT_CALL(m_logicOutput, handleSceneMappingRequest(sceneId, otherDisplayId));
        m_logic.setSceneState(sceneId, RendererSceneState::Ready);
        // will assign scene to other display's framebuffer with given render order
        EXPECT_CALL(m_logicOutput, handleSceneDisplayBufferAssignmentRequest(sceneId, OffscreenBufferHandle{}, 666));
        m_logic.sceneMapped(sceneId, RendererSceneControlLogic::EventResult::OK);

        RendererSceneControlLogic::Events expectedEvents;
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::SceneStateChanged, sceneId, RendererSceneState::Ready });
        expectEvents(expectedEvents);
    }

    TEST_F(ARendererSceneControlLogic, canChangeMappingPropertiesForReadySceneAfterGettingItToAvailableFirst)
    {
        m_logic.setSceneState(sceneId, RendererSceneState::Ready);
        publishAndExpectToGetToState(RendererSceneState::Ready);

        EXPECT_CALL(m_logicOutput, handleSceneUnmappingRequest(sceneId));
        m_logic.setSceneState(sceneId, RendererSceneState::Available);
        m_logic.sceneUnmapped(sceneId, RendererSceneControlLogic::EventResult::OK);

        RendererSceneControlLogic::Events expectedEvents;
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::SceneStateChanged, sceneId, RendererSceneState::Available });
        expectEvents(expectedEvents);

        // now can change mapping
        m_logic.setSceneMapping(sceneId, otherDisplayId);
        m_logic.setSceneDisplayBufferAssignment(sceneId, OffscreenBufferHandle{}, 666);

        EXPECT_CALL(m_logicOutput, handleSceneMappingRequest(sceneId, otherDisplayId));
        m_logic.setSceneState(sceneId, RendererSceneState::Ready);
        EXPECT_CALL(m_logicOutput, handleSceneDisplayBufferAssignmentRequest(sceneId, OffscreenBufferHandle{}, 666));
        m_logic.sceneMapped(sceneId, RendererSceneControlLogic::EventResult::OK);

        expectedEvents.clear();
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::SceneStateChanged, sceneId, RendererSceneState::Ready });
        expectEvents(expectedEvents);
    }

    /////////
    // Tests where scene is unpublished during a state transition, state transition fail is reported back
    // and scene gets re-published.
    // Expectation is to reach original target state.
    /////////

    TEST_F(ARendererSceneControlLogic, handlesSceneUnpublishWhileProcessingSubscribe)
    {
        m_logic.setSceneState(sceneId, RendererSceneState::Available);
        EXPECT_CALL(m_logicOutput, handleSceneSubscriptionRequest(sceneId));
        m_logic.scenePublished(sceneId);
        expectPublishedEvent();

        unpublish(ERendererCommand_PublishedScene);
        // renderer guarantees to send a response for every command
        // - here we simulate that unpublish came BEFORE the subscribe execution -> therefore it fails for unpublished scene
        m_logic.sceneSubscribed(sceneId, RendererSceneControlLogic::EventResult::Failed);

        // make sure target state can be reached once published again
        publishAndExpectToGetToState(RendererSceneState::Available);
        doAnotherFullCycleFromUnpublishToState(ERendererCommand_SubscribeScene, RendererSceneState::Available);
        doAnotherFullCycleFromUnpublishToShownWithShowTriggered(ERendererCommand_SubscribeScene);
    }

    TEST_F(ARendererSceneControlLogic, handlesSceneUnpublishWhileProcessingMap)
    {
        m_logic.setSceneState(sceneId, RendererSceneState::Ready);
        EXPECT_CALL(m_logicOutput, handleSceneSubscriptionRequest(sceneId));
        m_logic.scenePublished(sceneId);
        EXPECT_CALL(m_logicOutput, handleSceneMappingRequest(sceneId, displayId));
        m_logic.sceneSubscribed(sceneId, RendererSceneControlLogic::EventResult::OK);

        RendererSceneControlLogic::Events expectedEvents;
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::ScenePublished, sceneId, RendererSceneState::Unavailable });
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::SceneStateChanged, sceneId, RendererSceneState::Available });
        expectEvents(expectedEvents);

        unpublish(ERendererCommand_SubscribeScene);
        // renderer guarantees to send a response for every command
        // - here we simulate that unpublish came BEFORE the map execution -> therefore it fails for unpublished scene
        m_logic.sceneMapped(sceneId, RendererSceneControlLogic::EventResult::Failed);

        // make sure target state can be reached once published again
        publishAndExpectToGetToState(RendererSceneState::Ready);
        doAnotherFullCycleFromUnpublishToState(ERendererCommand_MapSceneToDisplay, RendererSceneState::Ready);
        doAnotherFullCycleFromUnpublishToShownWithShowTriggered(ERendererCommand_MapSceneToDisplay);
    }

    TEST_F(ARendererSceneControlLogic, handlesSceneUnpublishWhileProcessingRender)
    {
        m_logic.setSceneState(sceneId, RendererSceneState::Rendered);
        EXPECT_CALL(m_logicOutput, handleSceneSubscriptionRequest(sceneId));
        m_logic.scenePublished(sceneId);
        EXPECT_CALL(m_logicOutput, handleSceneMappingRequest(sceneId, displayId));
        m_logic.sceneSubscribed(sceneId, RendererSceneControlLogic::EventResult::OK);
        EXPECT_CALL(m_logicOutput, handleSceneDisplayBufferAssignmentRequest(sceneId, OffscreenBufferHandle{}, sceneRenderOrder));
        EXPECT_CALL(m_logicOutput, handleSceneShowRequest(sceneId));
        m_logic.sceneMapped(sceneId, RendererSceneControlLogic::EventResult::OK);

        RendererSceneControlLogic::Events expectedEvents;
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::ScenePublished, sceneId, RendererSceneState::Unavailable });
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::SceneStateChanged, sceneId, RendererSceneState::Available });
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::SceneStateChanged, sceneId, RendererSceneState::Ready });
        expectEvents(expectedEvents);

        unpublish(ERendererCommand_MapSceneToDisplay);
        // renderer guarantees to send a response for every command
        // - here we simulate that unpublish came BEFORE the render execution -> therefore it fails for unpublished scene
        m_logic.sceneShown(sceneId, RendererSceneControlLogic::EventResult::Failed);

        // make sure target state can be reached once published again
        publishAndExpectToGetToState(RendererSceneState::Rendered);
        doAnotherFullCycleFromUnpublishToState(ERendererCommand_MapSceneToDisplay, RendererSceneState::Rendered);
        doAnotherFullCycleFromUnpublishToShownWithShowTriggered(ERendererCommand_MapSceneToDisplay);
    }

    TEST_F(ARendererSceneControlLogic, handlesSceneUnpublishWhileProcessingUnsubscribe)
    {
        m_logic.setSceneState(sceneId, RendererSceneState::Available);
        publishAndExpectToGetToState(RendererSceneState::Available);

        EXPECT_CALL(m_logicOutput, handleSceneUnsubscriptionRequest(sceneId, false));
        m_logic.setSceneState(sceneId, RendererSceneState::Unavailable);

        unpublish(ERendererCommand_SubscribeScene);
        // renderer guarantees to send a response for every command
        // - here we simulate that unpublish came BEFORE the unsubscribe execution -> therefore it fails for unpublished scene
        m_logic.sceneUnsubscribed(sceneId, RendererSceneControlLogic::EventResult::Failed);

        // make sure target state can be reached once published again
        publishAndExpectToGetToState(RendererSceneState::Unavailable);
        doAnotherFullCycleFromUnpublishToState(ERendererCommand_PublishedScene, RendererSceneState::Unavailable);
        doAnotherFullCycleFromUnpublishToShownWithShowTriggered(ERendererCommand_PublishedScene);
    }

    TEST_F(ARendererSceneControlLogic, handlesSceneUnpublishWhileProcessingUnmap)
    {
        m_logic.setSceneState(sceneId, RendererSceneState::Ready);
        publishAndExpectToGetToState(RendererSceneState::Ready);

        EXPECT_CALL(m_logicOutput, handleSceneUnmappingRequest(sceneId));
        m_logic.setSceneState(sceneId, RendererSceneState::Available);

        unpublish(ERendererCommand_MapSceneToDisplay);
        // renderer guarantees to send a response for every command
        // - here we simulate that unpublish came BEFORE the unmap execution -> therefore it fails for unpublished scene
        m_logic.sceneUnmapped(sceneId, RendererSceneControlLogic::EventResult::Failed);

        // make sure target state can be reached once published again
        publishAndExpectToGetToState(RendererSceneState::Available);
        doAnotherFullCycleFromUnpublishToState(ERendererCommand_SubscribeScene, RendererSceneState::Available);
        doAnotherFullCycleFromUnpublishToShownWithShowTriggered(ERendererCommand_SubscribeScene);
    }

    TEST_F(ARendererSceneControlLogic, handlesSceneUnpublishWhileProcessingHide)
    {
        m_logic.setSceneState(sceneId, RendererSceneState::Rendered);
        publishAndExpectToGetToState(RendererSceneState::Rendered);

        EXPECT_CALL(m_logicOutput, handleSceneHideRequest(sceneId));
        m_logic.setSceneState(sceneId, RendererSceneState::Ready);

        unpublish(ERendererCommand_ShowScene);
        // renderer guarantees to send a response for every command
        // - here we simulate that unpublish came BEFORE the hide execution -> therefore it fails for unpublished scene
        m_logic.sceneHidden(sceneId, RendererSceneControlLogic::EventResult::Failed);

        // make sure target state can be reached once published again
        publishAndExpectToGetToState(RendererSceneState::Ready);
        doAnotherFullCycleFromUnpublishToState(ERendererCommand_MapSceneToDisplay, RendererSceneState::Ready);
        doAnotherFullCycleFromUnpublishToShownWithShowTriggered(ERendererCommand_MapSceneToDisplay);
    }

    /////////
    // Tests where scene is unpublished during a state transition and re-published right after unpublish.
    // State transition fail is reported back AFTER the re-publish.
    // Expectation it to reach original target state.
    /////////

    TEST_F(ARendererSceneControlLogic, recoversFromRepublishBeforeFailedSubscribe)
    {
        m_logic.setSceneState(sceneId, RendererSceneState::Available);

        EXPECT_CALL(m_logicOutput, handleSceneSubscriptionRequest(sceneId));
        m_logic.scenePublished(sceneId);
        expectPublishedEvent();

        // unpublish with all the rollback events ...
        unpublish(ERendererCommand_PublishedScene);
        // ... and re-publish right after
        m_logic.scenePublished(sceneId);
        // not trying to subscribe again because last subscribe not answered yet

        // renderer guarantees to send a response for every command
        EXPECT_CALL(m_logicOutput, handleSceneSubscriptionRequest(sceneId)); // now try again
        m_logic.sceneSubscribed(sceneId, RendererSceneControlLogic::EventResult::Failed);
        m_logic.sceneSubscribed(sceneId, RendererSceneControlLogic::EventResult::OK);

        RendererSceneControlLogic::Events expectedEvents;
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::ScenePublished, sceneId, RendererSceneState::Unavailable });
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::SceneStateChanged, sceneId, RendererSceneState::Available });
        expectEvents(expectedEvents);

        doAnotherFullCycleFromUnpublishToState(ERendererCommand_SubscribeScene, RendererSceneState::Available);
        doAnotherFullCycleFromUnpublishToShownWithShowTriggered(ERendererCommand_SubscribeScene);
    }

    TEST_F(ARendererSceneControlLogic, recoversFromRepublishBeforeFailedMap)
    {
        m_logic.setSceneState(sceneId, RendererSceneState::Ready);

        EXPECT_CALL(m_logicOutput, handleSceneSubscriptionRequest(sceneId));
        m_logic.scenePublished(sceneId);
        EXPECT_CALL(m_logicOutput, handleSceneMappingRequest(sceneId, displayId));
        m_logic.sceneSubscribed(sceneId, RendererSceneControlLogic::EventResult::OK);

        RendererSceneControlLogic::Events expectedEvents;
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::ScenePublished, sceneId, RendererSceneState::Unavailable });
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::SceneStateChanged, sceneId, RendererSceneState::Available });
        expectEvents(expectedEvents);

        // unpublish with all the rollback events ...
        unpublish(ERendererCommand_SubscribeScene);
        // ... and re-publish right after
        m_logic.scenePublished(sceneId);
        // not trying to map again because last map not answered yet

        // renderer guarantees to send a response for every command
        // will try again starting from subscription
        EXPECT_CALL(m_logicOutput, handleSceneSubscriptionRequest(sceneId));
        m_logic.sceneMapped(sceneId, RendererSceneControlLogic::EventResult::Failed);
        EXPECT_CALL(m_logicOutput, handleSceneMappingRequest(sceneId, displayId));
        m_logic.sceneSubscribed(sceneId, RendererSceneControlLogic::EventResult::OK);
        EXPECT_CALL(m_logicOutput, handleSceneDisplayBufferAssignmentRequest(sceneId, OffscreenBufferHandle{}, sceneRenderOrder));
        m_logic.sceneMapped(sceneId, RendererSceneControlLogic::EventResult::OK);

        expectedEvents.clear();
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::ScenePublished, sceneId, RendererSceneState::Unavailable });
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::SceneStateChanged, sceneId, RendererSceneState::Available });
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::SceneStateChanged, sceneId, RendererSceneState::Ready });
        expectEvents(expectedEvents);

        doAnotherFullCycleFromUnpublishToState(ERendererCommand_MapSceneToDisplay, RendererSceneState::Ready);
        doAnotherFullCycleFromUnpublishToShownWithShowTriggered(ERendererCommand_MapSceneToDisplay);
    }

    TEST_F(ARendererSceneControlLogic, recoversFromRepublishBeforeFailedShow)
    {
        m_logic.setSceneState(sceneId, RendererSceneState::Rendered);

        EXPECT_CALL(m_logicOutput, handleSceneSubscriptionRequest(sceneId));
        m_logic.scenePublished(sceneId);
        EXPECT_CALL(m_logicOutput, handleSceneMappingRequest(sceneId, displayId));
        m_logic.sceneSubscribed(sceneId, RendererSceneControlLogic::EventResult::OK);
        EXPECT_CALL(m_logicOutput, handleSceneDisplayBufferAssignmentRequest(sceneId, OffscreenBufferHandle{}, sceneRenderOrder));
        EXPECT_CALL(m_logicOutput, handleSceneShowRequest(sceneId));
        m_logic.sceneMapped(sceneId, RendererSceneControlLogic::EventResult::OK);

        RendererSceneControlLogic::Events expectedEvents;
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::ScenePublished, sceneId, RendererSceneState::Unavailable });
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::SceneStateChanged, sceneId, RendererSceneState::Available });
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::SceneStateChanged, sceneId, RendererSceneState::Ready });
        expectEvents(expectedEvents);

        // unpublish with all the rollback events ...
        unpublish(ERendererCommand_MapSceneToDisplay);
        // ... and re-publish right after
        m_logic.scenePublished(sceneId);
        // not trying to show again because last show not answered yet

        // renderer guarantees to send a response for every command
        // will try again starting from subscription
        EXPECT_CALL(m_logicOutput, handleSceneSubscriptionRequest(sceneId));
        m_logic.sceneShown(sceneId, RendererSceneControlLogic::EventResult::Failed);
        EXPECT_CALL(m_logicOutput, handleSceneMappingRequest(sceneId, displayId));
        m_logic.sceneSubscribed(sceneId, RendererSceneControlLogic::EventResult::OK);
        EXPECT_CALL(m_logicOutput, handleSceneDisplayBufferAssignmentRequest(sceneId, OffscreenBufferHandle{}, sceneRenderOrder));
        EXPECT_CALL(m_logicOutput, handleSceneShowRequest(sceneId));
        m_logic.sceneMapped(sceneId, RendererSceneControlLogic::EventResult::OK);
        m_logic.sceneShown(sceneId, RendererSceneControlLogic::EventResult::OK);

        expectedEvents.clear();
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::ScenePublished, sceneId, RendererSceneState::Unavailable });
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::SceneStateChanged, sceneId, RendererSceneState::Available });
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::SceneStateChanged, sceneId, RendererSceneState::Ready });
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::SceneStateChanged, sceneId, RendererSceneState::Rendered });
        expectEvents(expectedEvents);

        doAnotherFullCycleFromUnpublishToState(ERendererCommand_ShowScene, RendererSceneState::Rendered);
        doAnotherFullCycleFromUnpublishToShownWithShowTriggered(ERendererCommand_ShowScene);
    }

    TEST_F(ARendererSceneControlLogic, recoversFromRepublishBeforeFailedUnsubscribe)
    {
        m_logic.setSceneState(sceneId, RendererSceneState::Available);
        publishAndExpectToGetToState(RendererSceneState::Available);

        EXPECT_CALL(m_logicOutput, handleSceneUnsubscriptionRequest(sceneId, false));
        m_logic.setSceneState(sceneId, RendererSceneState::Unavailable);

        // unpublish with all the rollback events ...
        unpublish(ERendererCommand_SubscribeScene);
        // ... and re-publish right after
        m_logic.scenePublished(sceneId);
        // not trying to do anything yet because last unsubscribe not answered yet

        // renderer guarantees to send a response for every command
        m_logic.sceneUnsubscribed(sceneId, RendererSceneControlLogic::EventResult::Failed);
        // there is actually nothing to do - requested UNAVAILABLE state was reached

        RendererSceneControlLogic::Events expectedEvents;
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::ScenePublished, sceneId, RendererSceneState::Unavailable });
        expectEvents(expectedEvents);

        doAnotherFullCycleFromUnpublishToState(ERendererCommand_PublishedScene, RendererSceneState::Unavailable);
        doAnotherFullCycleFromUnpublishToShownWithShowTriggered(ERendererCommand_PublishedScene);
    }

    TEST_F(ARendererSceneControlLogic, recoversFromRepublishBeforeFailedUnmap)
    {
        m_logic.setSceneState(sceneId, RendererSceneState::Ready);
        publishAndExpectToGetToState(RendererSceneState::Ready);

        EXPECT_CALL(m_logicOutput, handleSceneUnmappingRequest(sceneId));
        m_logic.setSceneState(sceneId, RendererSceneState::Available);

        // unpublish with all the rollback events ...
        unpublish(ERendererCommand_MapSceneToDisplay);
        // ... and re-publish right after
        m_logic.scenePublished(sceneId);
        // not trying to do anything yet because last unmap not answered yet

        // renderer guarantees to send a response for every command
        // will try to reach last target state again starting from subscription
        EXPECT_CALL(m_logicOutput, handleSceneSubscriptionRequest(sceneId));
        m_logic.sceneUnmapped(sceneId, RendererSceneControlLogic::EventResult::Failed);
        m_logic.sceneSubscribed(sceneId, RendererSceneControlLogic::EventResult::OK);

        RendererSceneControlLogic::Events expectedEvents;
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::ScenePublished, sceneId, RendererSceneState::Unavailable });
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::SceneStateChanged, sceneId, RendererSceneState::Available });
        expectEvents(expectedEvents);

        doAnotherFullCycleFromUnpublishToState(ERendererCommand_SubscribeScene, RendererSceneState::Available);
        doAnotherFullCycleFromUnpublishToShownWithShowTriggered(ERendererCommand_SubscribeScene);
    }

    TEST_F(ARendererSceneControlLogic, recoversFromRepublishBeforeFailedHide)
    {
        m_logic.setSceneState(sceneId, RendererSceneState::Rendered);
        publishAndExpectToGetToState(RendererSceneState::Rendered);

        EXPECT_CALL(m_logicOutput, handleSceneHideRequest(sceneId));
        m_logic.setSceneState(sceneId, RendererSceneState::Ready);

        // unpublish with all the rollback events ...
        unpublish(ERendererCommand_ShowScene);
        // ... and re-publish right after
        m_logic.scenePublished(sceneId);
        // not trying to do anything yet because last hide not answered yet

        // renderer guarantees to send a response for every command
        // will try to reach last target state again starting from subscription
        EXPECT_CALL(m_logicOutput, handleSceneSubscriptionRequest(sceneId));
        m_logic.sceneHidden(sceneId, RendererSceneControlLogic::EventResult::Failed);
        EXPECT_CALL(m_logicOutput, handleSceneMappingRequest(sceneId, displayId));
        m_logic.sceneSubscribed(sceneId, RendererSceneControlLogic::EventResult::OK);
        EXPECT_CALL(m_logicOutput, handleSceneDisplayBufferAssignmentRequest(sceneId, OffscreenBufferHandle{}, sceneRenderOrder));
        m_logic.sceneMapped(sceneId, RendererSceneControlLogic::EventResult::OK);

        RendererSceneControlLogic::Events expectedEvents;
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::ScenePublished, sceneId, RendererSceneState::Unavailable });
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::SceneStateChanged, sceneId, RendererSceneState::Available });
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::SceneStateChanged, sceneId, RendererSceneState::Ready });
        expectEvents(expectedEvents);

        doAnotherFullCycleFromUnpublishToState(ERendererCommand_MapSceneToDisplay, RendererSceneState::Ready);
        doAnotherFullCycleFromUnpublishToShownWithShowTriggered(ERendererCommand_MapSceneToDisplay);
    }

    TEST_F(ARendererSceneControlLogic, canAssignSceneToBufferAfterMapped)
    {
        m_logic.setSceneState(sceneId, RendererSceneState::Ready);
        publishAndExpectToGetToState(RendererSceneState::Ready);

        EXPECT_CALL(m_logicOutput, handleSceneDisplayBufferAssignmentRequest(sceneId, offscreenBufferId, 11));
        m_logic.setSceneDisplayBufferAssignment(sceneId, offscreenBufferId, 11);
    }

    TEST_F(ARendererSceneControlLogic, canAssignSceneToBufferAfterMapCommandBeforeMappedEvent)
    {
        m_logic.setSceneState(sceneId, RendererSceneState::Ready);

        EXPECT_CALL(m_logicOutput, handleSceneSubscriptionRequest(sceneId));
        m_logic.scenePublished(sceneId);
        EXPECT_CALL(m_logicOutput, handleSceneMappingRequest(sceneId, displayId));
        m_logic.sceneSubscribed(sceneId, RendererSceneControlLogic::EventResult::OK);

        m_logic.setSceneDisplayBufferAssignment(sceneId, offscreenBufferId, 11);

        EXPECT_CALL(m_logicOutput, handleSceneDisplayBufferAssignmentRequest(sceneId, offscreenBufferId, 11));
        m_logic.sceneMapped(sceneId, RendererSceneControlLogic::EventResult::OK);

        RendererSceneControlLogic::Events expectedEvents;
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::ScenePublished, sceneId, RendererSceneState::Unavailable });
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::SceneStateChanged, sceneId, RendererSceneState::Available });
        expectedEvents.push_back({ RendererSceneControlLogic::Event::Type::SceneStateChanged, sceneId, RendererSceneState::Ready });
        expectEvents(expectedEvents);
    }

    TEST_F(ARendererSceneControlLogic, mappingSceneToAnotherDisplayResetsDisplayBufferAssignmentAndRenderOrder)
    {
        m_logic.setSceneState(sceneId, RendererSceneState::Ready);
        publishAndExpectToGetToState(RendererSceneState::Ready);

        // assign to OB + render order
        EXPECT_CALL(m_logicOutput, handleSceneDisplayBufferAssignmentRequest(sceneId, offscreenBufferId, 11));
        m_logic.setSceneDisplayBufferAssignment(sceneId, offscreenBufferId, 11);

        // unmap scene
        EXPECT_CALL(m_logicOutput, handleSceneUnmappingRequest(sceneId));
        m_logic.setSceneState(sceneId, RendererSceneState::Available);
        m_logic.sceneUnmapped(sceneId, RendererSceneControlLogic::EventResult::OK);

        expectEvents({ { RendererSceneControlLogic::Event::Type::SceneStateChanged, sceneId, RendererSceneState::Available } });

        // set mapping to other display
        m_logic.setSceneMapping(sceneId, otherDisplayId);

        // map scene
        EXPECT_CALL(m_logicOutput, handleSceneMappingRequest(sceneId, otherDisplayId));
        m_logic.setSceneState(sceneId, RendererSceneState::Ready);
        EXPECT_CALL(m_logicOutput, handleSceneDisplayBufferAssignmentRequest(sceneId, OffscreenBufferHandle{}, 0));
        m_logic.sceneMapped(sceneId, RendererSceneControlLogic::EventResult::OK);

        expectEvents({ { RendererSceneControlLogic::Event::Type::SceneStateChanged, sceneId, RendererSceneState::Ready } });
    }

    TEST_F(ARendererSceneControlLogic, getsInitialSceneInfoForUnknownScene)
    {
        constexpr SceneId someScene{ 666 };
        RendererSceneState state = RendererSceneState::Ready;
        DisplayHandle display{ 1 };
        OffscreenBufferHandle ob{ 2 };
        int32_t renderOrder = -1;

        m_logic.getSceneInfo(someScene, state, display, ob, renderOrder);
        EXPECT_EQ(RendererSceneState::Unavailable, state);
        EXPECT_EQ(DisplayHandle::Invalid(), display);
        EXPECT_EQ(OffscreenBufferHandle::Invalid(), ob);
        EXPECT_EQ(0, renderOrder);
    }

    TEST_F(ARendererSceneControlLogic, getsSceneInfo)
    {
        constexpr SceneId someScene{ 666 };
        RendererSceneState state;
        DisplayHandle display;
        OffscreenBufferHandle ob;
        int32_t renderOrder = -1;

        m_logic.setSceneState(someScene, RendererSceneState::Available);

        m_logic.getSceneInfo(someScene, state, display, ob, renderOrder);
        EXPECT_EQ(RendererSceneState::Available, state);
        EXPECT_EQ(DisplayHandle::Invalid(), display);
        EXPECT_EQ(OffscreenBufferHandle::Invalid(), ob);
        EXPECT_EQ(0, renderOrder);

        m_logic.setSceneMapping(someScene, otherDisplayId);
        m_logic.setSceneDisplayBufferAssignment(someScene, offscreenBufferId, -13);
        m_logic.setSceneState(someScene, RendererSceneState::Ready);

        m_logic.getSceneInfo(someScene, state, display, ob, renderOrder);
        EXPECT_EQ(RendererSceneState::Ready, state);
        EXPECT_EQ(otherDisplayId, display);
        EXPECT_EQ(offscreenBufferId, ob);
        EXPECT_EQ(-13, renderOrder);
    }
}
