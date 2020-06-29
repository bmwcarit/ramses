//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "renderer_common_gmock_header.h"
#include "gtest/gtest.h"
#include "RendererLib/RendererCommandBuffer.h"
#include "RendererLib/DisplayConfig.h"
#include "RendererCommands/Screenshot.h"
#include "Ramsh/RamshTools.h"
#include "ResourceProviderMock.h"
#include "ResourceUploaderMock.h"
#include "Math3d/Vector2i.h"
#include "Scene/SceneActionCollectionCreator.h"

namespace ramses_internal {
using namespace testing;

class ARendererCommandBuffer : public ::testing::Test
{
protected:
    RendererCommandBuffer queue;
};

TEST_F(ARendererCommandBuffer, gracefullyHandlesSceneActionsArrivingAfterUnsubscribe)
{
    const SceneId sceneId(12u);

    queue.publishScene(sceneId, EScenePublicationMode_LocalAndRemote);
    queue.receiveScene(SceneInfo(sceneId));

    queue.unsubscribeScene(sceneId, false);

    SceneActionCollection sceneActions;
    sceneActions.addRawSceneActionInformation(ESceneActionId_AddChildToNode, 0);
    sceneActions.addRawSceneActionInformation(ESceneActionId_RemoveRenderableFromRenderGroup, 0);
    queue.enqueueActionsForScene(sceneId, std::move(sceneActions));
}

TEST_F(ARendererCommandBuffer, parsesCommandsForScreenshotPrintingWithFilename)
{
    String in("screenshot -filename bla");
    RamshInput input = RamshTools::parseCommandString(in);

    Screenshot screenshotCommand(queue);
    screenshotCommand.executeInput(input);

    RendererCommandContainer commands;
    queue.swapCommandContainer(commands);
    EXPECT_EQ(1u, commands.getTotalCommandCount());
    ASSERT_EQ(ERendererCommand_ReadPixels, commands.getCommandType(0));
    EXPECT_EQ(String("bla"), commands.getCommandData<ReadPixelsCommand>(0).filename);
    EXPECT_EQ(DisplayHandle(0u), commands.getCommandData<ReadPixelsCommand>(0).displayHandle);
}

TEST_F(ARendererCommandBuffer, parsesCommandsForScreenshotPrintingWithDisplayIDs)
{
    String in("screenshot -filename bla -displayId 2");
    RamshInput input = RamshTools::parseCommandString(in);

    Screenshot screenshotCommand(queue);
    screenshotCommand.executeInput(input);

    RendererCommandContainer commands;
    queue.swapCommandContainer(commands);
    EXPECT_EQ(1u, commands.getTotalCommandCount());
    EXPECT_EQ(ERendererCommand_ReadPixels, commands.getCommandType(0));
    EXPECT_EQ(String("bla"), commands.getCommandData<ReadPixelsCommand>(0).filename);
    EXPECT_EQ(DisplayHandle(2u), commands.getCommandData<ReadPixelsCommand>(0).displayHandle);
}

TEST_F(ARendererCommandBuffer, canFetchAllTypesOfRendererCommands)
{
    RendererCommands queueToFetch;

    const SceneId sceneId(12u);
    const SceneInfo sceneInfo(sceneId, "testScene");
    const WarpingMeshData warpingData;
    const DisplayHandle displayHandle(1u);
    const DisplayConfig displayConfig;
    const OffscreenBufferHandle obHandle{ 6u };
    ResourceProviderMock resourceProvider;
    NiceMock<ResourceUploaderMock> resourceUploader;

    const SceneId providerSceneId;
    const SceneId consumerSceneId;
    const DataSlotId providerId(1);
    const DataSlotId consumerId(2);
    SceneIdVector scenes;
    scenes.push_back(sceneId);
    Vector4 clearColor(1.f, 0.f, 0.5f, 0.2f);

    //fill queue which is to be fetched by the RendererCommandBuffer
    queueToFetch.publishScene(sceneId, EScenePublicationMode_LocalAndRemote);
    queueToFetch.unpublishScene(sceneId);
    queueToFetch.receiveScene(sceneInfo);
    queueToFetch.setSceneState(sceneId, RendererSceneState::Rendered);
    queueToFetch.setSceneMapping(sceneId, displayHandle);
    queueToFetch.setSceneDisplayBufferAssignment(sceneId, obHandle, -13);
    queueToFetch.subscribeScene(sceneId);
    queueToFetch.unsubscribeScene(sceneId, false);
    queueToFetch.createDisplay(displayConfig, resourceProvider, resourceUploader, displayHandle);
    queueToFetch.destroyDisplay(displayHandle);
    queueToFetch.mapSceneToDisplay(sceneId, displayHandle);
    queueToFetch.assignSceneToDisplayBuffer(sceneId, obHandle, 11);
    queueToFetch.unmapScene(sceneId);
    queueToFetch.showScene(sceneId);
    queueToFetch.hideScene(sceneId);
    queueToFetch.updateWarpingData(displayHandle, warpingData);
    queueToFetch.readPixels(displayHandle, "testImage", false, 0, 0, 64, 64);
    queueToFetch.linkSceneData(providerSceneId, providerId, consumerSceneId, consumerId);
    queueToFetch.unlinkSceneData(consumerSceneId, consumerId);
    queueToFetch.moveView(Vector3(1.0f));
    queueToFetch.setViewPosition(Vector3(2.0f));
    queueToFetch.rotateView(Vector3(3.0f));
    queueToFetch.setViewRotation(Vector3(4.0f));
    queueToFetch.resetView();
    queueToFetch.logStatistics();
    queueToFetch.systemCompositorControllerListIviSurfaces();
    queueToFetch.systemCompositorControllerSetIviSurfaceVisibility(WaylandIviSurfaceId(6), true);
    queueToFetch.systemCompositorControllerSetIviSurfaceOpacity(WaylandIviSurfaceId(7), 0.5f);
    queueToFetch.systemCompositorControllerSetIviSurfaceDestRectangle(WaylandIviSurfaceId(8), 0, 0, 64, 128);
    queueToFetch.systemCompositorControllerScreenshot("testScreenshot.png", -1);
    queueToFetch.systemCompositorControllerSetIviLayerVisibility(WaylandIviLayerId(19), true);
    queueToFetch.systemCompositorControllerAddIviSurfaceToIviLayer(WaylandIviSurfaceId(9u), WaylandIviLayerId(831u));
    queueToFetch.systemCompositorControllerRemoveIviSurfaceFromIviLayer(WaylandIviSurfaceId(25u), WaylandIviLayerId(921u));
    queueToFetch.systemCompositorControllerDestroyIviSurface(WaylandIviSurfaceId(51u));
    queueToFetch.confirmationEcho("testEcho");
    queueToFetch.logRendererInfo(ERendererLogTopic_All, true, NodeHandle::Invalid());
    queueToFetch.setClearColor(displayHandle, obHandle, clearColor);
    queueToFetch.setFrameTimerLimits(4u, 1u, 3u);

    EXPECT_EQ(38u, queueToFetch.getCommands().getTotalCommandCount());

    queue.addCommands(queueToFetch); //fetchRendererCommands
    queueToFetch.clear(); //clear fetched command queue

    RendererCommandContainer commands;
    queue.swapCommandContainer(commands);

    EXPECT_EQ(0u, queueToFetch.getCommands().getTotalCommandCount());
    EXPECT_EQ(38u, commands.getTotalCommandCount());

    //check some details of the fetched commands
    EXPECT_EQ(ERendererCommand_PublishedScene, commands.getCommandType(0));
    const SceneInfoCommand& sceneCmd = commands.getCommandData<SceneInfoCommand>(0);
    EXPECT_EQ(sceneId, sceneCmd.sceneInformation.sceneID);

    const auto& sceneStateCmd = commands.getCommandData<SceneStateCommand>(3);
    EXPECT_EQ(sceneId, sceneStateCmd.sceneId);
    EXPECT_EQ(RendererSceneState::Rendered, sceneStateCmd.state);

    EXPECT_EQ(ERendererCommand_CreateDisplay, commands.getCommandType(8));
    const DisplayCommand& dispCmd = commands.getCommandData<DisplayCommand>(8);
    EXPECT_EQ(displayHandle, dispCmd.displayHandle);
    EXPECT_EQ(displayConfig, dispCmd.displayConfig);
    EXPECT_EQ(static_cast<IResourceProvider*>(&resourceProvider), dispCmd.resourceProvider);
    EXPECT_EQ(static_cast<IResourceUploader*>(&resourceUploader), dispCmd.resourceUploader);

    EXPECT_EQ(ERendererCommand_AssignSceneToDisplayBuffer, commands.getCommandType(11));
    const auto& assignCmd = commands.getCommandData<SceneMappingCommand>(11);
    EXPECT_EQ(sceneId, assignCmd.sceneId);
    EXPECT_EQ(obHandle, assignCmd.offscreenBuffer);
    EXPECT_EQ(11, assignCmd.sceneRenderOrder);

    EXPECT_EQ(ERendererCommand_ConfirmationEcho, commands.getCommandType(34));
    const ConfirmationEchoCommand& echoCmd = commands.getCommandData<ConfirmationEchoCommand>(34);
    EXPECT_TRUE(echoCmd.text == ramses_internal::String("testEcho"));

    EXPECT_EQ(ERendererCommand_SetClearColor, commands.getCommandType(36));
    const auto& clearColorCmd = commands.getCommandData<SetClearColorCommand>(36);
    EXPECT_EQ(displayHandle, clearColorCmd.displayHandle);
    EXPECT_EQ(obHandle, clearColorCmd.obHandle);
    EXPECT_EQ(clearColor, clearColorCmd.clearColor);
}
}
