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
public:
    ARendererCommandBuffer()
    {
    }

protected:
    RendererCommandBuffer queue;
};

TEST_F(ARendererCommandBuffer, gracefullyHandlesSceneActionsArrivingAfterUnsubscribe)
{
    const SceneId sceneId(12u);
    const Guid clientID(true);

    queue.publishScene(sceneId, clientID, EScenePublicationMode_LocalAndRemote);
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

    EXPECT_EQ(1u, queue.getCommands().getTotalCommandCount());
    ASSERT_EQ(ERendererCommand_ReadPixels, queue.getCommands().getCommandType(0));
    EXPECT_EQ(String("bla"), queue.getCommands().getCommandData<ReadPixelsCommand>(0).filename);
    EXPECT_EQ(DisplayHandle(0u), queue.getCommands().getCommandData<ReadPixelsCommand>(0).displayHandle);
}

TEST_F(ARendererCommandBuffer, parsesCommandsForScreenshotPrintingWithDisplayIDs)
{
    String in("screenshot -filename bla -displayId 2");
    RamshInput input = RamshTools::parseCommandString(in);

    Screenshot screenshotCommand(queue);
    screenshotCommand.executeInput(input);

    EXPECT_EQ(1u, queue.getCommands().getTotalCommandCount());
    EXPECT_EQ(ERendererCommand_ReadPixels, queue.getCommands().getCommandType(0));
    EXPECT_EQ(String("bla"), queue.getCommands().getCommandData<ReadPixelsCommand>(0).filename);
    EXPECT_EQ(DisplayHandle(2u), queue.getCommands().getCommandData<ReadPixelsCommand>(0).displayHandle);
}

TEST_F(ARendererCommandBuffer, canFetchAllTypesOfRendererCommands)
{
    RendererCommands queueToFetch;

    const SceneId sceneId(12u);
    const Guid clientID(true);
    const SceneInfo sceneInfo(sceneId, "testScene");
    const WarpingMeshData warpingData;
    const DisplayHandle displayHandle(1u);
    const DisplayConfig displayConfig;
    ResourceProviderMock resourceProvider;
    NiceMock<ResourceUploaderMock> resourceUploader;

    const SceneId providerSceneId;
    const SceneId consumerSceneId;
    const DataSlotId providerId(1);
    const DataSlotId consumerId(2);
    SceneIdVector scenes;
    scenes.push_back(sceneId);
    Vector2i mousePos(14, 12);
    Vector4 clearColor(1.f, 0.f, 0.5f, 0.2f);

    //fill queue which is to be fetched by the RendererCommandBuffer
    queueToFetch.publishScene(sceneId, clientID, EScenePublicationMode_LocalAndRemote);
    queueToFetch.unpublishScene(sceneId);
    queueToFetch.receiveScene(sceneInfo);
    queueToFetch.subscribeScene(sceneId);
    queueToFetch.unsubscribeScene(sceneId, false);
    queueToFetch.createDisplay(displayConfig, resourceProvider, resourceUploader, displayHandle);
    queueToFetch.destroyDisplay(displayHandle);
    queueToFetch.mapSceneToDisplay(sceneId, displayHandle, 0);
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
    queueToFetch.setClearColor(displayHandle, clearColor);
    queueToFetch.setFrameTimerLimits(4u, 1u, 2u, 3u);

    EXPECT_EQ(34u, queueToFetch.getCommands().getTotalCommandCount());

    queue.addCommands(queueToFetch); //fetchRendererCommands
    queueToFetch.clear(); //clear fetched command queue

    EXPECT_EQ(0u, queueToFetch.getCommands().getTotalCommandCount());
    EXPECT_EQ(34u, queue.getCommands().getTotalCommandCount());

    //check some details of the fetched commands
    EXPECT_EQ(ERendererCommand_PublishedScene, queue.getCommands().getCommandType(0));
    const SceneInfoCommand& sceneCmd = queue.getCommands().getCommandData<SceneInfoCommand>(0);
    EXPECT_EQ(sceneId, sceneCmd.sceneInformation.sceneID);
    EXPECT_EQ(clientID, sceneCmd.clientID);

    EXPECT_EQ(ERendererCommand_CreateDisplay, queue.getCommands().getCommandType(5));
    const DisplayCommand& dispCmd = queue.getCommands().getCommandData<DisplayCommand>(5);
    EXPECT_EQ(displayHandle, dispCmd.displayHandle);
    EXPECT_EQ(displayConfig, dispCmd.displayConfig);
    EXPECT_EQ(static_cast<IResourceProvider*>(&resourceProvider), dispCmd.resourceProvider);
    EXPECT_EQ(static_cast<IResourceUploader*>(&resourceUploader), dispCmd.resourceUploader);

    EXPECT_EQ(ERendererCommand_ConfirmationEcho, queue.getCommands().getCommandType(30));
    const ConfirmationEchoCommand& echoCmd = queue.getCommands().getCommandData<ConfirmationEchoCommand>(30);
    EXPECT_TRUE(echoCmd.text == ramses_internal::String("testEcho"));

    EXPECT_EQ(ERendererCommand_SetClearColor, queue.getCommands().getCommandType(32));
    const auto& clearColorCmd = queue.getCommands().getCommandData<SetClearColorCommand>(32);
    EXPECT_TRUE(clearColorCmd.clearColor == clearColor);
}
}
