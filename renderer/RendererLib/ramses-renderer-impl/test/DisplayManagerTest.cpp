//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gmock/gmock.h"
#include "DisplayManager/DisplayManager.h"
#include "ramses-framework-api/RamsesFramework.h"
#include "ramses-renderer-api/RamsesRenderer.h"
#include "ramses-renderer-api/RendererConfig.h"
#include "ramses-renderer-api/DisplayConfig.h"
#include "RendererLib/RendererCommandContainer.h"
#include "RendererLib/RendererCommands.h"
#include "RendererLib/RendererCommandTypes.h"
#include "RamsesRendererImpl.h"
#include "RendererSceneControlImpl_legacy.h"
#include "PlatformAbstraction/Macros.h"

using namespace ramses_internal;
using namespace testing;

class EventHandlerMock : public IEventHandler
{
public:
    MOCK_METHOD1(scenePublished, void(ramses::sceneId_t));
    MOCK_METHOD3(sceneStateChanged, void(ramses::sceneId_t, SceneState, ramses::displayId_t));
    MOCK_METHOD4(offscreenBufferLinked, void(ramses::displayBufferId_t, ramses::sceneId_t, ramses::dataConsumerId_t, bool));
    MOCK_METHOD5(dataLinked, void(ramses::sceneId_t, ramses::dataProviderId_t, ramses::sceneId_t, ramses::dataConsumerId_t, bool));
};

class ADisplayManager : public Test
{
public:
    ADisplayManager()
        : renderer(*ramsesFramework.createRenderer(ramses::RendererConfig()))
        , displayManager(renderer.impl, ramsesFramework.impl)
        , displayManagerRendererEventHandler(displayManager)
        , displayManagerEventHandler(displayManager)
        , sceneId(33u)
    {
        ramses::DisplayConfig displayConfig;
        displayConfig.setWaylandIviSurfaceID(0u);
        displayId = displayManager.createDisplay(displayConfig);

        displayConfig.setWaylandIviSurfaceID(1u);
        otherDisplayId = displayManager.createDisplay(displayConfig);

        // flush and loop to execute the creation
        displayManager.dispatchAndFlush(&eventHandlerMock);
        renderer.doOneLoop();
        EXPECT_EQ(2u, renderer.impl.getRenderer().getRenderer().getDisplayControllerCount());

        // dispatch to get the event of successful creation
        displayManager.dispatchAndFlush(&eventHandlerMock);
        EXPECT_TRUE(displayManager.isDisplayCreated(displayId));
        EXPECT_TRUE(displayManager.isDisplayCreated(otherDisplayId));

        displayManager.setSceneMapping(sceneId, displayId);

        displayFramebufferId = renderer.getDisplayFramebuffer(displayId);
        otherDisplayFramebufferId = renderer.getDisplayFramebuffer(otherDisplayId);
        offscreenBufferId = renderer.createOffscreenBuffer(displayId, 16, 16);
        // flush and loop to execute the creation
        renderer.flush();
        renderer.doOneLoop();
        // dispatch to get the event of successful creation
        displayManager.dispatchAndFlush(&eventHandlerMock);
    }

    ~ADisplayManager()
    {
        expectNoRendererCommand();
    }

protected:
    void expectRendererCommands(std::initializer_list<ramses_internal::ERendererCommand> expectedCmds)
    {
        const ramses_internal::RendererCommandContainer& cmds = renderer.impl.getPendingCommands().getCommands();
        ASSERT_EQ(expectedCmds.size(), cmds.getTotalCommandCount());

        uint32_t i = 0;
        for (const auto expectedCmd : expectedCmds)
            EXPECT_EQ(expectedCmd, cmds.getCommandType(i++));

        const_cast<ramses_internal::RendererCommands&>(renderer.impl.getPendingCommands()).clear();
    }

    void expectRendererSceneCommands(std::initializer_list<ramses_internal::ERendererCommand> expectedCmds)
    {
        const auto& cmds = renderer.getSceneControlAPI_legacy()->impl.getPendingCommands();
        ASSERT_EQ(expectedCmds.size(), cmds.getCommands().getTotalCommandCount());

        uint32_t i = 0;
        for (const auto expectedCmd : expectedCmds)
            EXPECT_EQ(expectedCmd, cmds.getCommands().getCommandType(i++));

        // to clear the commands
        renderer.getSceneControlAPI_legacy()->impl.flush();
    }

    void expectRendererCommand(ramses_internal::ERendererCommand expectedCmd)
    {
        expectRendererCommands({ expectedCmd });
    }

    void expectRendererSceneCommand(ramses_internal::ERendererCommand expectedCmd)
    {
        expectRendererSceneCommands({ expectedCmd });
    }

    void expectNoRendererCommand()
    {
        expectRendererSceneCommands({});
        expectRendererSceneCommands({});
    }

    template <typename COMMAND_TYPE>
    COMMAND_TYPE getRendererSceneCommand(uint32_t idx, ramses_internal::ERendererCommand expectedCmd)
    {
        const ramses_internal::RendererCommandContainer& cmds = renderer.getSceneControlAPI_legacy()->impl.getPendingCommands().getCommands();
        EXPECT_LT(idx, cmds.getTotalCommandCount());
        EXPECT_EQ(expectedCmd, cmds.getCommandType(idx));
        COMMAND_TYPE cmdData = cmds.getCommandData<COMMAND_TYPE>(idx);

        return cmdData;
    }

    void publishAndExpectToGetToState(SceneState state, bool expectConfirmation = false)
    {
        EXPECT_FALSE(isSceneShown());

        displayManagerEventHandler.scenePublished(sceneId);
        if (state != SceneState::Unavailable)
        {
            expectRendererSceneCommand(ramses_internal::ERendererCommand_SubscribeScene);
            displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_OK);

            if (state != SceneState::Available)
            {
                expectRendererSceneCommand(ramses_internal::ERendererCommand_MapSceneToDisplay);
                displayManagerEventHandler.sceneMapped(sceneId, ramses::ERendererEventResult_OK);

                if (state != SceneState::Ready)
                {
                    assert(state == SceneState::Rendered);
                    expectRendererSceneCommands({ ramses_internal::ERendererCommand_AssignSceneToDisplayBuffer, ramses_internal::ERendererCommand_ShowScene });
                    EXPECT_FALSE(isSceneShown());
                    displayManagerEventHandler.sceneShown(sceneId, ramses::ERendererEventResult_OK);
                    EXPECT_TRUE(isSceneShown());
                }
                else
                    expectRendererSceneCommand(ramses_internal::ERendererCommand_AssignSceneToDisplayBuffer);
            }
        }

        EXPECT_EQ(state, displayManager.getLastReportedSceneState(sceneId));
        if (expectConfirmation)
            expectRendererCommand(ramses_internal::ERendererCommand_ConfirmationEcho);
        else
            expectNoRendererCommand();
    }

    bool isSceneShown() const
    {
        return displayManager.getLastReportedSceneState(sceneId) == SceneState::Rendered;
    }

    void unpublish(ramses_internal::ERendererCommand lastSuccessfulCommand)
    {
        switch (lastSuccessfulCommand)
        {
        case ramses_internal::ERendererCommand_ShowScene:
            displayManagerEventHandler.sceneHidden(sceneId, ramses::ERendererEventResult_INDIRECT);
            RFALLTHROUGH;
        case ramses_internal::ERendererCommand_MapSceneToDisplay:
        case ramses_internal::ERendererCommand_HideScene:
            displayManagerEventHandler.sceneUnmapped(sceneId, ramses::ERendererEventResult_INDIRECT);
            RFALLTHROUGH;
        case ramses_internal::ERendererCommand_SubscribeScene:
        case ramses_internal::ERendererCommand_UnmapSceneFromDisplays:
            displayManagerEventHandler.sceneUnsubscribed(sceneId, ramses::ERendererEventResult_INDIRECT);
            RFALLTHROUGH;
        case ramses_internal::ERendererCommand_PublishedScene:
        case ramses_internal::ERendererCommand_UnsubscribeScene:
            displayManagerEventHandler.sceneUnpublished(sceneId);
            RFALLTHROUGH;
        case ramses_internal::ERendererCommand_UnpublishedScene:
            break;
        default:
            assert(false && "invalid starting state");
        }
    }

    void doAnotherFullCycleFromUnpublishToState(ramses_internal::ERendererCommand lastSuccessfulCommand = ramses_internal::ERendererCommand_ShowScene, SceneState state = SceneState::Rendered)
    {
        unpublish(lastSuccessfulCommand);
        publishAndExpectToGetToState(state);
    }

    void doAnotherFullCycleFromUnpublishToShownWithShowTriggered(ramses_internal::ERendererCommand lastSuccessfulCommand = ramses_internal::ERendererCommand_ShowScene)
    {
        unpublish(lastSuccessfulCommand);
        displayManager.setSceneState(sceneId, SceneState::Rendered);
        publishAndExpectToGetToState(SceneState::Rendered);
    }

    void doRenderLoop()
    {
        renderer.doOneLoop();
    }

private:
    ramses::RamsesFramework ramsesFramework;
    ramses::RamsesRenderer& renderer; // restrict access to renderer, acts only as dummy for DisplayManager and to create (dummy) displays

protected:
    DisplayManager displayManager;
    ramses::IRendererEventHandler& displayManagerRendererEventHandler;
    ramses::IRendererSceneControlEventHandler_legacy& displayManagerEventHandler;
    StrictMock<EventHandlerMock> eventHandlerMock;

    const ramses::sceneId_t sceneId;
    ramses::displayId_t displayId;
    ramses::displayId_t otherDisplayId;
    ramses::displayBufferId_t displayFramebufferId;
    ramses::displayBufferId_t otherDisplayFramebufferId;
    ramses::displayBufferId_t offscreenBufferId;
};

TEST_F(ADisplayManager, willShowSceneWhenPublished)
{
    displayManager.setSceneState(sceneId, SceneState::Rendered);
    expectNoRendererCommand();
    publishAndExpectToGetToState(SceneState::Rendered);
    EXPECT_EQ(displayId, displayManager.getDisplaySceneIsMappedTo(sceneId));

    doAnotherFullCycleFromUnpublishToState();
    doAnotherFullCycleFromUnpublishToShownWithShowTriggered();
}

TEST_F(ADisplayManager, willShowSceneWhenPublishedAndLogsConfirmation)
{
    displayManager.setSceneState(sceneId, SceneState::Rendered, "dummy msg");
    expectNoRendererCommand();
    publishAndExpectToGetToState(SceneState::Rendered, true);

    doAnotherFullCycleFromUnpublishToState();
    doAnotherFullCycleFromUnpublishToShownWithShowTriggered();
}

TEST_F(ADisplayManager, willLogConfirmationEvenIfSceneAlreadyRendered)
{
    displayManager.setSceneState(sceneId, SceneState::Rendered, ""); // no confirmation
    expectNoRendererCommand();
    publishAndExpectToGetToState(SceneState::Rendered);

    displayManager.setSceneState(sceneId, SceneState::Rendered, "confirmation"); // no confirmation
    expectRendererCommand(ramses_internal::ERendererCommand_ConfirmationEcho);

    doAnotherFullCycleFromUnpublishToState();
    doAnotherFullCycleFromUnpublishToShownWithShowTriggered();
}

TEST_F(ADisplayManager, willShowSceneAlreadyPublished)
{
    displayManagerEventHandler.scenePublished(sceneId);

    displayManager.setSceneState(sceneId, SceneState::Rendered);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_SubscribeScene);

    displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_OK);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_MapSceneToDisplay);

    displayManagerEventHandler.sceneMapped(sceneId, ramses::ERendererEventResult_OK);
    expectRendererSceneCommands({ ramses_internal::ERendererCommand_AssignSceneToDisplayBuffer, ramses_internal::ERendererCommand_ShowScene });

    displayManagerEventHandler.sceneShown(sceneId, ramses::ERendererEventResult_OK);
    expectNoRendererCommand();

    doAnotherFullCycleFromUnpublishToState();
    doAnotherFullCycleFromUnpublishToShownWithShowTriggered();
}

TEST_F(ADisplayManager, willRetryIfSubscribeFailed)
{
    displayManagerEventHandler.scenePublished(sceneId);

    displayManager.setSceneState(sceneId, SceneState::Available);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_SubscribeScene);

    displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_FAIL);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_SubscribeScene);
    displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_OK);
    expectNoRendererCommand();

    doAnotherFullCycleFromUnpublishToState(ramses_internal::ERendererCommand_SubscribeScene, SceneState::Available);
    doAnotherFullCycleFromUnpublishToShownWithShowTriggered();
}

TEST_F(ADisplayManager, willRetryIfMapFailed)
{
    displayManagerEventHandler.scenePublished(sceneId);

    displayManager.setSceneState(sceneId, SceneState::Ready);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_SubscribeScene);

    displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_OK);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_MapSceneToDisplay);

    displayManagerEventHandler.sceneMapped(sceneId, ramses::ERendererEventResult_FAIL);
    EXPECT_EQ(ramses::displayId_t::Invalid(), displayManager.getDisplaySceneIsMappedTo(sceneId));
    expectRendererSceneCommand(ramses_internal::ERendererCommand_MapSceneToDisplay);
    displayManagerEventHandler.sceneMapped(sceneId, ramses::ERendererEventResult_OK);
    EXPECT_EQ(displayId, displayManager.getDisplaySceneIsMappedTo(sceneId));
    expectRendererSceneCommand(ramses_internal::ERendererCommand_AssignSceneToDisplayBuffer);

    doAnotherFullCycleFromUnpublishToState(ramses_internal::ERendererCommand_SubscribeScene, SceneState::Ready);
    doAnotherFullCycleFromUnpublishToShownWithShowTriggered();
}

TEST_F(ADisplayManager, willRetryIfShowFailed)
{
    displayManagerEventHandler.scenePublished(sceneId);

    displayManager.setSceneState(sceneId, SceneState::Rendered);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_SubscribeScene);

    displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_OK);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_MapSceneToDisplay);

    displayManagerEventHandler.sceneMapped(sceneId, ramses::ERendererEventResult_OK);
    expectRendererSceneCommands({ ramses_internal::ERendererCommand_AssignSceneToDisplayBuffer, ramses_internal::ERendererCommand_ShowScene });

    displayManagerEventHandler.sceneShown(sceneId, ramses::ERendererEventResult_FAIL);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_ShowScene);
    displayManagerEventHandler.sceneShown(sceneId, ramses::ERendererEventResult_OK);
    expectNoRendererCommand();

    doAnotherFullCycleFromUnpublishToState(ramses_internal::ERendererCommand_MapSceneToDisplay);
    doAnotherFullCycleFromUnpublishToShownWithShowTriggered();
}

TEST_F(ADisplayManager, willRetryIfHideFailed)
{
    displayManager.setSceneState(sceneId, SceneState::Rendered);
    publishAndExpectToGetToState(SceneState::Rendered);

    displayManager.setSceneState(sceneId, SceneState::Ready);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_HideScene);
    displayManagerEventHandler.sceneHidden(sceneId, ramses::ERendererEventResult_FAIL);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_HideScene);
    displayManagerEventHandler.sceneHidden(sceneId, ramses::ERendererEventResult_OK);
    expectNoRendererCommand();

    doAnotherFullCycleFromUnpublishToState(ramses_internal::ERendererCommand_HideScene, SceneState::Ready);
    doAnotherFullCycleFromUnpublishToShownWithShowTriggered();
}

TEST_F(ADisplayManager, willRetryIfUnmapFailed)
{
    displayManager.setSceneState(sceneId, SceneState::Ready);
    publishAndExpectToGetToState(SceneState::Ready);

    displayManager.setSceneState(sceneId, SceneState::Available);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_UnmapSceneFromDisplays);
    displayManagerEventHandler.sceneUnmapped(sceneId, ramses::ERendererEventResult_FAIL);
    EXPECT_EQ(displayId, displayManager.getDisplaySceneIsMappedTo(sceneId));
    expectRendererSceneCommand(ramses_internal::ERendererCommand_UnmapSceneFromDisplays);
    displayManagerEventHandler.sceneUnmapped(sceneId, ramses::ERendererEventResult_OK);
    EXPECT_EQ(ramses::displayId_t::Invalid(), displayManager.getDisplaySceneIsMappedTo(sceneId));
    expectNoRendererCommand();

    doAnotherFullCycleFromUnpublishToState(ramses_internal::ERendererCommand_UnmapSceneFromDisplays, SceneState::Available);
    doAnotherFullCycleFromUnpublishToShownWithShowTriggered();
}

TEST_F(ADisplayManager, willRetryIfUnsubscribeFailed)
{
    displayManager.setSceneState(sceneId, SceneState::Available);
    publishAndExpectToGetToState(SceneState::Available);

    displayManager.setSceneState(sceneId, SceneState::Unavailable);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_UnsubscribeScene);
    displayManagerEventHandler.sceneUnsubscribed(sceneId, ramses::ERendererEventResult_FAIL);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_UnsubscribeScene);
    displayManagerEventHandler.sceneUnsubscribed(sceneId, ramses::ERendererEventResult_OK);
    expectNoRendererCommand();

    doAnotherFullCycleFromUnpublishToState(ramses_internal::ERendererCommand_UnsubscribeScene, SceneState::Unavailable);
    doAnotherFullCycleFromUnpublishToShownWithShowTriggered();
}

TEST_F(ADisplayManager, reportsDisplaySceneIsMappedToOnlyMapped)
{
    displayManager.setSceneState(sceneId, SceneState::Rendered);

    displayManagerEventHandler.scenePublished(sceneId);
    EXPECT_EQ(ramses::displayId_t::Invalid(), displayManager.getDisplaySceneIsMappedTo(sceneId));
    expectRendererSceneCommand(ramses_internal::ERendererCommand_SubscribeScene);

    displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_OK);
    EXPECT_EQ(ramses::displayId_t::Invalid(), displayManager.getDisplaySceneIsMappedTo(sceneId));
    expectRendererSceneCommand(ramses_internal::ERendererCommand_MapSceneToDisplay);

    displayManagerEventHandler.sceneMapped(sceneId, ramses::ERendererEventResult_OK);
    EXPECT_EQ(displayId, displayManager.getDisplaySceneIsMappedTo(sceneId));
    expectRendererSceneCommands({ ramses_internal::ERendererCommand_AssignSceneToDisplayBuffer, ramses_internal::ERendererCommand_ShowScene });

    EXPECT_FALSE(isSceneShown());
    displayManagerEventHandler.sceneShown(sceneId, ramses::ERendererEventResult_OK);
    EXPECT_EQ(displayId, displayManager.getDisplaySceneIsMappedTo(sceneId));
    EXPECT_TRUE(isSceneShown());

    doAnotherFullCycleFromUnpublishToState();
    doAnotherFullCycleFromUnpublishToShownWithShowTriggered();
}

TEST_F(ADisplayManager, reportsInvalidDisplaySceneIsMappedToAfterUnmapped)
{
    displayManager.setSceneState(sceneId, SceneState::Rendered);

    displayManagerEventHandler.scenePublished(sceneId);
    EXPECT_EQ(ramses::displayId_t::Invalid(), displayManager.getDisplaySceneIsMappedTo(sceneId));
    expectRendererSceneCommand(ramses_internal::ERendererCommand_SubscribeScene);

    displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_OK);
    EXPECT_EQ(ramses::displayId_t::Invalid(), displayManager.getDisplaySceneIsMappedTo(sceneId));
    expectRendererSceneCommand(ramses_internal::ERendererCommand_MapSceneToDisplay);

    displayManagerEventHandler.sceneMapped(sceneId, ramses::ERendererEventResult_OK);
    EXPECT_EQ(displayId, displayManager.getDisplaySceneIsMappedTo(sceneId));
    expectRendererSceneCommands({ ramses_internal::ERendererCommand_AssignSceneToDisplayBuffer, ramses_internal::ERendererCommand_ShowScene });

    EXPECT_FALSE(isSceneShown());
    displayManagerEventHandler.sceneShown(sceneId, ramses::ERendererEventResult_OK);
    EXPECT_EQ(displayId, displayManager.getDisplaySceneIsMappedTo(sceneId));
    EXPECT_TRUE(isSceneShown());

    displayManager.setSceneState(sceneId, SceneState::Available);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_HideScene);

    displayManagerEventHandler.sceneHidden(sceneId, ramses::ERendererEventResult_OK);
    EXPECT_EQ(displayId, displayManager.getDisplaySceneIsMappedTo(sceneId));
    EXPECT_FALSE(isSceneShown());
    expectRendererSceneCommand(ramses_internal::ERendererCommand_UnmapSceneFromDisplays);

    displayManagerEventHandler.sceneUnmapped(sceneId, ramses::ERendererEventResult_OK);
    EXPECT_EQ(ramses::displayId_t::Invalid(), displayManager.getDisplaySceneIsMappedTo(sceneId));
    EXPECT_FALSE(isSceneShown());
    expectNoRendererCommand();

    doAnotherFullCycleFromUnpublishToState(ramses_internal::ERendererCommand_UnmapSceneFromDisplays, SceneState::Available);
    doAnotherFullCycleFromUnpublishToShownWithShowTriggered(ramses_internal::ERendererCommand_SubscribeScene);
}

TEST_F(ADisplayManager, hidesShownScene)
{
    displayManager.setSceneState(sceneId, SceneState::Rendered);
    expectNoRendererCommand();
    publishAndExpectToGetToState(SceneState::Rendered);

    displayManager.setSceneState(sceneId, SceneState::Ready);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_HideScene);

    displayManagerEventHandler.sceneHidden(sceneId, ramses::ERendererEventResult_OK);
    expectNoRendererCommand();
    EXPECT_EQ(displayId, displayManager.getDisplaySceneIsMappedTo(sceneId));

    doAnotherFullCycleFromUnpublishToState(ramses_internal::ERendererCommand_HideScene, SceneState::Ready);
    doAnotherFullCycleFromUnpublishToShownWithShowTriggered(ramses_internal::ERendererCommand_MapSceneToDisplay);
}

TEST_F(ADisplayManager, unmapsShownScene)
{
    displayManager.setSceneState(sceneId, SceneState::Rendered);
    expectNoRendererCommand();
    publishAndExpectToGetToState(SceneState::Rendered);

    displayManager.setSceneState(sceneId, SceneState::Available);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_HideScene);

    displayManagerEventHandler.sceneHidden(sceneId, ramses::ERendererEventResult_OK);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_UnmapSceneFromDisplays);
    EXPECT_EQ(displayId, displayManager.getDisplaySceneIsMappedTo(sceneId));

    displayManagerEventHandler.sceneUnmapped(sceneId, ramses::ERendererEventResult_OK);
    expectNoRendererCommand();
    EXPECT_EQ(ramses::displayId_t::Invalid(), displayManager.getDisplaySceneIsMappedTo(sceneId));

    doAnotherFullCycleFromUnpublishToState(ramses_internal::ERendererCommand_UnmapSceneFromDisplays, SceneState::Available);
    doAnotherFullCycleFromUnpublishToShownWithShowTriggered(ramses_internal::ERendererCommand_SubscribeScene);
}

TEST_F(ADisplayManager, unmapsMappedScene)
{
    displayManager.setSceneState(sceneId, SceneState::Rendered);
    expectNoRendererCommand();
    publishAndExpectToGetToState(SceneState::Rendered);

    // hide first
    displayManager.setSceneState(sceneId, SceneState::Ready);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_HideScene);
    displayManagerEventHandler.sceneHidden(sceneId, ramses::ERendererEventResult_OK);
    expectNoRendererCommand();

    displayManager.setSceneState(sceneId, SceneState::Available);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_UnmapSceneFromDisplays);

    displayManagerEventHandler.sceneUnmapped(sceneId, ramses::ERendererEventResult_OK);
    expectNoRendererCommand();
    EXPECT_EQ(ramses::displayId_t::Invalid(), displayManager.getDisplaySceneIsMappedTo(sceneId));

    doAnotherFullCycleFromUnpublishToState(ramses_internal::ERendererCommand_UnmapSceneFromDisplays, SceneState::Available);
    doAnotherFullCycleFromUnpublishToShownWithShowTriggered(ramses_internal::ERendererCommand_SubscribeScene);
}

TEST_F(ADisplayManager, unsubscribesShownScene)
{
    displayManager.setSceneState(sceneId, SceneState::Rendered);
    expectNoRendererCommand();
    publishAndExpectToGetToState(SceneState::Rendered);

    displayManager.setSceneState(sceneId, SceneState::Unavailable);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_HideScene);

    displayManagerEventHandler.sceneHidden(sceneId, ramses::ERendererEventResult_OK);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_UnmapSceneFromDisplays);

    displayManagerEventHandler.sceneUnmapped(sceneId, ramses::ERendererEventResult_OK);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_UnsubscribeScene);

    displayManagerEventHandler.sceneUnsubscribed(sceneId, ramses::ERendererEventResult_OK);
    expectNoRendererCommand();
}

TEST_F(ADisplayManager, unsubscribesMappedScene)
{
    displayManager.setSceneState(sceneId, SceneState::Rendered);
    expectNoRendererCommand();
    publishAndExpectToGetToState(SceneState::Rendered);

    // hide first
    displayManager.setSceneState(sceneId, SceneState::Ready);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_HideScene);
    displayManagerEventHandler.sceneHidden(sceneId, ramses::ERendererEventResult_OK);
    expectNoRendererCommand();

    displayManager.setSceneState(sceneId, SceneState::Unavailable);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_UnmapSceneFromDisplays);

    displayManagerEventHandler.sceneUnmapped(sceneId, ramses::ERendererEventResult_OK);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_UnsubscribeScene);

    displayManagerEventHandler.sceneUnsubscribed(sceneId, ramses::ERendererEventResult_OK);
    expectNoRendererCommand();
}

TEST_F(ADisplayManager, unsubscribesSubscribedScene)
{
    displayManager.setSceneState(sceneId, SceneState::Rendered);
    expectNoRendererCommand();
    publishAndExpectToGetToState(SceneState::Rendered);

    // unmap first
    displayManager.setSceneState(sceneId, SceneState::Available);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_HideScene);
    displayManagerEventHandler.sceneHidden(sceneId, ramses::ERendererEventResult_OK);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_UnmapSceneFromDisplays);
    displayManagerEventHandler.sceneUnmapped(sceneId, ramses::ERendererEventResult_OK);
    expectNoRendererCommand();

    displayManager.setSceneState(sceneId, SceneState::Unavailable);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_UnsubscribeScene);

    displayManagerEventHandler.sceneUnsubscribed(sceneId, ramses::ERendererEventResult_OK);
    expectNoRendererCommand();
}

TEST_F(ADisplayManager, handlesSceneUnpublishWhileProcessingSubscribe)
{
    displayManager.setSceneState(sceneId, SceneState::Rendered);
    expectNoRendererCommand();

    displayManagerEventHandler.scenePublished(sceneId);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_SubscribeScene);

    //unexpected unpublish!!
    unpublish(ramses_internal::ERendererCommand_PublishedScene);
    displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_FAIL);

    doAnotherFullCycleFromUnpublishToState(ramses_internal::ERendererCommand_UnpublishedScene);
}

TEST_F(ADisplayManager, handlesSceneUnpublishWhileProcessingMap)
{
    displayManager.setSceneState(sceneId, SceneState::Rendered);
    expectNoRendererCommand();

    displayManagerEventHandler.scenePublished(sceneId);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_SubscribeScene);

    displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_OK);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_MapSceneToDisplay);

    //unexpected unpublish!!
    unpublish(ramses_internal::ERendererCommand_SubscribeScene);
    displayManagerEventHandler.sceneMapped(sceneId, ramses::ERendererEventResult_FAIL);

    doAnotherFullCycleFromUnpublishToState(ramses_internal::ERendererCommand_SubscribeScene);
}

TEST_F(ADisplayManager, handlesSceneUnpublishWhileProcessingShow)
{
    displayManager.setSceneState(sceneId, SceneState::Rendered);
    expectNoRendererCommand();

    displayManagerEventHandler.scenePublished(sceneId);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_SubscribeScene);

    displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_OK);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_MapSceneToDisplay);

    displayManagerEventHandler.sceneMapped(sceneId, ramses::ERendererEventResult_OK);
    expectRendererSceneCommands({ ramses_internal::ERendererCommand_AssignSceneToDisplayBuffer, ramses_internal::ERendererCommand_ShowScene });

    //unexpected unpublish!!
    unpublish(ramses_internal::ERendererCommand_MapSceneToDisplay);
    displayManagerEventHandler.sceneShown(sceneId, ramses::ERendererEventResult_FAIL);

    doAnotherFullCycleFromUnpublishToState(ramses_internal::ERendererCommand_MapSceneToDisplay);
}

TEST_F(ADisplayManager, failsToMakeSceneReadyOrRenderedIfNoMappingSet)
{
    const ramses::sceneId_t sceneWithNoMapping{ 1234 };
    EXPECT_FALSE(displayManager.setSceneState(sceneWithNoMapping, SceneState::Ready));
    expectNoRendererCommand();
    EXPECT_FALSE(displayManager.setSceneState(sceneWithNoMapping, SceneState::Rendered));
    expectNoRendererCommand();
}

TEST_F(ADisplayManager, failsToChangeMappingPropertiesIfSceneAlreadyReadyOrRendered)
{
    displayManager.setSceneState(sceneId, SceneState::Ready);
    publishAndExpectToGetToState(SceneState::Ready);

    // scene READY, cannot change mapping
    EXPECT_FALSE(displayManager.setSceneMapping(sceneId, otherDisplayId));
    expectNoRendererCommand();
    EXPECT_EQ(displayId, displayManager.getDisplaySceneIsMappedTo(sceneId));

    displayManager.setSceneState(sceneId, SceneState::Rendered);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_ShowScene);
    displayManagerEventHandler.sceneShown(sceneId, ramses::ERendererEventResult_OK);
    expectNoRendererCommand();
    EXPECT_EQ(displayId, displayManager.getDisplaySceneIsMappedTo(sceneId));

    // scene RENDERED, cannot change mapping
    EXPECT_FALSE(displayManager.setSceneMapping(sceneId, otherDisplayId));
    expectNoRendererCommand();
    EXPECT_EQ(displayId, displayManager.getDisplaySceneIsMappedTo(sceneId));

    doAnotherFullCycleFromUnpublishToState(ramses_internal::ERendererCommand_ShowScene);
    EXPECT_EQ(displayId, displayManager.getDisplaySceneIsMappedTo(sceneId));
}

TEST_F(ADisplayManager, failsToChangeMappingPropertiesIfSceneAlreadySetToReadyOrRendered)
{
    displayManager.setSceneState(sceneId, SceneState::Rendered);
    expectNoRendererCommand();
    EXPECT_FALSE(displayManager.setSceneMapping(sceneId, otherDisplayId));
    expectNoRendererCommand();
    EXPECT_EQ(ramses::displayId_t::Invalid(), displayManager.getDisplaySceneIsMappedTo(sceneId));

    displayManager.setSceneState(sceneId, SceneState::Ready);
    expectNoRendererCommand();
    EXPECT_FALSE(displayManager.setSceneMapping(sceneId, otherDisplayId));
    expectNoRendererCommand();
    EXPECT_EQ(ramses::displayId_t::Invalid(), displayManager.getDisplaySceneIsMappedTo(sceneId));

    displayManager.setSceneState(sceneId, SceneState::Rendered);
    publishAndExpectToGetToState(SceneState::Rendered);
    EXPECT_EQ(displayId, displayManager.getDisplaySceneIsMappedTo(sceneId));

    doAnotherFullCycleFromUnpublishToState(ramses_internal::ERendererCommand_ShowScene);
    EXPECT_EQ(displayId, displayManager.getDisplaySceneIsMappedTo(sceneId));
}

TEST_F(ADisplayManager, canChangeMappingPropertiesWhenSceneAvailableAndNotSetToReadyOrRenderedYet)
{
    displayManager.setSceneState(sceneId, SceneState::Available);
    publishAndExpectToGetToState(SceneState::Available);

    EXPECT_TRUE(displayManager.setSceneMapping(sceneId, otherDisplayId));
    expectNoRendererCommand();
    EXPECT_EQ(ramses::displayId_t::Invalid(), displayManager.getDisplaySceneIsMappedTo(sceneId));

    displayManager.setSceneState(sceneId, SceneState::Ready);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_MapSceneToDisplay);
    displayManagerEventHandler.sceneMapped(sceneId, ramses::ERendererEventResult_OK);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_AssignSceneToDisplayBuffer);
    EXPECT_EQ(otherDisplayId, displayManager.getDisplaySceneIsMappedTo(sceneId));

    doAnotherFullCycleFromUnpublishToState(ramses_internal::ERendererCommand_MapSceneToDisplay, SceneState::Ready);
    EXPECT_EQ(otherDisplayId, displayManager.getDisplaySceneIsMappedTo(sceneId));
}

TEST_F(ADisplayManager, canChangeMappingPropertiesForReadySceneAfterGettingItToAvailableFirst)
{
    displayManager.setSceneState(sceneId, SceneState::Ready);
    publishAndExpectToGetToState(SceneState::Ready);
    EXPECT_EQ(displayId, displayManager.getDisplaySceneIsMappedTo(sceneId));

    // cannot remap in READY state
    EXPECT_FALSE(displayManager.setSceneMapping(sceneId, otherDisplayId));
    expectNoRendererCommand();
    EXPECT_EQ(displayId, displayManager.getDisplaySceneIsMappedTo(sceneId));

    displayManager.setSceneState(sceneId, SceneState::Available);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_UnmapSceneFromDisplays);
    displayManagerEventHandler.sceneUnmapped(sceneId, ramses::ERendererEventResult_OK);
    expectNoRendererCommand();
    EXPECT_EQ(ramses::displayId_t::Invalid(), displayManager.getDisplaySceneIsMappedTo(sceneId));

    // now can change mapping
    EXPECT_TRUE(displayManager.setSceneMapping(sceneId, otherDisplayId));
    expectNoRendererCommand();
    EXPECT_EQ(ramses::displayId_t::Invalid(), displayManager.getDisplaySceneIsMappedTo(sceneId));

    displayManager.setSceneState(sceneId, SceneState::Ready);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_MapSceneToDisplay);
    displayManagerEventHandler.sceneMapped(sceneId, ramses::ERendererEventResult_OK);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_AssignSceneToDisplayBuffer);
    EXPECT_EQ(otherDisplayId, displayManager.getDisplaySceneIsMappedTo(sceneId));

    doAnotherFullCycleFromUnpublishToState(ramses_internal::ERendererCommand_MapSceneToDisplay, SceneState::Ready);
    EXPECT_EQ(otherDisplayId, displayManager.getDisplaySceneIsMappedTo(sceneId));
}

TEST_F(ADisplayManager, canShowASubscribedSceneOnAnotherDisplay)
{
    displayManager.setSceneState(sceneId, SceneState::Rendered);
    expectNoRendererCommand();
    publishAndExpectToGetToState(SceneState::Rendered);

    // unmap first
    displayManager.setSceneState(sceneId, SceneState::Available);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_HideScene);
    displayManagerEventHandler.sceneHidden(sceneId, ramses::ERendererEventResult_OK);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_UnmapSceneFromDisplays);
    displayManagerEventHandler.sceneUnmapped(sceneId, ramses::ERendererEventResult_OK);
    expectNoRendererCommand();

    EXPECT_TRUE(displayManager.setSceneMapping(sceneId, otherDisplayId));
    displayManager.setSceneState(sceneId, SceneState::Rendered);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_MapSceneToDisplay);
    displayManagerEventHandler.sceneMapped(sceneId, ramses::ERendererEventResult_OK);
    expectRendererSceneCommands({ ramses_internal::ERendererCommand_AssignSceneToDisplayBuffer, ramses_internal::ERendererCommand_ShowScene });
    displayManagerEventHandler.sceneShown(sceneId, ramses::ERendererEventResult_OK);
    expectNoRendererCommand();

    doAnotherFullCycleFromUnpublishToState(ramses_internal::ERendererCommand_ShowScene);
}

TEST_F(ADisplayManager, forwardsCommandForDisplayFrameBufferClearColorWithCorrectDisplayHandle)
{
    EXPECT_TRUE(displayManager.setDisplayBufferClearColor(otherDisplayFramebufferId, 1, 2, 3, 4));
    const auto cmd = getRendererSceneCommand<ramses_internal::SetClearColorCommand>(0u, ramses_internal::ERendererCommand_SetClearColor);
    EXPECT_EQ(otherDisplayId.getValue(), cmd.displayHandle.asMemoryHandle());
    EXPECT_FALSE(cmd.obHandle.isValid());
    EXPECT_EQ(ramses_internal::Vector4(1, 2, 3, 4), cmd.clearColor);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_SetClearColor);
}

TEST_F(ADisplayManager, forwardsCommandForDisplayOffscreenBufferClearColorWithCorrectDisplayHandle)
{
    EXPECT_TRUE(displayManager.setDisplayBufferClearColor(offscreenBufferId, 1, 2, 3, 4));
    const auto cmd = getRendererSceneCommand<ramses_internal::SetClearColorCommand>(0u, ramses_internal::ERendererCommand_SetClearColor);
    EXPECT_EQ(displayId.getValue(), cmd.displayHandle.asMemoryHandle());
    EXPECT_EQ(offscreenBufferId.getValue(), cmd.obHandle.asMemoryHandle());
    EXPECT_EQ(ramses_internal::Vector4(1, 2, 3, 4), cmd.clearColor);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_SetClearColor);
}

TEST_F(ADisplayManager, failsToSetDisplayBufferClearColorForUnknownBuffer)
{
    EXPECT_FALSE(displayManager.setDisplayBufferClearColor(ramses::displayBufferId_t{ 999 }, 1, 2, 3, 4));
    expectNoRendererCommand();
}

TEST_F(ADisplayManager, forwardsCommandForOffscreenBufferLink)
{
    const ramses::sceneId_t consumerSceneId(3u);
    const ramses::dataConsumerId_t consumerId(4u);

    displayManager.linkOffscreenBuffer(offscreenBufferId, consumerSceneId, consumerId);
    const auto cmd = getRendererSceneCommand<ramses_internal::DataLinkCommand>(0u, ramses_internal::ERendererCommand_LinkBufferToSceneData);
    EXPECT_EQ(offscreenBufferId.getValue(), cmd.providerBuffer.asMemoryHandle());
    EXPECT_EQ(consumerSceneId, ramses::sceneId_t(cmd.consumerScene.getValue()));
    EXPECT_EQ(consumerId.getValue(), cmd.consumerData.getValue());
    expectRendererSceneCommand(ramses_internal::ERendererCommand_LinkBufferToSceneData);
}

TEST_F(ADisplayManager, forwardsCommandForDataLink)
{
    const ramses::sceneId_t providerSceneId(1u);
    const ramses::dataProviderId_t providerId(2u);
    const ramses::sceneId_t consumerSceneId(3u);
    const ramses::dataConsumerId_t consumerId(4u);

    displayManager.linkData(providerSceneId, providerId, consumerSceneId, consumerId);
    const auto cmd = getRendererSceneCommand<ramses_internal::DataLinkCommand>(0u, ramses_internal::ERendererCommand_LinkSceneData);
    EXPECT_EQ(providerSceneId, ramses::sceneId_t(cmd.providerScene.getValue()));
    EXPECT_EQ(providerId.getValue(), cmd.providerData.getValue());
    EXPECT_EQ(consumerSceneId, ramses::sceneId_t(cmd.consumerScene.getValue()));
    EXPECT_EQ(consumerId.getValue(), cmd.consumerData.getValue());
    expectRendererSceneCommand(ramses_internal::ERendererCommand_LinkSceneData);
}

TEST_F(ADisplayManager, reportsCorrectSceneShowState)
{
    // show first time
    displayManager.setSceneState(sceneId, SceneState::Rendered);
    expectNoRendererCommand();
    publishAndExpectToGetToState(SceneState::Rendered);

    // explicitly hide
    displayManager.setSceneState(sceneId, SceneState::Ready);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_HideScene);
    EXPECT_TRUE(isSceneShown()); // hide command not processed yet
    displayManagerEventHandler.sceneHidden(sceneId, ramses::ERendererEventResult_OK);
    EXPECT_FALSE(isSceneShown());

    // show again
    displayManager.setSceneState(sceneId, SceneState::Rendered);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_ShowScene);
    EXPECT_FALSE(isSceneShown()); // show command not processed yet
    displayManagerEventHandler.sceneShown(sceneId, ramses::ERendererEventResult_OK);
    EXPECT_TRUE(isSceneShown());

    // unexpected unpublish!!
    unpublish(ramses_internal::ERendererCommand_ShowScene);
    EXPECT_FALSE(isSceneShown());

    doAnotherFullCycleFromUnpublishToState(ramses_internal::ERendererCommand_UnpublishedScene);
}

TEST_F(ADisplayManager, reportsCorrectDisplayCreationState)
{
    ramses::DisplayConfig displayConfig;
    ramses::displayId_t customDisplay = displayManager.createDisplay(displayConfig);
    displayManager.dispatchAndFlush(&eventHandlerMock);

    EXPECT_FALSE(displayManager.isDisplayCreated(customDisplay));
    doRenderLoop();
    displayManager.dispatchAndFlush(&eventHandlerMock);

    EXPECT_TRUE(displayManager.isDisplayCreated(customDisplay));

    displayManager.destroyDisplay(customDisplay);
    displayManager.dispatchAndFlush(&eventHandlerMock);
    EXPECT_TRUE(displayManager.isDisplayCreated(customDisplay));

    doRenderLoop();
    displayManager.dispatchAndFlush(&eventHandlerMock);

    EXPECT_FALSE(displayManager.isDisplayCreated(customDisplay));
}

TEST_F(ADisplayManager, sceneWillBeShownWhenDisplayIsCreated)
{
    ramses::displayId_t customDisplay(120u);

    EXPECT_TRUE(displayManager.setSceneMapping(sceneId, customDisplay));
    displayManager.setSceneState(sceneId, SceneState::Rendered);
    expectNoRendererCommand();

    displayManagerEventHandler.scenePublished(sceneId);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_SubscribeScene);

    displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_OK);
    expectNoRendererCommand(); // do not map, because display is not yet created

    displayManagerRendererEventHandler.displayCreated(customDisplay, ramses::ERendererEventResult_OK);

    // now request map
    expectRendererSceneCommand(ramses_internal::ERendererCommand_MapSceneToDisplay);

    displayManagerEventHandler.sceneMapped(sceneId, ramses::ERendererEventResult_OK);
    expectRendererSceneCommands({ ramses_internal::ERendererCommand_AssignSceneToDisplayBuffer, ramses_internal::ERendererCommand_ShowScene });

    displayManagerEventHandler.sceneShown(sceneId, ramses::ERendererEventResult_OK);

    doAnotherFullCycleFromUnpublishToState(ramses_internal::ERendererCommand_ShowScene);
}

TEST_F(ADisplayManager, continuesToShowSceneAndLogsConfirmationAfterReconnect)
{
    displayManagerEventHandler.scenePublished(sceneId);
    expectNoRendererCommand();

    displayManager.setSceneState(sceneId, SceneState::Rendered, "scene is shown now");
    expectRendererSceneCommand(ramses_internal::ERendererCommand_SubscribeScene);

    displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_OK);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_MapSceneToDisplay);

    // simulate disconnect, renderer framework sends unpublish for all its scenes
    unpublish(ramses_internal::ERendererCommand_SubscribeScene);
    expectNoRendererCommand();
    // after unpublish, previous command will fail on execution
    displayManagerEventHandler.sceneMapped(sceneId, ramses::ERendererEventResult_FAIL);

    publishAndExpectToGetToState(SceneState::Rendered, true);

    doAnotherFullCycleFromUnpublishToState();
}

/////////
// Tests where unpublish comes after already failed last command on renderer side
// and DM is supposed to get scene to shown state
/////////

TEST_F(ADisplayManager, recoversFromRepublishAfterFailedSubscribe)
{
    displayManager.setSceneState(sceneId, SceneState::Rendered, "scene is shown now");

    displayManagerEventHandler.scenePublished(sceneId);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_SubscribeScene);
    unpublish(ramses_internal::ERendererCommand_PublishedScene);
    expectNoRendererCommand();
    displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_FAIL);

    publishAndExpectToGetToState(SceneState::Rendered, true);
}

TEST_F(ADisplayManager, recoversFromRepublishAfterFailedMap)
{
    displayManager.setSceneState(sceneId, SceneState::Rendered, "scene is shown now");

    displayManagerEventHandler.scenePublished(sceneId);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_SubscribeScene);
    displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_OK);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_MapSceneToDisplay);

    unpublish(ramses_internal::ERendererCommand_SubscribeScene);
    expectNoRendererCommand();
    displayManagerEventHandler.sceneMapped(sceneId, ramses::ERendererEventResult_FAIL);

    publishAndExpectToGetToState(SceneState::Rendered, true);
}

TEST_F(ADisplayManager, recoversFromRepublishAfterFailedShow)
{
    displayManager.setSceneState(sceneId, SceneState::Rendered, "scene is shown now");

    displayManagerEventHandler.scenePublished(sceneId);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_SubscribeScene);
    displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_OK);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_MapSceneToDisplay);
    displayManagerEventHandler.sceneMapped(sceneId, ramses::ERendererEventResult_OK);
    expectRendererSceneCommands({ ramses_internal::ERendererCommand_AssignSceneToDisplayBuffer, ramses_internal::ERendererCommand_ShowScene });

    unpublish(ramses_internal::ERendererCommand_MapSceneToDisplay);
    expectNoRendererCommand();
    displayManagerEventHandler.sceneShown(sceneId, ramses::ERendererEventResult_FAIL);

    publishAndExpectToGetToState(SceneState::Rendered, true);
}

/////////
// Tests where unpublish comes before failed last command on renderer side
// and DM is supposed to get scene to shown state
/////////

TEST_F(ADisplayManager, recoversFromRepublishBeforeFailedSubscribe)
{
    displayManager.setSceneState(sceneId, SceneState::Rendered, "scene is shown now");

    displayManagerEventHandler.scenePublished(sceneId);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_SubscribeScene);

    unpublish(ramses_internal::ERendererCommand_PublishedScene);
    expectNoRendererCommand();
    displayManagerEventHandler.scenePublished(sceneId);
    expectNoRendererCommand(); // not trying to subscribe because last subscribe not answered yet

    displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_FAIL);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_SubscribeScene); // now try subscribe again
    displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_OK);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_MapSceneToDisplay);
    displayManagerEventHandler.sceneMapped(sceneId, ramses::ERendererEventResult_OK);
    expectRendererSceneCommands({ ramses_internal::ERendererCommand_AssignSceneToDisplayBuffer, ramses_internal::ERendererCommand_ShowScene });
    displayManagerEventHandler.sceneShown(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_ConfirmationEcho);

    doAnotherFullCycleFromUnpublishToState();
}

TEST_F(ADisplayManager, recoversFromRepublishBeforeFailedMap)
{
    displayManager.setSceneState(sceneId, SceneState::Rendered, "scene is shown now");

    displayManagerEventHandler.scenePublished(sceneId);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_SubscribeScene);
    displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_OK);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_MapSceneToDisplay);

    unpublish(ramses_internal::ERendererCommand_SubscribeScene);
    expectNoRendererCommand();
    displayManagerEventHandler.scenePublished(sceneId);
    expectNoRendererCommand(); // not trying to subscribe because last command not answered yet

    displayManagerEventHandler.sceneMapped(sceneId, ramses::ERendererEventResult_FAIL);

    expectRendererSceneCommand(ramses_internal::ERendererCommand_SubscribeScene); // now start again from subscribe
    displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_OK);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_MapSceneToDisplay);
    displayManagerEventHandler.sceneMapped(sceneId, ramses::ERendererEventResult_OK);
    expectRendererSceneCommands({ ramses_internal::ERendererCommand_AssignSceneToDisplayBuffer, ramses_internal::ERendererCommand_ShowScene });
    displayManagerEventHandler.sceneShown(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_ConfirmationEcho);

    doAnotherFullCycleFromUnpublishToState();
}

TEST_F(ADisplayManager, recoversFromRepublishBeforeFailedShow)
{
    displayManager.setSceneState(sceneId, SceneState::Rendered, "scene is shown now");

    displayManagerEventHandler.scenePublished(sceneId);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_SubscribeScene);
    displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_OK);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_MapSceneToDisplay);
    displayManagerEventHandler.sceneMapped(sceneId, ramses::ERendererEventResult_OK);
    expectRendererSceneCommands({ ramses_internal::ERendererCommand_AssignSceneToDisplayBuffer, ramses_internal::ERendererCommand_ShowScene });

    unpublish(ramses_internal::ERendererCommand_MapSceneToDisplay);
    expectNoRendererCommand();
    displayManagerEventHandler.scenePublished(sceneId);
    expectNoRendererCommand(); // not trying to subscribe because last command not answered yet

    displayManagerEventHandler.sceneShown(sceneId, ramses::ERendererEventResult_FAIL);

    expectRendererSceneCommand(ramses_internal::ERendererCommand_SubscribeScene); // now start again from subscribe
    displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_OK);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_MapSceneToDisplay);
    displayManagerEventHandler.sceneMapped(sceneId, ramses::ERendererEventResult_OK);
    expectRendererSceneCommands({ ramses_internal::ERendererCommand_AssignSceneToDisplayBuffer, ramses_internal::ERendererCommand_ShowScene });
    displayManagerEventHandler.sceneShown(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_ConfirmationEcho);

    doAnotherFullCycleFromUnpublishToState();
}

/////////
// Tests where unpublish comes after already failed last command on renderer side
// and DM can get scene to shown state again
// (DM does not do that automatically because states other than show are not automatically reached again after unpublish)
/////////

TEST_F(ADisplayManager, canRecoverFromRepublishAfterFailedUnsubscribe)
{
    displayManager.setSceneState(sceneId, SceneState::Rendered);
    publishAndExpectToGetToState(SceneState::Rendered);

    displayManager.setSceneState(sceneId, SceneState::Unavailable);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_HideScene);
    displayManagerEventHandler.sceneHidden(sceneId, ramses::ERendererEventResult_OK);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_UnmapSceneFromDisplays);
    displayManagerEventHandler.sceneUnmapped(sceneId, ramses::ERendererEventResult_OK);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_UnsubscribeScene);

    unpublish(ramses_internal::ERendererCommand_UnmapSceneFromDisplays);
    expectNoRendererCommand();
    displayManagerEventHandler.sceneUnsubscribed(sceneId, ramses::ERendererEventResult_FAIL);

    displayManager.setSceneState(sceneId, SceneState::Rendered);
    publishAndExpectToGetToState(SceneState::Rendered);
}

TEST_F(ADisplayManager, canRecoverFromRepublishAfterFailedUnmap)
{
    displayManager.setSceneState(sceneId, SceneState::Rendered);
    publishAndExpectToGetToState(SceneState::Rendered);

    displayManager.setSceneState(sceneId, SceneState::Unavailable);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_HideScene);
    displayManagerEventHandler.sceneHidden(sceneId, ramses::ERendererEventResult_OK);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_UnmapSceneFromDisplays);

    unpublish(ramses_internal::ERendererCommand_HideScene);
    expectNoRendererCommand();
    displayManagerEventHandler.sceneUnmapped(sceneId, ramses::ERendererEventResult_FAIL);

    displayManager.setSceneState(sceneId, SceneState::Rendered);
    publishAndExpectToGetToState(SceneState::Rendered);
}

TEST_F(ADisplayManager, canRecoverFromRepublishAfterFailedHide)
{
    displayManager.setSceneState(sceneId, SceneState::Rendered);
    publishAndExpectToGetToState(SceneState::Rendered);

    displayManager.setSceneState(sceneId, SceneState::Unavailable);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_HideScene);

    unpublish(ramses_internal::ERendererCommand_ShowScene);
    expectNoRendererCommand();
    displayManagerEventHandler.sceneHidden(sceneId, ramses::ERendererEventResult_FAIL);

    displayManager.setSceneState(sceneId, SceneState::Rendered);
    publishAndExpectToGetToState(SceneState::Rendered);
}

/////////
// Tests where unpublish comes before failed last command on renderer side
// and DM can get scene to shown state again
// (DM does not do that automatically because states other than show are not automatically reached again after unpublish)
/////////

TEST_F(ADisplayManager, canRecoverFromRepublishBeforeFailedUnsubscribe)
{
    displayManager.setSceneState(sceneId, SceneState::Rendered);
    publishAndExpectToGetToState(SceneState::Rendered);

    displayManager.setSceneState(sceneId, SceneState::Unavailable);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_HideScene);
    displayManagerEventHandler.sceneHidden(sceneId, ramses::ERendererEventResult_OK);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_UnmapSceneFromDisplays);
    displayManagerEventHandler.sceneUnmapped(sceneId, ramses::ERendererEventResult_OK);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_UnsubscribeScene);

    // unpublish and publish coming together
    unpublish(ramses_internal::ERendererCommand_UnmapSceneFromDisplays);
    expectNoRendererCommand();
    displayManagerEventHandler.scenePublished(sceneId);
    expectNoRendererCommand();

    // only now comes fail reply to unsubscribe command
    displayManagerEventHandler.sceneUnsubscribed(sceneId, ramses::ERendererEventResult_FAIL);

    // trigger new show
    displayManager.setSceneState(sceneId, SceneState::Rendered);

    expectRendererSceneCommand(ramses_internal::ERendererCommand_SubscribeScene);
    displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_OK);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_MapSceneToDisplay);
    displayManagerEventHandler.sceneMapped(sceneId, ramses::ERendererEventResult_OK);
    expectRendererSceneCommands({ ramses_internal::ERendererCommand_AssignSceneToDisplayBuffer, ramses_internal::ERendererCommand_ShowScene });
    displayManagerEventHandler.sceneShown(sceneId, ramses::ERendererEventResult_OK);

    doAnotherFullCycleFromUnpublishToState();
}

TEST_F(ADisplayManager, canRecoverFromRepublishBeforeFailedUnmap)
{
    displayManager.setSceneState(sceneId, SceneState::Rendered);
    publishAndExpectToGetToState(SceneState::Rendered);

    displayManager.setSceneState(sceneId, SceneState::Unavailable);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_HideScene);
    displayManagerEventHandler.sceneHidden(sceneId, ramses::ERendererEventResult_OK);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_UnmapSceneFromDisplays);

    // unpublish and publish coming together
    unpublish(ramses_internal::ERendererCommand_HideScene);
    expectNoRendererCommand();
    displayManagerEventHandler.scenePublished(sceneId);
    expectNoRendererCommand();

    // only now comes fail reply to unmap command
    displayManagerEventHandler.sceneUnmapped(sceneId, ramses::ERendererEventResult_FAIL);

    // trigger new show
    displayManager.setSceneState(sceneId, SceneState::Rendered);

    expectRendererSceneCommand(ramses_internal::ERendererCommand_SubscribeScene);
    displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_OK);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_MapSceneToDisplay);
    displayManagerEventHandler.sceneMapped(sceneId, ramses::ERendererEventResult_OK);
    expectRendererSceneCommands({ ramses_internal::ERendererCommand_AssignSceneToDisplayBuffer, ramses_internal::ERendererCommand_ShowScene });
    displayManagerEventHandler.sceneShown(sceneId, ramses::ERendererEventResult_OK);

    doAnotherFullCycleFromUnpublishToState();
}

TEST_F(ADisplayManager, canRecoverFromRepublishBeforeFailedHide)
{
    displayManager.setSceneState(sceneId, SceneState::Rendered);
    publishAndExpectToGetToState(SceneState::Rendered);

    displayManager.setSceneState(sceneId, SceneState::Unavailable);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_HideScene);

    // unpublish and publish coming together
    unpublish(ramses_internal::ERendererCommand_ShowScene);
    expectNoRendererCommand();
    displayManagerEventHandler.scenePublished(sceneId);
    expectNoRendererCommand();

    // only now comes fail reply to hide command
    displayManagerEventHandler.sceneHidden(sceneId, ramses::ERendererEventResult_FAIL);

    // trigger new show
    displayManager.setSceneState(sceneId, SceneState::Rendered);

    expectRendererSceneCommand(ramses_internal::ERendererCommand_SubscribeScene);
    displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_OK);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_MapSceneToDisplay);
    displayManagerEventHandler.sceneMapped(sceneId, ramses::ERendererEventResult_OK);
    expectRendererSceneCommands({ ramses_internal::ERendererCommand_AssignSceneToDisplayBuffer, ramses_internal::ERendererCommand_ShowScene });
    displayManagerEventHandler.sceneShown(sceneId, ramses::ERendererEventResult_OK);

    doAnotherFullCycleFromUnpublishToState();
}

TEST_F(ADisplayManager, failsToAssignSceneWithNoMappingInfo)
{
    constexpr ramses::sceneId_t sceneWithoutMapping{ 11 };
    displayManager.setSceneState(sceneWithoutMapping, SceneState::Rendered);

    EXPECT_FALSE(displayManager.setSceneDisplayBufferAssignment(sceneWithoutMapping, displayFramebufferId));
    EXPECT_FALSE(displayManager.setSceneDisplayBufferAssignment(sceneWithoutMapping, offscreenBufferId));
    expectNoRendererCommand();
}

TEST_F(ADisplayManager, assignsSceneToBufferAfterSceneIsMapped)
{
    displayManager.setSceneState(sceneId, SceneState::Rendered);

    displayManagerEventHandler.scenePublished(sceneId);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_SubscribeScene);
    displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_OK);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_MapSceneToDisplay);

    EXPECT_TRUE(displayManager.setSceneDisplayBufferAssignment(sceneId, displayFramebufferId, 11));
    expectNoRendererCommand();

    displayManagerEventHandler.sceneMapped(sceneId, ramses::ERendererEventResult_OK);
    const auto cmd = getRendererSceneCommand<ramses_internal::SceneMappingCommand>(0u, ramses_internal::ERendererCommand_AssignSceneToDisplayBuffer);
    EXPECT_FALSE(cmd.offscreenBuffer.isValid()); // FB is represented as invalid OB handle internally
    EXPECT_EQ(11, cmd.sceneRenderOrder);
    expectRendererSceneCommands({ ramses_internal::ERendererCommand_AssignSceneToDisplayBuffer, ramses_internal::ERendererCommand_ShowScene });
    displayManagerEventHandler.sceneShown(sceneId, ramses::ERendererEventResult_OK);
}

TEST_F(ADisplayManager, assignsSceneToBufferRightAwayIfSceneIsMapped)
{
    displayManager.setSceneState(sceneId, SceneState::Rendered);
    publishAndExpectToGetToState(SceneState::Rendered);

    EXPECT_TRUE(displayManager.setSceneDisplayBufferAssignment(sceneId, offscreenBufferId, 11));
    const auto cmd = getRendererSceneCommand<ramses_internal::SceneMappingCommand>(0u, ramses_internal::ERendererCommand_AssignSceneToDisplayBuffer);
    EXPECT_EQ(offscreenBufferId.getValue(), cmd.offscreenBuffer.asMemoryHandle());
    EXPECT_EQ(11, cmd.sceneRenderOrder);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_AssignSceneToDisplayBuffer);
}

TEST_F(ADisplayManager, failsToAssignSceneToUnknownBuffer)
{
    displayManager.setSceneState(sceneId, SceneState::Rendered);
    publishAndExpectToGetToState(SceneState::Rendered);

    EXPECT_FALSE(displayManager.setSceneDisplayBufferAssignment(sceneId, ramses::displayBufferId_t{ 9876 }, 11));
    expectNoRendererCommand();
}

TEST_F(ADisplayManager, mappingSceneToAnotherDisplayResetsDisplayBufferAssignmentAndRenderOrder)
{
    // get scene rendered
    displayManager.setSceneState(sceneId, SceneState::Rendered);
    publishAndExpectToGetToState(SceneState::Rendered);

    // assign to OB + render order
    EXPECT_TRUE(displayManager.setSceneDisplayBufferAssignment(sceneId, offscreenBufferId, 11));
    const auto cmd1 = getRendererSceneCommand<ramses_internal::SceneMappingCommand>(0u, ramses_internal::ERendererCommand_AssignSceneToDisplayBuffer);
    EXPECT_EQ(offscreenBufferId.getValue(), cmd1.offscreenBuffer.asMemoryHandle());
    EXPECT_EQ(11, cmd1.sceneRenderOrder);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_AssignSceneToDisplayBuffer);

    // unmap scene
    displayManager.setSceneState(sceneId, SceneState::Available);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_HideScene);
    displayManagerEventHandler.sceneHidden(sceneId, ramses::ERendererEventResult_OK);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_UnmapSceneFromDisplays);
    displayManagerEventHandler.sceneUnmapped(sceneId, ramses::ERendererEventResult_OK);
    EXPECT_EQ(ramses::displayId_t::Invalid(), displayManager.getDisplaySceneIsMappedTo(sceneId));
    expectNoRendererCommand();

    // change mapping
    EXPECT_TRUE(displayManager.setSceneMapping(sceneId, otherDisplayId));
    expectNoRendererCommand();

    // map scene
    displayManager.setSceneState(sceneId, SceneState::Ready);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_MapSceneToDisplay);
    displayManagerEventHandler.sceneMapped(sceneId, ramses::ERendererEventResult_OK);
    // expect assignment to default FB, render order 0
    const auto cmd2 = getRendererSceneCommand<ramses_internal::SceneMappingCommand>(0u, ramses_internal::ERendererCommand_AssignSceneToDisplayBuffer);
    EXPECT_FALSE(cmd2.offscreenBuffer.isValid()); // FB is represented as invalid OB handle internally
    EXPECT_EQ(0, cmd2.sceneRenderOrder);
    expectRendererSceneCommand(ramses_internal::ERendererCommand_AssignSceneToDisplayBuffer);
    displayManagerEventHandler.sceneAssignedToDisplayBuffer(sceneId, otherDisplayFramebufferId, ramses::ERendererEventResult_OK);
    EXPECT_EQ(otherDisplayId, displayManager.getDisplaySceneIsMappedTo(sceneId));
    expectNoRendererCommand();
}

TEST_F(ADisplayManager, willForwardOffscreenBufferLinkedEventFromRenderer)
{
    constexpr ramses::sceneId_t consumerScene{ 3 };
    constexpr ramses::dataConsumerId_t consumerId{ 4 };

    displayManagerEventHandler.offscreenBufferLinkedToSceneData(offscreenBufferId, consumerScene, consumerId, ramses::ERendererEventResult_OK);
    EXPECT_CALL(eventHandlerMock, offscreenBufferLinked(offscreenBufferId, consumerScene, consumerId, true));
    displayManager.dispatchAndFlush(&eventHandlerMock);

    displayManagerEventHandler.offscreenBufferLinkedToSceneData(offscreenBufferId, consumerScene, consumerId, ramses::ERendererEventResult_FAIL);
    EXPECT_CALL(eventHandlerMock, offscreenBufferLinked(offscreenBufferId, consumerScene, consumerId, false));
    displayManager.dispatchAndFlush(&eventHandlerMock);

    displayManagerEventHandler.offscreenBufferLinkedToSceneData(offscreenBufferId, consumerScene, consumerId, ramses::ERendererEventResult_OK);
    displayManagerEventHandler.offscreenBufferLinkedToSceneData(offscreenBufferId, consumerScene, consumerId, ramses::ERendererEventResult_FAIL);
    InSequence seq;
    EXPECT_CALL(eventHandlerMock, offscreenBufferLinked(offscreenBufferId, consumerScene, consumerId, true));
    EXPECT_CALL(eventHandlerMock, offscreenBufferLinked(offscreenBufferId, consumerScene, consumerId, false));
    displayManager.dispatchAndFlush(&eventHandlerMock);
}

TEST_F(ADisplayManager, willForwardDataLinkedEventFromRenderer)
{
    constexpr ramses::sceneId_t providerScene{ 1 };
    constexpr ramses::dataProviderId_t providerId{ 2 };
    constexpr ramses::sceneId_t consumerScene{ 3 };
    constexpr ramses::dataConsumerId_t consumerId{ 4 };

    displayManagerEventHandler.dataLinked(providerScene, providerId, consumerScene, consumerId, ramses::ERendererEventResult_OK);
    EXPECT_CALL(eventHandlerMock, dataLinked(providerScene, providerId, consumerScene, consumerId, true));
    displayManager.dispatchAndFlush(&eventHandlerMock);

    displayManagerEventHandler.dataLinked(providerScene, providerId, consumerScene, consumerId, ramses::ERendererEventResult_FAIL);
    EXPECT_CALL(eventHandlerMock, dataLinked(providerScene, providerId, consumerScene, consumerId, false));
    displayManager.dispatchAndFlush(&eventHandlerMock);

    displayManagerEventHandler.dataLinked(providerScene, providerId, consumerScene, consumerId, ramses::ERendererEventResult_OK);
    displayManagerEventHandler.dataLinked(providerScene, providerId, consumerScene, consumerId, ramses::ERendererEventResult_FAIL);
    InSequence seq;
    EXPECT_CALL(eventHandlerMock, dataLinked(providerScene, providerId, consumerScene, consumerId, true));
    EXPECT_CALL(eventHandlerMock, dataLinked(providerScene, providerId, consumerScene, consumerId, false));
    displayManager.dispatchAndFlush(&eventHandlerMock);
}
