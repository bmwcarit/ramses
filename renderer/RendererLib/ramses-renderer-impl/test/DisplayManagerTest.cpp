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
#include "RamsesRendererImpl.h"

using namespace ramses_display_manager;
using namespace testing;

class ADisplayManagerBase : public Test
{
public:
    explicit ADisplayManagerBase(bool autoMapping)
        : renderer(ramsesFramework, ramses::RendererConfig())
        , displayManager(renderer, ramsesFramework, autoMapping)
        , displayManagerEventHandler(displayManager)
        , sceneId(33u)
    {
        ramses::DisplayConfig displayConfig;
        displayConfig.setWaylandIviSurfaceID(0u);
        displayId = displayManager.createDisplay(displayConfig);

        displayConfig.setWaylandIviSurfaceID(1u);
        otherDisplayId = displayManager.createDisplay(displayConfig);

        // flush and loop to execute the creation
        displayManager.dispatchAndFlush();
        renderer.doOneLoop();
        EXPECT_EQ(2u, renderer.impl.getRenderer().getRenderer().getDisplayControllerCount());

        // dispatch to get the event of successful creation
        displayManager.dispatchAndFlush();
        EXPECT_TRUE(displayManager.isDisplayCreated(displayId));
        EXPECT_TRUE(displayManager.isDisplayCreated(otherDisplayId));
    }

    ~ADisplayManagerBase()
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

    void expectRendererCommand(ramses_internal::ERendererCommand expectedCmd)
    {
        expectRendererCommands({ expectedCmd });
    }

    void expectNoRendererCommand()
    {
        expectRendererCommands({});
    }

    void publishAndExpectToGetToState(SceneState state, bool expectConfirmation = false)
    {
        EXPECT_FALSE(isSceneShown());

        displayManagerEventHandler.scenePublished(sceneId);
        if (state != SceneState::Unavailable)
        {
            expectRendererCommand(ramses_internal::ERendererCommand_SubscribeScene);
            displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_OK);

            if (state != SceneState::Available)
            {
                expectRendererCommand(ramses_internal::ERendererCommand_MapSceneToDisplay);
                displayManagerEventHandler.sceneMapped(sceneId, ramses::ERendererEventResult_OK);

                if (state != SceneState::Ready)
                {
                    assert(state == SceneState::Rendered);
                    expectRendererCommand(ramses_internal::ERendererCommand_ShowScene);
                    EXPECT_FALSE(isSceneShown());
                    displayManagerEventHandler.sceneShown(sceneId, ramses::ERendererEventResult_OK);
                    EXPECT_TRUE(isSceneShown());
                }
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
        case ramses_internal::ERendererCommand_MapSceneToDisplay:
        case ramses_internal::ERendererCommand_HideScene:
            displayManagerEventHandler.sceneUnmapped(sceneId, ramses::ERendererEventResult_INDIRECT);
        case ramses_internal::ERendererCommand_SubscribeScene:
        case ramses_internal::ERendererCommand_UnmapSceneFromDisplays:
            displayManagerEventHandler.sceneUnsubscribed(sceneId, ramses::ERendererEventResult_INDIRECT);
        case ramses_internal::ERendererCommand_PublishedScene:
        case ramses_internal::ERendererCommand_UnsubscribeScene:
            displayManagerEventHandler.sceneUnpublished(sceneId);
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
    ramses::RamsesRenderer renderer; // restrict access to renderer, acts only as dummy for DisplayManager and to create (dummy) displays

protected:
    DisplayManager displayManager;
    ramses::IRendererEventHandler& displayManagerEventHandler;

    const ramses::sceneId_t sceneId;
    ramses::displayId_t displayId;
    ramses::displayId_t otherDisplayId;
};

class ADisplayManagerWithAutomapping : public ADisplayManagerBase
{
protected:
    ADisplayManagerWithAutomapping()
        : ADisplayManagerBase(true)
    {
    }
};

class ADisplayManager : public ADisplayManagerBase
{
protected:
    ADisplayManager()
        : ADisplayManagerBase(false)
    {
        displayManager.setSceneMapping(sceneId, displayId);
    }
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

TEST_F(ADisplayManagerWithAutomapping, willAutoShowSceneWhenPublished)
{
    publishAndExpectToGetToState(SceneState::Rendered);
    EXPECT_EQ(displayId, displayManager.getDisplaySceneIsMappedTo(sceneId));

    doAnotherFullCycleFromUnpublishToState();
    doAnotherFullCycleFromUnpublishToShownWithShowTriggered();
}

TEST_F(ADisplayManagerWithAutomapping, willAutoShowSceneWhenPublishedOnExplicitlySetDisplay)
{
    EXPECT_TRUE(displayManager.setSceneMapping(sceneId, otherDisplayId));
    publishAndExpectToGetToState(SceneState::Rendered);
    EXPECT_EQ(otherDisplayId, displayManager.getDisplaySceneIsMappedTo(sceneId));

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
    expectRendererCommand(ramses_internal::ERendererCommand_SubscribeScene);

    displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_MapSceneToDisplay);

    displayManagerEventHandler.sceneMapped(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_ShowScene);

    displayManagerEventHandler.sceneShown(sceneId, ramses::ERendererEventResult_OK);
    expectNoRendererCommand();

    doAnotherFullCycleFromUnpublishToState();
    doAnotherFullCycleFromUnpublishToShownWithShowTriggered();
}

TEST_F(ADisplayManager, willRetryIfSubscribeFailed)
{
    displayManagerEventHandler.scenePublished(sceneId);

    displayManager.setSceneState(sceneId, SceneState::Available);
    expectRendererCommand(ramses_internal::ERendererCommand_SubscribeScene);

    displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_FAIL);
    expectRendererCommand(ramses_internal::ERendererCommand_SubscribeScene);
    displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_OK);
    expectNoRendererCommand();

    doAnotherFullCycleFromUnpublishToState(ramses_internal::ERendererCommand_SubscribeScene, SceneState::Available);
    doAnotherFullCycleFromUnpublishToShownWithShowTriggered();
}

TEST_F(ADisplayManager, willRetryIfMapFailed)
{
    displayManagerEventHandler.scenePublished(sceneId);

    displayManager.setSceneState(sceneId, SceneState::Ready);
    expectRendererCommand(ramses_internal::ERendererCommand_SubscribeScene);

    displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_MapSceneToDisplay);

    displayManagerEventHandler.sceneMapped(sceneId, ramses::ERendererEventResult_FAIL);
    EXPECT_EQ(ramses::InvalidDisplayId, displayManager.getDisplaySceneIsMappedTo(sceneId));
    expectRendererCommand(ramses_internal::ERendererCommand_MapSceneToDisplay);
    displayManagerEventHandler.sceneMapped(sceneId, ramses::ERendererEventResult_OK);
    EXPECT_EQ(displayId, displayManager.getDisplaySceneIsMappedTo(sceneId));
    expectNoRendererCommand();

    doAnotherFullCycleFromUnpublishToState(ramses_internal::ERendererCommand_SubscribeScene, SceneState::Ready);
    doAnotherFullCycleFromUnpublishToShownWithShowTriggered();
}

TEST_F(ADisplayManager, willRetryIfShowFailed)
{
    displayManagerEventHandler.scenePublished(sceneId);

    displayManager.setSceneState(sceneId, SceneState::Rendered);
    expectRendererCommand(ramses_internal::ERendererCommand_SubscribeScene);

    displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_MapSceneToDisplay);

    displayManagerEventHandler.sceneMapped(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_ShowScene);

    displayManagerEventHandler.sceneShown(sceneId, ramses::ERendererEventResult_FAIL);
    expectRendererCommand(ramses_internal::ERendererCommand_ShowScene);
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
    expectRendererCommand(ramses_internal::ERendererCommand_HideScene);
    displayManagerEventHandler.sceneHidden(sceneId, ramses::ERendererEventResult_FAIL);
    expectRendererCommand(ramses_internal::ERendererCommand_HideScene);
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
    expectRendererCommand(ramses_internal::ERendererCommand_UnmapSceneFromDisplays);
    displayManagerEventHandler.sceneUnmapped(sceneId, ramses::ERendererEventResult_FAIL);
    EXPECT_EQ(displayId, displayManager.getDisplaySceneIsMappedTo(sceneId));
    expectRendererCommand(ramses_internal::ERendererCommand_UnmapSceneFromDisplays);
    displayManagerEventHandler.sceneUnmapped(sceneId, ramses::ERendererEventResult_OK);
    EXPECT_EQ(ramses::InvalidDisplayId, displayManager.getDisplaySceneIsMappedTo(sceneId));
    expectNoRendererCommand();

    doAnotherFullCycleFromUnpublishToState(ramses_internal::ERendererCommand_UnmapSceneFromDisplays, SceneState::Available);
    doAnotherFullCycleFromUnpublishToShownWithShowTriggered();
}

TEST_F(ADisplayManager, willRetryIfUnsubscribeFailed)
{
    displayManager.setSceneState(sceneId, SceneState::Available);
    publishAndExpectToGetToState(SceneState::Available);

    displayManager.setSceneState(sceneId, SceneState::Unavailable);
    expectRendererCommand(ramses_internal::ERendererCommand_UnsubscribeScene);
    displayManagerEventHandler.sceneUnsubscribed(sceneId, ramses::ERendererEventResult_FAIL);
    expectRendererCommand(ramses_internal::ERendererCommand_UnsubscribeScene);
    displayManagerEventHandler.sceneUnsubscribed(sceneId, ramses::ERendererEventResult_OK);
    expectNoRendererCommand();

    doAnotherFullCycleFromUnpublishToState(ramses_internal::ERendererCommand_UnsubscribeScene, SceneState::Unavailable);
    doAnotherFullCycleFromUnpublishToShownWithShowTriggered();
}

TEST_F(ADisplayManager, reportsDisplaySceneIsMappedToOnlyMapped)
{
    displayManager.setSceneState(sceneId, SceneState::Rendered);

    displayManagerEventHandler.scenePublished(sceneId);
    EXPECT_EQ(ramses::InvalidDisplayId, displayManager.getDisplaySceneIsMappedTo(sceneId));
    expectRendererCommand(ramses_internal::ERendererCommand_SubscribeScene);

    displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_OK);
    EXPECT_EQ(ramses::InvalidDisplayId, displayManager.getDisplaySceneIsMappedTo(sceneId));
    expectRendererCommand(ramses_internal::ERendererCommand_MapSceneToDisplay);

    displayManagerEventHandler.sceneMapped(sceneId, ramses::ERendererEventResult_OK);
    EXPECT_EQ(displayId, displayManager.getDisplaySceneIsMappedTo(sceneId));
    expectRendererCommand(ramses_internal::ERendererCommand_ShowScene);

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
    EXPECT_EQ(ramses::InvalidDisplayId, displayManager.getDisplaySceneIsMappedTo(sceneId));
    expectRendererCommand(ramses_internal::ERendererCommand_SubscribeScene);

    displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_OK);
    EXPECT_EQ(ramses::InvalidDisplayId, displayManager.getDisplaySceneIsMappedTo(sceneId));
    expectRendererCommand(ramses_internal::ERendererCommand_MapSceneToDisplay);

    displayManagerEventHandler.sceneMapped(sceneId, ramses::ERendererEventResult_OK);
    EXPECT_EQ(displayId, displayManager.getDisplaySceneIsMappedTo(sceneId));
    expectRendererCommand(ramses_internal::ERendererCommand_ShowScene);

    EXPECT_FALSE(isSceneShown());
    displayManagerEventHandler.sceneShown(sceneId, ramses::ERendererEventResult_OK);
    EXPECT_EQ(displayId, displayManager.getDisplaySceneIsMappedTo(sceneId));
    EXPECT_TRUE(isSceneShown());

    displayManager.setSceneState(sceneId, SceneState::Available);
    expectRendererCommand(ramses_internal::ERendererCommand_HideScene);

    displayManagerEventHandler.sceneHidden(sceneId, ramses::ERendererEventResult_OK);
    EXPECT_EQ(displayId, displayManager.getDisplaySceneIsMappedTo(sceneId));
    EXPECT_FALSE(isSceneShown());
    expectRendererCommand(ramses_internal::ERendererCommand_UnmapSceneFromDisplays);

    displayManagerEventHandler.sceneUnmapped(sceneId, ramses::ERendererEventResult_OK);
    EXPECT_EQ(ramses::InvalidDisplayId, displayManager.getDisplaySceneIsMappedTo(sceneId));
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
    expectRendererCommand(ramses_internal::ERendererCommand_HideScene);

    displayManagerEventHandler.sceneHidden(sceneId, ramses::ERendererEventResult_OK);
    expectNoRendererCommand();
    EXPECT_EQ(displayId, displayManager.getDisplaySceneIsMappedTo(sceneId));

    doAnotherFullCycleFromUnpublishToState(ramses_internal::ERendererCommand_HideScene, SceneState::Ready);
    doAnotherFullCycleFromUnpublishToShownWithShowTriggered(ramses_internal::ERendererCommand_MapSceneToDisplay);
}

TEST_F(ADisplayManagerWithAutomapping, hidesShownScene)
{
    publishAndExpectToGetToState(SceneState::Rendered);

    displayManager.setSceneState(sceneId, SceneState::Ready);
    expectRendererCommand(ramses_internal::ERendererCommand_HideScene);

    displayManagerEventHandler.sceneHidden(sceneId, ramses::ERendererEventResult_OK);
    expectNoRendererCommand();

    doAnotherFullCycleFromUnpublishToState(ramses_internal::ERendererCommand_HideScene, SceneState::Ready);
    doAnotherFullCycleFromUnpublishToShownWithShowTriggered(ramses_internal::ERendererCommand_MapSceneToDisplay);
}

TEST_F(ADisplayManager, unmapsShownScene)
{
    displayManager.setSceneState(sceneId, SceneState::Rendered);
    expectNoRendererCommand();
    publishAndExpectToGetToState(SceneState::Rendered);

    displayManager.setSceneState(sceneId, SceneState::Available);
    expectRendererCommand(ramses_internal::ERendererCommand_HideScene);

    displayManagerEventHandler.sceneHidden(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_UnmapSceneFromDisplays);
    EXPECT_EQ(displayId, displayManager.getDisplaySceneIsMappedTo(sceneId));

    displayManagerEventHandler.sceneUnmapped(sceneId, ramses::ERendererEventResult_OK);
    expectNoRendererCommand();
    EXPECT_EQ(ramses::InvalidDisplayId, displayManager.getDisplaySceneIsMappedTo(sceneId));

    doAnotherFullCycleFromUnpublishToState(ramses_internal::ERendererCommand_UnmapSceneFromDisplays, SceneState::Available);
    doAnotherFullCycleFromUnpublishToShownWithShowTriggered(ramses_internal::ERendererCommand_SubscribeScene);
}

TEST_F(ADisplayManagerWithAutomapping, unmapsShownScene)
{
    publishAndExpectToGetToState(SceneState::Rendered);

    displayManager.setSceneState(sceneId, SceneState::Available);
    expectRendererCommand(ramses_internal::ERendererCommand_HideScene);

    displayManagerEventHandler.sceneHidden(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_UnmapSceneFromDisplays);
    EXPECT_EQ(displayId, displayManager.getDisplaySceneIsMappedTo(sceneId));

    displayManagerEventHandler.sceneUnmapped(sceneId, ramses::ERendererEventResult_OK);
    expectNoRendererCommand();
    EXPECT_EQ(ramses::InvalidDisplayId, displayManager.getDisplaySceneIsMappedTo(sceneId));

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
    expectRendererCommand(ramses_internal::ERendererCommand_HideScene);
    displayManagerEventHandler.sceneHidden(sceneId, ramses::ERendererEventResult_OK);
    expectNoRendererCommand();

    displayManager.setSceneState(sceneId, SceneState::Available);
    expectRendererCommand(ramses_internal::ERendererCommand_UnmapSceneFromDisplays);

    displayManagerEventHandler.sceneUnmapped(sceneId, ramses::ERendererEventResult_OK);
    expectNoRendererCommand();
    EXPECT_EQ(ramses::InvalidDisplayId, displayManager.getDisplaySceneIsMappedTo(sceneId));

    doAnotherFullCycleFromUnpublishToState(ramses_internal::ERendererCommand_UnmapSceneFromDisplays, SceneState::Available);
    doAnotherFullCycleFromUnpublishToShownWithShowTriggered(ramses_internal::ERendererCommand_SubscribeScene);
}

TEST_F(ADisplayManagerWithAutomapping, unmapsMappedScene)
{
    publishAndExpectToGetToState(SceneState::Rendered);

    // hide first
    displayManager.setSceneState(sceneId, SceneState::Ready);
    expectRendererCommand(ramses_internal::ERendererCommand_HideScene);
    displayManagerEventHandler.sceneHidden(sceneId, ramses::ERendererEventResult_OK);
    expectNoRendererCommand();

    displayManager.setSceneState(sceneId, SceneState::Available);
    expectRendererCommand(ramses_internal::ERendererCommand_UnmapSceneFromDisplays);

    displayManagerEventHandler.sceneUnmapped(sceneId, ramses::ERendererEventResult_OK);
    expectNoRendererCommand();
    EXPECT_EQ(ramses::InvalidDisplayId, displayManager.getDisplaySceneIsMappedTo(sceneId));

    doAnotherFullCycleFromUnpublishToState(ramses_internal::ERendererCommand_UnmapSceneFromDisplays, SceneState::Available);
    doAnotherFullCycleFromUnpublishToShownWithShowTriggered(ramses_internal::ERendererCommand_SubscribeScene);
}

TEST_F(ADisplayManager, unsubscribesShownScene)
{
    displayManager.setSceneState(sceneId, SceneState::Rendered);
    expectNoRendererCommand();
    publishAndExpectToGetToState(SceneState::Rendered);

    displayManager.setSceneState(sceneId, SceneState::Unavailable);
    expectRendererCommand(ramses_internal::ERendererCommand_HideScene);

    displayManagerEventHandler.sceneHidden(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_UnmapSceneFromDisplays);

    displayManagerEventHandler.sceneUnmapped(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_UnsubscribeScene);

    displayManagerEventHandler.sceneUnsubscribed(sceneId, ramses::ERendererEventResult_OK);
    expectNoRendererCommand();
}

TEST_F(ADisplayManagerWithAutomapping, unsubscribesShownScene)
{
    publishAndExpectToGetToState(SceneState::Rendered);

    displayManager.setSceneState(sceneId, SceneState::Unavailable);
    expectRendererCommand(ramses_internal::ERendererCommand_HideScene);

    displayManagerEventHandler.sceneHidden(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_UnmapSceneFromDisplays);

    displayManagerEventHandler.sceneUnmapped(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_UnsubscribeScene);

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
    expectRendererCommand(ramses_internal::ERendererCommand_HideScene);
    displayManagerEventHandler.sceneHidden(sceneId, ramses::ERendererEventResult_OK);
    expectNoRendererCommand();

    displayManager.setSceneState(sceneId, SceneState::Unavailable);
    expectRendererCommand(ramses_internal::ERendererCommand_UnmapSceneFromDisplays);

    displayManagerEventHandler.sceneUnmapped(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_UnsubscribeScene);

    displayManagerEventHandler.sceneUnsubscribed(sceneId, ramses::ERendererEventResult_OK);
    expectNoRendererCommand();
}

TEST_F(ADisplayManagerWithAutomapping, unsubscribesMappedScene)
{
    publishAndExpectToGetToState(SceneState::Rendered);

    // hide first
    displayManager.setSceneState(sceneId, SceneState::Ready);
    expectRendererCommand(ramses_internal::ERendererCommand_HideScene);
    displayManagerEventHandler.sceneHidden(sceneId, ramses::ERendererEventResult_OK);
    expectNoRendererCommand();

    displayManager.setSceneState(sceneId, SceneState::Unavailable);
    expectRendererCommand(ramses_internal::ERendererCommand_UnmapSceneFromDisplays);

    displayManagerEventHandler.sceneUnmapped(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_UnsubscribeScene);

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
    expectRendererCommand(ramses_internal::ERendererCommand_HideScene);
    displayManagerEventHandler.sceneHidden(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_UnmapSceneFromDisplays);
    displayManagerEventHandler.sceneUnmapped(sceneId, ramses::ERendererEventResult_OK);
    expectNoRendererCommand();

    displayManager.setSceneState(sceneId, SceneState::Unavailable);
    expectRendererCommand(ramses_internal::ERendererCommand_UnsubscribeScene);

    displayManagerEventHandler.sceneUnsubscribed(sceneId, ramses::ERendererEventResult_OK);
    expectNoRendererCommand();
}

TEST_F(ADisplayManagerWithAutomapping, unsubscribesSubscribedScene)
{
    publishAndExpectToGetToState(SceneState::Rendered);

    // unmap first
    displayManager.setSceneState(sceneId, SceneState::Available);
    expectRendererCommand(ramses_internal::ERendererCommand_HideScene);
    displayManagerEventHandler.sceneHidden(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_UnmapSceneFromDisplays);
    displayManagerEventHandler.sceneUnmapped(sceneId, ramses::ERendererEventResult_OK);
    expectNoRendererCommand();

    displayManager.setSceneState(sceneId, SceneState::Unavailable);
    expectRendererCommand(ramses_internal::ERendererCommand_UnsubscribeScene);

    displayManagerEventHandler.sceneUnsubscribed(sceneId, ramses::ERendererEventResult_OK);
    expectNoRendererCommand();
}

TEST_F(ADisplayManager, handlesSceneUnpublishWhileProcessingSubscribe)
{
    displayManager.setSceneState(sceneId, SceneState::Rendered);
    expectNoRendererCommand();

    displayManagerEventHandler.scenePublished(sceneId);
    expectRendererCommand(ramses_internal::ERendererCommand_SubscribeScene);

    //unexpected unpublish!!
    unpublish(ramses_internal::ERendererCommand_PublishedScene);
    displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_FAIL);

    doAnotherFullCycleFromUnpublishToState(ramses_internal::ERendererCommand_UnpublishedScene);
}

TEST_F(ADisplayManagerWithAutomapping, handlesSceneUnpublishWhileProcessingSubscribe)
{
    displayManagerEventHandler.scenePublished(sceneId);
    expectRendererCommand(ramses_internal::ERendererCommand_SubscribeScene);

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
    expectRendererCommand(ramses_internal::ERendererCommand_SubscribeScene);

    displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_MapSceneToDisplay);

    //unexpected unpublish!!
    unpublish(ramses_internal::ERendererCommand_SubscribeScene);
    displayManagerEventHandler.sceneMapped(sceneId, ramses::ERendererEventResult_FAIL);

    doAnotherFullCycleFromUnpublishToState(ramses_internal::ERendererCommand_SubscribeScene);
}

TEST_F(ADisplayManagerWithAutomapping, handlesSceneUnpublishWhileProcessingMap)
{
    displayManagerEventHandler.scenePublished(sceneId);
    expectRendererCommand(ramses_internal::ERendererCommand_SubscribeScene);

    displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_MapSceneToDisplay);

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
    expectRendererCommand(ramses_internal::ERendererCommand_SubscribeScene);

    displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_MapSceneToDisplay);

    displayManagerEventHandler.sceneMapped(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_ShowScene);

    //unexpected unpublish!!
    unpublish(ramses_internal::ERendererCommand_MapSceneToDisplay);
    displayManagerEventHandler.sceneShown(sceneId, ramses::ERendererEventResult_FAIL);

    doAnotherFullCycleFromUnpublishToState(ramses_internal::ERendererCommand_MapSceneToDisplay);
}

TEST_F(ADisplayManagerWithAutomapping, handlesSceneUnpublishWhileProcessingShow)
{
    displayManagerEventHandler.scenePublished(sceneId);
    expectRendererCommand(ramses_internal::ERendererCommand_SubscribeScene);

    displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_MapSceneToDisplay);

    displayManagerEventHandler.sceneMapped(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_ShowScene);

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
    expectRendererCommand(ramses_internal::ERendererCommand_ShowScene);
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
    EXPECT_EQ(ramses::InvalidDisplayId, displayManager.getDisplaySceneIsMappedTo(sceneId));

    displayManager.setSceneState(sceneId, SceneState::Ready);
    expectNoRendererCommand();
    EXPECT_FALSE(displayManager.setSceneMapping(sceneId, otherDisplayId));
    expectNoRendererCommand();
    EXPECT_EQ(ramses::InvalidDisplayId, displayManager.getDisplaySceneIsMappedTo(sceneId));

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
    EXPECT_EQ(ramses::InvalidDisplayId, displayManager.getDisplaySceneIsMappedTo(sceneId));

    displayManager.setSceneState(sceneId, SceneState::Ready);
    expectRendererCommand(ramses_internal::ERendererCommand_MapSceneToDisplay);
    displayManagerEventHandler.sceneMapped(sceneId, ramses::ERendererEventResult_OK);
    expectNoRendererCommand();
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
    expectRendererCommand(ramses_internal::ERendererCommand_UnmapSceneFromDisplays);
    displayManagerEventHandler.sceneUnmapped(sceneId, ramses::ERendererEventResult_OK);
    expectNoRendererCommand();
    EXPECT_EQ(ramses::InvalidDisplayId, displayManager.getDisplaySceneIsMappedTo(sceneId));

    // now can change mapping
    EXPECT_TRUE(displayManager.setSceneMapping(sceneId, otherDisplayId));
    expectNoRendererCommand();
    EXPECT_EQ(ramses::InvalidDisplayId, displayManager.getDisplaySceneIsMappedTo(sceneId));

    displayManager.setSceneState(sceneId, SceneState::Ready);
    expectRendererCommand(ramses_internal::ERendererCommand_MapSceneToDisplay);
    displayManagerEventHandler.sceneMapped(sceneId, ramses::ERendererEventResult_OK);
    expectNoRendererCommand();
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
    expectRendererCommand(ramses_internal::ERendererCommand_HideScene);
    displayManagerEventHandler.sceneHidden(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_UnmapSceneFromDisplays);
    displayManagerEventHandler.sceneUnmapped(sceneId, ramses::ERendererEventResult_OK);
    expectNoRendererCommand();

    EXPECT_TRUE(displayManager.setSceneMapping(sceneId, otherDisplayId));
    displayManager.setSceneState(sceneId, SceneState::Rendered);
    expectRendererCommand(ramses_internal::ERendererCommand_MapSceneToDisplay);
    displayManagerEventHandler.sceneMapped(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_ShowScene);
    displayManagerEventHandler.sceneShown(sceneId, ramses::ERendererEventResult_OK);
    expectNoRendererCommand();

    doAnotherFullCycleFromUnpublishToState(ramses_internal::ERendererCommand_ShowScene);
}

TEST_F(ADisplayManager, forwardsCommandForDataLink)
{
    const ramses::sceneId_t providerSceneId(1u);
    const ramses::dataProviderId_t providerId(2u);
    const ramses::sceneId_t consumerSceneId(3u);
    const ramses::dataConsumerId_t consumerId(4u);

    displayManager.linkData(providerSceneId, providerId, consumerSceneId, consumerId);
    expectRendererCommand(ramses_internal::ERendererCommand_LinkSceneData);
}

TEST_F(ADisplayManager, reportsCorrectSceneShowState)
{
    // show first time
    displayManager.setSceneState(sceneId, SceneState::Rendered);
    expectNoRendererCommand();
    publishAndExpectToGetToState(SceneState::Rendered);

    // explicitly hide
    displayManager.setSceneState(sceneId, SceneState::Ready);
    expectRendererCommand(ramses_internal::ERendererCommand_HideScene);
    EXPECT_TRUE(isSceneShown()); // hide command not processed yet
    displayManagerEventHandler.sceneHidden(sceneId, ramses::ERendererEventResult_OK);
    EXPECT_FALSE(isSceneShown());

    // show again
    displayManager.setSceneState(sceneId, SceneState::Rendered);
    expectRendererCommand(ramses_internal::ERendererCommand_ShowScene);
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
    displayManager.dispatchAndFlush();

    EXPECT_FALSE(displayManager.isDisplayCreated(customDisplay));
    doRenderLoop();
    displayManager.dispatchAndFlush();

    EXPECT_TRUE(displayManager.isDisplayCreated(customDisplay));

    displayManager.destroyDisplay(customDisplay);
    displayManager.dispatchAndFlush();
    EXPECT_TRUE(displayManager.isDisplayCreated(customDisplay));

    doRenderLoop();
    displayManager.dispatchAndFlush();

    EXPECT_FALSE(displayManager.isDisplayCreated(customDisplay));
}

TEST_F(ADisplayManager, sceneWillBeShownWhenDisplayIsCreated)
{
    ramses::displayId_t customDisplay(120u);

    EXPECT_TRUE(displayManager.setSceneMapping(sceneId, customDisplay));
    displayManager.setSceneState(sceneId, SceneState::Rendered);
    expectNoRendererCommand();

    displayManagerEventHandler.scenePublished(sceneId);
    expectRendererCommand(ramses_internal::ERendererCommand_SubscribeScene);

    displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_OK);
    expectNoRendererCommand(); // do not map, because display is not yet created

    displayManagerEventHandler.displayCreated(customDisplay, ramses::ERendererEventResult_OK);

    // now request map
    expectRendererCommand(ramses_internal::ERendererCommand_MapSceneToDisplay);

    displayManagerEventHandler.sceneMapped(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_ShowScene);

    displayManagerEventHandler.sceneShown(sceneId, ramses::ERendererEventResult_OK);

    doAnotherFullCycleFromUnpublishToState(ramses_internal::ERendererCommand_ShowScene);
}

TEST_F(ADisplayManagerWithAutomapping, sceneWillBeShownWhenDisplayIsCreated)
{
    // cleanup displays first
    ASSERT_TRUE(displayManager.isDisplayCreated(displayId));
    ASSERT_TRUE(displayManager.isDisplayCreated(otherDisplayId));
    displayManager.destroyDisplay(displayId);
    displayManager.destroyDisplay(otherDisplayId);
    displayManager.dispatchAndFlush();
    doRenderLoop();
    displayManager.dispatchAndFlush();
    ASSERT_FALSE(displayManager.isDisplayCreated(displayId));
    ASSERT_FALSE(displayManager.isDisplayCreated(otherDisplayId));

    ramses::displayId_t customDisplay(0u);

    displayManagerEventHandler.scenePublished(sceneId);
    expectRendererCommand(ramses_internal::ERendererCommand_SubscribeScene);

    displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_OK);
    expectNoRendererCommand(); // do not map, because display is not yet created

    displayManagerEventHandler.displayCreated(customDisplay, ramses::ERendererEventResult_OK);

    // now request map
    expectRendererCommand(ramses_internal::ERendererCommand_MapSceneToDisplay);

    displayManagerEventHandler.sceneMapped(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_ShowScene);

    displayManagerEventHandler.sceneShown(sceneId, ramses::ERendererEventResult_OK);

    doAnotherFullCycleFromUnpublishToState(ramses_internal::ERendererCommand_ShowScene);
}

TEST_F(ADisplayManager, continuesToShowSceneAndLogsConfirmationAfterReconnect)
{
    displayManagerEventHandler.scenePublished(sceneId);
    expectNoRendererCommand();

    displayManager.setSceneState(sceneId, SceneState::Rendered, "scene is shown now");
    expectRendererCommand(ramses_internal::ERendererCommand_SubscribeScene);

    displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_MapSceneToDisplay);

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
    expectRendererCommand(ramses_internal::ERendererCommand_SubscribeScene);
    unpublish(ramses_internal::ERendererCommand_PublishedScene);
    expectNoRendererCommand();
    displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_FAIL);

    publishAndExpectToGetToState(SceneState::Rendered, true);
}

TEST_F(ADisplayManager, recoversFromRepublishAfterFailedMap)
{
    displayManager.setSceneState(sceneId, SceneState::Rendered, "scene is shown now");

    displayManagerEventHandler.scenePublished(sceneId);
    expectRendererCommand(ramses_internal::ERendererCommand_SubscribeScene);
    displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_MapSceneToDisplay);

    unpublish(ramses_internal::ERendererCommand_SubscribeScene);
    expectNoRendererCommand();
    displayManagerEventHandler.sceneMapped(sceneId, ramses::ERendererEventResult_FAIL);

    publishAndExpectToGetToState(SceneState::Rendered, true);
}

TEST_F(ADisplayManager, recoversFromRepublishAfterFailedShow)
{
    displayManager.setSceneState(sceneId, SceneState::Rendered, "scene is shown now");

    displayManagerEventHandler.scenePublished(sceneId);
    expectRendererCommand(ramses_internal::ERendererCommand_SubscribeScene);
    displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_MapSceneToDisplay);
    displayManagerEventHandler.sceneMapped(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_ShowScene);

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
    expectRendererCommand(ramses_internal::ERendererCommand_SubscribeScene);

    unpublish(ramses_internal::ERendererCommand_PublishedScene);
    expectNoRendererCommand();
    displayManagerEventHandler.scenePublished(sceneId);
    expectNoRendererCommand(); // not trying to subscribe because last subscribe not answered yet

    displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_FAIL);
    expectRendererCommand(ramses_internal::ERendererCommand_SubscribeScene); // now try subscribe again
    displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_MapSceneToDisplay);
    displayManagerEventHandler.sceneMapped(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_ShowScene);
    displayManagerEventHandler.sceneShown(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_ConfirmationEcho);

    doAnotherFullCycleFromUnpublishToState();
}

TEST_F(ADisplayManager, recoversFromRepublishBeforeFailedMap)
{
    displayManager.setSceneState(sceneId, SceneState::Rendered, "scene is shown now");

    displayManagerEventHandler.scenePublished(sceneId);
    expectRendererCommand(ramses_internal::ERendererCommand_SubscribeScene);
    displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_MapSceneToDisplay);

    unpublish(ramses_internal::ERendererCommand_SubscribeScene);
    expectNoRendererCommand();
    displayManagerEventHandler.scenePublished(sceneId);
    expectNoRendererCommand(); // not trying to subscribe because last command not answered yet

    displayManagerEventHandler.sceneMapped(sceneId, ramses::ERendererEventResult_FAIL);

    expectRendererCommand(ramses_internal::ERendererCommand_SubscribeScene); // now start again from subscribe
    displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_MapSceneToDisplay);
    displayManagerEventHandler.sceneMapped(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_ShowScene);
    displayManagerEventHandler.sceneShown(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_ConfirmationEcho);

    doAnotherFullCycleFromUnpublishToState();
}

TEST_F(ADisplayManager, recoversFromRepublishBeforeFailedShow)
{
    displayManager.setSceneState(sceneId, SceneState::Rendered, "scene is shown now");

    displayManagerEventHandler.scenePublished(sceneId);
    expectRendererCommand(ramses_internal::ERendererCommand_SubscribeScene);
    displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_MapSceneToDisplay);
    displayManagerEventHandler.sceneMapped(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_ShowScene);

    unpublish(ramses_internal::ERendererCommand_MapSceneToDisplay);
    expectNoRendererCommand();
    displayManagerEventHandler.scenePublished(sceneId);
    expectNoRendererCommand(); // not trying to subscribe because last command not answered yet

    displayManagerEventHandler.sceneShown(sceneId, ramses::ERendererEventResult_FAIL);

    expectRendererCommand(ramses_internal::ERendererCommand_SubscribeScene); // now start again from subscribe
    displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_MapSceneToDisplay);
    displayManagerEventHandler.sceneMapped(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_ShowScene);
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
    expectRendererCommand(ramses_internal::ERendererCommand_HideScene);
    displayManagerEventHandler.sceneHidden(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_UnmapSceneFromDisplays);
    displayManagerEventHandler.sceneUnmapped(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_UnsubscribeScene);

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
    expectRendererCommand(ramses_internal::ERendererCommand_HideScene);
    displayManagerEventHandler.sceneHidden(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_UnmapSceneFromDisplays);

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
    expectRendererCommand(ramses_internal::ERendererCommand_HideScene);

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
    expectRendererCommand(ramses_internal::ERendererCommand_HideScene);
    displayManagerEventHandler.sceneHidden(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_UnmapSceneFromDisplays);
    displayManagerEventHandler.sceneUnmapped(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_UnsubscribeScene);

    // unpublish and publish coming together
    unpublish(ramses_internal::ERendererCommand_UnmapSceneFromDisplays);
    expectNoRendererCommand();
    displayManagerEventHandler.scenePublished(sceneId);
    expectNoRendererCommand();

    // only now comes fail reply to unsubscribe command
    displayManagerEventHandler.sceneUnsubscribed(sceneId, ramses::ERendererEventResult_FAIL);

    // trigger new show
    displayManager.setSceneState(sceneId, SceneState::Rendered);

    expectRendererCommand(ramses_internal::ERendererCommand_SubscribeScene);
    displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_MapSceneToDisplay);
    displayManagerEventHandler.sceneMapped(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_ShowScene);
    displayManagerEventHandler.sceneShown(sceneId, ramses::ERendererEventResult_OK);

    doAnotherFullCycleFromUnpublishToState();
}

TEST_F(ADisplayManager, canRecoverFromRepublishBeforeFailedUnmap)
{
    displayManager.setSceneState(sceneId, SceneState::Rendered);
    publishAndExpectToGetToState(SceneState::Rendered);

    displayManager.setSceneState(sceneId, SceneState::Unavailable);
    expectRendererCommand(ramses_internal::ERendererCommand_HideScene);
    displayManagerEventHandler.sceneHidden(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_UnmapSceneFromDisplays);

    // unpublish and publish coming together
    unpublish(ramses_internal::ERendererCommand_HideScene);
    expectNoRendererCommand();
    displayManagerEventHandler.scenePublished(sceneId);
    expectNoRendererCommand();

    // only now comes fail reply to unmap command
    displayManagerEventHandler.sceneUnmapped(sceneId, ramses::ERendererEventResult_FAIL);

    // trigger new show
    displayManager.setSceneState(sceneId, SceneState::Rendered);

    expectRendererCommand(ramses_internal::ERendererCommand_SubscribeScene);
    displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_MapSceneToDisplay);
    displayManagerEventHandler.sceneMapped(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_ShowScene);
    displayManagerEventHandler.sceneShown(sceneId, ramses::ERendererEventResult_OK);

    doAnotherFullCycleFromUnpublishToState();
}

TEST_F(ADisplayManager, canRecoverFromRepublishBeforeFailedHide)
{
    displayManager.setSceneState(sceneId, SceneState::Rendered);
    publishAndExpectToGetToState(SceneState::Rendered);

    displayManager.setSceneState(sceneId, SceneState::Unavailable);
    expectRendererCommand(ramses_internal::ERendererCommand_HideScene);

    // unpublish and publish coming together
    unpublish(ramses_internal::ERendererCommand_ShowScene);
    expectNoRendererCommand();
    displayManagerEventHandler.scenePublished(sceneId);
    expectNoRendererCommand();

    // only now comes fail reply to hide command
    displayManagerEventHandler.sceneHidden(sceneId, ramses::ERendererEventResult_FAIL);

    // trigger new show
    displayManager.setSceneState(sceneId, SceneState::Rendered);

    expectRendererCommand(ramses_internal::ERendererCommand_SubscribeScene);
    displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_MapSceneToDisplay);
    displayManagerEventHandler.sceneMapped(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_ShowScene);
    displayManagerEventHandler.sceneShown(sceneId, ramses::ERendererEventResult_OK);

    doAnotherFullCycleFromUnpublishToState();
}
