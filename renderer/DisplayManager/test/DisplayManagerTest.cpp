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
        displayConfig.setIntegrityEGLDisplayID(1u);
        displayConfig.setWaylandIviSurfaceID(0u);
        displayId = displayManager.createDisplay(displayConfig);

        displayConfig.setIntegrityEGLDisplayID(2u);
        displayConfig.setWaylandIviSurfaceID(1u);
        otherDisplayId = displayManager.createDisplay(displayConfig);
        renderer.doOneLoop();
        displayManager.dispatchAndFlush();

        EXPECT_EQ(2u, renderer.impl.getRenderer().getRenderer().getDisplayControllerCount());
    }

protected:
    void expectRendererCommand(ramses_internal::ERendererCommand cmd)
    {
        const ramses_internal::RendererCommands& cmdBuffer = renderer.impl.getCommands();
        const ramses_internal::RendererCommandContainer& cmds = cmdBuffer.getCommands();
        const uint32_t cmdCount = cmds.getTotalCommandCount();
        ASSERT_GT(cmdCount, 0u);
        EXPECT_EQ(cmd, cmds.getCommandType(cmdCount - 1u));
    }

    void expectNoRendererCommand()
    {
        const ramses_internal::RendererCommands& cmdBuffer = renderer.impl.getCommands();
        const ramses_internal::RendererCommandContainer& cmds = cmdBuffer.getCommands();
        const uint32_t cmdCount = cmds.getTotalCommandCount();
        EXPECT_EQ(0u, cmdCount);
    }

    void clearRendererCommands()
    {
        const_cast<ramses_internal::RendererCommands&>(renderer.impl.getCommands()).clear();
    }

    void scenePublished()
    {
        displayManagerEventHandler.scenePublished(sceneId);
    }

    void sceneUnpublished()
    {
        displayManagerEventHandler.sceneUnpublished(sceneId);
    }

    void sceneSubscribed(ramses::ERendererEventResult result = ramses::ERendererEventResult_OK)
    {
        displayManagerEventHandler.sceneSubscribed(sceneId, result);
    }

    void sceneUnsubscribed(ramses::ERendererEventResult result = ramses::ERendererEventResult_OK)
    {
        displayManagerEventHandler.sceneUnsubscribed(sceneId, result);
    }

    void sceneMapped(ramses::ERendererEventResult result = ramses::ERendererEventResult_OK)
    {
        displayManagerEventHandler.sceneMapped(sceneId, result);
    }

    void sceneUnmapped(ramses::ERendererEventResult result = ramses::ERendererEventResult_OK)
    {
        displayManagerEventHandler.sceneUnmapped(sceneId, result);
    }

    void sceneShown(ramses::ERendererEventResult result = ramses::ERendererEventResult_OK)
    {
        displayManagerEventHandler.sceneShown(sceneId, result);
    }

    void sceneHidden(ramses::ERendererEventResult result = ramses::ERendererEventResult_OK)
    {
        displayManagerEventHandler.sceneHidden(sceneId, result);
    }

    void displayCreated(ramses::displayId_t newDisplayId, ramses::ERendererEventResult result = ramses::ERendererEventResult_OK)
    {
        displayManagerEventHandler.displayCreated(newDisplayId, result);
    }

    void expectSubscribeScene()
    {
        expectRendererCommand(ramses_internal::ERendererCommand_SubscribeScene);
    }

    void expectUnsubscribeScene()
    {
        expectRendererCommand(ramses_internal::ERendererCommand_UnsubscribeScene);
    }

    void expectMapScene()
    {
        expectRendererCommand(ramses_internal::ERendererCommand_MapSceneToDisplay);
    }

    void expectUnmapScene()
    {
        expectRendererCommand(ramses_internal::ERendererCommand_UnmapSceneFromDisplays);
    }

    void expectShowScene()
    {
        expectRendererCommand(ramses_internal::ERendererCommand_ShowScene);
    }

    void expectConfirmationMessage()
    {
        expectRendererCommand(ramses_internal::ERendererCommand_ConfirmationEcho);
    }

    void expectHideScene()
    {
        expectRendererCommand(ramses_internal::ERendererCommand_HideScene);
    }

    void showSceneAndExpectShow()
    {
        displayManager.showSceneOnDisplay(sceneId, displayId, 0);
        expectNoRendererCommand();

        scenePublished();
        expectSubscribeScene();

        sceneSubscribed();
        expectMapScene();

        sceneMapped();
        expectShowScene();
        clearRendererCommands();

        sceneShown();
        expectNoRendererCommand();
    }

    ramses::RamsesFramework ramsesFramework;
    ramses::RamsesRenderer renderer;
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

TEST_F(ADisplayManager, showsPublishedScene)
{
    scenePublished();
    expectNoRendererCommand();

    displayManager.showSceneOnDisplay(sceneId, displayId);
    expectSubscribeScene();

    sceneSubscribed();
    expectMapScene();

    sceneMapped();
    expectShowScene();
    clearRendererCommands();

    sceneShown();
    expectNoRendererCommand();
}

TEST_F(ADisplayManager, showsPublishedSceneAndLogsConfirmation)
{
    scenePublished();
    expectNoRendererCommand();

    displayManager.showSceneOnDisplay(sceneId, displayId, 0, "scene is shown now");
    expectSubscribeScene();

    sceneSubscribed();
    expectMapScene();

    sceneMapped();
    expectShowScene();

    sceneShown();
    expectConfirmationMessage();
}

TEST_F(ADisplayManagerWithAutomapping, showsPublishedScene)
{
    scenePublished();
    expectSubscribeScene();

    sceneSubscribed();
    expectMapScene();

    sceneMapped();
    expectShowScene();

    sceneShown();
}

TEST_F(ADisplayManager, showsSceneWhenPublishedAfterShowSceneRequest)
{
    displayManager.showSceneOnDisplay(sceneId, displayId);
    expectNoRendererCommand();

    scenePublished();
    expectSubscribeScene();

    sceneSubscribed();
    expectMapScene();

    sceneMapped();
    expectShowScene();

    sceneShown();
}

TEST_F(ADisplayManagerWithAutomapping, showsSceneWhenPublishedAfterShowSceneRequest)
{
    displayManager.showSceneOnDisplay(sceneId, displayId);
    expectNoRendererCommand();

    scenePublished();
    expectSubscribeScene();

    sceneSubscribed();
    expectMapScene();

    sceneMapped();
    expectShowScene();

    sceneShown();
}

TEST_F(ADisplayManager, doesNothingIfSubscribeFailed)
{
    scenePublished();
    expectNoRendererCommand();

    displayManager.showSceneOnDisplay(sceneId, displayId);
    expectSubscribeScene();

    clearRendererCommands();
    sceneSubscribed(ramses::ERendererEventResult_FAIL);
    expectNoRendererCommand();
}

TEST_F(ADisplayManager, doesNothingIfMapFailed)
{
    scenePublished();
    expectNoRendererCommand();

    displayManager.showSceneOnDisplay(sceneId, displayId);
    expectSubscribeScene();

    sceneSubscribed();
    expectMapScene();

    clearRendererCommands();
    sceneMapped(ramses::ERendererEventResult_FAIL);
    expectNoRendererCommand();
}

TEST_F(ADisplayManager, doesNothingIfShowFailed)
{
    scenePublished();
    expectNoRendererCommand();

    displayManager.showSceneOnDisplay(sceneId, displayId);
    expectSubscribeScene();

    sceneSubscribed();
    expectMapScene();

    sceneMapped();
    expectShowScene();

    clearRendererCommands();
    sceneShown(ramses::ERendererEventResult_FAIL);
    expectNoRendererCommand();
}

TEST_F(ADisplayManager, hidesShownScene)
{
    showSceneAndExpectShow();
    //scene shown

    displayManager.hideScene(sceneId);
    expectHideScene();

    //don't expect further commands
    clearRendererCommands();
    sceneHidden();
    expectNoRendererCommand();
}

TEST_F(ADisplayManagerWithAutomapping, hidesShownScene)
{
    showSceneAndExpectShow();
    //scene shown

    displayManager.hideScene(sceneId);
    expectHideScene();

    //don't expect further commands
    clearRendererCommands();
    sceneHidden();
    expectNoRendererCommand();
}

TEST_F(ADisplayManager, unmapsShownScene)
{
    showSceneAndExpectShow();
    //scene shown

    displayManager.unmapScene(sceneId);
    expectHideScene();

    sceneHidden();
    expectUnmapScene();

    //don't expect further commands
    clearRendererCommands();
    sceneUnmapped();
    expectNoRendererCommand();
}

TEST_F(ADisplayManagerWithAutomapping, unmapsShownScene)
{
    showSceneAndExpectShow();
    //scene shown

    displayManager.unmapScene(sceneId);
    expectHideScene();

    sceneHidden();
    expectUnmapScene();

    //don't expect further commands
    clearRendererCommands();
    sceneUnmapped();
    expectNoRendererCommand();
}

TEST_F(ADisplayManager, unmapsMappedScene)
{
    showSceneAndExpectShow();
    //scene shown

    //hide scene first
    displayManager.hideScene(sceneId);
    expectHideScene();

    //don't expect further commands
    clearRendererCommands();
    sceneHidden();
    expectNoRendererCommand();
    //scene mapped

    displayManager.unmapScene(sceneId);
    expectUnmapScene();

    //don't expect further commands
    clearRendererCommands();
    sceneUnmapped();
    expectNoRendererCommand();
}

TEST_F(ADisplayManagerWithAutomapping, unmapsMappedScene)
{

    showSceneAndExpectShow();
    //scene shown

    //hide scene first
    displayManager.hideScene(sceneId);
    expectHideScene();

    //don't expect further commands
    clearRendererCommands();
    sceneHidden();
    expectNoRendererCommand();
    //scene mapped

    displayManager.unmapScene(sceneId);
    expectUnmapScene();

    //don't expect further commands
    clearRendererCommands();
    sceneUnmapped();
    expectNoRendererCommand();
}

TEST_F(ADisplayManager, unsubscribesShownScene)
{
    showSceneAndExpectShow();
    //scene shown

    displayManager.unsubscribeScene(sceneId);
    expectHideScene();

    sceneHidden();
    expectUnmapScene();

    sceneUnmapped();
    expectUnsubscribeScene();

    //don't expect further commands
    clearRendererCommands();
    sceneUnsubscribed();
    expectNoRendererCommand();
}

TEST_F(ADisplayManagerWithAutomapping, unsubscribesShownScene)
{
    showSceneAndExpectShow();
    //scene shown

    displayManager.unsubscribeScene(sceneId);
    expectHideScene();

    sceneHidden();
    expectUnmapScene();

    sceneUnmapped();
    expectUnsubscribeScene();

    //don't expect further commands
    clearRendererCommands();
    sceneUnsubscribed();
    expectNoRendererCommand();
}

TEST_F(ADisplayManager, unsubscribesMappedScene)
{
    showSceneAndExpectShow();
    //scene shown

    //hide scene first
    displayManager.hideScene(sceneId);
    expectHideScene();

    //don't expect further commands
    clearRendererCommands();
    sceneHidden();
    expectNoRendererCommand();
    //scene mapped

    displayManager.unsubscribeScene(sceneId);
    expectUnmapScene();

    sceneUnmapped();
    expectUnsubscribeScene();

    //don't expect further commands
    clearRendererCommands();
    sceneUnsubscribed();
    expectNoRendererCommand();
}

TEST_F(ADisplayManagerWithAutomapping, unsubscribesMappedScene)
{
    showSceneAndExpectShow();
    //scene shown

    //hide scene first
    displayManager.hideScene(sceneId);
    expectHideScene();

    //don't expect further commands
    clearRendererCommands();
    sceneHidden();
    expectNoRendererCommand();
    //scene mapped

    displayManager.unsubscribeScene(sceneId);
    expectUnmapScene();

    sceneUnmapped();
    expectUnsubscribeScene();

    //don't expect further commands
    clearRendererCommands();
    sceneUnsubscribed();
    expectNoRendererCommand();
}

TEST_F(ADisplayManager, unsubscribesSubscribedScene)
{
    showSceneAndExpectShow();

    displayManager.unmapScene(sceneId);
    expectHideScene();

    sceneHidden();
    expectUnmapScene();

    clearRendererCommands();
    sceneUnmapped();
    expectNoRendererCommand();

    displayManager.unsubscribeScene(sceneId);
    expectUnsubscribeScene();
}

TEST_F(ADisplayManagerWithAutomapping, unsubscribesSubscribedScene)
{
    showSceneAndExpectShow();

    displayManager.unmapScene(sceneId);
    expectHideScene();

    sceneHidden();
    expectUnmapScene();

    sceneUnmapped();

    displayManager.unsubscribeScene(sceneId);
    expectUnsubscribeScene();
}

TEST_F(ADisplayManager, handlesSceneUnpublishWithoutAsserts)
{
    showSceneAndExpectShow();

    clearRendererCommands();
    sceneHidden(ramses::ERendererEventResult_INDIRECT);
    sceneUnmapped(ramses::ERendererEventResult_INDIRECT);
    sceneUnsubscribed(ramses::ERendererEventResult_INDIRECT);
    sceneUnpublished();
    expectNoRendererCommand();
}

TEST_F(ADisplayManagerWithAutomapping, handlesSceneUnpublishWithoutAsserts)
{
    showSceneAndExpectShow();

    clearRendererCommands();
    sceneHidden(ramses::ERendererEventResult_INDIRECT);
    sceneUnmapped(ramses::ERendererEventResult_INDIRECT);
    sceneUnsubscribed(ramses::ERendererEventResult_INDIRECT);
    sceneUnpublished();
    expectNoRendererCommand();
}

TEST_F(ADisplayManager, handlesSceneUnpublishWithoutAssertsWhileProcessingSubscribe)
{
    displayManager.showSceneOnDisplay(sceneId, displayId, 0);
    expectNoRendererCommand();

    scenePublished();
    expectSubscribeScene();

    //unexpected unpublish!!
    sceneUnpublished();
    sceneSubscribed(ramses::ERendererEventResult_FAIL);
}

TEST_F(ADisplayManager, handlesSceneUnpublishWithoutAssertsWhileProcessingShow)
{
    displayManager.showSceneOnDisplay(sceneId, displayId, 0);
    expectNoRendererCommand();

    scenePublished();
    expectSubscribeScene();

    sceneSubscribed();
    expectMapScene();

    sceneMapped();
    expectShowScene();

    //unexpected unpublish!!
    sceneUnmapped(ramses::ERendererEventResult_INDIRECT);
    sceneUnsubscribed(ramses::ERendererEventResult_INDIRECT);
    sceneUnpublished();
    sceneShown(ramses::ERendererEventResult_FAIL);
}

TEST_F(ADisplayManager, canNotShowAShownSceneOnAnotherDisplay)
{
    showSceneAndExpectShow();

    clearRendererCommands();
    displayManager.showSceneOnDisplay(sceneId, otherDisplayId);
    expectNoRendererCommand();
}

TEST_F(ADisplayManager, canNotShowAMappedSceneOnAnotherDisplay)
{
    showSceneAndExpectShow();

    displayManager.hideScene(sceneId);
    expectHideScene();

    sceneHidden();
    //scene mapped

    clearRendererCommands();
    displayManager.showSceneOnDisplay(sceneId, otherDisplayId);
    expectNoRendererCommand();
}

TEST_F(ADisplayManager, canShowASubscribedSceneOnAnotherDisplay)
{
    showSceneAndExpectShow();

    displayManager.unmapScene(sceneId);
    expectHideScene();
    sceneHidden();
    //scene mapped
    expectUnmapScene();
    sceneUnmapped();
    //scene subscribed

    displayManager.showSceneOnDisplay(sceneId, otherDisplayId);
    expectMapScene();
    sceneMapped();
    expectShowScene();
    sceneShown();
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
    EXPECT_FALSE(displayManager.isSceneShown(sceneId));

    // show first time
    showSceneAndExpectShow();
    EXPECT_TRUE(displayManager.isSceneShown(sceneId));

    // explicitely hide
    displayManager.hideScene(sceneId);
    EXPECT_FALSE(displayManager.isSceneShown(sceneId));
    sceneHidden();

    // show again
    displayManager.showSceneOnDisplay(sceneId, displayId, 0);
    EXPECT_FALSE(displayManager.isSceneShown(sceneId));
    sceneShown();
    EXPECT_TRUE(displayManager.isSceneShown(sceneId));

    // unexpected unpublish!!
    sceneHidden(ramses::ERendererEventResult_INDIRECT);
    sceneUnmapped(ramses::ERendererEventResult_INDIRECT);
    sceneUnsubscribed(ramses::ERendererEventResult_INDIRECT);
    sceneUnpublished();
    EXPECT_FALSE(displayManager.isSceneShown(sceneId));
}

TEST_F(ADisplayManager, reportsCorrectDisplayCreationState)
{
    ramses::DisplayConfig displayConfig;
    ramses::displayId_t customDisplay = displayManager.createDisplay(displayConfig);

    EXPECT_FALSE(displayManager.isDisplayCreated(customDisplay));
    renderer.doOneLoop();
    displayManager.dispatchAndFlush();

    EXPECT_TRUE(displayManager.isDisplayCreated(customDisplay));

    displayManager.destroyDisplay(customDisplay);
    EXPECT_TRUE(displayManager.isDisplayCreated(customDisplay));

    renderer.doOneLoop();
    displayManager.dispatchAndFlush();

    EXPECT_FALSE(displayManager.isDisplayCreated(customDisplay));
}

TEST_F(ADisplayManager, sceneWillBeShownWhenDisplayIsCreated)
{
    ramses::displayId_t customDisplay(120u);

    displayManager.showSceneOnDisplay(sceneId, customDisplay);
    expectNoRendererCommand();

    scenePublished();
    expectSubscribeScene();
    clearRendererCommands();

    sceneSubscribed();
    expectNoRendererCommand(); // do not map, because display is not yet created

    displayCreated(customDisplay); // map when display is created!
    expectMapScene();

    sceneMapped();
    expectShowScene();
    clearRendererCommands();

    sceneShown();
    expectNoRendererCommand();
}

TEST_F(ADisplayManagerWithAutomapping, willAutomaticallyShowSceneWhenDisplayIsCreated)
{
    // cleanup displays first
    ASSERT_TRUE(displayManager.isDisplayCreated(displayId));
    ASSERT_TRUE(displayManager.isDisplayCreated(otherDisplayId));
    displayManager.destroyDisplay(displayId);
    displayManager.destroyDisplay(otherDisplayId);
    renderer.doOneLoop();
    displayManager.dispatchAndFlush();
    ASSERT_FALSE(displayManager.isDisplayCreated(displayId));
    ASSERT_FALSE(displayManager.isDisplayCreated(otherDisplayId));


    ramses::displayId_t defaultDisplay(0u);
    scenePublished();
    expectSubscribeScene();
    clearRendererCommands();

    sceneSubscribed();
    expectNoRendererCommand(); // do not map, because no display has been created yet!

    displayCreated(defaultDisplay); // map when display is created!
    expectMapScene();

    sceneMapped();
    expectShowScene();
    clearRendererCommands();

    sceneShown();
    expectNoRendererCommand();
}

TEST_F(ADisplayManager, continuesToShowSceneAndLogsConfirmationAfterReconnect)
{
    scenePublished();
    expectNoRendererCommand();

    displayManager.showSceneOnDisplay(sceneId, displayId, 0, "scene is shown now");
    expectSubscribeScene();

    sceneSubscribed();
    expectMapScene();

    //simulate disconnect
    sceneUnmapped(ramses::ERendererEventResult_INDIRECT);
    sceneUnsubscribed(ramses::ERendererEventResult_INDIRECT);
    sceneUnpublished();

    scenePublished();

    expectSubscribeScene();

    sceneSubscribed();
    expectMapScene();

    sceneMapped();
    expectShowScene();

    sceneShown();
    expectConfirmationMessage();
}

TEST_F(ADisplayManager, showsPublishedSceneAgainAfterReconnect)
{
    scenePublished();
    expectNoRendererCommand();

    displayManager.showSceneOnDisplay(sceneId, displayId, 0, "scene is shown now");
    expectSubscribeScene();

    sceneSubscribed();
    expectMapScene();

    sceneMapped();
    expectShowScene();

    sceneShown();
    expectConfirmationMessage();

    //simulate disconnect
    sceneHidden(ramses::ERendererEventResult_INDIRECT);
    sceneUnmapped(ramses::ERendererEventResult_INDIRECT);
    sceneUnsubscribed(ramses::ERendererEventResult_INDIRECT);
    sceneUnpublished();

    scenePublished();

    expectSubscribeScene();

    sceneSubscribed();
    expectMapScene();

    sceneMapped();
    expectShowScene();

    sceneShown();
}
