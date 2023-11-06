//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gmock/gmock.h"
#include "RendererSceneStateControlMock.h"
#include "internal/RendererLib/RendererSceneControlLogic.h"

using namespace testing;

namespace ramses::internal
{
    class ARendererSceneControlLogic : public Test
    {
    public:
        ARendererSceneControlLogic()
            : m_logic(m_logicOutput)
        {
        }

        void SetUp() override
        {
            m_logic.setSceneDisplayBufferAssignment(sceneId, OffscreenBufferHandle{}, sceneRenderOrder);
        }

        void TearDown() override
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
                EXPECT_EQ(expectedEvts[i].sceneId, events[i].sceneId) << " Event #" << i;
                EXPECT_EQ(expectedEvts[i].state, events[i].state) << " Event #" << i;
            }
        }

        void expectPublishedEvent()
        {
            expectEvents({ { sceneId, RendererSceneState::Available } });
        }

        void publishAndExpectToGetToState(RendererSceneState state)
        {
            assert(state != RendererSceneState::Unavailable);

            // output expectations
            if (state != RendererSceneState::Available)
            {
                EXPECT_CALL(m_logicOutput, handleSceneSubscriptionRequest(sceneId));
                EXPECT_CALL(m_logicOutput, handleSceneMappingRequest(sceneId));
                EXPECT_CALL(m_logicOutput, handleSceneDisplayBufferAssignmentRequest(sceneId, OffscreenBufferHandle{}, sceneRenderOrder));
                if (state != RendererSceneState::Ready)
                    EXPECT_CALL(m_logicOutput, handleSceneShowRequest(sceneId));
            }

            // simulate response for each step
            m_logic.scenePublished(sceneId);
            if (state != RendererSceneState::Available)
            {
                m_logic.sceneSubscribed(sceneId, RendererSceneControlLogic::EventResult::OK);
                m_logic.sceneMapped(sceneId, RendererSceneControlLogic::EventResult::OK);
                if (state != RendererSceneState::Ready)
                    m_logic.sceneShown(sceneId, RendererSceneControlLogic::EventResult::OK);
            }

            // events expectations
            RendererSceneControlLogic::Events expectedEvents;
            expectedEvents.push_back({ sceneId, RendererSceneState::Available });
            if (state != RendererSceneState::Available)
            {
                expectedEvents.push_back({ sceneId, RendererSceneState::Ready });
                if (state != RendererSceneState::Ready)
                    expectedEvents.push_back({ sceneId, RendererSceneState::Rendered });
            }
            expectEvents(expectedEvents);
        }

        enum class LastSuccessfulCommand
        {
            Publish,
            Unpublish,
            Subscribe,
            Unsubscribe,
            Map,
            Unmap,
            Show,
            Hide
        };

        void unpublish(LastSuccessfulCommand lastSuccessfulCommand)
        {
            RendererSceneControlLogic::Events expectedEvents;
            // these are events that internal renderer would send on unpublish depending on current state
            switch (lastSuccessfulCommand)
            {
            case LastSuccessfulCommand::Show:
                expectedEvents.push_back({ sceneId, RendererSceneState::Ready });
                m_logic.sceneHidden(sceneId, RendererSceneControlLogic::EventResult::Indirect);
                RFALLTHROUGH;
            case LastSuccessfulCommand::Map:
            case LastSuccessfulCommand::Hide:
                expectedEvents.push_back({ sceneId, RendererSceneState::Available });
                m_logic.sceneUnmapped(sceneId, RendererSceneControlLogic::EventResult::Indirect);
                RFALLTHROUGH;
            case LastSuccessfulCommand::Subscribe:
            case LastSuccessfulCommand::Unmap:
                m_logic.sceneUnsubscribed(sceneId, RendererSceneControlLogic::EventResult::Indirect);
                RFALLTHROUGH;
            case LastSuccessfulCommand::Publish:
            case LastSuccessfulCommand::Unsubscribe:
                m_logic.sceneUnpublished(sceneId);
                RFALLTHROUGH;
            case LastSuccessfulCommand::Unpublish:
                expectedEvents.push_back({ sceneId, RendererSceneState::Unavailable });
                break;
            default:
                assert(false && "invalid starting state");
            }
            expectEvents(expectedEvents);
        }

        void doAnotherFullCycleFromUnpublishToShownWithShowTriggered(LastSuccessfulCommand lastSuccessfulCommand = LastSuccessfulCommand::Show)
        {
            unpublish(lastSuccessfulCommand);
            m_logic.setSceneState(sceneId, RendererSceneState::Rendered);
            publishAndExpectToGetToState(RendererSceneState::Rendered);
        }

    protected:
        StrictMock<RendererSceneStateControlMock> m_logicOutput;
        RendererSceneControlLogic                 m_logic;

        const SceneId sceneId{ 33 };
        const DisplayHandle displayId{ 1u };
        const OffscreenBufferHandle offscreenBufferId{ 19u };
        const int sceneRenderOrder{ -3 };
    };

    TEST_F(ARendererSceneControlLogic, willShowSceneWhenPublished)
    {
        m_logic.setSceneState(sceneId, RendererSceneState::Rendered);
        publishAndExpectToGetToState(RendererSceneState::Rendered);

        doAnotherFullCycleFromUnpublishToShownWithShowTriggered();
    }

    TEST_F(ARendererSceneControlLogic, willShowSceneAlreadyPublished)
    {
        m_logic.scenePublished(sceneId);

        EXPECT_CALL(m_logicOutput, handleSceneSubscriptionRequest(sceneId));
        m_logic.setSceneState(sceneId, RendererSceneState::Rendered);
        EXPECT_CALL(m_logicOutput, handleSceneMappingRequest(sceneId));
        m_logic.sceneSubscribed(sceneId, RendererSceneControlLogic::EventResult::OK);
        EXPECT_CALL(m_logicOutput, handleSceneDisplayBufferAssignmentRequest(sceneId, OffscreenBufferHandle{}, sceneRenderOrder));
        EXPECT_CALL(m_logicOutput, handleSceneShowRequest(sceneId));
        m_logic.sceneMapped(sceneId, RendererSceneControlLogic::EventResult::OK);
        m_logic.sceneShown(sceneId, RendererSceneControlLogic::EventResult::OK);

        RendererSceneControlLogic::Events expectedEvents;
        expectedEvents.push_back({ sceneId, RendererSceneState::Available });
        expectedEvents.push_back({ sceneId, RendererSceneState::Ready });
        expectedEvents.push_back({ sceneId, RendererSceneState::Rendered });
        expectEvents(expectedEvents);

        doAnotherFullCycleFromUnpublishToShownWithShowTriggered();
    }

    TEST_F(ARendererSceneControlLogic, doesNotAttemptToReachTargetStateAfterUnpublishAndRepublish)
    {
        m_logic.setSceneState(sceneId, RendererSceneState::Rendered);
        publishAndExpectToGetToState(RendererSceneState::Rendered);

        unpublish(LastSuccessfulCommand::Show);

        publishAndExpectToGetToState(RendererSceneState::Available);
    }

    TEST_F(ARendererSceneControlLogic, willRetryIfSubscribeFailed)
    {
        m_logic.scenePublished(sceneId);
        EXPECT_CALL(m_logicOutput, handleSceneSubscriptionRequest(sceneId));
        m_logic.setSceneState(sceneId, RendererSceneState::Rendered);

        EXPECT_CALL(m_logicOutput, handleSceneSubscriptionRequest(sceneId));
        m_logic.sceneSubscribed(sceneId, RendererSceneControlLogic::EventResult::Failed);

        EXPECT_CALL(m_logicOutput, handleSceneMappingRequest(sceneId));
        m_logic.sceneSubscribed(sceneId, RendererSceneControlLogic::EventResult::OK);
        EXPECT_CALL(m_logicOutput, handleSceneDisplayBufferAssignmentRequest(sceneId, OffscreenBufferHandle{}, sceneRenderOrder));
        EXPECT_CALL(m_logicOutput, handleSceneShowRequest(sceneId));
        m_logic.sceneMapped(sceneId, RendererSceneControlLogic::EventResult::OK);
        m_logic.sceneShown(sceneId, RendererSceneControlLogic::EventResult::OK);

        RendererSceneControlLogic::Events expectedEvents;
        expectedEvents.push_back({ sceneId, RendererSceneState::Available });
        expectedEvents.push_back({ sceneId, RendererSceneState::Ready });
        expectedEvents.push_back({ sceneId, RendererSceneState::Rendered });
        expectEvents(expectedEvents);

        doAnotherFullCycleFromUnpublishToShownWithShowTriggered();
    }

    TEST_F(ARendererSceneControlLogic, willRetryIfMapFailed)
    {
        m_logic.scenePublished(sceneId);
        EXPECT_CALL(m_logicOutput, handleSceneSubscriptionRequest(sceneId));
        m_logic.setSceneState(sceneId, RendererSceneState::Rendered);
        EXPECT_CALL(m_logicOutput, handleSceneMappingRequest(sceneId));
        m_logic.sceneSubscribed(sceneId, RendererSceneControlLogic::EventResult::OK);

        EXPECT_CALL(m_logicOutput, handleSceneMappingRequest(sceneId));
        m_logic.sceneMapped(sceneId, RendererSceneControlLogic::EventResult::Failed);

        EXPECT_CALL(m_logicOutput, handleSceneDisplayBufferAssignmentRequest(sceneId, OffscreenBufferHandle{}, sceneRenderOrder));
        EXPECT_CALL(m_logicOutput, handleSceneShowRequest(sceneId));
        m_logic.sceneMapped(sceneId, RendererSceneControlLogic::EventResult::OK);
        m_logic.sceneShown(sceneId, RendererSceneControlLogic::EventResult::OK);

        RendererSceneControlLogic::Events expectedEvents;
        expectedEvents.push_back({ sceneId, RendererSceneState::Available });
        expectedEvents.push_back({ sceneId, RendererSceneState::Ready });
        expectedEvents.push_back({ sceneId, RendererSceneState::Rendered });
        expectEvents(expectedEvents);

        doAnotherFullCycleFromUnpublishToShownWithShowTriggered();
    }

    TEST_F(ARendererSceneControlLogic, willRetryIfShowFailed)
    {
        m_logic.scenePublished(sceneId);
        EXPECT_CALL(m_logicOutput, handleSceneSubscriptionRequest(sceneId));
        m_logic.setSceneState(sceneId, RendererSceneState::Rendered);
        EXPECT_CALL(m_logicOutput, handleSceneMappingRequest(sceneId));
        m_logic.sceneSubscribed(sceneId, RendererSceneControlLogic::EventResult::OK);
        EXPECT_CALL(m_logicOutput, handleSceneDisplayBufferAssignmentRequest(sceneId, OffscreenBufferHandle{}, sceneRenderOrder));
        EXPECT_CALL(m_logicOutput, handleSceneShowRequest(sceneId));
        m_logic.sceneMapped(sceneId, RendererSceneControlLogic::EventResult::OK);

        EXPECT_CALL(m_logicOutput, handleSceneShowRequest(sceneId));
        m_logic.sceneShown(sceneId, RendererSceneControlLogic::EventResult::Failed);

        m_logic.sceneShown(sceneId, RendererSceneControlLogic::EventResult::OK);

        RendererSceneControlLogic::Events expectedEvents;
        expectedEvents.push_back({ sceneId, RendererSceneState::Available });
        expectedEvents.push_back({ sceneId, RendererSceneState::Ready });
        expectedEvents.push_back({ sceneId, RendererSceneState::Rendered });
        expectEvents(expectedEvents);

        doAnotherFullCycleFromUnpublishToShownWithShowTriggered();
    }

    TEST_F(ARendererSceneControlLogic, willRetryIfHideFailed)
    {
        m_logic.scenePublished(sceneId);

        EXPECT_CALL(m_logicOutput, handleSceneSubscriptionRequest(sceneId));
        m_logic.setSceneState(sceneId, RendererSceneState::Rendered);
        EXPECT_CALL(m_logicOutput, handleSceneMappingRequest(sceneId));
        m_logic.sceneSubscribed(sceneId, RendererSceneControlLogic::EventResult::OK);
        EXPECT_CALL(m_logicOutput, handleSceneDisplayBufferAssignmentRequest(sceneId, OffscreenBufferHandle{}, sceneRenderOrder));
        EXPECT_CALL(m_logicOutput, handleSceneShowRequest(sceneId));
        m_logic.sceneMapped(sceneId, RendererSceneControlLogic::EventResult::OK);
        m_logic.sceneShown(sceneId, RendererSceneControlLogic::EventResult::OK);

        RendererSceneControlLogic::Events expectedEvents;
        expectedEvents.push_back({ sceneId, RendererSceneState::Available });
        expectedEvents.push_back({ sceneId, RendererSceneState::Ready });
        expectedEvents.push_back({ sceneId, RendererSceneState::Rendered });
        expectEvents(expectedEvents);

        EXPECT_CALL(m_logicOutput, handleSceneHideRequest(sceneId));
        m_logic.setSceneState(sceneId, RendererSceneState::Ready);
        EXPECT_CALL(m_logicOutput, handleSceneHideRequest(sceneId));
        m_logic.sceneHidden(sceneId, RendererSceneControlLogic::EventResult::Failed);

        m_logic.sceneHidden(sceneId, RendererSceneControlLogic::EventResult::OK);
        expectEvents({ { sceneId, RendererSceneState::Ready } });

        doAnotherFullCycleFromUnpublishToShownWithShowTriggered(LastSuccessfulCommand::Hide);
    }

    TEST_F(ARendererSceneControlLogic, willRetryIfUnmapFailed)
    {
        m_logic.scenePublished(sceneId);

        EXPECT_CALL(m_logicOutput, handleSceneSubscriptionRequest(sceneId));
        m_logic.setSceneState(sceneId, RendererSceneState::Rendered);
        EXPECT_CALL(m_logicOutput, handleSceneMappingRequest(sceneId));
        m_logic.sceneSubscribed(sceneId, RendererSceneControlLogic::EventResult::OK);
        EXPECT_CALL(m_logicOutput, handleSceneDisplayBufferAssignmentRequest(sceneId, OffscreenBufferHandle{}, sceneRenderOrder));
        EXPECT_CALL(m_logicOutput, handleSceneShowRequest(sceneId));
        m_logic.sceneMapped(sceneId, RendererSceneControlLogic::EventResult::OK);
        m_logic.sceneShown(sceneId, RendererSceneControlLogic::EventResult::OK);

        RendererSceneControlLogic::Events expectedEvents;
        expectedEvents.push_back({ sceneId, RendererSceneState::Available });
        expectedEvents.push_back({ sceneId, RendererSceneState::Ready });
        expectedEvents.push_back({ sceneId, RendererSceneState::Rendered });
        expectEvents(expectedEvents);

        EXPECT_CALL(m_logicOutput, handleSceneHideRequest(sceneId));
        m_logic.setSceneState(sceneId, RendererSceneState::Available);
        EXPECT_CALL(m_logicOutput, handleSceneUnmappingRequest(sceneId));
        m_logic.sceneHidden(sceneId, RendererSceneControlLogic::EventResult::OK);
        EXPECT_CALL(m_logicOutput, handleSceneUnmappingRequest(sceneId));
        EXPECT_CALL(m_logicOutput, handleSceneUnsubscriptionRequest(sceneId, false));
        m_logic.sceneUnmapped(sceneId, RendererSceneControlLogic::EventResult::Failed);

        m_logic.sceneUnmapped(sceneId, RendererSceneControlLogic::EventResult::OK);
        m_logic.sceneUnsubscribed(sceneId, RendererSceneControlLogic::EventResult::OK);

        expectedEvents.clear();
        expectedEvents.push_back({ sceneId, RendererSceneState::Ready });
        expectedEvents.push_back({ sceneId, RendererSceneState::Available });
        expectEvents(expectedEvents);

        doAnotherFullCycleFromUnpublishToShownWithShowTriggered(LastSuccessfulCommand::Unsubscribe);
    }

    TEST_F(ARendererSceneControlLogic, willRetryIfUnsubscribeFailed)
    {
        m_logic.scenePublished(sceneId);

        EXPECT_CALL(m_logicOutput, handleSceneSubscriptionRequest(sceneId));
        m_logic.setSceneState(sceneId, RendererSceneState::Rendered);
        EXPECT_CALL(m_logicOutput, handleSceneMappingRequest(sceneId));
        m_logic.sceneSubscribed(sceneId, RendererSceneControlLogic::EventResult::OK);
        EXPECT_CALL(m_logicOutput, handleSceneDisplayBufferAssignmentRequest(sceneId, OffscreenBufferHandle{}, sceneRenderOrder));
        EXPECT_CALL(m_logicOutput, handleSceneShowRequest(sceneId));
        m_logic.sceneMapped(sceneId, RendererSceneControlLogic::EventResult::OK);
        m_logic.sceneShown(sceneId, RendererSceneControlLogic::EventResult::OK);

        RendererSceneControlLogic::Events expectedEvents;
        expectedEvents.push_back({ sceneId, RendererSceneState::Available });
        expectedEvents.push_back({ sceneId, RendererSceneState::Ready });
        expectedEvents.push_back({ sceneId, RendererSceneState::Rendered });
        expectEvents(expectedEvents);

        EXPECT_CALL(m_logicOutput, handleSceneHideRequest(sceneId));
        m_logic.setSceneState(sceneId, RendererSceneState::Available);
        EXPECT_CALL(m_logicOutput, handleSceneUnmappingRequest(sceneId));
        m_logic.sceneHidden(sceneId, RendererSceneControlLogic::EventResult::OK);
        EXPECT_CALL(m_logicOutput, handleSceneUnsubscriptionRequest(sceneId, false));
        m_logic.sceneUnmapped(sceneId, RendererSceneControlLogic::EventResult::OK);
        EXPECT_CALL(m_logicOutput, handleSceneUnsubscriptionRequest(sceneId, false));
        m_logic.sceneUnsubscribed(sceneId, RendererSceneControlLogic::EventResult::Failed);

        m_logic.sceneUnsubscribed(sceneId, RendererSceneControlLogic::EventResult::OK);

        expectedEvents.clear();
        expectedEvents.push_back({ sceneId, RendererSceneState::Ready });
        expectedEvents.push_back({ sceneId, RendererSceneState::Available });
        expectEvents(expectedEvents);

        doAnotherFullCycleFromUnpublishToShownWithShowTriggered(LastSuccessfulCommand::Unsubscribe);
    }

    TEST_F(ARendererSceneControlLogic, hidesShownScene)
    {
        m_logic.setSceneState(sceneId, RendererSceneState::Rendered);
        publishAndExpectToGetToState(RendererSceneState::Rendered);

        EXPECT_CALL(m_logicOutput, handleSceneHideRequest(sceneId));
        m_logic.setSceneState(sceneId, RendererSceneState::Ready);
        m_logic.sceneHidden(sceneId, RendererSceneControlLogic::EventResult::OK);

        expectEvents({ { sceneId, RendererSceneState::Ready } });

        doAnotherFullCycleFromUnpublishToShownWithShowTriggered(LastSuccessfulCommand::Hide);
    }

    TEST_F(ARendererSceneControlLogic, unmapsAndUnsubscribesShownScene)
    {
        m_logic.setSceneState(sceneId, RendererSceneState::Rendered);
        publishAndExpectToGetToState(RendererSceneState::Rendered);

        EXPECT_CALL(m_logicOutput, handleSceneHideRequest(sceneId));
        m_logic.setSceneState(sceneId, RendererSceneState::Available);
        EXPECT_CALL(m_logicOutput, handleSceneUnmappingRequest(sceneId));
        EXPECT_CALL(m_logicOutput, handleSceneUnsubscriptionRequest(sceneId, false));
        m_logic.sceneHidden(sceneId, RendererSceneControlLogic::EventResult::OK);
        m_logic.sceneUnmapped(sceneId, RendererSceneControlLogic::EventResult::OK);
        m_logic.sceneUnsubscribed(sceneId, RendererSceneControlLogic::EventResult::OK);

        RendererSceneControlLogic::Events expectedEvents;
        expectedEvents.push_back({ sceneId, RendererSceneState::Ready });
        expectedEvents.push_back({ sceneId, RendererSceneState::Available });
        expectEvents(expectedEvents);

        doAnotherFullCycleFromUnpublishToShownWithShowTriggered(LastSuccessfulCommand::Unsubscribe);
    }

    TEST_F(ARendererSceneControlLogic, unmapsAndUnsubscribesMappedScene)
    {
        m_logic.setSceneState(sceneId, RendererSceneState::Ready);
        publishAndExpectToGetToState(RendererSceneState::Ready);

        EXPECT_CALL(m_logicOutput, handleSceneUnmappingRequest(sceneId));
        EXPECT_CALL(m_logicOutput, handleSceneUnsubscriptionRequest(sceneId, false));
        m_logic.setSceneState(sceneId, RendererSceneState::Available);
        m_logic.sceneUnmapped(sceneId, RendererSceneControlLogic::EventResult::OK);
        m_logic.sceneUnsubscribed(sceneId, RendererSceneControlLogic::EventResult::OK);

        expectEvents({ { sceneId, RendererSceneState::Available } });

        doAnotherFullCycleFromUnpublishToShownWithShowTriggered(LastSuccessfulCommand::Unmap);
    }

    /////////
    // Tests where scene is unpublished during a state transition, state transition fail is reported back
    // and scene gets re-published.
    // Expectation is to be able to start new cycle and reach a newly set state.
    /////////

    TEST_F(ARendererSceneControlLogic, handlesSceneUnpublishWhileProcessingSubscribe)
    {
        m_logic.setSceneState(sceneId, RendererSceneState::Ready);
        EXPECT_CALL(m_logicOutput, handleSceneSubscriptionRequest(sceneId));
        m_logic.scenePublished(sceneId);
        expectPublishedEvent();

        unpublish(LastSuccessfulCommand::Publish);
        // renderer guarantees to send a response for every command
        // - here we simulate that unpublish came BEFORE the subscribe execution -> therefore it fails for unpublished scene
        m_logic.sceneSubscribed(sceneId, RendererSceneControlLogic::EventResult::Failed);

        publishAndExpectToGetToState(RendererSceneState::Available);
        // make sure target state can be reached once published again
        doAnotherFullCycleFromUnpublishToShownWithShowTriggered(LastSuccessfulCommand::Publish);
    }

    TEST_F(ARendererSceneControlLogic, handlesSceneUnpublishWhileProcessingMap)
    {
        m_logic.setSceneState(sceneId, RendererSceneState::Ready);
        EXPECT_CALL(m_logicOutput, handleSceneSubscriptionRequest(sceneId));
        m_logic.scenePublished(sceneId);
        EXPECT_CALL(m_logicOutput, handleSceneMappingRequest(sceneId));
        m_logic.sceneSubscribed(sceneId, RendererSceneControlLogic::EventResult::OK);

        RendererSceneControlLogic::Events expectedEvents;
        expectedEvents.push_back({ sceneId, RendererSceneState::Available });
        expectEvents(expectedEvents);

        unpublish(LastSuccessfulCommand::Subscribe);
        // renderer guarantees to send a response for every command
        // - here we simulate that unpublish came BEFORE the map execution -> therefore it fails for unpublished scene
        m_logic.sceneMapped(sceneId, RendererSceneControlLogic::EventResult::Failed);

        // make sure target state can be reached once published again
        publishAndExpectToGetToState(RendererSceneState::Available);
        doAnotherFullCycleFromUnpublishToShownWithShowTriggered(LastSuccessfulCommand::Publish);
    }

    TEST_F(ARendererSceneControlLogic, handlesSceneUnpublishWhileProcessingRender)
    {
        m_logic.setSceneState(sceneId, RendererSceneState::Rendered);
        EXPECT_CALL(m_logicOutput, handleSceneSubscriptionRequest(sceneId));
        m_logic.scenePublished(sceneId);
        EXPECT_CALL(m_logicOutput, handleSceneMappingRequest(sceneId));
        m_logic.sceneSubscribed(sceneId, RendererSceneControlLogic::EventResult::OK);
        EXPECT_CALL(m_logicOutput, handleSceneDisplayBufferAssignmentRequest(sceneId, OffscreenBufferHandle{}, sceneRenderOrder));
        EXPECT_CALL(m_logicOutput, handleSceneShowRequest(sceneId));
        m_logic.sceneMapped(sceneId, RendererSceneControlLogic::EventResult::OK);

        RendererSceneControlLogic::Events expectedEvents;
        expectedEvents.push_back({ sceneId, RendererSceneState::Available });
        expectedEvents.push_back({ sceneId, RendererSceneState::Ready });
        expectEvents(expectedEvents);

        unpublish(LastSuccessfulCommand::Map);
        // renderer guarantees to send a response for every command
        // - here we simulate that unpublish came BEFORE the render execution -> therefore it fails for unpublished scene
        m_logic.sceneShown(sceneId, RendererSceneControlLogic::EventResult::Failed);

        // make sure target state can be reached once published again
        publishAndExpectToGetToState(RendererSceneState::Available);
        doAnotherFullCycleFromUnpublishToShownWithShowTriggered(LastSuccessfulCommand::Publish);
    }

    TEST_F(ARendererSceneControlLogic, handlesSceneUnpublishWhileProcessingUnsubscribe)
    {
        m_logic.setSceneState(sceneId, RendererSceneState::Ready);
        publishAndExpectToGetToState(RendererSceneState::Ready);

        EXPECT_CALL(m_logicOutput, handleSceneUnmappingRequest(sceneId));
        EXPECT_CALL(m_logicOutput, handleSceneUnsubscriptionRequest(sceneId, false));
        m_logic.setSceneState(sceneId, RendererSceneState::Available);
        m_logic.sceneUnmapped(sceneId, RendererSceneControlLogic::EventResult::OK);

        RendererSceneControlLogic::Events expectedEvents;
        expectedEvents.push_back({ sceneId, RendererSceneState::Available });
        expectEvents(expectedEvents);

        unpublish(LastSuccessfulCommand::Unmap);
        // renderer guarantees to send a response for every command
        // - here we simulate that unpublish came BEFORE the unsubscribe execution -> therefore it fails for unpublished scene
        m_logic.sceneUnsubscribed(sceneId, RendererSceneControlLogic::EventResult::Failed);

        // make sure target state can be reached once published again
        publishAndExpectToGetToState(RendererSceneState::Available);
        doAnotherFullCycleFromUnpublishToShownWithShowTriggered(LastSuccessfulCommand::Publish);
    }

    TEST_F(ARendererSceneControlLogic, handlesSceneUnpublishWhileProcessingUnmap)
    {
        m_logic.setSceneState(sceneId, RendererSceneState::Ready);
        publishAndExpectToGetToState(RendererSceneState::Ready);

        EXPECT_CALL(m_logicOutput, handleSceneUnmappingRequest(sceneId));
        m_logic.setSceneState(sceneId, RendererSceneState::Available);

        unpublish(LastSuccessfulCommand::Map);
        // renderer guarantees to send a response for every command
        // - here we simulate that unpublish came BEFORE the unmap execution -> therefore it fails for unpublished scene
        m_logic.sceneUnmapped(sceneId, RendererSceneControlLogic::EventResult::Failed);

        // make sure target state can be reached once published again
        publishAndExpectToGetToState(RendererSceneState::Available);
        doAnotherFullCycleFromUnpublishToShownWithShowTriggered(LastSuccessfulCommand::Publish);
    }

    TEST_F(ARendererSceneControlLogic, handlesSceneUnpublishWhileProcessingHide)
    {
        m_logic.setSceneState(sceneId, RendererSceneState::Rendered);
        publishAndExpectToGetToState(RendererSceneState::Rendered);

        EXPECT_CALL(m_logicOutput, handleSceneHideRequest(sceneId));
        m_logic.setSceneState(sceneId, RendererSceneState::Ready);

        unpublish(LastSuccessfulCommand::Show);
        // renderer guarantees to send a response for every command
        // - here we simulate that unpublish came BEFORE the hide execution -> therefore it fails for unpublished scene
        m_logic.sceneHidden(sceneId, RendererSceneControlLogic::EventResult::Failed);

        // make sure target state can be reached once published again
        publishAndExpectToGetToState(RendererSceneState::Available);
        doAnotherFullCycleFromUnpublishToShownWithShowTriggered(LastSuccessfulCommand::Publish);
    }

    /////////
    // Tests where scene is unpublished during a state transition and re-published right after unpublish.
    // State transition fail is reported back AFTER the re-publish.
    // Expectation is to be able to start new cycle and reach a newly set state.
    /////////

    TEST_F(ARendererSceneControlLogic, recoversFromRepublishBeforeFailedSubscribe)
    {
        m_logic.setSceneState(sceneId, RendererSceneState::Ready);

        EXPECT_CALL(m_logicOutput, handleSceneSubscriptionRequest(sceneId));
        m_logic.scenePublished(sceneId);
        expectPublishedEvent();

        // unpublish with all the rollback events ...
        unpublish(LastSuccessfulCommand::Publish);
        // ... and re-publish right after
        m_logic.scenePublished(sceneId);

        // renderer guarantees to send a response for every command
        m_logic.sceneSubscribed(sceneId, RendererSceneControlLogic::EventResult::Failed);

        expectPublishedEvent();
        doAnotherFullCycleFromUnpublishToShownWithShowTriggered(LastSuccessfulCommand::Publish);
    }

    TEST_F(ARendererSceneControlLogic, recoversFromRepublishBeforeFailedMap)
    {
        m_logic.setSceneState(sceneId, RendererSceneState::Ready);

        EXPECT_CALL(m_logicOutput, handleSceneSubscriptionRequest(sceneId));
        m_logic.scenePublished(sceneId);
        EXPECT_CALL(m_logicOutput, handleSceneMappingRequest(sceneId));
        m_logic.sceneSubscribed(sceneId, RendererSceneControlLogic::EventResult::OK);

        RendererSceneControlLogic::Events expectedEvents;
        expectedEvents.push_back({ sceneId, RendererSceneState::Available });
        expectEvents(expectedEvents);

        // unpublish with all the rollback events ...
        unpublish(LastSuccessfulCommand::Subscribe);
        // ... and re-publish right after
        m_logic.scenePublished(sceneId);

        // renderer guarantees to send a response for every command
        m_logic.sceneMapped(sceneId, RendererSceneControlLogic::EventResult::Failed);

        expectPublishedEvent();
        doAnotherFullCycleFromUnpublishToShownWithShowTriggered(LastSuccessfulCommand::Publish);
    }

    TEST_F(ARendererSceneControlLogic, recoversFromRepublishBeforeFailedShow)
    {
        m_logic.setSceneState(sceneId, RendererSceneState::Rendered);

        EXPECT_CALL(m_logicOutput, handleSceneSubscriptionRequest(sceneId));
        m_logic.scenePublished(sceneId);
        EXPECT_CALL(m_logicOutput, handleSceneMappingRequest(sceneId));
        m_logic.sceneSubscribed(sceneId, RendererSceneControlLogic::EventResult::OK);
        EXPECT_CALL(m_logicOutput, handleSceneDisplayBufferAssignmentRequest(sceneId, OffscreenBufferHandle{}, sceneRenderOrder));
        EXPECT_CALL(m_logicOutput, handleSceneShowRequest(sceneId));
        m_logic.sceneMapped(sceneId, RendererSceneControlLogic::EventResult::OK);

        RendererSceneControlLogic::Events expectedEvents;
        expectedEvents.push_back({ sceneId, RendererSceneState::Available });
        expectedEvents.push_back({ sceneId, RendererSceneState::Ready });
        expectEvents(expectedEvents);

        // unpublish with all the rollback events ...
        unpublish(LastSuccessfulCommand::Map);
        // ... and re-publish right after
        m_logic.scenePublished(sceneId);

        // renderer guarantees to send a response for every command
        m_logic.sceneShown(sceneId, RendererSceneControlLogic::EventResult::Failed);

        expectPublishedEvent();
        doAnotherFullCycleFromUnpublishToShownWithShowTriggered(LastSuccessfulCommand::Publish);
    }

    TEST_F(ARendererSceneControlLogic, recoversFromRepublishBeforeFailedUnsubscribe)
    {
        m_logic.setSceneState(sceneId, RendererSceneState::Ready);
        publishAndExpectToGetToState(RendererSceneState::Ready);

        EXPECT_CALL(m_logicOutput, handleSceneUnmappingRequest(sceneId));
        EXPECT_CALL(m_logicOutput, handleSceneUnsubscriptionRequest(sceneId, false));
        m_logic.setSceneState(sceneId, RendererSceneState::Available);
        m_logic.sceneUnmapped(sceneId, RendererSceneControlLogic::EventResult::OK);

        RendererSceneControlLogic::Events expectedEvents;
        expectedEvents.push_back({ sceneId, RendererSceneState::Available });
        expectEvents(expectedEvents);

        // unpublish with all the rollback events ...
        unpublish(LastSuccessfulCommand::Subscribe);
        // ... and re-publish right after
        m_logic.scenePublished(sceneId);

        // renderer guarantees to send a response for every command
        m_logic.sceneUnsubscribed(sceneId, RendererSceneControlLogic::EventResult::Failed);

        expectPublishedEvent();
        doAnotherFullCycleFromUnpublishToShownWithShowTriggered(LastSuccessfulCommand::Publish);
    }

    TEST_F(ARendererSceneControlLogic, recoversFromRepublishBeforeFailedUnmap)
    {
        m_logic.setSceneState(sceneId, RendererSceneState::Ready);
        publishAndExpectToGetToState(RendererSceneState::Ready);

        EXPECT_CALL(m_logicOutput, handleSceneUnmappingRequest(sceneId));
        m_logic.setSceneState(sceneId, RendererSceneState::Available);

        // unpublish with all the rollback events ...
        unpublish(LastSuccessfulCommand::Map);
        // ... and re-publish right after
        m_logic.scenePublished(sceneId);

        // renderer guarantees to send a response for every command
        m_logic.sceneUnmapped(sceneId, RendererSceneControlLogic::EventResult::Failed);

        expectPublishedEvent();
        doAnotherFullCycleFromUnpublishToShownWithShowTriggered(LastSuccessfulCommand::Publish);
    }

    TEST_F(ARendererSceneControlLogic, recoversFromRepublishBeforeFailedHide)
    {
        m_logic.setSceneState(sceneId, RendererSceneState::Rendered);
        publishAndExpectToGetToState(RendererSceneState::Rendered);

        EXPECT_CALL(m_logicOutput, handleSceneHideRequest(sceneId));
        m_logic.setSceneState(sceneId, RendererSceneState::Ready);

        // unpublish with all the rollback events ...
        unpublish(LastSuccessfulCommand::Show);
        // ... and re-publish right after
        m_logic.scenePublished(sceneId);

        // renderer guarantees to send a response for every command
        m_logic.sceneHidden(sceneId, RendererSceneControlLogic::EventResult::Failed);

        expectPublishedEvent();
        doAnotherFullCycleFromUnpublishToShownWithShowTriggered(LastSuccessfulCommand::Publish);
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
        EXPECT_CALL(m_logicOutput, handleSceneMappingRequest(sceneId));
        m_logic.sceneSubscribed(sceneId, RendererSceneControlLogic::EventResult::OK);

        m_logic.setSceneDisplayBufferAssignment(sceneId, offscreenBufferId, 11);

        EXPECT_CALL(m_logicOutput, handleSceneDisplayBufferAssignmentRequest(sceneId, offscreenBufferId, 11));
        m_logic.sceneMapped(sceneId, RendererSceneControlLogic::EventResult::OK);

        RendererSceneControlLogic::Events expectedEvents;
        expectedEvents.push_back({ sceneId, RendererSceneState::Available });
        expectedEvents.push_back({ sceneId, RendererSceneState::Ready });
        expectEvents(expectedEvents);
    }

    TEST_F(ARendererSceneControlLogic, getsInitialSceneInfoForUnknownScene)
    {
        constexpr SceneId someScene{ 666 };
        RendererSceneState state = RendererSceneState::Ready;
        OffscreenBufferHandle ob{ 2 };
        int32_t renderOrder = -1;

        m_logic.getSceneInfo(someScene, state, ob, renderOrder);
        EXPECT_EQ(RendererSceneState::Unavailable, state);
        EXPECT_EQ(OffscreenBufferHandle::Invalid(), ob);
        EXPECT_EQ(0, renderOrder);
    }

    TEST_F(ARendererSceneControlLogic, getsSceneInfo)
    {
        constexpr SceneId someScene{ 666 };
        RendererSceneState state;
        OffscreenBufferHandle ob;
        int32_t renderOrder = -1;

        m_logic.setSceneState(someScene, RendererSceneState::Available);

        m_logic.getSceneInfo(someScene, state, ob, renderOrder);
        EXPECT_EQ(RendererSceneState::Available, state);
        EXPECT_EQ(OffscreenBufferHandle::Invalid(), ob);
        EXPECT_EQ(0, renderOrder);

        m_logic.setSceneDisplayBufferAssignment(someScene, offscreenBufferId, -13);
        m_logic.setSceneState(someScene, RendererSceneState::Ready);

        m_logic.getSceneInfo(someScene, state, ob, renderOrder);
        EXPECT_EQ(RendererSceneState::Ready, state);
        EXPECT_EQ(offscreenBufferId, ob);
        EXPECT_EQ(-13, renderOrder);
    }

    TEST_F(ARendererSceneControlLogic, whileProcessingReadyToAvailableRequestReadyAgain_DuringUnsubscribe)
    {
        m_logic.setSceneState(sceneId, RendererSceneState::Ready);
        publishAndExpectToGetToState(RendererSceneState::Ready);

        // unmap scene
        EXPECT_CALL(m_logicOutput, handleSceneUnmappingRequest(sceneId));
        EXPECT_CALL(m_logicOutput, handleSceneUnsubscriptionRequest(sceneId, false));
        m_logic.setSceneState(sceneId, RendererSceneState::Available);
        m_logic.sceneUnmapped(sceneId, RendererSceneControlLogic::EventResult::OK);
        EXPECT_CALL(m_logicOutput, handleSceneSubscriptionRequest(sceneId));
        EXPECT_CALL(m_logicOutput, handleSceneMappingRequest(sceneId));
        m_logic.setSceneState(sceneId, RendererSceneState::Ready);
        m_logic.sceneUnsubscribed(sceneId, RendererSceneControlLogic::EventResult::OK);

        expectEvents({ { sceneId, RendererSceneState::Available } });

        EXPECT_CALL(m_logicOutput, handleSceneDisplayBufferAssignmentRequest(sceneId, _, _));
        m_logic.sceneSubscribed(sceneId, RendererSceneControlLogic::EventResult::OK);
        m_logic.sceneMapped(sceneId, RendererSceneControlLogic::EventResult::OK);

        expectEvents({ { sceneId, RendererSceneState::Ready } });
    }

    TEST_F(ARendererSceneControlLogic, whileProcessingReadyToAvailableRequestReadyAgain_DuringUnmap)
    {
        m_logic.setSceneState(sceneId, RendererSceneState::Ready);
        publishAndExpectToGetToState(RendererSceneState::Ready);

        // unmap scene
        EXPECT_CALL(m_logicOutput, handleSceneUnmappingRequest(sceneId));
        EXPECT_CALL(m_logicOutput, handleSceneUnsubscriptionRequest(sceneId, false));
        m_logic.setSceneState(sceneId, RendererSceneState::Available);
        EXPECT_CALL(m_logicOutput, handleSceneSubscriptionRequest(sceneId));
        EXPECT_CALL(m_logicOutput, handleSceneMappingRequest(sceneId));
        m_logic.setSceneState(sceneId, RendererSceneState::Ready);
        m_logic.sceneUnmapped(sceneId, RendererSceneControlLogic::EventResult::OK);
        m_logic.sceneUnsubscribed(sceneId, RendererSceneControlLogic::EventResult::OK);

        expectEvents({ { sceneId, RendererSceneState::Available } });

        EXPECT_CALL(m_logicOutput, handleSceneDisplayBufferAssignmentRequest(sceneId, _, _));
        m_logic.sceneSubscribed(sceneId, RendererSceneControlLogic::EventResult::OK);
        m_logic.sceneMapped(sceneId, RendererSceneControlLogic::EventResult::OK);

        expectEvents({ { sceneId, RendererSceneState::Ready } });
    }

    TEST_F(ARendererSceneControlLogic, doesNotTriggerReadyNotificationBeforeMappedAndAssignedIsReached)
    {
        m_logic.setSceneState(sceneId, RendererSceneState::Ready);
        EXPECT_CALL(m_logicOutput, handleSceneSubscriptionRequest(sceneId));
        EXPECT_CALL(m_logicOutput, handleSceneMappingRequest(sceneId));
        EXPECT_CALL(m_logicOutput, handleSceneUnmappingRequest(sceneId));
        EXPECT_CALL(m_logicOutput, handleSceneUnsubscriptionRequest(sceneId, false));

        m_logic.scenePublished(sceneId);
        m_logic.sceneSubscribed(sceneId, RendererSceneControlLogic::EventResult::OK);

        m_logic.setSceneState(sceneId, RendererSceneState::Available);

        m_logic.sceneMapped(sceneId, RendererSceneControlLogic::EventResult::OK);
        m_logic.sceneUnmapped(sceneId, RendererSceneControlLogic::EventResult::OK);
        m_logic.sceneUnsubscribed(sceneId, RendererSceneControlLogic::EventResult::OK);

        expectEvents({ { sceneId, RendererSceneState::Available } });
    }

}
