//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "renderer_common_gmock_header.h"
#include "gtest/gtest.h"
#include "RendererLib/RendererCommands.h"
#include "RendererLib/DisplayConfig.h"
#include "RendererCommands/Screenshot.h"
#include "Ramsh/RamshTools.h"
#include "ResourceProviderMock.h"
#include "ResourceUploaderMock.h"
#include "RendererLib/EKeyModifier.h"
#include "Math3d/Vector2i.h"
#include "Scene/SceneActionCollectionCreator.h"

namespace ramses_internal {
using namespace testing;

class ARendererCommands : public ::testing::Test
{
public:
    ARendererCommands()
    {
    }

protected:
    RendererCommands queue;
};

TEST_F(ARendererCommands, hasEmptyCommandQueueAfterCreation)
{
    EXPECT_EQ(0u, queue.getCommands().getTotalCommandCount());
}

TEST_F(ARendererCommands, createsCommandForScenePublication)
{
    const SceneId sceneId(12u);
    const Guid clientID(true);
    queue.publishScene(sceneId, clientID, EScenePublicationMode_LocalAndRemote);

    EXPECT_EQ(1u, queue.getCommands().getTotalCommandCount());
    EXPECT_EQ(ERendererCommand_PublishedScene, queue.getCommands().getCommandType(0));

    const SceneInfoCommand& command = queue.getCommands().getCommandData<SceneInfoCommand>(0);
    EXPECT_EQ(sceneId, command.sceneInformation.sceneID);
    EXPECT_EQ(EScenePublicationMode_LocalAndRemote, command.sceneInformation.publicationMode);
    EXPECT_EQ(clientID, command.clientID);
}

TEST_F(ARendererCommands, createsCommandForSceneUnpublication)
{
    const SceneId sceneId(12u);
    queue.unpublishScene(sceneId);

    EXPECT_EQ(1u, queue.getCommands().getTotalCommandCount());
    EXPECT_EQ(ERendererCommand_UnpublishedScene, queue.getCommands().getCommandType(0));

    const SceneInfoCommand& command = queue.getCommands().getCommandData<SceneInfoCommand>(0);
    EXPECT_EQ(sceneId, command.sceneInformation.sceneID);
}


TEST_F(ARendererCommands, createsCommandForSceneCreation)
{
    String sceneName = "the scene";
    SceneId sceneId(12u);
    const SceneInfo sceneInfo(sceneId, sceneName);
    queue.receiveScene(sceneInfo);

    EXPECT_EQ(1u, queue.getCommands().getTotalCommandCount());
    EXPECT_EQ(ERendererCommand_ReceivedScene, queue.getCommands().getCommandType(0));

    const SceneInfoCommand& command = queue.getCommands().getCommandData<SceneInfoCommand>(0);

    EXPECT_EQ(sceneName, command.sceneInformation.friendlyName);
    EXPECT_EQ(sceneId, command.sceneInformation.sceneID);
}

TEST_F(ARendererCommands, createsCommandForSceneSubscription)
{
    const SceneId sceneId(12u);
    queue.subscribeScene(sceneId);

    EXPECT_EQ(1u, queue.getCommands().getTotalCommandCount());
    EXPECT_EQ(ERendererCommand_SubscribeScene, queue.getCommands().getCommandType(0));

    const SceneInfoCommand& command = queue.getCommands().getCommandData<SceneInfoCommand>(0);
    EXPECT_EQ(sceneId, command.sceneInformation.sceneID);
}

TEST_F(ARendererCommands, createsCommandForSceneUnsubscription)
{
    const SceneId sceneId(12u);
    queue.unsubscribeScene(sceneId, false);
    queue.unsubscribeScene(sceneId, true);

    EXPECT_EQ(2u, queue.getCommands().getTotalCommandCount());
    EXPECT_EQ(ERendererCommand_UnsubscribeScene, queue.getCommands().getCommandType(0));
    EXPECT_EQ(ERendererCommand_UnsubscribeScene, queue.getCommands().getCommandType(1));

    const SceneInfoCommand& command = queue.getCommands().getCommandData<SceneInfoCommand>(0);
    EXPECT_EQ(sceneId, command.sceneInformation.sceneID);
    EXPECT_FALSE(command.indirect);

    const SceneInfoCommand& command2 = queue.getCommands().getCommandData<SceneInfoCommand>(1);
    EXPECT_EQ(sceneId, command2.sceneInformation.sceneID);
    EXPECT_TRUE(command2.indirect);
}

TEST_F(ARendererCommands, createsCommandForSceneDeletion)
{
    SceneId sceneId(12u);
    queue.receiveScene(SceneInfo(sceneId));   // default parameters
    queue.unpublishScene(sceneId);

    EXPECT_EQ(2u, queue.getCommands().getTotalCommandCount());
    EXPECT_EQ(ERendererCommand_UnpublishedScene, queue.getCommands().getCommandType(1));

    const SceneInfoCommand& command = queue.getCommands().getCommandData<SceneInfoCommand>(1);
    EXPECT_EQ(sceneId, command.sceneInformation.sceneID);
}

TEST_F(ARendererCommands, createsCommandForSceneActions)
{
    const SceneId sceneId(12u);
    const NodeHandle parent(1u);
    const NodeHandle child(2u);

    SceneActionCollection actions;
    SceneActionCollectionCreator creator(actions);
    creator.allocateNode(0u, parent);
    creator.allocateNode(0u, child);
    creator.addChildToNode(parent, child);

    queue.enqueueActionsForScene(sceneId, std::move(actions));

    EXPECT_EQ(1u, queue.getCommands().getTotalCommandCount());
    EXPECT_EQ(ERendererCommand_SceneActions, queue.getCommands().getCommandType(0u));

    const SceneActionsCommand& command = queue.getCommands().getCommandData<SceneActionsCommand>(0u);
    EXPECT_EQ(sceneId, command.sceneId);
    const SceneActionCollection& actionsInCommand = command.sceneActions;
    ASSERT_EQ(3u, actionsInCommand.numberOfActions());
    EXPECT_EQ(ESceneActionId_AllocateNode, actionsInCommand[0].type());
    EXPECT_EQ(ESceneActionId_AllocateNode, actionsInCommand[1].type());
    EXPECT_EQ(ESceneActionId_AddChildToNode, actionsInCommand[2].type());
}

TEST_F(ARendererCommands, createsCommandForCreatingDisplay)
{
    const DisplayHandle displayHandle(1u);
    const DisplayConfig displayConfig;
    ResourceProviderMock resourceProvider;
    NiceMock<ResourceUploaderMock> resourceUploader;

    queue.createDisplay(displayConfig, resourceProvider, resourceUploader, displayHandle);

    EXPECT_EQ(1u, queue.getCommands().getTotalCommandCount());
    EXPECT_EQ(ERendererCommand_CreateDisplay, queue.getCommands().getCommandType(0u));

    const DisplayCommand& command = queue.getCommands().getCommandData<DisplayCommand>(0u);

    EXPECT_EQ(displayHandle, command.displayHandle);
    EXPECT_EQ(displayConfig, command.displayConfig);
    EXPECT_EQ(&resourceProvider, command.resourceProvider);
    EXPECT_EQ(&resourceUploader, command.resourceUploader);
}

TEST_F(ARendererCommands, createsCommandForDestroyingDisplay)
{
    const DisplayHandle displayHandle(1u);

    queue.destroyDisplay(displayHandle);

    EXPECT_EQ(1u, queue.getCommands().getTotalCommandCount());
    EXPECT_EQ(ERendererCommand_DestroyDisplay, queue.getCommands().getCommandType(0u));

    const DisplayCommand& command = queue.getCommands().getCommandData<DisplayCommand>(0u);

    EXPECT_EQ(displayHandle, command.displayHandle);
}

TEST_F(ARendererCommands, createsCommandForMappingSceneOnDisplay)
{
    const DisplayHandle displayHandle(1u);
    const SceneId sceneId(33u);
    const Int32 sceneRenderOrder(1);

    queue.mapSceneToDisplay(sceneId, displayHandle, sceneRenderOrder);

    EXPECT_EQ(1u, queue.getCommands().getTotalCommandCount());
    EXPECT_EQ(ERendererCommand_MapSceneToDisplay, queue.getCommands().getCommandType(0u));

    const SceneMappingCommand& command = queue.getCommands().getCommandData<SceneMappingCommand>(0u);

    EXPECT_EQ(sceneId, command.sceneId);
    EXPECT_EQ(displayHandle, command.displayHandle);
    EXPECT_EQ(sceneRenderOrder, command.sceneRenderOrder);
}

TEST_F(ARendererCommands, createsCommandForShowingSceneOnDisplay)
{
    const SceneId sceneId(33u);

    queue.showScene(sceneId);

    EXPECT_EQ(1u, queue.getCommands().getTotalCommandCount());
    EXPECT_EQ(ERendererCommand_ShowScene, queue.getCommands().getCommandType(0u));

    const SceneRenderCommand& command = queue.getCommands().getCommandData<SceneRenderCommand>(0u);

    EXPECT_EQ(sceneId, command.sceneId);
}

TEST_F(ARendererCommands, createsCommandForWarpingDataUpdate)
{
    const DisplayHandle displayHandle(1u);
    const WarpingMeshData warpingData;

    queue.updateWarpingData(displayHandle, warpingData);

    EXPECT_EQ(1u, queue.getCommands().getTotalCommandCount());
    EXPECT_EQ(ERendererCommand_UpdateWarpingData, queue.getCommands().getCommandType(0u));

    const WarpingDataCommand& command = queue.getCommands().getCommandData<WarpingDataCommand>(0u);

    EXPECT_EQ(displayHandle, command.displayHandle);
    EXPECT_EQ(warpingData.getIndices().size(), command.warpingData.getIndices().size());
    EXPECT_EQ(warpingData.getVertexPositions().size(), command.warpingData.getVertexPositions().size());
    EXPECT_EQ(warpingData.getTextureCoordinates().size(), command.warpingData.getTextureCoordinates().size());
}

TEST_F(ARendererCommands, createsCommandForReadPixels)
{
    const DisplayHandle displayHandle(1u);
    const UInt32 x = 2;
    const UInt32 y = 33;
    const UInt32 width = 201;
    const UInt32 height = 4;
    const String filename = "testPixels";

    queue.readPixels(displayHandle, filename, false, x, y, width, height);

    EXPECT_EQ(1u, queue.getCommands().getTotalCommandCount());
    EXPECT_EQ(ERendererCommand_ReadPixels, queue.getCommands().getCommandType(0u));

    const ReadPixelsCommand& command = queue.getCommands().getCommandData<ReadPixelsCommand>(0u);

    EXPECT_EQ(displayHandle, command.displayHandle);
    EXPECT_EQ(x, command.x);
    EXPECT_EQ(y, command.y);
    EXPECT_EQ(width, command.width);
    EXPECT_EQ(height, command.height);
    EXPECT_FALSE(command.fullScreen);
    EXPECT_EQ(filename, command.filename);
}

TEST_F(ARendererCommands, createsCommandsForDataLinking)
{
    const SceneId providerScene(1u);
    const DataSlotId providerData(2u);
    const SceneId consumerScene(3u);
    const DataSlotId consumerData(4u);
    const OffscreenBufferHandle providerBuffer(5u);

    queue.linkSceneData(providerScene, providerData, consumerScene, consumerData);
    queue.unlinkSceneData(consumerScene, consumerData);
    queue.linkBufferToSceneData(providerBuffer, consumerScene, consumerData);

    EXPECT_EQ(3u, queue.getCommands().getTotalCommandCount());
    EXPECT_EQ(ERendererCommand_LinkSceneData, queue.getCommands().getCommandType(0u));
    EXPECT_EQ(ERendererCommand_UnlinkSceneData, queue.getCommands().getCommandType(1u));
    EXPECT_EQ(ERendererCommand_LinkBufferToSceneData, queue.getCommands().getCommandType(2u));

    const DataLinkCommand& link = queue.getCommands().getCommandData<DataLinkCommand>(0u);
    const DataLinkCommand& unlink= queue.getCommands().getCommandData<DataLinkCommand>(1u);
    const DataLinkCommand& bufferLink = queue.getCommands().getCommandData<DataLinkCommand>(2u);

    EXPECT_EQ(providerScene, link.providerScene);
    EXPECT_EQ(providerData, link.providerData);
    EXPECT_EQ(consumerScene, link.consumerScene);
    EXPECT_EQ(consumerData, link.consumerData);

    EXPECT_EQ(consumerScene, unlink.consumerScene);
    EXPECT_EQ(consumerData, unlink.consumerData);

    EXPECT_EQ(providerBuffer, bufferLink.providerBuffer);
    EXPECT_EQ(consumerScene, bufferLink.consumerScene);
    EXPECT_EQ(consumerData, bufferLink.consumerData);
}

TEST_F(ARendererCommands, createsCommandsOffscreenBuffer)
{
    const DisplayHandle display(1u);
    const OffscreenBufferHandle buffer(2u);
    const UInt32 width(16u);
    const UInt32 height(32u);

    queue.createOffscreenBuffer(buffer, display, width, height, true);
    queue.destroyOffscreenBuffer(buffer, display);

    EXPECT_EQ(2u, queue.getCommands().getTotalCommandCount());
    EXPECT_EQ(ERendererCommand_CreateOffscreenBuffer, queue.getCommands().getCommandType(0u));
    EXPECT_EQ(ERendererCommand_DestroyOffscreenBuffer, queue.getCommands().getCommandType(1u));

    const OffscreenBufferCommand& cmd1 = queue.getCommands().getCommandData<OffscreenBufferCommand>(0u);
    const OffscreenBufferCommand& cmd2 = queue.getCommands().getCommandData<OffscreenBufferCommand>(1u);

    EXPECT_EQ(display, cmd1.displayHandle);
    EXPECT_EQ(buffer, cmd1.bufferHandle);
    EXPECT_EQ(width, cmd1.bufferWidth);
    EXPECT_EQ(height, cmd1.bufferHeight);
    EXPECT_TRUE(cmd1.interruptible);

    EXPECT_EQ(display, cmd2.displayHandle);
    EXPECT_EQ(buffer, cmd2.bufferHandle);
}

TEST_F(ARendererCommands, createsCommandsSceneToBufferAssignment)
{
    const OffscreenBufferHandle buffer(2u);
    const SceneId scene(3u);

    queue.assignSceneToOffscreenBuffer(scene, buffer);
    queue.assignSceneToFramebuffer(scene);

    EXPECT_EQ(2u, queue.getCommands().getTotalCommandCount());
    EXPECT_EQ(ERendererCommand_AssignSceneToOffscreenBuffer, queue.getCommands().getCommandType(0u));
    EXPECT_EQ(ERendererCommand_AssignSceneToFramebuffer, queue.getCommands().getCommandType(1u));

    const OffscreenBufferCommand& cmd1 = queue.getCommands().getCommandData<OffscreenBufferCommand>(0u);
    const OffscreenBufferCommand& cmd2 = queue.getCommands().getCommandData<OffscreenBufferCommand>(1u);

    EXPECT_EQ(buffer, cmd1.bufferHandle);
    EXPECT_EQ(scene, cmd1.assignedScene);

    EXPECT_EQ(OffscreenBufferHandle::Invalid(), cmd2.bufferHandle);
    EXPECT_EQ(scene, cmd2.assignedScene);
}

TEST_F(ARendererCommands, createsCommandForUnmappingSceneFromDisplays)
{
    const DisplayHandle displayId(1u);
    const SceneId sceneId(33u);
    const Int32 sceneRenderOrder(1);

    queue.receiveScene(SceneInfo(sceneId));   // default parameters
    queue.mapSceneToDisplay(sceneId, displayId, sceneRenderOrder);
    queue.clear();

    queue.unmapScene(sceneId);

    EXPECT_EQ(1u, queue.getCommands().getTotalCommandCount());
    EXPECT_EQ(ERendererCommand_UnmapSceneFromDisplays, queue.getCommands().getCommandType(0u));

    const SceneMappingCommand& command = queue.getCommands().getCommandData<SceneMappingCommand>(0u);
    EXPECT_EQ(sceneId, command.sceneId);
}

TEST_F(ARendererCommands, createsCommandForHidingSceneOnDisplay)
{
    const SceneId sceneId(1412u);

    queue.hideScene(sceneId);

    EXPECT_EQ(1u, queue.getCommands().getTotalCommandCount());
    EXPECT_EQ(ERendererCommand_HideScene, queue.getCommands().getCommandType(0u));

    const SceneRenderCommand& command = queue.getCommands().getCommandData<SceneRenderCommand>(0u);

    EXPECT_EQ(sceneId, command.sceneId);
}

TEST_F(ARendererCommands, createsCommandsForDisplayMovement)
{
    queue.moveView          (Vector3(1.0f, 1.0f, 1.0f));
    queue.setViewPosition   (Vector3(2.0f, 2.0f, 2.0f));
    queue.rotateView        (Vector3(3.0f, 3.0f, 3.0f));
    queue.setViewRotation   (Vector3(4.0f, 4.0f, 4.0f));

    EXPECT_EQ(4u, queue.getCommands().getTotalCommandCount());
    EXPECT_EQ(ERendererCommand_RelativeTranslation  , queue.getCommands().getCommandType(0));
    EXPECT_EQ(ERendererCommand_AbsoluteTranslation  , queue.getCommands().getCommandType(1));
    EXPECT_EQ(ERendererCommand_RelativeRotation     , queue.getCommands().getCommandType(2));
    EXPECT_EQ(ERendererCommand_AbsoluteRotation     , queue.getCommands().getCommandType(3));

    const RendererViewCommand& command1 = queue.getCommands().getCommandData<RendererViewCommand>(0);
    const RendererViewCommand& command2 = queue.getCommands().getCommandData<RendererViewCommand>(1);
    const RendererViewCommand& command3 = queue.getCommands().getCommandData<RendererViewCommand>(2);
    const RendererViewCommand& command4 = queue.getCommands().getCommandData<RendererViewCommand>(3);

    EXPECT_EQ(Vector3(1.0f, 1.0f, 1.0f), command1.displayMovement);
    EXPECT_EQ(Vector3(2.0f, 2.0f, 2.0f), command2.displayMovement);
    EXPECT_EQ(Vector3(3.0f, 3.0f, 3.0f), command3.displayMovement);
    EXPECT_EQ(Vector3(4.0f, 4.0f, 4.0f), command4.displayMovement);
}

TEST_F(ARendererCommands, createsCommandsForLogging)
{
    const NodeHandle meshHandle(123u);
    queue.logRendererInfo(ERendererLogTopic_Resources, false, NodeHandle::Invalid());
    queue.logRendererInfo(ERendererLogTopic_Resources, true, NodeHandle::Invalid());
    queue.logRendererInfo(ERendererLogTopic_RenderQueue, false, NodeHandle::Invalid());
    queue.logRendererInfo(ERendererLogTopic_RenderQueue, true, meshHandle);
    queue.logRendererInfo(ERendererLogTopic_SceneStates, true, NodeHandle::Invalid());
    queue.logStatistics();

    EXPECT_EQ(6u, queue.getCommands().getTotalCommandCount());

    EXPECT_EQ(ERendererCommand_LogRendererInfo, queue.getCommands().getCommandType(0));
    EXPECT_EQ(ERendererLogTopic_Resources, queue.getCommands().getCommandData<LogCommand>(0).topic);
    EXPECT_FALSE(queue.getCommands().getCommandData<LogCommand>(0).verbose);

    EXPECT_EQ(ERendererCommand_LogRendererInfo, queue.getCommands().getCommandType(1));
    EXPECT_EQ(ERendererLogTopic_Resources, queue.getCommands().getCommandData<LogCommand>(1).topic);
    EXPECT_TRUE(queue.getCommands().getCommandData<LogCommand>(1).verbose);

    EXPECT_EQ(ERendererCommand_LogRendererInfo, queue.getCommands().getCommandType(2));
    EXPECT_EQ(ERendererLogTopic_RenderQueue, queue.getCommands().getCommandData<LogCommand>(2).topic);
    EXPECT_FALSE(queue.getCommands().getCommandData<LogCommand>(2).verbose);

    EXPECT_EQ(ERendererCommand_LogRendererInfo, queue.getCommands().getCommandType(3));
    EXPECT_EQ(ERendererLogTopic_RenderQueue, queue.getCommands().getCommandData<LogCommand>(3).topic);
    EXPECT_TRUE(queue.getCommands().getCommandData<LogCommand>(3).verbose);
    EXPECT_EQ(meshHandle, queue.getCommands().getCommandData<LogCommand>(3).nodeHandleFilter);

    EXPECT_EQ(ERendererCommand_LogRendererInfo, queue.getCommands().getCommandType(4));
    EXPECT_EQ(ERendererLogTopic_SceneStates, queue.getCommands().getCommandData<LogCommand>(4).topic);

    EXPECT_EQ(ERendererCommand_LogRendererStatistics, queue.getCommands().getCommandType(5));
}

TEST_F(ARendererCommands, clearsCommands)
{
    const SceneId sceneId(12u);
    queue.receiveScene(SceneInfo(sceneId));   // default parameters
    queue.mapSceneToDisplay(SceneId(0u), DisplayHandle(0), 0);
    EXPECT_EQ(2u, queue.getCommands().getTotalCommandCount());

    queue.clear();

    EXPECT_EQ(0u, queue.getCommands().getTotalCommandCount());
}

TEST_F(ARendererCommands, createsCommandsToListIviSurfacesInSystemCompositorController)
{
    queue.systemCompositorControllerListIviSurfaces();

    EXPECT_EQ(1u, queue.getCommands().getTotalCommandCount());
}

TEST_F(ARendererCommands, createsCommandsForSettingVisibilityInSystemCompositorController)
{
    queue.systemCompositorControllerSetIviSurfaceVisibility(WaylandIviSurfaceId(1), false);
    queue.systemCompositorControllerSetIviSurfaceVisibility(WaylandIviSurfaceId(2), true);

    EXPECT_EQ(2u, queue.getCommands().getTotalCommandCount());
    {
        const CompositorCommand& command = queue.getCommands().getCommandData<CompositorCommand>(0);
        EXPECT_EQ(ERendererCommand_SystemCompositorControllerSetIviSurfaceVisibility, queue.getCommands().getCommandType(0));
        EXPECT_EQ(1u, command.waylandIviSurfaceId.getValue());
        EXPECT_FALSE(command.visibility);
    }
    {
        const CompositorCommand& command = queue.getCommands().getCommandData<CompositorCommand>(1);
        EXPECT_EQ(ERendererCommand_SystemCompositorControllerSetIviSurfaceVisibility, queue.getCommands().getCommandType(1));
        EXPECT_EQ(2u, command.waylandIviSurfaceId.getValue());
        EXPECT_TRUE(command.visibility);
    }
}

TEST_F(ARendererCommands, createsCommandsForSettingSurfaceOpacityInSystemCompositorController)
{
    queue.systemCompositorControllerSetIviSurfaceOpacity(WaylandIviSurfaceId(1), 0.3f);
    queue.systemCompositorControllerSetIviSurfaceOpacity(WaylandIviSurfaceId(2), 0.5f);

    EXPECT_EQ(2u, queue.getCommands().getTotalCommandCount());
    {
        const CompositorCommand& command = queue.getCommands().getCommandData<CompositorCommand>(0);
        EXPECT_EQ(ERendererCommand_SystemCompositorControllerSetIviSurfaceOpacity, queue.getCommands().getCommandType(0));
        EXPECT_EQ(1u, command.waylandIviSurfaceId.getValue());
        EXPECT_EQ(0.3f, command.opacity);
    }
    {
        const CompositorCommand& command = queue.getCommands().getCommandData<CompositorCommand>(1);
        EXPECT_EQ(ERendererCommand_SystemCompositorControllerSetIviSurfaceOpacity, queue.getCommands().getCommandType(1));
        EXPECT_EQ(2u, command.waylandIviSurfaceId.getValue());
        EXPECT_EQ(0.5f, command.opacity);
    }
}

TEST_F(ARendererCommands, createsCommandsForSettingSurfaceRectangleInSystemCompositorController)
{
    queue.systemCompositorControllerSetIviSurfaceDestRectangle(WaylandIviSurfaceId(1), 0, 0, 10, 10);
    queue.systemCompositorControllerSetIviSurfaceDestRectangle(WaylandIviSurfaceId(2), 11, 22, 33, 44);

    EXPECT_EQ(2u, queue.getCommands().getTotalCommandCount());
    {
        const CompositorCommand& command = queue.getCommands().getCommandData<CompositorCommand>(0);
        EXPECT_EQ(ERendererCommand_SystemCompositorControllerSetIviSurfaceDestRectangle, queue.getCommands().getCommandType(0));
        EXPECT_EQ(1u, command.waylandIviSurfaceId.getValue());
        EXPECT_EQ(0, command.x);
        EXPECT_EQ(0, command.y);
        EXPECT_EQ(10, command.width);
        EXPECT_EQ(10, command.height);
    }
    {
        const CompositorCommand& command = queue.getCommands().getCommandData<CompositorCommand>(1);
        EXPECT_EQ(ERendererCommand_SystemCompositorControllerSetIviSurfaceDestRectangle, queue.getCommands().getCommandType(1));
        EXPECT_EQ(2u, command.waylandIviSurfaceId.getValue());
        EXPECT_EQ(11, command.x);
        EXPECT_EQ(22, command.y);
        EXPECT_EQ(33, command.width);
        EXPECT_EQ(44, command.height);
    }
}

TEST_F(ARendererCommands, createsCommandsForAddingSurfaceToLayerInSystemCompositorController)
{
    queue.systemCompositorControllerAddIviSurfaceToIviLayer(WaylandIviSurfaceId(54), WaylandIviLayerId(814));
    queue.systemCompositorControllerAddIviSurfaceToIviLayer(WaylandIviSurfaceId(2), WaylandIviLayerId(3));

    EXPECT_EQ(2u, queue.getCommands().getTotalCommandCount());
    {
        const CompositorCommand& command = queue.getCommands().getCommandData<CompositorCommand>(0);
        EXPECT_EQ(ERendererCommand_SystemCompositorControllerAddIviSurfaceToIviLayer, queue.getCommands().getCommandType(0));
        EXPECT_EQ(54u, command.waylandIviSurfaceId.getValue());
        EXPECT_EQ(814u, command.waylandIviLayerId.getValue());
    }
    {
        const CompositorCommand& command = queue.getCommands().getCommandData<CompositorCommand>(1);
        EXPECT_EQ(ERendererCommand_SystemCompositorControllerAddIviSurfaceToIviLayer, queue.getCommands().getCommandType(0));
        EXPECT_EQ(2u, command.waylandIviSurfaceId.getValue());
        EXPECT_EQ(3u, command.waylandIviLayerId.getValue());
    }
}

TEST_F(ARendererCommands, createsCommandsForSettingLayerVisibilityInSystemCompositorController)
{
    queue.systemCompositorControllerSetIviLayerVisibility(WaylandIviLayerId(18), false);
    queue.systemCompositorControllerSetIviLayerVisibility(WaylandIviLayerId(17), true);

    EXPECT_EQ(2u, queue.getCommands().getTotalCommandCount());
    {
        const CompositorCommand& command = queue.getCommands().getCommandData<CompositorCommand>(0);
        EXPECT_EQ(ERendererCommand_SystemCompositorControllerSetIviLayerVisibility, queue.getCommands().getCommandType(0));
        EXPECT_EQ(18u, command.waylandIviLayerId.getValue());
        EXPECT_FALSE(command.visibility);
    }
    {
        const CompositorCommand& command = queue.getCommands().getCommandData<CompositorCommand>(1);
        EXPECT_EQ(ERendererCommand_SystemCompositorControllerSetIviLayerVisibility, queue.getCommands().getCommandType(1));
        EXPECT_EQ(17u, command.waylandIviLayerId.getValue());
        EXPECT_TRUE(command.visibility);
    }
}

TEST_F(ARendererCommands, createsCommandsForRemovingSurfaceFromLayerInSystemCompositorController)
{
    queue.systemCompositorControllerRemoveIviSurfaceFromIviLayer(WaylandIviSurfaceId(73), WaylandIviLayerId(348));
    queue.systemCompositorControllerRemoveIviSurfaceFromIviLayer(WaylandIviSurfaceId(5), WaylandIviLayerId(4));

    EXPECT_EQ(2u, queue.getCommands().getTotalCommandCount());
    {
        const CompositorCommand& command = queue.getCommands().getCommandData<CompositorCommand>(0);
        EXPECT_EQ(ERendererCommand_SystemCompositorControllerRemoveIviSurfaceFromIviLayer, queue.getCommands().getCommandType(0));
        EXPECT_EQ(73u, command.waylandIviSurfaceId.getValue());
        EXPECT_EQ(348u, command.waylandIviLayerId.getValue());
    }
    {
        const CompositorCommand& command = queue.getCommands().getCommandData<CompositorCommand>(1);
        EXPECT_EQ(ERendererCommand_SystemCompositorControllerRemoveIviSurfaceFromIviLayer, queue.getCommands().getCommandType(0));
        EXPECT_EQ(5u, command.waylandIviSurfaceId.getValue());
        EXPECT_EQ(4u, command.waylandIviLayerId.getValue());
    }
}

TEST_F(ARendererCommands, createsCommandsForDestroySurfaceInSystemCompositorController)
{
    queue.systemCompositorControllerDestroyIviSurface(WaylandIviSurfaceId(73));
    queue.systemCompositorControllerDestroyIviSurface(WaylandIviSurfaceId(5));

    EXPECT_EQ(2u, queue.getCommands().getTotalCommandCount());
    {
        const CompositorCommand& command = queue.getCommands().getCommandData<CompositorCommand>(0);
        EXPECT_EQ(ERendererCommand_SystemCompositorControllerDestroyIviSurface, queue.getCommands().getCommandType(0));
        EXPECT_EQ(73u, command.waylandIviSurfaceId.getValue());
    }
    {
        const CompositorCommand& command = queue.getCommands().getCommandData<CompositorCommand>(1);
        EXPECT_EQ(ERendererCommand_SystemCompositorControllerDestroyIviSurface, queue.getCommands().getCommandType(0));
        EXPECT_EQ(5u, command.waylandIviSurfaceId.getValue());
    }
}

TEST_F(ARendererCommands, createsCommandsForTakingScreenshotInSystemCompositorController)
{
    String fileName("image.png");
    queue.systemCompositorControllerScreenshot(fileName, 3);

    EXPECT_EQ(1u, queue.getCommands().getTotalCommandCount());
    {
        const CompositorCommand& command = queue.getCommands().getCommandData<CompositorCommand>(0);
        EXPECT_EQ(ERendererCommand_SystemCompositorControllerScreenshot, queue.getCommands().getCommandType(0));
        EXPECT_EQ(fileName, command.fileName);
        EXPECT_EQ(3, command.screenIviId);
    }
}

TEST_F(ARendererCommands, createsCommandsForSetClearColor)
{
    const DisplayHandle displayHandle(1u);
    const Vector4 clearColor(1.f, 0.f, 0.2f, 0.3f);
    queue.setClearColor(displayHandle, clearColor);

    EXPECT_EQ(1u, queue.getCommands().getTotalCommandCount());
    {
        const auto& command = queue.getCommands().getCommandData<SetClearColorCommand>(0);
        EXPECT_EQ(ERendererCommand_SetClearColor, queue.getCommands().getCommandType(0));
        EXPECT_EQ(clearColor, command.clearColor);
        EXPECT_EQ(displayHandle, command.displayHandle);
    }
}

TEST_F(ARendererCommands, createsCommandForSettingFrameTimerLimits)
{
    queue.setFrameTimerLimits(5u, 10u, 20u, 30u);

    EXPECT_EQ(1u, queue.getCommands().getTotalCommandCount());
    {
        const auto& command = queue.getCommands().getCommandData<SetFrameTimerLimitsCommmand>(0);
        EXPECT_EQ(ERendererCommand_SetFrameTimerLimits, queue.getCommands().getCommandType(0));
        EXPECT_EQ(5u, command.limitForSceneResourcesUploadMicrosec);
        EXPECT_EQ(10u, command.limitForClientResourcesUploadMicrosec);
        EXPECT_EQ(20u, command.limitForSceneActionsApplyMicrosec);
        EXPECT_EQ(30u, command.limitForOffscreenBufferRenderMicrosec);
    }
}

TEST_F(ARendererCommands, createsCommandForSettingLimitFlushesBeforeForceApply)
{
    queue.setForceApplyPendingFlushesLimit(99u);

    EXPECT_EQ(1u, queue.getCommands().getTotalCommandCount());
    {
        const auto& command = queue.getCommands().getCommandData<SetFrameTimerLimitsCommmand>(0);
        EXPECT_EQ(ERendererCommand_SetLimits_FlushesForceApply, queue.getCommands().getCommandType(0));
        EXPECT_EQ(99u, command.limitForPendingFlushesForceApply);
    }
}

TEST_F(ARendererCommands, createsCommandForSettingLimitFlushesBeforeSceneForceUnsubscribed)
{
    queue.setForceUnsubscribeLimits(55u);

    EXPECT_EQ(1u, queue.getCommands().getTotalCommandCount());
    {
        const auto& command = queue.getCommands().getCommandData<SetFrameTimerLimitsCommmand>(0);
        EXPECT_EQ(ERendererCommand_SetLimits_FlushesForceUnsubscribe, queue.getCommands().getCommandType(0));
        EXPECT_EQ(55u, command.limitForPendingFlushesForceUnsubscribe);
    }
}

TEST_F(ARendererCommands, createsCommandForSettingSkippingUnmodifiedBuffersFeature)
{
    queue.setSkippingOfUnmodifiedBuffers(true);

    EXPECT_EQ(1u, queue.getCommands().getTotalCommandCount());
    {
        const auto& command = queue.getCommands().getCommandData<SetFeatureCommand>(0);
        EXPECT_EQ(ERendererCommand_SetSkippingOfUnmodifiedBuffers, queue.getCommands().getCommandType(0));
        EXPECT_TRUE(command.enable);
    }
}
}
