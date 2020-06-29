//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gmock/gmock.h"
#include "RendererSceneControl_legacyMock.h"
#include "RendererSceneControlLogic.h"
#include "RendererSceneControlEventHandlerMock.h"

using namespace ramses;
using namespace testing;

class ARamsesRendererSceneControlLogic : public Test
{
public:
    ARamsesRendererSceneControlLogic()
        : m_logic(m_logicOutput)
        , m_logicInput(m_logic) // base class is legacy scene control API
    {
    }

    virtual void SetUp() override
    {
        m_logic.setSceneMapping(sceneId, displayId);
        m_logic.setSceneDisplayBufferAssignment(sceneId, displayFramebufferId, sceneRenderOrder);
    }

    virtual void TearDown() override
    {
        // expect nothing to dispatch
        dispatchEventsViaMockedHandler();
    }

protected:
    void dispatchEventsViaMockedHandler()
    {
        const auto events = m_logic.consumeEvents();

        // adapter to transform events into mockable interface calls
        for (const auto& evt : events)
        {
            switch (evt.type)
            {
            case RendererSceneControlLogic::Event::Type::ScenePublished:
                m_logicEventHandler.scenePublished(evt.sceneId);
                break;
            case RendererSceneControlLogic::Event::Type::SceneStateChanged:
                m_logicEventHandler.sceneStateChanged(evt.sceneId, evt.state);
                break;
            case RendererSceneControlLogic::Event::Type::OffscreenBufferLinked:
                m_logicEventHandler.offscreenBufferLinked(evt.displayBufferId, evt.consumerSceneId, evt.consumerId, evt.dataLinked);
                break;
            case RendererSceneControlLogic::Event::Type::DataLinked:
                m_logicEventHandler.dataLinked(evt.providerSceneId, evt.providerId, evt.consumerSceneId, evt.consumerId, evt.dataLinked);
                break;
            case RendererSceneControlLogic::Event::Type::DataUnlinked:
                m_logicEventHandler.dataUnlinked(evt.consumerSceneId, evt.consumerId, evt.dataLinked);
                break;
            case RendererSceneControlLogic::Event::Type::SceneFlushed:
                m_logicEventHandler.sceneFlushed(evt.sceneId, evt.sceneVersion);
                break;
            case RendererSceneControlLogic::Event::Type::SceneExpired:
                m_logicEventHandler.sceneExpired(evt.sceneId);
                break;
            case RendererSceneControlLogic::Event::Type::SceneRecoveredFromExpiration:
                m_logicEventHandler.sceneRecoveredFromExpiration(evt.sceneId);
                break;
            case RendererSceneControlLogic::Event::Type::StreamAvailable:
                m_logicEventHandler.streamAvailabilityChanged(evt.streamSourceId, evt.streamAvailable);
                break;
            }
        }
    }

    void publishAndExpectToGetToState(RendererSceneState state)
    {
        // output expectations
        if (state != RendererSceneState::Unavailable)
        {
            EXPECT_CALL(m_logicOutput, subscribeScene(sceneId));
            if (state != RendererSceneState::Available)
            {
                EXPECT_CALL(m_logicOutput, mapScene(displayId, sceneId));
                EXPECT_CALL(m_logicOutput, assignSceneToDisplayBuffer(sceneId, displayFramebufferId, sceneRenderOrder));
                if (state != RendererSceneState::Ready)
                    EXPECT_CALL(m_logicOutput, showScene(sceneId));
            }
        }

        // simulate response for each step
        m_logicInput.scenePublished(sceneId);
        if (state != RendererSceneState::Unavailable)
        {
            m_logicInput.sceneSubscribed(sceneId, ERendererEventResult_OK);
            if (state != RendererSceneState::Available)
            {
                m_logicInput.sceneMapped(sceneId, ERendererEventResult_OK);
                if (state != RendererSceneState::Ready)
                    m_logicInput.sceneShown(sceneId, ERendererEventResult_OK);
            }
        }

        // events expectations
        EXPECT_CALL(m_logicEventHandler, scenePublished(sceneId));
        if (state != RendererSceneState::Unavailable)
        {
            EXPECT_CALL(m_logicEventHandler, sceneStateChanged(sceneId, RendererSceneState::Available));
            if (state != RendererSceneState::Available)
            {
                EXPECT_CALL(m_logicEventHandler, sceneStateChanged(sceneId, RendererSceneState::Ready));
                if (state != RendererSceneState::Ready)
                    EXPECT_CALL(m_logicEventHandler, sceneStateChanged(sceneId, RendererSceneState::Rendered));
            }
        }
        dispatchEventsViaMockedHandler();
    }

    void unpublish(ramses_internal::ERendererCommand lastSuccessfulCommand)
    {
        // these are events that internal renderer would send on unpublish depending on current state
        switch (lastSuccessfulCommand)
        {
        case ramses_internal::ERendererCommand_ShowScene:
            EXPECT_CALL(m_logicEventHandler, sceneStateChanged(sceneId, RendererSceneState::Ready));
            m_logicInput.sceneHidden(sceneId, ERendererEventResult_INDIRECT);
            RFALLTHROUGH;
        case ramses_internal::ERendererCommand_MapSceneToDisplay:
        case ramses_internal::ERendererCommand_HideScene:
            EXPECT_CALL(m_logicEventHandler, sceneStateChanged(sceneId, RendererSceneState::Available));
            m_logicInput.sceneUnmapped(sceneId, ERendererEventResult_INDIRECT);
            RFALLTHROUGH;
        case ramses_internal::ERendererCommand_SubscribeScene:
        case ramses_internal::ERendererCommand_UnmapSceneFromDisplays:
            EXPECT_CALL(m_logicEventHandler, sceneStateChanged(sceneId, RendererSceneState::Unavailable));
            m_logicInput.sceneUnsubscribed(sceneId, ERendererEventResult_INDIRECT);
            RFALLTHROUGH;
        case ramses_internal::ERendererCommand_PublishedScene:
        case ramses_internal::ERendererCommand_UnsubscribeScene:
            m_logicInput.sceneUnpublished(sceneId);
            RFALLTHROUGH;
        case ramses_internal::ERendererCommand_UnpublishedScene:
            break;
        default:
            assert(false && "invalid starting state");
        }
        dispatchEventsViaMockedHandler();
    }

    void doAnotherFullCycleFromUnpublishToState(ramses_internal::ERendererCommand lastSuccessfulCommand = ramses_internal::ERendererCommand_ShowScene, RendererSceneState state = RendererSceneState::Rendered)
    {
        unpublish(lastSuccessfulCommand);
        publishAndExpectToGetToState(state);
    }

    void doAnotherFullCycleFromUnpublishToShownWithShowTriggered(ramses_internal::ERendererCommand lastSuccessfulCommand = ramses_internal::ERendererCommand_ShowScene)
    {
        unpublish(lastSuccessfulCommand);
        m_logic.setSceneState(sceneId, RendererSceneState::Rendered);
        publishAndExpectToGetToState(RendererSceneState::Rendered);
    }

protected:
    StrictMock<RendererSceneControl_legacyMock>      m_logicOutput;
    RendererSceneControlLogic                        m_logic;
    RendererSceneControlEventHandlerEmpty_legacy&    m_logicInput;
    StrictMock<RendererSceneControlEventHandlerMock> m_logicEventHandler;

    const sceneId_t sceneId{ 33 };
    const displayId_t displayId{ 1u };
    const displayId_t otherDisplayId{ 3u };
    const displayBufferId_t displayFramebufferId{ 11u };
    const displayBufferId_t otherDisplayFramebufferId{ 13u };
    const displayBufferId_t offscreenBufferId{ 19u };
    const int sceneRenderOrder{ -3 };
};

TEST_F(ARamsesRendererSceneControlLogic, willShowSceneWhenPublished)
{
    m_logic.setSceneState(sceneId, RendererSceneState::Rendered);
    publishAndExpectToGetToState(RendererSceneState::Rendered);

    doAnotherFullCycleFromUnpublishToState();
    doAnotherFullCycleFromUnpublishToShownWithShowTriggered();
}

TEST_F(ARamsesRendererSceneControlLogic, willShowSceneAlreadyPublished)
{
    m_logicInput.scenePublished(sceneId);

    EXPECT_CALL(m_logicOutput, subscribeScene(sceneId));
    m_logic.setSceneState(sceneId, RendererSceneState::Rendered);
    EXPECT_CALL(m_logicOutput, mapScene(displayId, sceneId));
    m_logicInput.sceneSubscribed(sceneId, ERendererEventResult_OK);
    EXPECT_CALL(m_logicOutput, assignSceneToDisplayBuffer(sceneId, displayFramebufferId, sceneRenderOrder));
    EXPECT_CALL(m_logicOutput, showScene(sceneId));
    m_logicInput.sceneMapped(sceneId, ERendererEventResult_OK);
    m_logicInput.sceneShown(sceneId, ERendererEventResult_OK);

    EXPECT_CALL(m_logicEventHandler, scenePublished(sceneId));
    EXPECT_CALL(m_logicEventHandler, sceneStateChanged(sceneId, RendererSceneState::Available));
    EXPECT_CALL(m_logicEventHandler, sceneStateChanged(sceneId, RendererSceneState::Ready));
    EXPECT_CALL(m_logicEventHandler, sceneStateChanged(sceneId, RendererSceneState::Rendered));
    dispatchEventsViaMockedHandler();

    doAnotherFullCycleFromUnpublishToState();
    doAnotherFullCycleFromUnpublishToShownWithShowTriggered();
}

TEST_F(ARamsesRendererSceneControlLogic, willRetryIfSubscribeFailed)
{
    m_logicInput.scenePublished(sceneId);
    EXPECT_CALL(m_logicOutput, subscribeScene(sceneId));
    m_logic.setSceneState(sceneId, RendererSceneState::Rendered);

    EXPECT_CALL(m_logicOutput, subscribeScene(sceneId));
    m_logicInput.sceneSubscribed(sceneId, ERendererEventResult_FAIL);

    EXPECT_CALL(m_logicOutput, mapScene(displayId, sceneId));
    m_logicInput.sceneSubscribed(sceneId, ERendererEventResult_OK);
    EXPECT_CALL(m_logicOutput, assignSceneToDisplayBuffer(sceneId, displayFramebufferId, sceneRenderOrder));
    EXPECT_CALL(m_logicOutput, showScene(sceneId));
    m_logicInput.sceneMapped(sceneId, ERendererEventResult_OK);
    m_logicInput.sceneShown(sceneId, ERendererEventResult_OK);

    EXPECT_CALL(m_logicEventHandler, scenePublished(sceneId));
    EXPECT_CALL(m_logicEventHandler, sceneStateChanged(sceneId, RendererSceneState::Available));
    EXPECT_CALL(m_logicEventHandler, sceneStateChanged(sceneId, RendererSceneState::Ready));
    EXPECT_CALL(m_logicEventHandler, sceneStateChanged(sceneId, RendererSceneState::Rendered));
    dispatchEventsViaMockedHandler();

    doAnotherFullCycleFromUnpublishToState();
    doAnotherFullCycleFromUnpublishToShownWithShowTriggered();
}

TEST_F(ARamsesRendererSceneControlLogic, willRetryIfMapFailed)
{
    m_logicInput.scenePublished(sceneId);
    EXPECT_CALL(m_logicOutput, subscribeScene(sceneId));
    m_logic.setSceneState(sceneId, RendererSceneState::Rendered);
    EXPECT_CALL(m_logicOutput, mapScene(displayId, sceneId));
    m_logicInput.sceneSubscribed(sceneId, ERendererEventResult_OK);

    EXPECT_CALL(m_logicOutput, mapScene(displayId, sceneId));
    m_logicInput.sceneMapped(sceneId, ERendererEventResult_FAIL);

    EXPECT_CALL(m_logicOutput, assignSceneToDisplayBuffer(sceneId, displayFramebufferId, sceneRenderOrder));
    EXPECT_CALL(m_logicOutput, showScene(sceneId));
    m_logicInput.sceneMapped(sceneId, ERendererEventResult_OK);
    m_logicInput.sceneShown(sceneId, ERendererEventResult_OK);

    EXPECT_CALL(m_logicEventHandler, scenePublished(sceneId));
    EXPECT_CALL(m_logicEventHandler, sceneStateChanged(sceneId, RendererSceneState::Available));
    EXPECT_CALL(m_logicEventHandler, sceneStateChanged(sceneId, RendererSceneState::Ready));
    EXPECT_CALL(m_logicEventHandler, sceneStateChanged(sceneId, RendererSceneState::Rendered));
    dispatchEventsViaMockedHandler();

    doAnotherFullCycleFromUnpublishToState();
    doAnotherFullCycleFromUnpublishToShownWithShowTriggered();
}

TEST_F(ARamsesRendererSceneControlLogic, willRetryIfShowFailed)
{
    m_logicInput.scenePublished(sceneId);
    EXPECT_CALL(m_logicOutput, subscribeScene(sceneId));
    m_logic.setSceneState(sceneId, RendererSceneState::Rendered);
    EXPECT_CALL(m_logicOutput, mapScene(displayId, sceneId));
    m_logicInput.sceneSubscribed(sceneId, ERendererEventResult_OK);
    EXPECT_CALL(m_logicOutput, assignSceneToDisplayBuffer(sceneId, displayFramebufferId, sceneRenderOrder));
    EXPECT_CALL(m_logicOutput, showScene(sceneId));
    m_logicInput.sceneMapped(sceneId, ERendererEventResult_OK);

    EXPECT_CALL(m_logicOutput, showScene(sceneId));
    m_logicInput.sceneShown(sceneId, ERendererEventResult_FAIL);

    m_logicInput.sceneShown(sceneId, ERendererEventResult_OK);

    EXPECT_CALL(m_logicEventHandler, scenePublished(sceneId));
    EXPECT_CALL(m_logicEventHandler, sceneStateChanged(sceneId, RendererSceneState::Available));
    EXPECT_CALL(m_logicEventHandler, sceneStateChanged(sceneId, RendererSceneState::Ready));
    EXPECT_CALL(m_logicEventHandler, sceneStateChanged(sceneId, RendererSceneState::Rendered));
    dispatchEventsViaMockedHandler();

    doAnotherFullCycleFromUnpublishToState();
    doAnotherFullCycleFromUnpublishToShownWithShowTriggered();
}

TEST_F(ARamsesRendererSceneControlLogic, willRetryIfHideFailed)
{
    m_logicInput.scenePublished(sceneId);

    EXPECT_CALL(m_logicOutput, subscribeScene(sceneId));
    m_logic.setSceneState(sceneId, RendererSceneState::Rendered);
    EXPECT_CALL(m_logicOutput, mapScene(displayId, sceneId));
    m_logicInput.sceneSubscribed(sceneId, ERendererEventResult_OK);
    EXPECT_CALL(m_logicOutput, assignSceneToDisplayBuffer(sceneId, displayFramebufferId, sceneRenderOrder));
    EXPECT_CALL(m_logicOutput, showScene(sceneId));
    m_logicInput.sceneMapped(sceneId, ERendererEventResult_OK);
    m_logicInput.sceneShown(sceneId, ERendererEventResult_OK);

    EXPECT_CALL(m_logicEventHandler, scenePublished(sceneId));
    EXPECT_CALL(m_logicEventHandler, sceneStateChanged(sceneId, RendererSceneState::Available));
    EXPECT_CALL(m_logicEventHandler, sceneStateChanged(sceneId, RendererSceneState::Ready));
    EXPECT_CALL(m_logicEventHandler, sceneStateChanged(sceneId, RendererSceneState::Rendered));
    dispatchEventsViaMockedHandler();

    EXPECT_CALL(m_logicOutput, hideScene(sceneId));
    m_logic.setSceneState(sceneId, RendererSceneState::Ready);
    EXPECT_CALL(m_logicOutput, hideScene(sceneId));
    m_logicInput.sceneHidden(sceneId, ERendererEventResult_FAIL);

    m_logicInput.sceneHidden(sceneId, ERendererEventResult_OK);

    EXPECT_CALL(m_logicEventHandler, sceneStateChanged(sceneId, RendererSceneState::Ready));
    dispatchEventsViaMockedHandler();

    doAnotherFullCycleFromUnpublishToState(ramses_internal::ERendererCommand_HideScene, RendererSceneState::Ready);
    doAnotherFullCycleFromUnpublishToShownWithShowTriggered(ramses_internal::ERendererCommand_HideScene);
}

TEST_F(ARamsesRendererSceneControlLogic, willRetryIfUnmapFailed)
{
    m_logicInput.scenePublished(sceneId);

    EXPECT_CALL(m_logicOutput, subscribeScene(sceneId));
    m_logic.setSceneState(sceneId, RendererSceneState::Rendered);
    EXPECT_CALL(m_logicOutput, mapScene(displayId, sceneId));
    m_logicInput.sceneSubscribed(sceneId, ERendererEventResult_OK);
    EXPECT_CALL(m_logicOutput, assignSceneToDisplayBuffer(sceneId, displayFramebufferId, sceneRenderOrder));
    EXPECT_CALL(m_logicOutput, showScene(sceneId));
    m_logicInput.sceneMapped(sceneId, ERendererEventResult_OK);
    m_logicInput.sceneShown(sceneId, ERendererEventResult_OK);

    EXPECT_CALL(m_logicEventHandler, scenePublished(sceneId));
    EXPECT_CALL(m_logicEventHandler, sceneStateChanged(sceneId, RendererSceneState::Available));
    EXPECT_CALL(m_logicEventHandler, sceneStateChanged(sceneId, RendererSceneState::Ready));
    EXPECT_CALL(m_logicEventHandler, sceneStateChanged(sceneId, RendererSceneState::Rendered));
    dispatchEventsViaMockedHandler();

    EXPECT_CALL(m_logicOutput, hideScene(sceneId));
    m_logic.setSceneState(sceneId, RendererSceneState::Available);
    EXPECT_CALL(m_logicOutput, unmapScene(sceneId));
    m_logicInput.sceneHidden(sceneId, ERendererEventResult_OK);
    EXPECT_CALL(m_logicOutput, unmapScene(sceneId));
    m_logicInput.sceneUnmapped(sceneId, ERendererEventResult_FAIL);

    m_logicInput.sceneUnmapped(sceneId, ERendererEventResult_OK);

    EXPECT_CALL(m_logicEventHandler, sceneStateChanged(sceneId, RendererSceneState::Ready));
    EXPECT_CALL(m_logicEventHandler, sceneStateChanged(sceneId, RendererSceneState::Available));
    dispatchEventsViaMockedHandler();

    doAnotherFullCycleFromUnpublishToState(ramses_internal::ERendererCommand_UnmapSceneFromDisplays, RendererSceneState::Available);
    doAnotherFullCycleFromUnpublishToShownWithShowTriggered(ramses_internal::ERendererCommand_UnmapSceneFromDisplays);
}

TEST_F(ARamsesRendererSceneControlLogic, willRetryIfUnsubscribeFailed)
{
    m_logicInput.scenePublished(sceneId);

    EXPECT_CALL(m_logicOutput, subscribeScene(sceneId));
    m_logic.setSceneState(sceneId, RendererSceneState::Rendered);
    EXPECT_CALL(m_logicOutput, mapScene(displayId, sceneId));
    m_logicInput.sceneSubscribed(sceneId, ERendererEventResult_OK);
    EXPECT_CALL(m_logicOutput, assignSceneToDisplayBuffer(sceneId, displayFramebufferId, sceneRenderOrder));
    EXPECT_CALL(m_logicOutput, showScene(sceneId));
    m_logicInput.sceneMapped(sceneId, ERendererEventResult_OK);
    m_logicInput.sceneShown(sceneId, ERendererEventResult_OK);

    EXPECT_CALL(m_logicEventHandler, scenePublished(sceneId));
    EXPECT_CALL(m_logicEventHandler, sceneStateChanged(sceneId, RendererSceneState::Available));
    EXPECT_CALL(m_logicEventHandler, sceneStateChanged(sceneId, RendererSceneState::Ready));
    EXPECT_CALL(m_logicEventHandler, sceneStateChanged(sceneId, RendererSceneState::Rendered));
    dispatchEventsViaMockedHandler();

    EXPECT_CALL(m_logicOutput, hideScene(sceneId));
    m_logic.setSceneState(sceneId, RendererSceneState::Unavailable);
    EXPECT_CALL(m_logicOutput, unmapScene(sceneId));
    m_logicInput.sceneHidden(sceneId, ERendererEventResult_OK);
    EXPECT_CALL(m_logicOutput, unsubscribeScene(sceneId));
    m_logicInput.sceneUnmapped(sceneId, ERendererEventResult_OK);
    EXPECT_CALL(m_logicOutput, unsubscribeScene(sceneId));
    m_logicInput.sceneUnsubscribed(sceneId, ERendererEventResult_FAIL);

    m_logicInput.sceneUnsubscribed(sceneId, ERendererEventResult_OK);

    EXPECT_CALL(m_logicEventHandler, sceneStateChanged(sceneId, RendererSceneState::Ready));
    EXPECT_CALL(m_logicEventHandler, sceneStateChanged(sceneId, RendererSceneState::Available));
    EXPECT_CALL(m_logicEventHandler, sceneStateChanged(sceneId, RendererSceneState::Unavailable));
    dispatchEventsViaMockedHandler();

    doAnotherFullCycleFromUnpublishToState(ramses_internal::ERendererCommand_UnsubscribeScene, RendererSceneState::Unavailable);
    doAnotherFullCycleFromUnpublishToShownWithShowTriggered(ramses_internal::ERendererCommand_UnsubscribeScene);
}

TEST_F(ARamsesRendererSceneControlLogic, hidesShownScene)
{
    m_logic.setSceneState(sceneId, RendererSceneState::Rendered);
    publishAndExpectToGetToState(RendererSceneState::Rendered);

    EXPECT_CALL(m_logicOutput, hideScene(sceneId));
    m_logic.setSceneState(sceneId, RendererSceneState::Ready);
    m_logicInput.sceneHidden(sceneId, ERendererEventResult_OK);

    EXPECT_CALL(m_logicEventHandler, sceneStateChanged(sceneId, RendererSceneState::Ready));
    dispatchEventsViaMockedHandler();

    doAnotherFullCycleFromUnpublishToState(ramses_internal::ERendererCommand_HideScene, RendererSceneState::Ready);
    doAnotherFullCycleFromUnpublishToShownWithShowTriggered(ramses_internal::ERendererCommand_HideScene);
}

TEST_F(ARamsesRendererSceneControlLogic, unmapsShownScene)
{
    m_logic.setSceneState(sceneId, RendererSceneState::Rendered);
    publishAndExpectToGetToState(RendererSceneState::Rendered);

    EXPECT_CALL(m_logicOutput, hideScene(sceneId));
    m_logic.setSceneState(sceneId, RendererSceneState::Available);
    EXPECT_CALL(m_logicOutput, unmapScene(sceneId));
    m_logicInput.sceneHidden(sceneId, ERendererEventResult_OK);
    m_logicInput.sceneUnmapped(sceneId, ERendererEventResult_OK);

    EXPECT_CALL(m_logicEventHandler, sceneStateChanged(sceneId, RendererSceneState::Ready));
    EXPECT_CALL(m_logicEventHandler, sceneStateChanged(sceneId, RendererSceneState::Available));
    dispatchEventsViaMockedHandler();

    doAnotherFullCycleFromUnpublishToState(ramses_internal::ERendererCommand_UnmapSceneFromDisplays, RendererSceneState::Available);
    doAnotherFullCycleFromUnpublishToShownWithShowTriggered(ramses_internal::ERendererCommand_UnmapSceneFromDisplays);
}

TEST_F(ARamsesRendererSceneControlLogic, unmapsMappedScene)
{
    m_logic.setSceneState(sceneId, RendererSceneState::Ready);
    publishAndExpectToGetToState(RendererSceneState::Ready);

    EXPECT_CALL(m_logicOutput, unmapScene(sceneId));
    m_logic.setSceneState(sceneId, RendererSceneState::Available);
    m_logicInput.sceneUnmapped(sceneId, ERendererEventResult_OK);

    EXPECT_CALL(m_logicEventHandler, sceneStateChanged(sceneId, RendererSceneState::Available));
    dispatchEventsViaMockedHandler();

    doAnotherFullCycleFromUnpublishToState(ramses_internal::ERendererCommand_UnmapSceneFromDisplays, RendererSceneState::Available);
    doAnotherFullCycleFromUnpublishToShownWithShowTriggered(ramses_internal::ERendererCommand_UnmapSceneFromDisplays);
}

TEST_F(ARamsesRendererSceneControlLogic, unsubscribesShownScene)
{
    m_logic.setSceneState(sceneId, RendererSceneState::Rendered);
    publishAndExpectToGetToState(RendererSceneState::Rendered);

    EXPECT_CALL(m_logicOutput, hideScene(sceneId));
    m_logic.setSceneState(sceneId, RendererSceneState::Unavailable);
    EXPECT_CALL(m_logicOutput, unmapScene(sceneId));
    m_logicInput.sceneHidden(sceneId, ERendererEventResult_OK);
    EXPECT_CALL(m_logicOutput, unsubscribeScene(sceneId));
    m_logicInput.sceneUnmapped(sceneId, ERendererEventResult_OK);
    m_logicInput.sceneUnsubscribed(sceneId, ERendererEventResult_OK);

    EXPECT_CALL(m_logicEventHandler, sceneStateChanged(sceneId, RendererSceneState::Ready));
    EXPECT_CALL(m_logicEventHandler, sceneStateChanged(sceneId, RendererSceneState::Available));
    EXPECT_CALL(m_logicEventHandler, sceneStateChanged(sceneId, RendererSceneState::Unavailable));
    dispatchEventsViaMockedHandler();

    doAnotherFullCycleFromUnpublishToState(ramses_internal::ERendererCommand_UnsubscribeScene, RendererSceneState::Unavailable);
    doAnotherFullCycleFromUnpublishToShownWithShowTriggered(ramses_internal::ERendererCommand_UnsubscribeScene);
}

TEST_F(ARamsesRendererSceneControlLogic, unsubscribesMappedScene)
{
    m_logic.setSceneState(sceneId, RendererSceneState::Ready);
    publishAndExpectToGetToState(RendererSceneState::Ready);

    EXPECT_CALL(m_logicOutput, unmapScene(sceneId));
    m_logic.setSceneState(sceneId, RendererSceneState::Unavailable);
    EXPECT_CALL(m_logicOutput, unsubscribeScene(sceneId));
    m_logicInput.sceneUnmapped(sceneId, ERendererEventResult_OK);
    m_logicInput.sceneUnsubscribed(sceneId, ERendererEventResult_OK);

    EXPECT_CALL(m_logicEventHandler, sceneStateChanged(sceneId, RendererSceneState::Available));
    EXPECT_CALL(m_logicEventHandler, sceneStateChanged(sceneId, RendererSceneState::Unavailable));
    dispatchEventsViaMockedHandler();

    doAnotherFullCycleFromUnpublishToState(ramses_internal::ERendererCommand_UnsubscribeScene, RendererSceneState::Unavailable);
    doAnotherFullCycleFromUnpublishToShownWithShowTriggered(ramses_internal::ERendererCommand_UnsubscribeScene);
}

TEST_F(ARamsesRendererSceneControlLogic, unsubscribesSubscribedScene)
{
    m_logic.setSceneState(sceneId, RendererSceneState::Available);
    publishAndExpectToGetToState(RendererSceneState::Available);

    EXPECT_CALL(m_logicOutput, unsubscribeScene(sceneId));
    m_logic.setSceneState(sceneId, RendererSceneState::Unavailable);
    m_logicInput.sceneUnsubscribed(sceneId, ERendererEventResult_OK);

    EXPECT_CALL(m_logicEventHandler, sceneStateChanged(sceneId, RendererSceneState::Unavailable));
    dispatchEventsViaMockedHandler();

    doAnotherFullCycleFromUnpublishToState(ramses_internal::ERendererCommand_UnsubscribeScene, RendererSceneState::Unavailable);
    doAnotherFullCycleFromUnpublishToShownWithShowTriggered(ramses_internal::ERendererCommand_UnsubscribeScene);
}

TEST_F(ARamsesRendererSceneControlLogic, canChangeMappingPropertiesWhenSceneAvailableAndNotSetToReadyOrRenderedYet)
{
    m_logic.setSceneState(sceneId, RendererSceneState::Available);
    publishAndExpectToGetToState(RendererSceneState::Available);

    m_logic.setSceneMapping(sceneId, otherDisplayId);
    m_logic.setSceneDisplayBufferAssignment(sceneId, otherDisplayFramebufferId, 666);

    // will map scene to other display
    EXPECT_CALL(m_logicOutput, mapScene(otherDisplayId, sceneId));
    m_logic.setSceneState(sceneId, RendererSceneState::Ready);
    // will assign scene to other display's framebuffer with given render order
    EXPECT_CALL(m_logicOutput, assignSceneToDisplayBuffer(sceneId, otherDisplayFramebufferId, 666));
    m_logicInput.sceneMapped(sceneId, ERendererEventResult_OK);

    EXPECT_CALL(m_logicEventHandler, sceneStateChanged(sceneId, RendererSceneState::Ready));
    dispatchEventsViaMockedHandler();
}

TEST_F(ARamsesRendererSceneControlLogic, canChangeMappingPropertiesForReadySceneAfterGettingItToAvailableFirst)
{
    m_logic.setSceneState(sceneId, RendererSceneState::Ready);
    publishAndExpectToGetToState(RendererSceneState::Ready);

    EXPECT_CALL(m_logicOutput, unmapScene(sceneId));
    m_logic.setSceneState(sceneId, RendererSceneState::Available);
    m_logicInput.sceneUnmapped(sceneId, ERendererEventResult_OK);

    EXPECT_CALL(m_logicEventHandler, sceneStateChanged(sceneId, RendererSceneState::Available));
    dispatchEventsViaMockedHandler();

    // now can change mapping
    m_logic.setSceneMapping(sceneId, otherDisplayId);
    m_logic.setSceneDisplayBufferAssignment(sceneId, otherDisplayFramebufferId, 666);

    EXPECT_CALL(m_logicOutput, mapScene(otherDisplayId, sceneId));
    m_logic.setSceneState(sceneId, RendererSceneState::Ready);
    EXPECT_CALL(m_logicOutput, assignSceneToDisplayBuffer(sceneId, otherDisplayFramebufferId, 666));
    m_logicInput.sceneMapped(sceneId, ERendererEventResult_OK);

    EXPECT_CALL(m_logicEventHandler, sceneStateChanged(sceneId, RendererSceneState::Ready));
    dispatchEventsViaMockedHandler();
}

TEST_F(ARamsesRendererSceneControlLogic, forwardsCommandForDisplayFrameBufferClearColor)
{
    EXPECT_CALL(m_logicOutput, setDisplayBufferClearColor(otherDisplayId, otherDisplayFramebufferId, 1, 2, 3, 4));
    m_logic.setDisplayBufferClearColor(otherDisplayId, otherDisplayFramebufferId, 1, 2, 3, 4);
}

TEST_F(ARamsesRendererSceneControlLogic, forwardsCommandForDisplayOffscreenBufferClearColor)
{
    EXPECT_CALL(m_logicOutput, setDisplayBufferClearColor(displayId, offscreenBufferId, 1, 2, 3, 4));
    m_logic.setDisplayBufferClearColor(displayId, offscreenBufferId, 1, 2, 3, 4);
}

TEST_F(ARamsesRendererSceneControlLogic, forwardsCommandForOffscreenBufferLink)
{
    constexpr sceneId_t consumerSceneId{ 3u };
    constexpr dataConsumerId_t consumerId{ 4u };

    EXPECT_CALL(m_logicOutput, linkOffscreenBufferToSceneData(offscreenBufferId, consumerSceneId, consumerId));
    m_logic.linkOffscreenBuffer(offscreenBufferId, consumerSceneId, consumerId);
}

TEST_F(ARamsesRendererSceneControlLogic, forwardsCommandForDataLink)
{
    constexpr sceneId_t providerSceneId{ 1u };
    constexpr dataProviderId_t providerId{ 2u };
    constexpr sceneId_t consumerSceneId{ 3u };
    constexpr dataConsumerId_t consumerId{ 4u };

    EXPECT_CALL(m_logicOutput, linkData(providerSceneId, providerId, consumerSceneId, consumerId));
    m_logic.linkData(providerSceneId, providerId, consumerSceneId, consumerId);
}

TEST_F(ARamsesRendererSceneControlLogic, forwardsCommandForDataUnlink)
{
    constexpr sceneId_t consumerSceneId{ 3u };
    constexpr dataConsumerId_t consumerId{ 4u };

    EXPECT_CALL(m_logicOutput, unlinkData(consumerSceneId, consumerId));
    m_logic.unlinkData(consumerSceneId, consumerId);
}

/////////
// Tests where scene is unpublished during a state transition, state transition fail is reported back
// and scene gets re-published.
// Expectation is to reach original target state.
/////////

TEST_F(ARamsesRendererSceneControlLogic, handlesSceneUnpublishWhileProcessingSubscribe)
{
    m_logic.setSceneState(sceneId, RendererSceneState::Available);
    EXPECT_CALL(m_logicOutput, subscribeScene(sceneId));
    m_logicInput.scenePublished(sceneId);
    EXPECT_CALL(m_logicEventHandler, scenePublished(sceneId));
    dispatchEventsViaMockedHandler();

    unpublish(ramses_internal::ERendererCommand_PublishedScene);
    // renderer guarantees to send a response for every command
    // - here we simulate that unpublish came BEFORE the subscribe execution -> therefore it fails for unpublished scene
    m_logicInput.sceneSubscribed(sceneId, ERendererEventResult_FAIL);

    // make sure target state can be reached once published again
    publishAndExpectToGetToState(RendererSceneState::Available);
    doAnotherFullCycleFromUnpublishToState(ramses_internal::ERendererCommand_SubscribeScene, RendererSceneState::Available);
    doAnotherFullCycleFromUnpublishToShownWithShowTriggered(ramses_internal::ERendererCommand_SubscribeScene);
}

TEST_F(ARamsesRendererSceneControlLogic, handlesSceneUnpublishWhileProcessingMap)
{
    m_logic.setSceneState(sceneId, RendererSceneState::Ready);
    EXPECT_CALL(m_logicOutput, subscribeScene(sceneId));
    m_logicInput.scenePublished(sceneId);
    EXPECT_CALL(m_logicOutput, mapScene(displayId, sceneId));
    m_logicInput.sceneSubscribed(sceneId, ERendererEventResult_OK);

    EXPECT_CALL(m_logicEventHandler, scenePublished(sceneId));
    EXPECT_CALL(m_logicEventHandler, sceneStateChanged(sceneId, RendererSceneState::Available));
    dispatchEventsViaMockedHandler();

    unpublish(ramses_internal::ERendererCommand_SubscribeScene);
    // renderer guarantees to send a response for every command
    // - here we simulate that unpublish came BEFORE the map execution -> therefore it fails for unpublished scene
    m_logicInput.sceneMapped(sceneId, ERendererEventResult_FAIL);

    // make sure target state can be reached once published again
    publishAndExpectToGetToState(RendererSceneState::Ready);
    doAnotherFullCycleFromUnpublishToState(ramses_internal::ERendererCommand_MapSceneToDisplay, RendererSceneState::Ready);
    doAnotherFullCycleFromUnpublishToShownWithShowTriggered(ramses_internal::ERendererCommand_MapSceneToDisplay);
}

TEST_F(ARamsesRendererSceneControlLogic, handlesSceneUnpublishWhileProcessingRender)
{
    m_logic.setSceneState(sceneId, RendererSceneState::Rendered);
    EXPECT_CALL(m_logicOutput, subscribeScene(sceneId));
    m_logicInput.scenePublished(sceneId);
    EXPECT_CALL(m_logicOutput, mapScene(displayId, sceneId));
    m_logicInput.sceneSubscribed(sceneId, ERendererEventResult_OK);
    EXPECT_CALL(m_logicOutput, assignSceneToDisplayBuffer(sceneId, displayFramebufferId, sceneRenderOrder));
    EXPECT_CALL(m_logicOutput, showScene(sceneId));
    m_logicInput.sceneMapped(sceneId, ERendererEventResult_OK);

    EXPECT_CALL(m_logicEventHandler, scenePublished(sceneId));
    EXPECT_CALL(m_logicEventHandler, sceneStateChanged(sceneId, RendererSceneState::Available));
    EXPECT_CALL(m_logicEventHandler, sceneStateChanged(sceneId, RendererSceneState::Ready));
    dispatchEventsViaMockedHandler();

    unpublish(ramses_internal::ERendererCommand_MapSceneToDisplay);
    // renderer guarantees to send a response for every command
    // - here we simulate that unpublish came BEFORE the render execution -> therefore it fails for unpublished scene
    m_logicInput.sceneShown(sceneId, ERendererEventResult_FAIL);

    // make sure target state can be reached once published again
    publishAndExpectToGetToState(RendererSceneState::Rendered);
    doAnotherFullCycleFromUnpublishToState(ramses_internal::ERendererCommand_MapSceneToDisplay, RendererSceneState::Rendered);
    doAnotherFullCycleFromUnpublishToShownWithShowTriggered(ramses_internal::ERendererCommand_MapSceneToDisplay);
}

TEST_F(ARamsesRendererSceneControlLogic, handlesSceneUnpublishWhileProcessingUnsubscribe)
{
    m_logic.setSceneState(sceneId, RendererSceneState::Available);
    publishAndExpectToGetToState(RendererSceneState::Available);

    EXPECT_CALL(m_logicOutput, unsubscribeScene(sceneId));
    m_logic.setSceneState(sceneId, RendererSceneState::Unavailable);

    unpublish(ramses_internal::ERendererCommand_SubscribeScene);
    // renderer guarantees to send a response for every command
    // - here we simulate that unpublish came BEFORE the unsubscribe execution -> therefore it fails for unpublished scene
    m_logicInput.sceneUnsubscribed(sceneId, ERendererEventResult_FAIL);

    // make sure target state can be reached once published again
    publishAndExpectToGetToState(RendererSceneState::Unavailable);
    doAnotherFullCycleFromUnpublishToState(ramses_internal::ERendererCommand_PublishedScene, RendererSceneState::Unavailable);
    doAnotherFullCycleFromUnpublishToShownWithShowTriggered(ramses_internal::ERendererCommand_PublishedScene);
}

TEST_F(ARamsesRendererSceneControlLogic, handlesSceneUnpublishWhileProcessingUnmap)
{
    m_logic.setSceneState(sceneId, RendererSceneState::Ready);
    publishAndExpectToGetToState(RendererSceneState::Ready);

    EXPECT_CALL(m_logicOutput, unmapScene(sceneId));
    m_logic.setSceneState(sceneId, RendererSceneState::Available);

    unpublish(ramses_internal::ERendererCommand_MapSceneToDisplay);
    // renderer guarantees to send a response for every command
    // - here we simulate that unpublish came BEFORE the unmap execution -> therefore it fails for unpublished scene
    m_logicInput.sceneUnmapped(sceneId, ERendererEventResult_FAIL);

    // make sure target state can be reached once published again
    publishAndExpectToGetToState(RendererSceneState::Available);
    doAnotherFullCycleFromUnpublishToState(ramses_internal::ERendererCommand_SubscribeScene, RendererSceneState::Available);
    doAnotherFullCycleFromUnpublishToShownWithShowTriggered(ramses_internal::ERendererCommand_SubscribeScene);
}

TEST_F(ARamsesRendererSceneControlLogic, handlesSceneUnpublishWhileProcessingHide)
{
    m_logic.setSceneState(sceneId, RendererSceneState::Rendered);
    publishAndExpectToGetToState(RendererSceneState::Rendered);

    EXPECT_CALL(m_logicOutput, hideScene(sceneId));
    m_logic.setSceneState(sceneId, RendererSceneState::Ready);

    unpublish(ramses_internal::ERendererCommand_ShowScene);
    // renderer guarantees to send a response for every command
    // - here we simulate that unpublish came BEFORE the hide execution -> therefore it fails for unpublished scene
    m_logicInput.sceneHidden(sceneId, ERendererEventResult_FAIL);

    // make sure target state can be reached once published again
    publishAndExpectToGetToState(RendererSceneState::Ready);
    doAnotherFullCycleFromUnpublishToState(ramses_internal::ERendererCommand_MapSceneToDisplay, RendererSceneState::Ready);
    doAnotherFullCycleFromUnpublishToShownWithShowTriggered(ramses_internal::ERendererCommand_MapSceneToDisplay);
}

/////////
// Tests where scene is unpublished during a state transition and re-published right after unpublish.
// State transition fail is reported back AFTER the re-publish.
// Expectation it to reach original target state.
/////////

TEST_F(ARamsesRendererSceneControlLogic, recoversFromRepublishBeforeFailedSubscribe)
{
    m_logic.setSceneState(sceneId, RendererSceneState::Available);

    EXPECT_CALL(m_logicEventHandler, scenePublished(sceneId));
    EXPECT_CALL(m_logicOutput, subscribeScene(sceneId));
    m_logicInput.scenePublished(sceneId);

    // unpublish with all the rollback events ...
    unpublish(ramses_internal::ERendererCommand_PublishedScene);
    // ... and re-publish right after
    m_logicInput.scenePublished(sceneId);
    // not trying to subscribe again because last subscribe not answered yet

    // renderer guarantees to send a response for every command
    EXPECT_CALL(m_logicOutput, subscribeScene(sceneId)); // now try again
    m_logicInput.sceneSubscribed(sceneId, ERendererEventResult_FAIL);
    m_logicInput.sceneSubscribed(sceneId, ERendererEventResult_OK);

    EXPECT_CALL(m_logicEventHandler, scenePublished(sceneId));
    EXPECT_CALL(m_logicEventHandler, sceneStateChanged(sceneId, RendererSceneState::Available));
    dispatchEventsViaMockedHandler();

    doAnotherFullCycleFromUnpublishToState(ramses_internal::ERendererCommand_SubscribeScene, RendererSceneState::Available);
    doAnotherFullCycleFromUnpublishToShownWithShowTriggered(ramses_internal::ERendererCommand_SubscribeScene);
}

TEST_F(ARamsesRendererSceneControlLogic, recoversFromRepublishBeforeFailedMap)
{
    m_logic.setSceneState(sceneId, RendererSceneState::Ready);

    EXPECT_CALL(m_logicEventHandler, scenePublished(sceneId));
    EXPECT_CALL(m_logicOutput, subscribeScene(sceneId));
    m_logicInput.scenePublished(sceneId);
    EXPECT_CALL(m_logicEventHandler, sceneStateChanged(sceneId, RendererSceneState::Available));
    EXPECT_CALL(m_logicOutput, mapScene(displayId, sceneId));
    m_logicInput.sceneSubscribed(sceneId, ERendererEventResult_OK);

    // unpublish with all the rollback events ...
    unpublish(ramses_internal::ERendererCommand_SubscribeScene);
    // ... and re-publish right after
    m_logicInput.scenePublished(sceneId);
    // not trying to map again because last map not answered yet

    // renderer guarantees to send a response for every command
    // will try again starting from subscription
    EXPECT_CALL(m_logicOutput, subscribeScene(sceneId));
    m_logicInput.sceneMapped(sceneId, ERendererEventResult_FAIL);
    EXPECT_CALL(m_logicOutput, mapScene(displayId, sceneId));
    m_logicInput.sceneSubscribed(sceneId, ERendererEventResult_OK);
    EXPECT_CALL(m_logicOutput, assignSceneToDisplayBuffer(sceneId, displayFramebufferId, sceneRenderOrder));
    m_logicInput.sceneMapped(sceneId, ERendererEventResult_OK);

    EXPECT_CALL(m_logicEventHandler, scenePublished(sceneId));
    EXPECT_CALL(m_logicEventHandler, sceneStateChanged(sceneId, RendererSceneState::Available));
    EXPECT_CALL(m_logicEventHandler, sceneStateChanged(sceneId, RendererSceneState::Ready));
    dispatchEventsViaMockedHandler();

    doAnotherFullCycleFromUnpublishToState(ramses_internal::ERendererCommand_MapSceneToDisplay, RendererSceneState::Ready);
    doAnotherFullCycleFromUnpublishToShownWithShowTriggered(ramses_internal::ERendererCommand_MapSceneToDisplay);
}

TEST_F(ARamsesRendererSceneControlLogic, recoversFromRepublishBeforeFailedShow)
{
    m_logic.setSceneState(sceneId, RendererSceneState::Rendered);

    EXPECT_CALL(m_logicOutput, subscribeScene(sceneId));
    m_logicInput.scenePublished(sceneId);
    EXPECT_CALL(m_logicOutput, mapScene(displayId, sceneId));
    m_logicInput.sceneSubscribed(sceneId, ERendererEventResult_OK);
    EXPECT_CALL(m_logicOutput, assignSceneToDisplayBuffer(sceneId, displayFramebufferId, sceneRenderOrder));
    EXPECT_CALL(m_logicOutput, showScene(sceneId));
    m_logicInput.sceneMapped(sceneId, ERendererEventResult_OK);

    EXPECT_CALL(m_logicEventHandler, scenePublished(sceneId));
    EXPECT_CALL(m_logicEventHandler, sceneStateChanged(sceneId, RendererSceneState::Available));
    EXPECT_CALL(m_logicEventHandler, sceneStateChanged(sceneId, RendererSceneState::Ready));
    dispatchEventsViaMockedHandler();

    // unpublish with all the rollback events ...
    unpublish(ramses_internal::ERendererCommand_MapSceneToDisplay);
    // ... and re-publish right after
    m_logicInput.scenePublished(sceneId);
    // not trying to show again because last show not answered yet

    // renderer guarantees to send a response for every command
    // will try again starting from subscription
    EXPECT_CALL(m_logicOutput, subscribeScene(sceneId));
    m_logicInput.sceneShown(sceneId, ERendererEventResult_FAIL);
    EXPECT_CALL(m_logicOutput, mapScene(displayId, sceneId));
    m_logicInput.sceneSubscribed(sceneId, ERendererEventResult_OK);
    EXPECT_CALL(m_logicOutput, assignSceneToDisplayBuffer(sceneId, displayFramebufferId, sceneRenderOrder));
    EXPECT_CALL(m_logicOutput, showScene(sceneId));
    m_logicInput.sceneMapped(sceneId, ERendererEventResult_OK);
    m_logicInput.sceneShown(sceneId, ERendererEventResult_OK);

    EXPECT_CALL(m_logicEventHandler, scenePublished(sceneId));
    EXPECT_CALL(m_logicEventHandler, sceneStateChanged(sceneId, RendererSceneState::Available));
    EXPECT_CALL(m_logicEventHandler, sceneStateChanged(sceneId, RendererSceneState::Ready));
    EXPECT_CALL(m_logicEventHandler, sceneStateChanged(sceneId, RendererSceneState::Rendered));
    dispatchEventsViaMockedHandler();

    doAnotherFullCycleFromUnpublishToState(ramses_internal::ERendererCommand_ShowScene, RendererSceneState::Rendered);
    doAnotherFullCycleFromUnpublishToShownWithShowTriggered(ramses_internal::ERendererCommand_ShowScene);
}

TEST_F(ARamsesRendererSceneControlLogic, recoversFromRepublishBeforeFailedUnsubscribe)
{
    m_logic.setSceneState(sceneId, RendererSceneState::Available);
    publishAndExpectToGetToState(RendererSceneState::Available);

    EXPECT_CALL(m_logicOutput, unsubscribeScene(sceneId));
    m_logic.setSceneState(sceneId, RendererSceneState::Unavailable);

    // unpublish with all the rollback events ...
    unpublish(ramses_internal::ERendererCommand_SubscribeScene);
    // ... and re-publish right after
    m_logicInput.scenePublished(sceneId);
    // not trying to do anything yet because last unsubscribe not answered yet

    // renderer guarantees to send a response for every command
    m_logicInput.sceneUnsubscribed(sceneId, ERendererEventResult_FAIL);
    // there is actually nothing to do - requested UNAVAILABLE state was reached

    EXPECT_CALL(m_logicEventHandler, scenePublished(sceneId));
    dispatchEventsViaMockedHandler();

    doAnotherFullCycleFromUnpublishToState(ramses_internal::ERendererCommand_PublishedScene, RendererSceneState::Unavailable);
    doAnotherFullCycleFromUnpublishToShownWithShowTriggered(ramses_internal::ERendererCommand_PublishedScene);
}

TEST_F(ARamsesRendererSceneControlLogic, recoversFromRepublishBeforeFailedUnmap)
{
    m_logic.setSceneState(sceneId, RendererSceneState::Ready);
    publishAndExpectToGetToState(RendererSceneState::Ready);

    EXPECT_CALL(m_logicOutput, unmapScene(sceneId));
    m_logic.setSceneState(sceneId, RendererSceneState::Available);

    // unpublish with all the rollback events ...
    unpublish(ramses_internal::ERendererCommand_MapSceneToDisplay);
    // ... and re-publish right after
    m_logicInput.scenePublished(sceneId);
    // not trying to do anything yet because last unmap not answered yet

    // renderer guarantees to send a response for every command
    // will try to reach last target state again starting from subscription
    EXPECT_CALL(m_logicOutput, subscribeScene(sceneId));
    m_logicInput.sceneUnmapped(sceneId, ERendererEventResult_FAIL);
    m_logicInput.sceneSubscribed(sceneId, ERendererEventResult_OK);

    EXPECT_CALL(m_logicEventHandler, scenePublished(sceneId));
    EXPECT_CALL(m_logicEventHandler, sceneStateChanged(sceneId, RendererSceneState::Available));
    dispatchEventsViaMockedHandler();

    doAnotherFullCycleFromUnpublishToState(ramses_internal::ERendererCommand_SubscribeScene, RendererSceneState::Available);
    doAnotherFullCycleFromUnpublishToShownWithShowTriggered(ramses_internal::ERendererCommand_SubscribeScene);
}

TEST_F(ARamsesRendererSceneControlLogic, recoversFromRepublishBeforeFailedHide)
{
    m_logic.setSceneState(sceneId, RendererSceneState::Rendered);
    publishAndExpectToGetToState(RendererSceneState::Rendered);

    EXPECT_CALL(m_logicOutput, hideScene(sceneId));
    m_logic.setSceneState(sceneId, RendererSceneState::Ready);

    // unpublish with all the rollback events ...
    unpublish(ramses_internal::ERendererCommand_ShowScene);
    // ... and re-publish right after
    m_logicInput.scenePublished(sceneId);
    // not trying to do anything yet because last hide not answered yet

    // renderer guarantees to send a response for every command
    // will try to reach last target state again starting from subscription
    EXPECT_CALL(m_logicOutput, subscribeScene(sceneId));
    m_logicInput.sceneHidden(sceneId, ERendererEventResult_FAIL);
    EXPECT_CALL(m_logicOutput, mapScene(displayId, sceneId));
    m_logicInput.sceneSubscribed(sceneId, ERendererEventResult_OK);
    EXPECT_CALL(m_logicOutput, assignSceneToDisplayBuffer(sceneId, displayFramebufferId, sceneRenderOrder));
    m_logicInput.sceneMapped(sceneId, ERendererEventResult_OK);

    EXPECT_CALL(m_logicEventHandler, scenePublished(sceneId));
    EXPECT_CALL(m_logicEventHandler, sceneStateChanged(sceneId, RendererSceneState::Available));
    EXPECT_CALL(m_logicEventHandler, sceneStateChanged(sceneId, RendererSceneState::Ready));
    dispatchEventsViaMockedHandler();

    doAnotherFullCycleFromUnpublishToState(ramses_internal::ERendererCommand_MapSceneToDisplay, RendererSceneState::Ready);
    doAnotherFullCycleFromUnpublishToShownWithShowTriggered(ramses_internal::ERendererCommand_MapSceneToDisplay);
}

TEST_F(ARamsesRendererSceneControlLogic, canAssignSceneToBufferAfterMapped)
{
    m_logic.setSceneState(sceneId, RendererSceneState::Ready);
    publishAndExpectToGetToState(RendererSceneState::Ready);

    EXPECT_CALL(m_logicOutput, assignSceneToDisplayBuffer(sceneId, offscreenBufferId, 11));
    m_logic.setSceneDisplayBufferAssignment(sceneId, offscreenBufferId, 11);
}

TEST_F(ARamsesRendererSceneControlLogic, canAssignSceneToBufferAfterMapCommandBeforeMappedEvent)
{
    m_logic.setSceneState(sceneId, RendererSceneState::Ready);

    EXPECT_CALL(m_logicOutput, subscribeScene(sceneId));
    m_logicInput.scenePublished(sceneId);
    EXPECT_CALL(m_logicOutput, mapScene(displayId, sceneId));
    m_logicInput.sceneSubscribed(sceneId, ERendererEventResult_OK);

    m_logic.setSceneDisplayBufferAssignment(sceneId, offscreenBufferId, 11);

    EXPECT_CALL(m_logicOutput, assignSceneToDisplayBuffer(sceneId, offscreenBufferId, 11));
    m_logicInput.sceneMapped(sceneId, ERendererEventResult_OK);

    EXPECT_CALL(m_logicEventHandler, scenePublished(sceneId));
    EXPECT_CALL(m_logicEventHandler, sceneStateChanged(sceneId, RendererSceneState::Available));
    EXPECT_CALL(m_logicEventHandler, sceneStateChanged(sceneId, RendererSceneState::Ready));
    dispatchEventsViaMockedHandler();
}

TEST_F(ARamsesRendererSceneControlLogic, mappingSceneToAnotherDisplayResetsDisplayBufferAssignmentAndRenderOrder)
{
    m_logic.setSceneState(sceneId, RendererSceneState::Ready);
    publishAndExpectToGetToState(RendererSceneState::Ready);

    // assign to OB + render order
    EXPECT_CALL(m_logicOutput, assignSceneToDisplayBuffer(sceneId, offscreenBufferId, 11));
    m_logic.setSceneDisplayBufferAssignment(sceneId, offscreenBufferId, 11);

    // unmap scene
    EXPECT_CALL(m_logicOutput, unmapScene(sceneId));
    m_logic.setSceneState(sceneId, RendererSceneState::Available);
    m_logicInput.sceneUnmapped(sceneId, ERendererEventResult_OK);

    EXPECT_CALL(m_logicEventHandler, sceneStateChanged(sceneId, RendererSceneState::Available));
    dispatchEventsViaMockedHandler();

    // set mapping to other display
    m_logic.setSceneMapping(sceneId, otherDisplayId);

    // map scene
    EXPECT_CALL(m_logicOutput, mapScene(otherDisplayId, sceneId));
    m_logic.setSceneState(sceneId, RendererSceneState::Ready);
    EXPECT_CALL(m_logicOutput, assignSceneToDisplayBuffer(sceneId, displayBufferId_t::Invalid(), 0));
    m_logicInput.sceneMapped(sceneId, ERendererEventResult_OK);

    EXPECT_CALL(m_logicEventHandler, sceneStateChanged(sceneId, RendererSceneState::Ready));
    dispatchEventsViaMockedHandler();
}

TEST_F(ARamsesRendererSceneControlLogic, willForwardOffscreenBufferLinkedEventFromRenderer)
{
    constexpr sceneId_t consumerScene{ 3 };
    constexpr dataConsumerId_t consumerId{ 4 };

    InSequence seq;
    EXPECT_CALL(m_logicEventHandler, offscreenBufferLinked(offscreenBufferId, consumerScene, consumerId, true));
    m_logicInput.offscreenBufferLinkedToSceneData(offscreenBufferId, consumerScene, consumerId, ERendererEventResult_OK);

    EXPECT_CALL(m_logicEventHandler, offscreenBufferLinked(offscreenBufferId, consumerScene, consumerId, false));
    m_logicInput.offscreenBufferLinkedToSceneData(offscreenBufferId, consumerScene, consumerId, ERendererEventResult_FAIL);

    EXPECT_CALL(m_logicEventHandler, offscreenBufferLinked(offscreenBufferId, consumerScene, consumerId, true));
    m_logicInput.offscreenBufferLinkedToSceneData(offscreenBufferId, consumerScene, consumerId, ERendererEventResult_OK);
    EXPECT_CALL(m_logicEventHandler, offscreenBufferLinked(offscreenBufferId, consumerScene, consumerId, false));
    m_logicInput.offscreenBufferLinkedToSceneData(offscreenBufferId, consumerScene, consumerId, ERendererEventResult_FAIL);

    dispatchEventsViaMockedHandler();
}

TEST_F(ARamsesRendererSceneControlLogic, willForwardDataLinkedEventFromRenderer)
{
    constexpr sceneId_t providerScene{ 1 };
    constexpr dataProviderId_t providerId{ 2 };
    constexpr sceneId_t consumerScene{ 3 };
    constexpr dataConsumerId_t consumerId{ 4 };

    InSequence seq;
    EXPECT_CALL(m_logicEventHandler, dataLinked(providerScene, providerId, consumerScene, consumerId, true));
    m_logicInput.dataLinked(providerScene, providerId, consumerScene, consumerId, ERendererEventResult_OK);

    EXPECT_CALL(m_logicEventHandler, dataLinked(providerScene, providerId, consumerScene, consumerId, false));
    m_logicInput.dataLinked(providerScene, providerId, consumerScene, consumerId, ERendererEventResult_FAIL);

    EXPECT_CALL(m_logicEventHandler, dataLinked(providerScene, providerId, consumerScene, consumerId, true));
    m_logicInput.dataLinked(providerScene, providerId, consumerScene, consumerId, ERendererEventResult_OK);
    EXPECT_CALL(m_logicEventHandler, dataLinked(providerScene, providerId, consumerScene, consumerId, false));
    m_logicInput.dataLinked(providerScene, providerId, consumerScene, consumerId, ERendererEventResult_FAIL);

    dispatchEventsViaMockedHandler();
}

TEST_F(ARamsesRendererSceneControlLogic, willForwardDataUnlinkedEventFromRenderer)
{
    constexpr sceneId_t consumerScene{ 3 };
    constexpr dataConsumerId_t consumerId{ 4 };

    InSequence seq;
    EXPECT_CALL(m_logicEventHandler, dataUnlinked(consumerScene, consumerId, true));
    m_logicInput.dataUnlinked(consumerScene, consumerId, ERendererEventResult_OK);

    EXPECT_CALL(m_logicEventHandler, dataUnlinked(consumerScene, consumerId, false));
    m_logicInput.dataUnlinked(consumerScene, consumerId, ERendererEventResult_FAIL);

    EXPECT_CALL(m_logicEventHandler, dataUnlinked(consumerScene, consumerId, true));
    m_logicInput.dataUnlinked(consumerScene, consumerId, ERendererEventResult_OK);
    EXPECT_CALL(m_logicEventHandler, dataUnlinked(consumerScene, consumerId, false));
    m_logicInput.dataUnlinked(consumerScene, consumerId, ERendererEventResult_FAIL);

    dispatchEventsViaMockedHandler();
}

TEST_F(ARamsesRendererSceneControlLogic, willForwardNamedFlushEventFromRenderer)
{
    constexpr sceneId_t scene{ 3 };
    constexpr sceneVersionTag_t tag{ 4 };

    EXPECT_CALL(m_logicEventHandler, sceneFlushed(scene, tag));
    m_logicInput.sceneFlushed(scene, tag, ESceneResourceStatus_Ready);

    dispatchEventsViaMockedHandler();
}

TEST_F(ARamsesRendererSceneControlLogic, willForwardSceneExpirationEventFromRenderer)
{
    constexpr sceneId_t scene{ 3 };

    InSequence seq;
    EXPECT_CALL(m_logicEventHandler, sceneExpired(scene));
    m_logicInput.sceneExpired(scene);

    EXPECT_CALL(m_logicEventHandler, sceneRecoveredFromExpiration(scene));
    m_logicInput.sceneRecoveredFromExpiration(scene);

    dispatchEventsViaMockedHandler();
}

TEST_F(ARamsesRendererSceneControlLogic, willForwardStreamAvailabilityEventFromRenderer)
{
    constexpr streamSource_t stream{ 3 };

    InSequence seq;
    EXPECT_CALL(m_logicEventHandler, streamAvailabilityChanged(stream, true));
    m_logicInput.streamAvailabilityChanged(stream, true);

    EXPECT_CALL(m_logicEventHandler, streamAvailabilityChanged(stream, false));
    m_logicInput.streamAvailabilityChanged(stream, false);

    dispatchEventsViaMockedHandler();
}
