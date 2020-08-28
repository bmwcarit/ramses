//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/RendererCommandBuffer.h"
#include "Utils/LogMacros.h"
#include "RendererLib/EMouseEventType.h"
#include "Scene/EScenePublicationMode.h"
#include "Components/SceneUpdate.h"

namespace ramses_internal
{
    void RendererCommandBuffer::publishScene(SceneId sceneId, EScenePublicationMode mode)
    {
        PlatformGuard guard(m_lock);
        m_commands.publishScene(sceneId, mode);
    }

    void RendererCommandBuffer::unpublishScene(SceneId sceneId)
    {
        PlatformGuard guard(m_lock);
        m_commands.unpublishScene(sceneId);
    }

    void RendererCommandBuffer::receiveScene(const SceneInfo& sceneInfo)
    {
        PlatformGuard guard(m_lock);
        m_commands.receiveScene(sceneInfo);
    }

    void RendererCommandBuffer::setSceneState(SceneId sceneId, RendererSceneState state)
    {
        PlatformGuard guard(m_lock);
        m_commands.setSceneState(sceneId, state);
    }

    void RendererCommandBuffer::setSceneMapping(SceneId sceneId, DisplayHandle display)
    {
        PlatformGuard guard(m_lock);
        m_commands.setSceneMapping(sceneId, display);
    }

    void RendererCommandBuffer::setSceneDisplayBufferAssignment(SceneId sceneId, OffscreenBufferHandle displayBuffer, int32_t sceneRenderOrder)
    {
        PlatformGuard guard(m_lock);
        m_commands.setSceneDisplayBufferAssignment(sceneId, displayBuffer, sceneRenderOrder);
    }

    void RendererCommandBuffer::subscribeScene(SceneId sceneId)
    {
        PlatformGuard guard(m_lock);
        m_commands.subscribeScene(sceneId);
    }

    void RendererCommandBuffer::unsubscribeScene(SceneId sceneId, bool indirect)
    {
        PlatformGuard guard(m_lock);
        m_commands.unsubscribeScene(sceneId, indirect);
    }

    void RendererCommandBuffer::enqueueActionsForScene(SceneId sceneId, SceneUpdate&& sceneUpdate)
    {
        PlatformGuard guard(m_lock);
        m_commands.enqueueActionsForScene(sceneId, std::move(sceneUpdate));
    }

    void RendererCommandBuffer::createDisplay(const DisplayConfig& displayConfig, IResourceProvider& resourceProvider, IResourceUploader& resourceUploader, DisplayHandle handle)
    {
        PlatformGuard guard(m_lock);
        m_commands.createDisplay(displayConfig, resourceProvider, resourceUploader, handle);
    }

    void RendererCommandBuffer::destroyDisplay(DisplayHandle handle)
    {
        PlatformGuard guard(m_lock);
        m_commands.destroyDisplay(handle);
    }

    void RendererCommandBuffer::createOffscreenBuffer(OffscreenBufferHandle buffer, DisplayHandle display, UInt32 width, UInt32 height, bool interruptible)
    {
        PlatformGuard guard(m_lock);
        m_commands.createOffscreenBuffer(buffer, display, width, height, interruptible);
    }

    void RendererCommandBuffer::destroyOffscreenBuffer(OffscreenBufferHandle buffer, DisplayHandle display)
    {
        PlatformGuard guard(m_lock);
        m_commands.destroyOffscreenBuffer(buffer, display);
    }

    void RendererCommandBuffer::assignSceneToDisplayBuffer(SceneId sceneId, OffscreenBufferHandle buffer, Int32 sceneRenderOrder)
    {
        PlatformGuard guard(m_lock);
        m_commands.assignSceneToDisplayBuffer(sceneId, buffer, sceneRenderOrder);
    }

    void RendererCommandBuffer::mapSceneToDisplay(SceneId sceneId, DisplayHandle displayHandle)
    {
        PlatformGuard guard(m_lock);
        m_commands.mapSceneToDisplay(sceneId, displayHandle);
    }

    void RendererCommandBuffer::unmapScene(SceneId sceneId)
    {
        PlatformGuard guard(m_lock);
        m_commands.unmapScene(sceneId);
    }

    void RendererCommandBuffer::showScene(SceneId sceneId)
    {
        PlatformGuard guard(m_lock);
        m_commands.showScene(sceneId);
    }

    void RendererCommandBuffer::hideScene(SceneId sceneId)
    {
        PlatformGuard guard(m_lock);
        m_commands.hideScene(sceneId);
    }

    void RendererCommandBuffer::updateWarpingData(DisplayHandle displayHandle, const WarpingMeshData& warpingData)
    {
        PlatformGuard guard(m_lock);
        m_commands.updateWarpingData(displayHandle, warpingData);
    }

    void RendererCommandBuffer::readPixels(DisplayHandle displayHandle, OffscreenBufferHandle obHandle, const String& filename, Bool fullScreen, UInt32 x, UInt32 y, UInt32 width, UInt32 height, Bool sendViaDLT)
    {
        PlatformGuard guard(m_lock);
        m_commands.readPixels(displayHandle, obHandle, filename, fullScreen, x, y, width, height, sendViaDLT);
    }

    void RendererCommandBuffer::setClearColor(DisplayHandle displayHandle, OffscreenBufferHandle obHandle, const Vector4& color)
    {
        PlatformGuard guard(m_lock);
        m_commands.setClearColor(displayHandle, obHandle, color);
    }

    void RendererCommandBuffer::linkSceneData(SceneId providerSceneId, DataSlotId providerDataSlotId, SceneId consumerSceneId, DataSlotId consumerDataSlotId)
    {
        PlatformGuard guard(m_lock);
        m_commands.linkSceneData(providerSceneId, providerDataSlotId, consumerSceneId, consumerDataSlotId);
    }

    void RendererCommandBuffer::linkBufferToSceneData(OffscreenBufferHandle providerBuffer, SceneId consumerSceneId, DataSlotId consumerDataSlotId)
    {
        PlatformGuard guard(m_lock);
        m_commands.linkBufferToSceneData(providerBuffer, consumerSceneId, consumerDataSlotId);
    }

    void RendererCommandBuffer::unlinkSceneData(SceneId consumerSceneId, DataSlotId consumerDataSlotId)
    {
        PlatformGuard guard(m_lock);
        m_commands.unlinkSceneData(consumerSceneId, consumerDataSlotId);
    }

    void RendererCommandBuffer::moveView(const Vector3& offset)
    {
        PlatformGuard guard(m_lock);
        m_commands.moveView(offset);
    }

    void RendererCommandBuffer::setViewPosition(const Vector3& position)
    {
        PlatformGuard guard(m_lock);
        m_commands.setViewPosition(position);
    }

    void RendererCommandBuffer::rotateView(const Vector3& rotationDiff)
    {
        PlatformGuard guard(m_lock);
        m_commands.rotateView(rotationDiff);
    }

    void RendererCommandBuffer::setViewRotation(const Vector3& rotation)
    {
        PlatformGuard guard(m_lock);
        m_commands.setViewRotation(rotation);
    }

    void RendererCommandBuffer::resetView()
    {
        PlatformGuard guard(m_lock);
        m_commands.resetView();
    }

    void RendererCommandBuffer::logStatistics()
    {
        PlatformGuard guard(m_lock);
        m_commands.logStatistics();
    }

    void RendererCommandBuffer::logRendererInfo(ERendererLogTopic topic, Bool verbose, NodeHandle nodeHandleFilter)
    {
        PlatformGuard guard(m_lock);
        m_commands.logRendererInfo(topic, verbose, nodeHandleFilter);
    }

    void RendererCommandBuffer::systemCompositorControllerListIviSurfaces()
    {
        PlatformGuard guard(m_lock);
        m_commands.systemCompositorControllerListIviSurfaces();
    }

    void RendererCommandBuffer::systemCompositorControllerSetIviSurfaceVisibility(WaylandIviSurfaceId surfaceId, Bool visibility)
    {
        PlatformGuard guard(m_lock);
        m_commands.systemCompositorControllerSetIviSurfaceVisibility(surfaceId, visibility);
    }

    void RendererCommandBuffer::systemCompositorControllerSetIviSurfaceOpacity(WaylandIviSurfaceId surfaceId, Float opacity)
    {
        PlatformGuard guard(m_lock);
        m_commands.systemCompositorControllerSetIviSurfaceOpacity(surfaceId, opacity);
    }

    void RendererCommandBuffer::systemCompositorControllerSetIviSurfaceDestRectangle(WaylandIviSurfaceId surfaceId, Int32 x, Int32 y, Int32 width, Int32 height)
    {
        PlatformGuard guard(m_lock);
        m_commands.systemCompositorControllerSetIviSurfaceDestRectangle(surfaceId, x, y, width, height);
    }

    void RendererCommandBuffer::systemCompositorControllerSetIviLayerVisibility(WaylandIviLayerId layerId, Bool visibility)
    {
        PlatformGuard guard(m_lock);
        m_commands.systemCompositorControllerSetIviLayerVisibility(layerId, visibility);
    }

    void RendererCommandBuffer::systemCompositorControllerScreenshot(const String& fileName, int32_t screenIviId)
    {
        PlatformGuard guard(m_lock);
        m_commands.systemCompositorControllerScreenshot(fileName, screenIviId);
    }

    void RendererCommandBuffer::systemCompositorControllerAddIviSurfaceToIviLayer(WaylandIviSurfaceId surfaceId, WaylandIviLayerId layerId)
    {
        PlatformGuard guard(m_lock);
        m_commands.systemCompositorControllerAddIviSurfaceToIviLayer(surfaceId,layerId);
    }

    void RendererCommandBuffer::systemCompositorControllerRemoveIviSurfaceFromIviLayer(WaylandIviSurfaceId surfaceId, WaylandIviLayerId layerId)
    {
        PlatformGuard guard(m_lock);
        m_commands.systemCompositorControllerRemoveIviSurfaceFromIviLayer(surfaceId,layerId);
    }

    void RendererCommandBuffer::systemCompositorControllerDestroyIviSurface(WaylandIviSurfaceId surfaceId)
    {
        PlatformGuard guard(m_lock);
        m_commands.systemCompositorControllerDestroyIviSurface(surfaceId);
    }

    void RendererCommandBuffer::confirmationEcho(const String& text)
    {
        PlatformGuard guard(m_lock);
        m_commands.confirmationEcho(text);
    }

    void RendererCommandBuffer::toggleFrameProfilerVisibility(Bool setVisibleInsteadOfToggle)
    {
        PlatformGuard guard(m_lock);
        m_commands.toggleFrameProfilerVisibility(setVisibleInsteadOfToggle);
    }

    void RendererCommandBuffer::setFrameProfilerTimingGraphHeight(UInt32 height)
    {
        PlatformGuard guard(m_lock);
        m_commands.setFrameProfilerTimingGraphHeight(height);
    }

    void RendererCommandBuffer::setFrameProfilerCounterGraphHeight(UInt32 height)
    {
        PlatformGuard guard(m_lock);
        m_commands.setFrameProfilerCounterGraphHeight(height);
    }

    void RendererCommandBuffer::setFrameProfilerFilteredRegionFlags(UInt32 flags)
    {
        PlatformGuard guard(m_lock);
        m_commands.setFrameProfilerFilteredRegionFlags(flags);
    }

    void RendererCommandBuffer::setFrameTimerLimits(UInt64 limitForSceneResourcesUploadMicrosec, UInt64 limitForClientResourcesUploadMicrosec, UInt64 limitForOffscreenBufferRenderMicrosec)
    {
        PlatformGuard guard(m_lock);
        m_commands.setFrameTimerLimits(limitForSceneResourcesUploadMicrosec, limitForClientResourcesUploadMicrosec, limitForOffscreenBufferRenderMicrosec);
    }

    void RendererCommandBuffer::setLimitsFlushesForceApply(UInt limitFlushesForceApply)
    {
        PlatformGuard guard(m_lock);
        m_commands.setForceApplyPendingFlushesLimit(limitFlushesForceApply);
    }

    void RendererCommandBuffer::setLimitsFlushesForceUnsubscribe(UInt limitFlushesForceUnsubscribe)
    {
        PlatformGuard guard(m_lock);
        m_commands.setForceUnsubscribeLimits(limitFlushesForceUnsubscribe);
    }

    void RendererCommandBuffer::setSkippingOfUnmodifiedBuffers(Bool enable)
    {
        PlatformGuard guard(m_lock);
        m_commands.setSkippingOfUnmodifiedBuffers(enable);
    }

    void RendererCommandBuffer::handlePickEvent(SceneId sceneId, Vector2 sceneViewportCoords)
    {
        PlatformGuard guard(m_lock);
        m_commands.handlePickEvent(sceneId, sceneViewportCoords);
    }

    void RendererCommandBuffer::addCommands(const RendererCommands& renderCommands)
    {
        PlatformGuard guard(m_lock);
        const RendererCommandContainer& commands = renderCommands.getCommands();
        for (UInt32 i = 0; i < commands.getTotalCommandCount(); i++)
        {
            ERendererCommand commandType = commands.getCommandType(i);
            switch (commandType)
            {
            case ERendererCommand_PublishedScene:
            {
                const SceneInfoCommand& cmd = commands.getCommandData<SceneInfoCommand>(i);
                m_commands.publishScene(cmd.sceneInformation.sceneID, cmd.sceneInformation.publicationMode);
            }
            break;
            case ERendererCommand_UnpublishedScene:
            {
                const SceneInfoCommand& cmd = commands.getCommandData<SceneInfoCommand>(i);
                m_commands.unpublishScene(cmd.sceneInformation.sceneID);
            }
            break;
            case ERendererCommand_ReceivedScene:
            {
                const SceneInfoCommand& cmd = commands.getCommandData<SceneInfoCommand>(i);
                m_commands.receiveScene(cmd.sceneInformation);
            }
            break;
            case ERendererCommand_SetSceneState:
            {
                const auto& cmd = commands.getCommandData<SceneStateCommand>(i);
                m_commands.setSceneState(cmd.sceneId, cmd.state);
            }
            break;
            case ERendererCommand_SetSceneMapping:
            {
                const auto& cmd = commands.getCommandData<SceneMappingCommand>(i);
                m_commands.setSceneMapping(cmd.sceneId, cmd.displayHandle);
            }
            break;
            case ERendererCommand_SetSceneDisplayBufferAssignment:
            {
                const auto& cmd = commands.getCommandData<SceneMappingCommand>(i);
                m_commands.setSceneDisplayBufferAssignment(cmd.sceneId, cmd.offscreenBuffer, cmd.sceneRenderOrder);
            }
            break;
            case ERendererCommand_SubscribeScene:
            {
                const SceneInfoCommand& cmd = commands.getCommandData<SceneInfoCommand>(i);
                m_commands.subscribeScene(cmd.sceneInformation.sceneID);
            }
            break;
            case ERendererCommand_UnsubscribeScene:
            {
                const SceneInfoCommand& cmd = commands.getCommandData<SceneInfoCommand>(i);
                m_commands.unsubscribeScene(cmd.sceneInformation.sceneID, cmd.indirect);
            }
            break;
            case ERendererCommand_CreateDisplay:
            {
                const DisplayCommand& cmd = commands.getCommandData<DisplayCommand>(i);
                m_commands.createDisplay(cmd.displayConfig, *cmd.resourceProvider, *cmd.resourceUploader, cmd.displayHandle);
            }
            break;
            case ERendererCommand_DestroyDisplay:
            {
                const DisplayCommand& cmd = commands.getCommandData<DisplayCommand>(i);
                m_commands.destroyDisplay(cmd.displayHandle);
            }
            break;
            case ERendererCommand_MapSceneToDisplay:
            {
                const SceneMappingCommand& cmd = commands.getCommandData<SceneMappingCommand>(i);
                m_commands.mapSceneToDisplay(cmd.sceneId, cmd.displayHandle);
            }
            break;
            case ERendererCommand_UnmapSceneFromDisplays:
            {
                const SceneMappingCommand& cmd = commands.getCommandData<SceneMappingCommand>(i);
                m_commands.unmapScene(cmd.sceneId);
            }
            break;
            case ERendererCommand_ShowScene:
            {
                const SceneStateCommand& cmd = commands.getCommandData<SceneStateCommand>(i);
                m_commands.showScene(cmd.sceneId);
            }
            break;
            case ERendererCommand_HideScene:
            {
                const SceneStateCommand& cmd = commands.getCommandData<SceneStateCommand>(i);
                m_commands.hideScene(cmd.sceneId);
            }
            break;
            case ERendererCommand_RelativeTranslation:
            {
                const RendererViewCommand& cmd = commands.getCommandData<RendererViewCommand>(i);
                m_commands.moveView(cmd.displayMovement);
            }
            break;
            case ERendererCommand_AbsoluteTranslation:
            {
                const RendererViewCommand& cmd = commands.getCommandData<RendererViewCommand>(i);
                m_commands.setViewPosition(cmd.displayMovement);
            }
            break;
            case ERendererCommand_RelativeRotation:
            {
                const RendererViewCommand& cmd = commands.getCommandData<RendererViewCommand>(i);
                m_commands.rotateView(cmd.displayMovement);
            }
            break;
            case ERendererCommand_AbsoluteRotation:
            {
                const RendererViewCommand& cmd = commands.getCommandData<RendererViewCommand>(i);
                m_commands.setViewRotation(cmd.displayMovement);
            }
            break;
            case ERendererCommand_ResetRenderView:
            {
                m_commands.resetView();
            }
            break;
            case ERendererCommand_UpdateWarpingData:
            {
                const WarpingDataCommand& cmd = commands.getCommandData<WarpingDataCommand>(i);
                m_commands.updateWarpingData(cmd.displayHandle, cmd.warpingData);
            }
            break;
            case ERendererCommand_ReadPixels:
            {
                const ReadPixelsCommand& cmd = commands.getCommandData<ReadPixelsCommand>(i);
                m_commands.readPixels(cmd.displayHandle, cmd.offscreenBufferHandle, cmd.filename, cmd.fullScreen, cmd.x, cmd.y, cmd.width, cmd.height, cmd.sendViaDLT);
            }
            break;
            case ERendererCommand_LinkSceneData:
            {
                const DataLinkCommand& cmd = commands.getCommandData<DataLinkCommand>(i);
                m_commands.linkSceneData(cmd.providerScene, cmd.providerData, cmd.consumerScene, cmd.consumerData);
            }
            break;
            case ERendererCommand_LinkBufferToSceneData:
            {
                const DataLinkCommand& cmd = commands.getCommandData<DataLinkCommand>(i);
                m_commands.linkBufferToSceneData(cmd.providerBuffer, cmd.consumerScene, cmd.consumerData);
            }
            break;
            case ERendererCommand_UnlinkSceneData:
            {
                const DataLinkCommand& cmd = commands.getCommandData<DataLinkCommand>(i);
                m_commands.unlinkSceneData(cmd.consumerScene, cmd.consumerData);
            }
            break;
            case ERendererCommand_CreateOffscreenBuffer:
            {
                const OffscreenBufferCommand& cmd = commands.getCommandData<OffscreenBufferCommand>(i);
                m_commands.createOffscreenBuffer(cmd.bufferHandle, cmd.displayHandle, cmd.bufferWidth, cmd.bufferHeight, cmd.interruptible);
            }
            break;
            case ERendererCommand_DestroyOffscreenBuffer:
            {
                const OffscreenBufferCommand& cmd = commands.getCommandData<OffscreenBufferCommand>(i);
                m_commands.destroyOffscreenBuffer(cmd.bufferHandle, cmd.displayHandle);
            }
            break;
            case ERendererCommand_AssignSceneToDisplayBuffer:
            {
                const auto& cmd = commands.getCommandData<SceneMappingCommand>(i);
                m_commands.assignSceneToDisplayBuffer(cmd.sceneId, cmd.offscreenBuffer, cmd.sceneRenderOrder);
            }
            break;
            case ERendererCommand_LogRendererStatistics:
            {
                m_commands.logStatistics();
            }
            break;
            case ERendererCommand_LogRendererInfo:
            {
                const LogCommand& cmd = commands.getCommandData<LogCommand>(i);
                m_commands.logRendererInfo(cmd.topic, cmd.verbose, cmd.nodeHandleFilter);
            }
            break;
            case ERendererCommand_SystemCompositorControllerListIviSurfaces:
            {
                m_commands.systemCompositorControllerListIviSurfaces();
            }
            break;
            case ERendererCommand_SystemCompositorControllerSetIviSurfaceVisibility:
            {
                const CompositorCommand& cmd = commands.getCommandData<CompositorCommand>(i);
                m_commands.systemCompositorControllerSetIviSurfaceVisibility(cmd.waylandIviSurfaceId, cmd.visibility);
            }
            break;
            case ERendererCommand_SystemCompositorControllerSetIviSurfaceOpacity:
            {
                const CompositorCommand& cmd = commands.getCommandData<CompositorCommand>(i);
                m_commands.systemCompositorControllerSetIviSurfaceOpacity(cmd.waylandIviSurfaceId, cmd.opacity);
            }
            break;
            case ERendererCommand_SystemCompositorControllerSetIviSurfaceDestRectangle:
            {
                const CompositorCommand& cmd = commands.getCommandData<CompositorCommand>(i);
                m_commands.systemCompositorControllerSetIviSurfaceDestRectangle(cmd.waylandIviSurfaceId,cmd.x, cmd.y, cmd.width, cmd.height);
            }
            break;
            case ERendererCommand_SystemCompositorControllerSetIviLayerVisibility:
            {
                const CompositorCommand& cmd = commands.getCommandData<CompositorCommand>(i);
                m_commands.systemCompositorControllerSetIviLayerVisibility(cmd.waylandIviLayerId, cmd.visibility);
            }
            break;
            case ERendererCommand_SystemCompositorControllerScreenshot:
            {
                const CompositorCommand& cmd = commands.getCommandData<CompositorCommand>(i);
                m_commands.systemCompositorControllerScreenshot(cmd.fileName, cmd.screenIviId);
            }
            break;
            case ERendererCommand_SystemCompositorControllerAddIviSurfaceToIviLayer:
            {
                const CompositorCommand& cmd = commands.getCommandData<CompositorCommand>(i);
                m_commands.systemCompositorControllerAddIviSurfaceToIviLayer(cmd.waylandIviSurfaceId, cmd.waylandIviLayerId);
            }
            break;
            case ERendererCommand_SystemCompositorControllerRemoveIviSurfaceFromIviLayer:
            {
                const CompositorCommand& cmd = commands.getCommandData<CompositorCommand>(i);
                m_commands.systemCompositorControllerRemoveIviSurfaceFromIviLayer(cmd.waylandIviSurfaceId, cmd.waylandIviLayerId);
            }
            break;
            case ERendererCommand_SystemCompositorControllerDestroyIviSurface:
            {
                const CompositorCommand& cmd = commands.getCommandData<CompositorCommand>(i);
                m_commands.systemCompositorControllerDestroyIviSurface(cmd.waylandIviSurfaceId);
            }
            break;
            case ERendererCommand_ConfirmationEcho:
            {
                const ConfirmationEchoCommand& cmd = commands.getCommandData<ConfirmationEchoCommand>(i);
                m_commands.confirmationEcho(cmd.text);
            }
            break;
            case ERendererCommand_FrameProfiler_Toggle:
            {
                const auto& cmd = commands.getCommandData<UpdateFrameProfilerCommand>(i);
                m_commands.toggleFrameProfilerVisibility(cmd.toggleVisibility);
            }
            break;
            case ERendererCommand_FrameProfiler_TimingGraphHeight:
            {
                const auto& cmd = commands.getCommandData<UpdateFrameProfilerCommand>(i);
                m_commands.setFrameProfilerTimingGraphHeight(cmd.newTimingGraphHeight);
            }
            break;
            case ERendererCommand_FrameProfiler_CounterGraphHeight:
            {
                const auto& cmd = commands.getCommandData<UpdateFrameProfilerCommand>(i);
                m_commands.setFrameProfilerCounterGraphHeight(cmd.newCounterGraphHeight);
            }
            break;
            case ERendererCommand_FrameProfiler_RegionFilterFlags:
            {
                const auto& cmd = commands.getCommandData<UpdateFrameProfilerCommand>(i);
                m_commands.setFrameProfilerFilteredRegionFlags(cmd.newRegionFilterFlags);
            }
            break;
            case ERendererCommand_SetClearColor:
            {
                const auto& cmd = commands.getCommandData<SetClearColorCommand>(i);
                m_commands.setClearColor(cmd.displayHandle, cmd.obHandle, cmd.clearColor);
            }
            break;
            case ERendererCommand_SetFrameTimerLimits:
            {
                const auto& cmd = commands.getCommandData<SetFrameTimerLimitsCommmand>(i);
                m_commands.setFrameTimerLimits(cmd.limitForSceneResourcesUploadMicrosec, cmd.limitForClientResourcesUploadMicrosec, cmd.limitForOffscreenBufferRenderMicrosec);
            }
            break;
            case ERendererCommand_SetLimits_FlushesForceApply:
            {
                const auto& cmd = commands.getCommandData<SetFrameTimerLimitsCommmand>(i);
                m_commands.setForceApplyPendingFlushesLimit(cmd.limitForPendingFlushesForceApply);
            }
            break;
            case ERendererCommand_SetLimits_FlushesForceUnsubscribe:
            {
                const auto& cmd = commands.getCommandData<SetFrameTimerLimitsCommmand>(i);
                m_commands.setForceUnsubscribeLimits(cmd.limitForPendingFlushesForceUnsubscribe);
            }
            break;
            case ERendererCommand_SetSkippingOfUnmodifiedBuffers:
            {
                const auto& cmd = commands.getCommandData<SetFeatureCommand>(i);
                m_commands.setSkippingOfUnmodifiedBuffers(cmd.enable);
            }
            break;
            case ERendererCommand_PickEvent:
            {
                const auto& cmd = commands.getCommandData<PickingCommand>(i);
                m_commands.handlePickEvent(cmd.sceneId, cmd.coordsNormalizedToBufferSize);
            }
            break;
            default:
                assert(false);
                break;
            }
        }
    }

    void RendererCommandBuffer::swapCommandContainer(RendererCommandContainer& commandContainer)
    {
        PlatformGuard guard(m_lock);
        m_commands.swapCommandContainer(commandContainer);
    }
}
