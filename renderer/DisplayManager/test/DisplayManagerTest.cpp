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

// to give linker missing definition of platform specific factory create
#include "Platform_Base/PlatformFactory_Base.h"
#include "PlatformFactoryMock.h"
ramses_internal::IPlatformFactory* ramses_internal::PlatformFactory_Base::CreatePlatformFactory(const ramses_internal::RendererConfig&)
{
    return new ::testing::NiceMock<ramses_internal::PlatformFactoryNiceMock>();
}

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

    void publishAndGetToShownState()
    {
        EXPECT_FALSE(isSceneShown());

        displayManagerEventHandler.scenePublished(sceneId);
        expectRendererCommand(ramses_internal::ERendererCommand_SubscribeScene);

        displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_OK);
        expectRendererCommand(ramses_internal::ERendererCommand_MapSceneToDisplay);

        displayManagerEventHandler.sceneMapped(sceneId, ramses::ERendererEventResult_OK);
        expectRendererCommand(ramses_internal::ERendererCommand_ShowScene);

        EXPECT_FALSE(isSceneShown());
        displayManagerEventHandler.sceneShown(sceneId, ramses::ERendererEventResult_OK);
        EXPECT_TRUE(isSceneShown());
    }

    bool isSceneShown() const
    {
        return displayManager.getSceneState(sceneId) == DisplayManager::ESceneState::Rendered;
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

    void doAnotherFullCycleFromUnpublishToShown(ramses_internal::ERendererCommand lastSuccessfulCommand = ramses_internal::ERendererCommand_ShowScene)
    {
        unpublish(lastSuccessfulCommand);
        publishAndGetToShownState();
        expectNoRendererCommand();
    }

    void doAnotherFullCycleFromUnpublishToShownWithShowTriggered(ramses_internal::ERendererCommand lastSuccessfulCommand = ramses_internal::ERendererCommand_ShowScene)
    {
        unpublish(lastSuccessfulCommand);
        displayManager.showSceneOnDisplay(sceneId, displayId);
        publishAndGetToShownState();
        expectNoRendererCommand();
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
    }

};

TEST_F(ADisplayManager, willShowSceneWhenPublished)
{
    displayManager.showSceneOnDisplay(sceneId, displayId, 0);
    expectNoRendererCommand();
    publishAndGetToShownState();
    expectNoRendererCommand();
    EXPECT_EQ(displayId, displayManager.getDisplaySceneIsMappedTo(sceneId));

    doAnotherFullCycleFromUnpublishToShown();
}

TEST_F(ADisplayManagerWithAutomapping, willShowSceneWhenPublished)
{
    publishAndGetToShownState();
    expectNoRendererCommand();
    EXPECT_EQ(displayId, displayManager.getDisplaySceneIsMappedTo(sceneId));

    doAnotherFullCycleFromUnpublishToShown();
}

TEST_F(ADisplayManager, willShowSceneWhenPublishedAndLogsConfirmation)
{
    displayManager.showSceneOnDisplay(sceneId, displayId, 0, "dummy msg");
    expectNoRendererCommand();
    publishAndGetToShownState();
    expectRendererCommand(ramses_internal::ERendererCommand_ConfirmationEcho);

    doAnotherFullCycleFromUnpublishToShown();
}

TEST_F(ADisplayManager, willShowSceneAlreadyPublished)
{
    displayManagerEventHandler.scenePublished(sceneId);

    displayManager.showSceneOnDisplay(sceneId, displayId, 0);
    expectRendererCommand(ramses_internal::ERendererCommand_SubscribeScene);

    displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_MapSceneToDisplay);

    displayManagerEventHandler.sceneMapped(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_ShowScene);

    displayManagerEventHandler.sceneShown(sceneId, ramses::ERendererEventResult_OK);
    expectNoRendererCommand();

    doAnotherFullCycleFromUnpublishToShown();
}

TEST_F(ADisplayManager, doesNothingIfSubscribeFailed)
{
    displayManagerEventHandler.scenePublished(sceneId);

    displayManager.showSceneOnDisplay(sceneId, displayId, 0);
    expectRendererCommand(ramses_internal::ERendererCommand_SubscribeScene);

    displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_FAIL);
    expectNoRendererCommand();

    doAnotherFullCycleFromUnpublishToShown(ramses_internal::ERendererCommand_PublishedScene);
}

TEST_F(ADisplayManager, doesNothingIfMapFailed)
{
    displayManagerEventHandler.scenePublished(sceneId);

    displayManager.showSceneOnDisplay(sceneId, displayId, 0);
    expectRendererCommand(ramses_internal::ERendererCommand_SubscribeScene);

    displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_MapSceneToDisplay);

    displayManagerEventHandler.sceneMapped(sceneId, ramses::ERendererEventResult_FAIL);
    expectNoRendererCommand();

    doAnotherFullCycleFromUnpublishToShown(ramses_internal::ERendererCommand_SubscribeScene);
}

TEST_F(ADisplayManager, doesNothingIfShowFailed)
{
    displayManagerEventHandler.scenePublished(sceneId);

    displayManager.showSceneOnDisplay(sceneId, displayId, 0);
    expectRendererCommand(ramses_internal::ERendererCommand_SubscribeScene);

    displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_MapSceneToDisplay);

    displayManagerEventHandler.sceneMapped(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_ShowScene);

    displayManagerEventHandler.sceneShown(sceneId, ramses::ERendererEventResult_FAIL);
    expectNoRendererCommand();

    doAnotherFullCycleFromUnpublishToShown(ramses_internal::ERendererCommand_MapSceneToDisplay);
}

TEST_F(ADisplayManager, hidesShownScene)
{
    displayManager.showSceneOnDisplay(sceneId, displayId, 0);
    expectNoRendererCommand();
    publishAndGetToShownState();
    expectNoRendererCommand();

    displayManager.hideScene(sceneId);
    expectRendererCommand(ramses_internal::ERendererCommand_HideScene);

    displayManagerEventHandler.sceneHidden(sceneId, ramses::ERendererEventResult_OK);
    expectNoRendererCommand();
    EXPECT_EQ(displayId, displayManager.getDisplaySceneIsMappedTo(sceneId));

    // show has to be re-triggered because explicit state was requested before and target state is not shown anymore
    doAnotherFullCycleFromUnpublishToShownWithShowTriggered(ramses_internal::ERendererCommand_HideScene);
}

TEST_F(ADisplayManagerWithAutomapping, hidesShownScene)
{
    publishAndGetToShownState();
    expectNoRendererCommand();

    displayManager.hideScene(sceneId);
    expectRendererCommand(ramses_internal::ERendererCommand_HideScene);

    displayManagerEventHandler.sceneHidden(sceneId, ramses::ERendererEventResult_OK);
    expectNoRendererCommand();

    doAnotherFullCycleFromUnpublishToShown(ramses_internal::ERendererCommand_HideScene);
}

TEST_F(ADisplayManager, unmapsShownScene)
{
    displayManager.showSceneOnDisplay(sceneId, displayId, 0);
    expectNoRendererCommand();
    publishAndGetToShownState();
    expectNoRendererCommand();

    displayManager.unmapScene(sceneId);
    expectRendererCommand(ramses_internal::ERendererCommand_HideScene);

    displayManagerEventHandler.sceneHidden(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_UnmapSceneFromDisplays);
    EXPECT_EQ(displayId, displayManager.getDisplaySceneIsMappedTo(sceneId));

    displayManagerEventHandler.sceneUnmapped(sceneId, ramses::ERendererEventResult_OK);
    expectNoRendererCommand();
    EXPECT_EQ(ramses::InvalidDisplayId, displayManager.getDisplaySceneIsMappedTo(sceneId));

    // show has to be re-triggered because explicit state was requested before and target state is not shown anymore
    doAnotherFullCycleFromUnpublishToShownWithShowTriggered(ramses_internal::ERendererCommand_UnmapSceneFromDisplays);
}

TEST_F(ADisplayManagerWithAutomapping, unmapsShownScene)
{
    publishAndGetToShownState();
    expectNoRendererCommand();

    displayManager.unmapScene(sceneId);
    expectRendererCommand(ramses_internal::ERendererCommand_HideScene);

    displayManagerEventHandler.sceneHidden(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_UnmapSceneFromDisplays);
    EXPECT_EQ(displayId, displayManager.getDisplaySceneIsMappedTo(sceneId));

    displayManagerEventHandler.sceneUnmapped(sceneId, ramses::ERendererEventResult_OK);
    expectNoRendererCommand();
    EXPECT_EQ(ramses::InvalidDisplayId, displayManager.getDisplaySceneIsMappedTo(sceneId));

    doAnotherFullCycleFromUnpublishToShown(ramses_internal::ERendererCommand_UnmapSceneFromDisplays);
}

TEST_F(ADisplayManager, unmapsMappedScene)
{
    displayManager.showSceneOnDisplay(sceneId, displayId, 0);
    expectNoRendererCommand();
    publishAndGetToShownState();
    expectNoRendererCommand();

    // hide first
    displayManager.hideScene(sceneId);
    expectRendererCommand(ramses_internal::ERendererCommand_HideScene);
    displayManagerEventHandler.sceneHidden(sceneId, ramses::ERendererEventResult_OK);
    expectNoRendererCommand();

    displayManager.unmapScene(sceneId);
    expectRendererCommand(ramses_internal::ERendererCommand_UnmapSceneFromDisplays);

    displayManagerEventHandler.sceneUnmapped(sceneId, ramses::ERendererEventResult_OK);
    expectNoRendererCommand();
    EXPECT_EQ(ramses::InvalidDisplayId, displayManager.getDisplaySceneIsMappedTo(sceneId));

    // show has to be re-triggered because explicit state was requested before and target state is not shown anymore
    doAnotherFullCycleFromUnpublishToShownWithShowTriggered(ramses_internal::ERendererCommand_UnmapSceneFromDisplays);
}

TEST_F(ADisplayManagerWithAutomapping, unmapsMappedScene)
{
    publishAndGetToShownState();
    expectNoRendererCommand();

    // hide first
    displayManager.hideScene(sceneId);
    expectRendererCommand(ramses_internal::ERendererCommand_HideScene);
    displayManagerEventHandler.sceneHidden(sceneId, ramses::ERendererEventResult_OK);
    expectNoRendererCommand();

    displayManager.unmapScene(sceneId);
    expectRendererCommand(ramses_internal::ERendererCommand_UnmapSceneFromDisplays);

    displayManagerEventHandler.sceneUnmapped(sceneId, ramses::ERendererEventResult_OK);
    expectNoRendererCommand();
    EXPECT_EQ(ramses::InvalidDisplayId, displayManager.getDisplaySceneIsMappedTo(sceneId));

    doAnotherFullCycleFromUnpublishToShown(ramses_internal::ERendererCommand_UnmapSceneFromDisplays);
}

TEST_F(ADisplayManager, unsubscribesShownScene)
{
    displayManager.showSceneOnDisplay(sceneId, displayId, 0);
    expectNoRendererCommand();
    publishAndGetToShownState();
    expectNoRendererCommand();

    displayManager.unsubscribeScene(sceneId);
    expectRendererCommand(ramses_internal::ERendererCommand_HideScene);

    displayManagerEventHandler.sceneHidden(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_UnmapSceneFromDisplays);

    displayManagerEventHandler.sceneUnmapped(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_UnsubscribeScene);

    displayManagerEventHandler.sceneUnsubscribed(sceneId, ramses::ERendererEventResult_OK);
    expectNoRendererCommand();

    // show has to be re-triggered because explicit state was requested before and target state is not shown anymore
    doAnotherFullCycleFromUnpublishToShownWithShowTriggered(ramses_internal::ERendererCommand_UnsubscribeScene);
}

TEST_F(ADisplayManagerWithAutomapping, unsubscribesShownScene)
{
    publishAndGetToShownState();
    expectNoRendererCommand();

    displayManager.unsubscribeScene(sceneId);
    expectRendererCommand(ramses_internal::ERendererCommand_HideScene);

    displayManagerEventHandler.sceneHidden(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_UnmapSceneFromDisplays);

    displayManagerEventHandler.sceneUnmapped(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_UnsubscribeScene);

    displayManagerEventHandler.sceneUnsubscribed(sceneId, ramses::ERendererEventResult_OK);
    expectNoRendererCommand();

    doAnotherFullCycleFromUnpublishToShown(ramses_internal::ERendererCommand_UnsubscribeScene);
}

TEST_F(ADisplayManager, unsubscribesMappedScene)
{
    displayManager.showSceneOnDisplay(sceneId, displayId, 0);
    expectNoRendererCommand();
    publishAndGetToShownState();
    expectNoRendererCommand();

    // hide first
    displayManager.hideScene(sceneId);
    expectRendererCommand(ramses_internal::ERendererCommand_HideScene);
    displayManagerEventHandler.sceneHidden(sceneId, ramses::ERendererEventResult_OK);
    expectNoRendererCommand();

    displayManager.unsubscribeScene(sceneId);
    expectRendererCommand(ramses_internal::ERendererCommand_UnmapSceneFromDisplays);

    displayManagerEventHandler.sceneUnmapped(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_UnsubscribeScene);

    displayManagerEventHandler.sceneUnsubscribed(sceneId, ramses::ERendererEventResult_OK);
    expectNoRendererCommand();

    // show has to be re-triggered because explicit state was requested before and target state is not shown anymore
    doAnotherFullCycleFromUnpublishToShownWithShowTriggered(ramses_internal::ERendererCommand_UnsubscribeScene);
}

TEST_F(ADisplayManagerWithAutomapping, unsubscribesMappedScene)
{
    publishAndGetToShownState();
    expectNoRendererCommand();

    // hide first
    displayManager.hideScene(sceneId);
    expectRendererCommand(ramses_internal::ERendererCommand_HideScene);
    displayManagerEventHandler.sceneHidden(sceneId, ramses::ERendererEventResult_OK);
    expectNoRendererCommand();

    displayManager.unsubscribeScene(sceneId);
    expectRendererCommand(ramses_internal::ERendererCommand_UnmapSceneFromDisplays);

    displayManagerEventHandler.sceneUnmapped(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_UnsubscribeScene);

    displayManagerEventHandler.sceneUnsubscribed(sceneId, ramses::ERendererEventResult_OK);
    expectNoRendererCommand();

    doAnotherFullCycleFromUnpublishToShown(ramses_internal::ERendererCommand_UnsubscribeScene);
}

TEST_F(ADisplayManager, unsubscribesSubscribedScene)
{
    displayManager.showSceneOnDisplay(sceneId, displayId, 0);
    expectNoRendererCommand();
    publishAndGetToShownState();
    expectNoRendererCommand();

    // unmap first
    displayManager.unmapScene(sceneId);
    expectRendererCommand(ramses_internal::ERendererCommand_HideScene);
    displayManagerEventHandler.sceneHidden(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_UnmapSceneFromDisplays);
    displayManagerEventHandler.sceneUnmapped(sceneId, ramses::ERendererEventResult_OK);
    expectNoRendererCommand();

    displayManager.unsubscribeScene(sceneId);
    expectRendererCommand(ramses_internal::ERendererCommand_UnsubscribeScene);

    displayManagerEventHandler.sceneUnsubscribed(sceneId, ramses::ERendererEventResult_OK);
    expectNoRendererCommand();

    // show has to be re-triggered because explicit state was requested before and target state is not shown anymore
    doAnotherFullCycleFromUnpublishToShownWithShowTriggered(ramses_internal::ERendererCommand_UnsubscribeScene);
}

TEST_F(ADisplayManagerWithAutomapping, unsubscribesSubscribedScene)
{
    publishAndGetToShownState();
    expectNoRendererCommand();

    // unmap first
    displayManager.unmapScene(sceneId);
    expectRendererCommand(ramses_internal::ERendererCommand_HideScene);
    displayManagerEventHandler.sceneHidden(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_UnmapSceneFromDisplays);
    displayManagerEventHandler.sceneUnmapped(sceneId, ramses::ERendererEventResult_OK);
    expectNoRendererCommand();

    displayManager.unsubscribeScene(sceneId);
    expectRendererCommand(ramses_internal::ERendererCommand_UnsubscribeScene);

    displayManagerEventHandler.sceneUnsubscribed(sceneId, ramses::ERendererEventResult_OK);
    expectNoRendererCommand();

    doAnotherFullCycleFromUnpublishToShown(ramses_internal::ERendererCommand_UnsubscribeScene);
}

TEST_F(ADisplayManager, handlesSceneUnpublishWhileProcessingSubscribe)
{
    displayManager.showSceneOnDisplay(sceneId, displayId, 0);
    expectNoRendererCommand();

    displayManagerEventHandler.scenePublished(sceneId);
    expectRendererCommand(ramses_internal::ERendererCommand_SubscribeScene);

    //unexpected unpublish!!
    unpublish(ramses_internal::ERendererCommand_PublishedScene);
    displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_FAIL);

    doAnotherFullCycleFromUnpublishToShown(ramses_internal::ERendererCommand_UnpublishedScene);
}

TEST_F(ADisplayManagerWithAutomapping, handlesSceneUnpublishWhileProcessingSubscribe)
{
    displayManagerEventHandler.scenePublished(sceneId);
    expectRendererCommand(ramses_internal::ERendererCommand_SubscribeScene);

    //unexpected unpublish!!
    unpublish(ramses_internal::ERendererCommand_PublishedScene);
    displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_FAIL);

    doAnotherFullCycleFromUnpublishToShown(ramses_internal::ERendererCommand_UnpublishedScene);
}

TEST_F(ADisplayManager, handlesSceneUnpublishWhileProcessingMap)
{
    displayManager.showSceneOnDisplay(sceneId, displayId, 0);
    expectNoRendererCommand();

    displayManagerEventHandler.scenePublished(sceneId);
    expectRendererCommand(ramses_internal::ERendererCommand_SubscribeScene);

    displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_MapSceneToDisplay);

    //unexpected unpublish!!
    unpublish(ramses_internal::ERendererCommand_SubscribeScene);
    displayManagerEventHandler.sceneMapped(sceneId, ramses::ERendererEventResult_FAIL);

    doAnotherFullCycleFromUnpublishToShown(ramses_internal::ERendererCommand_SubscribeScene);
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

    doAnotherFullCycleFromUnpublishToShown(ramses_internal::ERendererCommand_SubscribeScene);
}

TEST_F(ADisplayManager, handlesSceneUnpublishWhileProcessingShow)
{
    displayManager.showSceneOnDisplay(sceneId, displayId, 0);
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

    doAnotherFullCycleFromUnpublishToShown(ramses_internal::ERendererCommand_MapSceneToDisplay);
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

    doAnotherFullCycleFromUnpublishToShown(ramses_internal::ERendererCommand_MapSceneToDisplay);
}

TEST_F(ADisplayManager, canNotShowAShownSceneOnAnotherDisplay)
{
    displayManager.showSceneOnDisplay(sceneId, displayId, 0);
    expectNoRendererCommand();
    publishAndGetToShownState();
    expectNoRendererCommand();

    displayManager.showSceneOnDisplay(sceneId, otherDisplayId);
    expectNoRendererCommand();

    doAnotherFullCycleFromUnpublishToShown(ramses_internal::ERendererCommand_ShowScene);
}

TEST_F(ADisplayManager, canNotShowAMappedSceneOnAnotherDisplay)
{
    displayManager.showSceneOnDisplay(sceneId, displayId, 0);
    expectNoRendererCommand();
    publishAndGetToShownState();
    expectNoRendererCommand();

    // hide first
    displayManager.hideScene(sceneId);
    expectRendererCommand(ramses_internal::ERendererCommand_HideScene);
    displayManagerEventHandler.sceneHidden(sceneId, ramses::ERendererEventResult_OK);
    expectNoRendererCommand();
    EXPECT_EQ(displayId, displayManager.getDisplaySceneIsMappedTo(sceneId));

    displayManager.showSceneOnDisplay(sceneId, otherDisplayId); // produces error message
    expectNoRendererCommand();

    // show has to be re-triggered because explicit state was requested before and target state is not shown anymore
    doAnotherFullCycleFromUnpublishToShownWithShowTriggered(ramses_internal::ERendererCommand_HideScene);
    // scene will be shown on default display because show attempt on other display failed
    EXPECT_EQ(displayId, displayManager.getDisplaySceneIsMappedTo(sceneId));
}

TEST_F(ADisplayManager, canShowASubscribedSceneOnAnotherDisplay)
{
    displayManager.showSceneOnDisplay(sceneId, displayId, 0);
    expectNoRendererCommand();
    publishAndGetToShownState();
    expectNoRendererCommand();

    // unmap first
    displayManager.unmapScene(sceneId);
    expectRendererCommand(ramses_internal::ERendererCommand_HideScene);
    displayManagerEventHandler.sceneHidden(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_UnmapSceneFromDisplays);
    displayManagerEventHandler.sceneUnmapped(sceneId, ramses::ERendererEventResult_OK);
    expectNoRendererCommand();

    displayManager.showSceneOnDisplay(sceneId, otherDisplayId);
    expectRendererCommand(ramses_internal::ERendererCommand_MapSceneToDisplay);
    displayManagerEventHandler.sceneMapped(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_ShowScene);
    displayManagerEventHandler.sceneShown(sceneId, ramses::ERendererEventResult_OK);
    expectNoRendererCommand();

    doAnotherFullCycleFromUnpublishToShown(ramses_internal::ERendererCommand_ShowScene);
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
    displayManager.showSceneOnDisplay(sceneId, displayId, 0);
    expectNoRendererCommand();
    publishAndGetToShownState();
    expectNoRendererCommand();

    // explicitly hide
    displayManager.hideScene(sceneId);
    expectRendererCommand(ramses_internal::ERendererCommand_HideScene);
    EXPECT_FALSE(isSceneShown()); // hide changes internal shown state right away
    displayManagerEventHandler.sceneHidden(sceneId, ramses::ERendererEventResult_OK);

    // show again
    displayManager.showSceneOnDisplay(sceneId, displayId, 0);
    expectRendererCommand(ramses_internal::ERendererCommand_ShowScene);
    EXPECT_FALSE(isSceneShown());
    displayManagerEventHandler.sceneShown(sceneId, ramses::ERendererEventResult_OK);
    EXPECT_TRUE(isSceneShown());

    // unexpected unpublish!!
    unpublish(ramses_internal::ERendererCommand_ShowScene);
    EXPECT_FALSE(isSceneShown());

    doAnotherFullCycleFromUnpublishToShown(ramses_internal::ERendererCommand_UnpublishedScene);
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

    displayManager.showSceneOnDisplay(sceneId, customDisplay);
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

    doAnotherFullCycleFromUnpublishToShown(ramses_internal::ERendererCommand_ShowScene);
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

    doAnotherFullCycleFromUnpublishToShown(ramses_internal::ERendererCommand_ShowScene);
}

TEST_F(ADisplayManager, continuesToShowSceneAndLogsConfirmationAfterReconnect)
{
    displayManagerEventHandler.scenePublished(sceneId);
    expectNoRendererCommand();

    displayManager.showSceneOnDisplay(sceneId, displayId, 0, "scene is shown now");
    expectRendererCommand(ramses_internal::ERendererCommand_SubscribeScene);

    displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_MapSceneToDisplay);

    // simulate disconnect, renderer framework sends unpublish for all its scenes
    unpublish(ramses_internal::ERendererCommand_SubscribeScene);
    expectNoRendererCommand();
    // after unpublish, previous command will fail on execution
    displayManagerEventHandler.sceneMapped(sceneId, ramses::ERendererEventResult_FAIL);

    publishAndGetToShownState();
    expectRendererCommand(ramses_internal::ERendererCommand_ConfirmationEcho);

    doAnotherFullCycleFromUnpublishToShown();
}

/////////
// Tests where unpublish comes after already failed last command on renderer side
// and DM is supposed to get scene to shown state
/////////

TEST_F(ADisplayManager, recoversFromRepublishAfterFailedSubscribe)
{
    displayManager.showSceneOnDisplay(sceneId, displayId, 0, "scene is shown now");

    displayManagerEventHandler.scenePublished(sceneId);
    expectRendererCommand(ramses_internal::ERendererCommand_SubscribeScene);
    unpublish(ramses_internal::ERendererCommand_PublishedScene);
    expectNoRendererCommand();
    displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_FAIL);

    publishAndGetToShownState();
    expectRendererCommand(ramses_internal::ERendererCommand_ConfirmationEcho);
}

TEST_F(ADisplayManager, recoversFromRepublishAfterFailedMap)
{
    displayManager.showSceneOnDisplay(sceneId, displayId, 0, "scene is shown now");

    displayManagerEventHandler.scenePublished(sceneId);
    expectRendererCommand(ramses_internal::ERendererCommand_SubscribeScene);
    displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_MapSceneToDisplay);

    unpublish(ramses_internal::ERendererCommand_SubscribeScene);
    expectNoRendererCommand();
    displayManagerEventHandler.sceneMapped(sceneId, ramses::ERendererEventResult_FAIL);

    publishAndGetToShownState();
    expectRendererCommand(ramses_internal::ERendererCommand_ConfirmationEcho);
}

TEST_F(ADisplayManager, recoversFromRepublishAfterFailedShow)
{
    displayManager.showSceneOnDisplay(sceneId, displayId, 0, "scene is shown now");

    displayManagerEventHandler.scenePublished(sceneId);
    expectRendererCommand(ramses_internal::ERendererCommand_SubscribeScene);
    displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_MapSceneToDisplay);
    displayManagerEventHandler.sceneMapped(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_ShowScene);

    unpublish(ramses_internal::ERendererCommand_MapSceneToDisplay);
    expectNoRendererCommand();
    displayManagerEventHandler.sceneShown(sceneId, ramses::ERendererEventResult_FAIL);

    publishAndGetToShownState();
    expectRendererCommand(ramses_internal::ERendererCommand_ConfirmationEcho);
}

/////////
// Tests where unpublish comes before failed last command on renderer side
// and DM is supposed to get scene to shown state
/////////

TEST_F(ADisplayManager, recoversFromRepublishBeforeFailedSubscribe)
{
    displayManager.showSceneOnDisplay(sceneId, displayId, 0, "scene is shown now");

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

    doAnotherFullCycleFromUnpublishToShown();
}

TEST_F(ADisplayManager, recoversFromRepublishBeforeFailedMap)
{
    displayManager.showSceneOnDisplay(sceneId, displayId, 0, "scene is shown now");

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

    doAnotherFullCycleFromUnpublishToShown();
}

TEST_F(ADisplayManager, recoversFromRepublishBeforeFailedShow)
{
    displayManager.showSceneOnDisplay(sceneId, displayId, 0, "scene is shown now");

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

    doAnotherFullCycleFromUnpublishToShown();
}

/////////
// Tests where unpublish comes after already failed last command on renderer side
// and DM can get scene to shown state again
// (DM does not do that automatically because states other than show are not automatically reached again after unpublish)
/////////

TEST_F(ADisplayManager, canRecoverFromRepublishAfterFailedUnsubscribe)
{
    displayManager.showSceneOnDisplay(sceneId, displayId);
    publishAndGetToShownState();
    expectNoRendererCommand();

    displayManager.unsubscribeScene(sceneId);
    expectRendererCommand(ramses_internal::ERendererCommand_HideScene);
    displayManagerEventHandler.sceneHidden(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_UnmapSceneFromDisplays);
    displayManagerEventHandler.sceneUnmapped(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_UnsubscribeScene);

    unpublish(ramses_internal::ERendererCommand_UnmapSceneFromDisplays);
    expectNoRendererCommand();
    displayManagerEventHandler.sceneUnsubscribed(sceneId, ramses::ERendererEventResult_FAIL);

    displayManager.showSceneOnDisplay(sceneId, displayId);
    publishAndGetToShownState();
}

TEST_F(ADisplayManager, canRecoverFromRepublishAfterFailedUnmap)
{
    displayManager.showSceneOnDisplay(sceneId, displayId);
    publishAndGetToShownState();
    expectNoRendererCommand();

    displayManager.unsubscribeScene(sceneId);
    expectRendererCommand(ramses_internal::ERendererCommand_HideScene);
    displayManagerEventHandler.sceneHidden(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_UnmapSceneFromDisplays);

    unpublish(ramses_internal::ERendererCommand_HideScene);
    expectNoRendererCommand();
    displayManagerEventHandler.sceneUnmapped(sceneId, ramses::ERendererEventResult_FAIL);

    displayManager.showSceneOnDisplay(sceneId, displayId);
    publishAndGetToShownState();
}

TEST_F(ADisplayManager, canRecoverFromRepublishAfterFailedHide)
{
    displayManager.showSceneOnDisplay(sceneId, displayId);
    publishAndGetToShownState();
    expectNoRendererCommand();

    displayManager.unsubscribeScene(sceneId);
    expectRendererCommand(ramses_internal::ERendererCommand_HideScene);

    unpublish(ramses_internal::ERendererCommand_ShowScene);
    expectNoRendererCommand();
    displayManagerEventHandler.sceneHidden(sceneId, ramses::ERendererEventResult_FAIL);

    displayManager.showSceneOnDisplay(sceneId, displayId);
    publishAndGetToShownState();
}

/////////
// Tests where unpublish comes before failed last command on renderer side
// and DM can get scene to shown state again
// (DM does not do that automatically because states other than show are not automatically reached again after unpublish)
/////////

TEST_F(ADisplayManager, canRecoverFromRepublishBeforeFailedUnsubscribe)
{
    displayManager.showSceneOnDisplay(sceneId, displayId);
    publishAndGetToShownState();
    expectNoRendererCommand();

    displayManager.unsubscribeScene(sceneId);
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
    displayManager.showSceneOnDisplay(sceneId, displayId);

    expectRendererCommand(ramses_internal::ERendererCommand_SubscribeScene);
    displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_MapSceneToDisplay);
    displayManagerEventHandler.sceneMapped(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_ShowScene);
    displayManagerEventHandler.sceneShown(sceneId, ramses::ERendererEventResult_OK);

    doAnotherFullCycleFromUnpublishToShown();
}

TEST_F(ADisplayManager, canRecoverFromRepublishBeforeFailedUnmap)
{
    displayManager.showSceneOnDisplay(sceneId, displayId);
    publishAndGetToShownState();
    expectNoRendererCommand();

    displayManager.unsubscribeScene(sceneId);
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
    displayManager.showSceneOnDisplay(sceneId, displayId);

    expectRendererCommand(ramses_internal::ERendererCommand_SubscribeScene);
    displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_MapSceneToDisplay);
    displayManagerEventHandler.sceneMapped(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_ShowScene);
    displayManagerEventHandler.sceneShown(sceneId, ramses::ERendererEventResult_OK);

    doAnotherFullCycleFromUnpublishToShown();
}

TEST_F(ADisplayManager, canRecoverFromRepublishBeforeFailedHide)
{
    displayManager.showSceneOnDisplay(sceneId, displayId);
    publishAndGetToShownState();
    expectNoRendererCommand();

    displayManager.unsubscribeScene(sceneId);
    expectRendererCommand(ramses_internal::ERendererCommand_HideScene);

    // unpublish and publish coming together
    unpublish(ramses_internal::ERendererCommand_ShowScene);
    expectNoRendererCommand();
    displayManagerEventHandler.scenePublished(sceneId);
    expectNoRendererCommand();

    // only now comes fail reply to hide command
    displayManagerEventHandler.sceneHidden(sceneId, ramses::ERendererEventResult_FAIL);

    // trigger new show
    displayManager.showSceneOnDisplay(sceneId, displayId);

    expectRendererCommand(ramses_internal::ERendererCommand_SubscribeScene);
    displayManagerEventHandler.sceneSubscribed(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_MapSceneToDisplay);
    displayManagerEventHandler.sceneMapped(sceneId, ramses::ERendererEventResult_OK);
    expectRendererCommand(ramses_internal::ERendererCommand_ShowScene);
    displayManagerEventHandler.sceneShown(sceneId, ramses::ERendererEventResult_OK);

    doAnotherFullCycleFromUnpublishToShown();
}
