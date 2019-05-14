//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/RendererCommands.h"
#include "Utils/LogMacros.h"
#include "RendererLib/RendererCommandTypes.h"
#include "RendererLib/EKeyModifier.h"

namespace ramses_internal
{
    RendererCommands::RendererCommands()
    {
    }

    void RendererCommands::publishScene(SceneId sceneId, const Guid& clientID, EScenePublicationMode mode)
    {
        SceneInfoCommand cmd;
        cmd.sceneInformation.sceneID = sceneId;
        cmd.clientID = clientID;
        cmd.sceneInformation.publicationMode = mode;
        m_commands.addCommand(ERendererCommand_PublishedScene, cmd);
    }

    void RendererCommands::unpublishScene(SceneId sceneId)
    {
        SceneInfoCommand cmd;
        cmd.sceneInformation.sceneID = sceneId;
        m_commands.addCommand(ERendererCommand_UnpublishedScene, cmd);
    }

    void RendererCommands::receiveScene(const SceneInfo& sceneInfo)
    {
        SceneInfoCommand cmd;
        cmd.sceneInformation = sceneInfo;
        m_commands.addCommand(ERendererCommand_ReceivedScene, cmd);
    }

    void RendererCommands::subscribeScene(SceneId sceneId)
    {
        SceneInfoCommand cmd;
        cmd.sceneInformation.sceneID = sceneId;
        m_commands.addCommand(ERendererCommand_SubscribeScene, cmd);
    }

    void RendererCommands::unsubscribeScene(SceneId sceneId, bool indirect)
    {
        SceneInfoCommand cmd;
        cmd.sceneInformation.sceneID = sceneId;
        cmd.indirect = indirect;
        m_commands.addCommand(ERendererCommand_UnsubscribeScene, cmd);
    }

    void RendererCommands::enqueueActionsForScene(SceneId sceneId, SceneActionCollection&& newActions)
    {
        SceneActionsCommand cmd;
        cmd.sceneId = sceneId;
        cmd.sceneActions = std::move(newActions);
        m_commands.addCommand(ERendererCommand_SceneActions, std::move(cmd));
    }

    void RendererCommands::createDisplay(const DisplayConfig& displayConfig, IResourceProvider& resourceProvider, IResourceUploader& resourceUploader, DisplayHandle handle)
    {
        DisplayCommand cmd;
        cmd.displayHandle = handle;
        cmd.displayConfig = displayConfig;
        cmd.resourceProvider = &resourceProvider;
        cmd.resourceUploader = &resourceUploader;
        m_commands.addCommand(ERendererCommand_CreateDisplay, cmd);
    }

    void RendererCommands::destroyDisplay(DisplayHandle handle)
    {
        DisplayCommand cmd;
        cmd.displayHandle = handle;
        m_commands.addCommand(ERendererCommand_DestroyDisplay, cmd);
    }

    void RendererCommands::mapSceneToDisplay(SceneId sceneId, DisplayHandle displayHandle, const Int32 sceneRenderOrder)
    {
        SceneMappingCommand cmd;
        cmd.sceneId = sceneId;
        cmd.displayHandle = displayHandle;
        cmd.sceneRenderOrder = sceneRenderOrder;
        m_commands.addCommand(ERendererCommand_MapSceneToDisplay, cmd);
    }

    void RendererCommands::unmapScene(SceneId sceneId)
    {
        SceneMappingCommand cmd;
        cmd.sceneId = sceneId;
        cmd.displayHandle = DisplayHandle::Invalid();
        cmd.sceneRenderOrder = 0;
        m_commands.addCommand(ERendererCommand_UnmapSceneFromDisplays, cmd);
    }

    void RendererCommands::showScene(SceneId sceneId)
    {
        SceneRenderCommand cmd;
        cmd.sceneId = sceneId;
        m_commands.addCommand(ERendererCommand_ShowScene, cmd);
    }

    void RendererCommands::hideScene(SceneId sceneId)
    {
        SceneRenderCommand cmd;
        cmd.sceneId = sceneId;
        m_commands.addCommand(ERendererCommand_HideScene, cmd);
    }

    void RendererCommands::updateWarpingData(DisplayHandle displayHandle, const WarpingMeshData& warpingData)
    {
        WarpingDataCommand cmd;
        cmd.displayHandle = displayHandle;
        cmd.warpingData = warpingData;
        m_commands.addCommand(ERendererCommand_UpdateWarpingData, cmd);
    }

    void RendererCommands::readPixels(DisplayHandle displayHandle, const String& filename, Bool fullScreen, UInt32 x, UInt32 y, UInt32 width, UInt32 height, Bool sendViaDLT)
    {
        ReadPixelsCommand cmd;
        cmd.displayHandle = displayHandle;
        cmd.x = x;
        cmd.y = y;
        cmd.width = width;
        cmd.height = height;
        cmd.fullScreen = fullScreen;
        cmd.sendViaDLT = sendViaDLT;
        cmd.filename = filename;
        m_commands.addCommand(ERendererCommand_ReadPixels, cmd);
    }

    void RendererCommands::setClearColor(DisplayHandle displayHandle, const Vector4& color)
    {
        SetClearColorCommand cmd;
        cmd.displayHandle = displayHandle;
        cmd.clearColor = color;
        m_commands.addCommand(ERendererCommand_SetClearColor, cmd);
    }

    void RendererCommands::linkSceneData(SceneId providerSceneId, DataSlotId providerDataSlotId, SceneId consumerSceneId, DataSlotId consumerDataSlotId)
    {
        DataLinkCommand cmd;
        cmd.providerScene = providerSceneId;
        cmd.providerData = providerDataSlotId;
        cmd.consumerScene = consumerSceneId;
        cmd.consumerData = consumerDataSlotId;
        m_commands.addCommand(ERendererCommand_LinkSceneData, cmd);
    }

    void RendererCommands::linkBufferToSceneData(OffscreenBufferHandle providerBuffer, SceneId consumerSceneId, DataSlotId consumerDataSlotId)
    {
        DataLinkCommand cmd;
        cmd.providerBuffer = providerBuffer;
        cmd.consumerScene = consumerSceneId;
        cmd.consumerData = consumerDataSlotId;
        m_commands.addCommand(ERendererCommand_LinkBufferToSceneData, cmd);
    }

    void RendererCommands::unlinkSceneData(SceneId consumerSceneId, DataSlotId consumerDataSlotId)
    {
        DataLinkCommand cmd;
        cmd.providerScene = SceneId(0);
        cmd.providerData = DataSlotId(0);
        cmd.consumerScene = consumerSceneId;
        cmd.consumerData = consumerDataSlotId;
        m_commands.addCommand(ERendererCommand_UnlinkSceneData, cmd);
    }

    void RendererCommands::createOffscreenBuffer(OffscreenBufferHandle buffer, DisplayHandle display, UInt32 width, UInt32 height, Bool interruptible)
    {
        OffscreenBufferCommand cmd;
        cmd.displayHandle = display;
        cmd.bufferHandle = buffer;
        cmd.bufferWidth = width;
        cmd.bufferHeight = height;
        cmd.interruptible = interruptible;
        m_commands.addCommand(ERendererCommand_CreateOffscreenBuffer, cmd);
    }

    void RendererCommands::destroyOffscreenBuffer(OffscreenBufferHandle buffer, DisplayHandle display)
    {
        OffscreenBufferCommand cmd;
        cmd.displayHandle = display;
        cmd.bufferHandle = buffer;
        m_commands.addCommand(ERendererCommand_DestroyOffscreenBuffer, cmd);
    }

    void RendererCommands::assignSceneToOffscreenBuffer(SceneId sceneId, OffscreenBufferHandle buffer)
    {
        OffscreenBufferCommand cmd;
        cmd.bufferHandle = buffer;
        cmd.assignedScene = sceneId;
        m_commands.addCommand(ERendererCommand_AssignSceneToOffscreenBuffer, cmd);
    }

    void RendererCommands::assignSceneToFramebuffer(SceneId sceneId)
    {
        OffscreenBufferCommand cmd;
        cmd.assignedScene = sceneId;
        m_commands.addCommand(ERendererCommand_AssignSceneToFramebuffer, cmd);
    }

    void RendererCommands::moveView(const Vector3& offset)
    {
        RendererViewCommand cmd;
        cmd.displayMovement = offset;
        m_commands.addCommand(ERendererCommand_RelativeTranslation, cmd);
    }

    void RendererCommands::setViewPosition(const Vector3& position)
    {
        RendererViewCommand cmd;
        cmd.displayMovement = position;
        m_commands.addCommand(ERendererCommand_AbsoluteTranslation, cmd);
    }

    void RendererCommands::rotateView(const Vector3& rotationDiff)
    {
        RendererViewCommand cmd;
        cmd.displayMovement = rotationDiff;
        m_commands.addCommand(ERendererCommand_RelativeRotation, cmd);
    }

    void RendererCommands::setViewRotation(const Vector3& rotation)
    {
        RendererViewCommand cmd;
        cmd.displayMovement = rotation;
        m_commands.addCommand(ERendererCommand_AbsoluteRotation, cmd);
    }

    void RendererCommands::resetView()
    {
        RendererViewCommand cmd;
        cmd.displayMovement = Vector3(0.0f);
        m_commands.addCommand(ERendererCommand_ResetRenderView, cmd);
    }

    void RendererCommands::logRendererInfo(ERendererLogTopic topic, Bool verbose, NodeHandle nodeHandleFilter)
    {
        LogCommand cmd;
        cmd.topic = topic;
        cmd.verbose = verbose;
        cmd.nodeHandleFilter = nodeHandleFilter;
        m_commands.addCommand(ERendererCommand_LogRendererInfo, cmd);
    }

    void RendererCommands::logStatistics()
    {
        LogCommand cmd;
        cmd.topic = ERendererLogTopic_All;
        cmd.verbose = true;
        cmd.nodeHandleFilter = NodeHandle::Invalid();
        m_commands.addCommand(ERendererCommand_LogRendererStatistics, cmd);
    }

    void RendererCommands::systemCompositorControllerListIviSurfaces()
    {
        m_commands.addCommand(ERendererCommand_SystemCompositorControllerListIviSurfaces, CompositorCommand());
    }

    void RendererCommands::systemCompositorControllerSetIviSurfaceVisibility(WaylandIviSurfaceId surfaceId, Bool visibility)
    {
        CompositorCommand cmd;
        cmd.waylandIviSurfaceId = surfaceId;
        cmd.visibility = visibility;
        m_commands.addCommand(ERendererCommand_SystemCompositorControllerSetIviSurfaceVisibility, cmd);
    }

    void RendererCommands::systemCompositorControllerSetIviSurfaceOpacity(WaylandIviSurfaceId surfaceId, Float opacity)
    {
        CompositorCommand cmd;
        cmd.waylandIviSurfaceId = surfaceId;
        cmd.opacity = opacity;
        m_commands.addCommand(ERendererCommand_SystemCompositorControllerSetIviSurfaceOpacity, cmd);
    }

    void RendererCommands::systemCompositorControllerSetIviSurfaceDestRectangle(WaylandIviSurfaceId surfaceId, Int32 x, Int32 y, Int32 width, Int32 height)
    {
        CompositorCommand cmd;
        cmd.waylandIviSurfaceId = surfaceId;
        cmd.x = x;
        cmd.y = y;
        cmd.width = width;
        cmd.height = height;
        m_commands.addCommand(ERendererCommand_SystemCompositorControllerSetIviSurfaceDestRectangle, cmd);
    }

    void RendererCommands::systemCompositorControllerSetIviLayerVisibility(WaylandIviLayerId layerId, Bool visibility)
    {
        CompositorCommand cmd;
        cmd.waylandIviLayerId = layerId;
        cmd.visibility = visibility;
        m_commands.addCommand(ERendererCommand_SystemCompositorControllerSetIviLayerVisibility, cmd);
    }

    void RendererCommands::systemCompositorControllerScreenshot(const String& fileName, int32_t screenIviId)
    {
        CompositorCommand cmd;
        cmd.fileName = fileName;
        cmd.screenIviId = screenIviId;
        m_commands.addCommand(ERendererCommand_SystemCompositorControllerScreenshot, cmd);
    }

    void RendererCommands::systemCompositorControllerAddIviSurfaceToIviLayer(WaylandIviSurfaceId surfaceId, WaylandIviLayerId layerId)
    {
        CompositorCommand cmd;
        cmd.waylandIviSurfaceId = surfaceId;
        cmd.waylandIviLayerId = layerId;
        m_commands.addCommand(ERendererCommand_SystemCompositorControllerAddIviSurfaceToIviLayer, cmd);
    }

    void RendererCommands::systemCompositorControllerRemoveIviSurfaceFromIviLayer(WaylandIviSurfaceId surfaceId, WaylandIviLayerId layerId)
    {
        CompositorCommand cmd;
        cmd.waylandIviSurfaceId = surfaceId;
        cmd.waylandIviLayerId = layerId;
        m_commands.addCommand(ERendererCommand_SystemCompositorControllerRemoveIviSurfaceFromIviLayer, cmd);
    }

    void RendererCommands::systemCompositorControllerDestroyIviSurface(WaylandIviSurfaceId surfaceId)
    {
        CompositorCommand cmd;
        cmd.waylandIviSurfaceId = surfaceId;
        m_commands.addCommand(ERendererCommand_SystemCompositorControllerDestroyIviSurface, cmd);
    }

    void RendererCommands::confirmationEcho(const String& text)
    {
        ConfirmationEchoCommand cmd;
        cmd.text = text;
        m_commands.addCommand(ERendererCommand_ConfirmationEcho, cmd);
    }

    void RendererCommands::toggleFrameProfilerVisibility(Bool setVisibleInsteadOfToggle)
    {
        UpdateFrameProfilerCommand cmd;
        cmd.toggleVisibility = setVisibleInsteadOfToggle;
        m_commands.addCommand(ERendererCommand_FrameProfiler_Toggle, cmd);
    }

    void RendererCommands::setFrameProfilerTimingGraphHeight(UInt32 height)
    {
        UpdateFrameProfilerCommand cmd;
        cmd.newTimingGraphHeight = height;
        m_commands.addCommand(ERendererCommand_FrameProfiler_TimingGraphHeight, cmd);
    }

    void RendererCommands::setFrameProfilerCounterGraphHeight(UInt32 height)
    {
        UpdateFrameProfilerCommand cmd;
        cmd.newCounterGraphHeight = height;
        m_commands.addCommand(ERendererCommand_FrameProfiler_CounterGraphHeight, cmd);
    }

    void RendererCommands::setFrameProfilerFilteredRegionFlags(UInt32 flags)
    {
        UpdateFrameProfilerCommand cmd;
        cmd.newRegionFilterFlags = flags;
        m_commands.addCommand(ERendererCommand_FrameProfiler_RegionFilterFlags, cmd);
    }

    void RendererCommands::setFrameTimerLimits(UInt64 limitForSceneResourcesUpload, UInt64 limitForClientResourcesUploadMicrosec, UInt64 limitForSceneActionsApplyMicrosec, UInt64 limitForOffscreenBufferRenderMicrosec)
    {
        SetFrameTimerLimitsCommmand cmd;
        cmd.limitForSceneResourcesUploadMicrosec = limitForSceneResourcesUpload;
        cmd.limitForClientResourcesUploadMicrosec = limitForClientResourcesUploadMicrosec;
        cmd.limitForSceneActionsApplyMicrosec = limitForSceneActionsApplyMicrosec;
        cmd.limitForOffscreenBufferRenderMicrosec = limitForOffscreenBufferRenderMicrosec;
        m_commands.addCommand(ERendererCommand_SetFrameTimerLimits, cmd);
    }

    void RendererCommands::setForceApplyPendingFlushesLimit(UInt maximumPendingFlushes)
    {
        SetFrameTimerLimitsCommmand cmd;
        cmd.limitForPendingFlushesForceApply = maximumPendingFlushes;
        m_commands.addCommand(ERendererCommand_SetLimits_FlushesForceApply, cmd);
    }

    void RendererCommands::setForceUnsubscribeLimits(UInt maximumPendingFlushes)
    {
        SetFrameTimerLimitsCommmand cmd;
        cmd.limitForPendingFlushesForceUnsubscribe = maximumPendingFlushes;
        m_commands.addCommand(ERendererCommand_SetLimits_FlushesForceUnsubscribe, cmd);
    }

    void RendererCommands::setSkippingOfUnmodifiedBuffers(Bool enable)
    {
        SetFeatureCommand cmd;
        cmd.enable = enable;
        m_commands.addCommand(ERendererCommand_SetSkippingOfUnmodifiedBuffers, cmd);
    }

    const RendererCommandContainer& RendererCommands::getCommands() const
    {
        return m_commands;
    }

    void RendererCommands::swapCommandContainer(RendererCommandContainer& commandContainer)
    {
        m_commands.swap(commandContainer);
    }

    void RendererCommands::clear()
    {
        m_commands.clear();
    }
}
