//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/RendererCommandBuffer.h"
#include "Utils/LogMacros.h"
#include "PlatformAbstraction/PlatformGuard.h"
#include "RendererLib/EMouseEventType.h"
#include "Scene/EScenePublicationMode.h"

namespace ramses_internal
{
    RendererCommandBuffer::RendererCommandBuffer()
    {
    }

    void RendererCommandBuffer::publishScene(SceneId sceneId, const Guid& clientID, EScenePublicationMode mode)
    {
        PlatformGuard guard(m_lock);
        RendererCommands::publishScene(sceneId, clientID, mode);
    }

    void RendererCommandBuffer::unpublishScene(SceneId sceneId)
    {
        PlatformGuard guard(m_lock);
        RendererCommands::unpublishScene(sceneId);
    }

    void RendererCommandBuffer::receiveScene(const SceneInfo& sceneInfo)
    {
        PlatformGuard guard(m_lock);
        RendererCommands::receiveScene(sceneInfo);
    }

    void RendererCommandBuffer::subscribeScene(SceneId sceneId)
    {
        PlatformGuard guard(m_lock);
        RendererCommands::subscribeScene(sceneId);
    }

    void RendererCommandBuffer::unsubscribeScene(SceneId sceneId, bool indirect)
    {
        PlatformGuard guard(m_lock);
        RendererCommands::unsubscribeScene(sceneId, indirect);
    }

    void RendererCommandBuffer::enqueueActionsForScene(SceneId sceneId, SceneActionCollection&& newActions)
    {
        PlatformGuard guard(m_lock);
        RendererCommands::enqueueActionsForScene(sceneId, std::move(newActions));
    }

    void RendererCommandBuffer::createDisplay(const DisplayConfig& displayConfig, IResourceProvider& resourceProvider, IResourceUploader& resourceUploader, DisplayHandle handle)
    {
        PlatformGuard guard(m_lock);
        RendererCommands::createDisplay(displayConfig, resourceProvider, resourceUploader, handle);
    }

    void RendererCommandBuffer::destroyDisplay(DisplayHandle handle)
    {
        PlatformGuard guard(m_lock);
        RendererCommands::destroyDisplay(handle);
    }

    void RendererCommandBuffer::mapSceneToDisplay(SceneId sceneId, DisplayHandle displayHandle, const Int32 sceneRenderOrder)
    {
        PlatformGuard guard(m_lock);
        RendererCommands::mapSceneToDisplay(sceneId, displayHandle, sceneRenderOrder);
    }

    void RendererCommandBuffer::unmapScene(SceneId sceneId)
    {
        PlatformGuard guard(m_lock);
        RendererCommands::unmapScene(sceneId);
    }

    void RendererCommandBuffer::updateWarpingData(DisplayHandle displayHandle, const WarpingMeshData& warpingData)
    {
        PlatformGuard guard(m_lock);
        RendererCommands::updateWarpingData(displayHandle, warpingData);
    }

    void RendererCommandBuffer::readPixels(DisplayHandle displayHandle, const String& filename, Bool fullScreen, UInt32 x, UInt32 y, UInt32 width, UInt32 height, Bool sendViaDLT)
    {
        PlatformGuard guard(m_lock);
        RendererCommands::readPixels(displayHandle, filename, fullScreen, x, y, width, height, sendViaDLT);
    }

    void RendererCommandBuffer::setClearColor(DisplayHandle displayHandle, const Vector4& color)
    {
        PlatformGuard guard(m_lock);
        RendererCommands::setClearColor(displayHandle, color);
    }

    void RendererCommandBuffer::linkSceneData(SceneId providerSceneId, DataSlotId providerDataSlotId, SceneId consumerSceneId, DataSlotId consumerDataSlotId)
    {
        PlatformGuard guard(m_lock);
        RendererCommands::linkSceneData(providerSceneId, providerDataSlotId, consumerSceneId, consumerDataSlotId);
    }

    void RendererCommandBuffer::unlinkSceneData(SceneId consumerSceneId, DataSlotId consumerDataSlotId)
    {
        PlatformGuard guard(m_lock);
        RendererCommands::unlinkSceneData(consumerSceneId, consumerDataSlotId);
    }

    void RendererCommandBuffer::moveView(const Vector3& offset)
    {
        PlatformGuard guard(m_lock);
        RendererCommands::moveView(offset);
    }

    void RendererCommandBuffer::setViewPosition(const Vector3& position)
    {
        PlatformGuard guard(m_lock);
        RendererCommands::setViewPosition(position);
    }

    void RendererCommandBuffer::rotateView(const Vector3& rotationDiff)
    {
        PlatformGuard guard(m_lock);
        RendererCommands::rotateView(rotationDiff);
    }

    void RendererCommandBuffer::setViewRotation(const Vector3& rotation)
    {
        PlatformGuard guard(m_lock);
        RendererCommands::setViewRotation(rotation);
    }

    void RendererCommandBuffer::resetView()
    {
        PlatformGuard guard(m_lock);
        RendererCommands::resetView();
    }

    void RendererCommandBuffer::logStatistics()
    {
        PlatformGuard guard(m_lock);
        RendererCommands::logStatistics();
    }

    void RendererCommandBuffer::logRendererInfo(ERendererLogTopic topic, Bool verbose, NodeHandle nodeHandleFilter)
    {
        PlatformGuard guard(m_lock);
        RendererCommands::logRendererInfo(topic, verbose, nodeHandleFilter);
    }

    void RendererCommandBuffer::systemCompositorControllerListIviSurfaces()
    {
        PlatformGuard guard(m_lock);
        RendererCommands::systemCompositorControllerListIviSurfaces();
    }

    void RendererCommandBuffer::systemCompositorControllerSetIviSurfaceVisibility(WaylandIviSurfaceId surfaceId, Bool visibility)
    {
        PlatformGuard guard(m_lock);
        RendererCommands::systemCompositorControllerSetIviSurfaceVisibility(surfaceId, visibility);
    }

    void RendererCommandBuffer::systemCompositorControllerSetIviSurfaceOpacity(WaylandIviSurfaceId surfaceId, Float opacity)
    {
        PlatformGuard guard(m_lock);
        RendererCommands::systemCompositorControllerSetIviSurfaceOpacity(surfaceId, opacity);
    }

    void RendererCommandBuffer::systemCompositorControllerSetIviSurfaceDestRectangle(WaylandIviSurfaceId surfaceId, Int32 x, Int32 y, Int32 width, Int32 height)
    {
        PlatformGuard guard(m_lock);
        RendererCommands::systemCompositorControllerSetIviSurfaceDestRectangle(surfaceId, x, y, width, height);
    }

    void RendererCommandBuffer::systemCompositorControllerSetIviLayerVisibility(WaylandIviLayerId layerId, Bool visibility)
    {
        PlatformGuard guard(m_lock);
        RendererCommands::systemCompositorControllerSetIviLayerVisibility(layerId, visibility);
    }

    void RendererCommandBuffer::systemCompositorControllerScreenshot(const String& fileName, int32_t screenIviId)
    {
        PlatformGuard guard(m_lock);
        RendererCommands::systemCompositorControllerScreenshot(fileName, screenIviId);
    }

    void RendererCommandBuffer::systemCompositorControllerAddIviSurfaceToIviLayer(WaylandIviSurfaceId surfaceId, WaylandIviLayerId layerId)
    {
        PlatformGuard guard(m_lock);
        RendererCommands::systemCompositorControllerAddIviSurfaceToIviLayer(surfaceId,layerId);
    }

    void RendererCommandBuffer::systemCompositorControllerRemoveIviSurfaceFromIviLayer(WaylandIviSurfaceId surfaceId, WaylandIviLayerId layerId)
    {
        PlatformGuard guard(m_lock);
        RendererCommands::systemCompositorControllerRemoveIviSurfaceFromIviLayer(surfaceId,layerId);
    }

    void RendererCommandBuffer::systemCompositorControllerDestroyIviSurface(WaylandIviSurfaceId surfaceId)
    {
        PlatformGuard guard(m_lock);
        RendererCommands::systemCompositorControllerDestroyIviSurface(surfaceId);
    }

    void RendererCommandBuffer::confirmationEcho(const String& text)
    {
        PlatformGuard guard(m_lock);
        RendererCommands::confirmationEcho(text);
    }

    void RendererCommandBuffer::toggleFrameProfilerVisibility(Bool setVisibleInsteadOfToggle)
    {
        PlatformGuard guard(m_lock);
        RendererCommands::toggleFrameProfilerVisibility(setVisibleInsteadOfToggle);
    }

    void RendererCommandBuffer::setFrameProfilerTimingGraphHeight(UInt32 height)
    {
        PlatformGuard guard(m_lock);
        RendererCommands::setFrameProfilerTimingGraphHeight(height);
    }

    void RendererCommandBuffer::setFrameProfilerCounterGraphHeight(UInt32 height)
    {
        PlatformGuard guard(m_lock);
        RendererCommands::setFrameProfilerCounterGraphHeight(height);
    }

    void RendererCommandBuffer::setFrameProfilerFilteredRegionFlags(UInt32 flags)
    {
        PlatformGuard guard(m_lock);
        RendererCommands::setFrameProfilerFilteredRegionFlags(flags);
    }

    void RendererCommandBuffer::setFrameTimerLimits(UInt64 limitForSceneResourcesUploadMicrosec, UInt64 limitForClientResourcesUploadMicrosec, UInt64 limitForSceneActionsApplyMicrosec, UInt64 limitForOffscreenBufferRenderMicrosec)
    {
        PlatformGuard guard(m_lock);
        RendererCommands::setFrameTimerLimits(limitForSceneResourcesUploadMicrosec, limitForClientResourcesUploadMicrosec, limitForSceneActionsApplyMicrosec, limitForOffscreenBufferRenderMicrosec);
    }

    void RendererCommandBuffer::setLimitsFlushesForceApply(UInt limitFlushesForceApply)
    {
        PlatformGuard guard(m_lock);
        RendererCommands::setForceApplyPendingFlushesLimit(limitFlushesForceApply);
    }

    void RendererCommandBuffer::setLimitsFlushesForceUnsubscribe(UInt limitFlushesForceUnsubscribe)
    {
        PlatformGuard guard(m_lock);
        RendererCommands::setForceUnsubscribeLimits(limitFlushesForceUnsubscribe);
    }

    void RendererCommandBuffer::setSkippingOfUnmodifiedBuffers(Bool enable)
    {
        PlatformGuard guard(m_lock);
        RendererCommands::setSkippingOfUnmodifiedBuffers(enable);
    }

    void RendererCommandBuffer::lock()
    {
        m_lock.lock();
    }

    void RendererCommandBuffer::unlock()
    {
        m_lock.unlock();
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
                publishScene(cmd.sceneInformation.sceneID, cmd.clientID, cmd.sceneInformation.publicationMode);
            }
            break;
            case ERendererCommand_UnpublishedScene:
            {
                const SceneInfoCommand& cmd = commands.getCommandData<SceneInfoCommand>(i);
                unpublishScene(cmd.sceneInformation.sceneID);
            }
            break;
            case ERendererCommand_ReceivedScene:
            {
                const SceneInfoCommand& cmd = commands.getCommandData<SceneInfoCommand>(i);
                receiveScene(cmd.sceneInformation);
            }
            break;
            case ERendererCommand_SubscribeScene:
            {
                const SceneInfoCommand& cmd = commands.getCommandData<SceneInfoCommand>(i);
                subscribeScene(cmd.sceneInformation.sceneID);
            }
            break;
            case ERendererCommand_UnsubscribeScene:
            {
                const SceneInfoCommand& cmd = commands.getCommandData<SceneInfoCommand>(i);
                unsubscribeScene(cmd.sceneInformation.sceneID, cmd.indirect);
            }
            break;
            case ERendererCommand_CreateDisplay:
            {
                const DisplayCommand& cmd = commands.getCommandData<DisplayCommand>(i);
                createDisplay(cmd.displayConfig, *cmd.resourceProvider, *cmd.resourceUploader, cmd.displayHandle);
            }
            break;
            case ERendererCommand_DestroyDisplay:
            {
                const DisplayCommand& cmd = commands.getCommandData<DisplayCommand>(i);
                destroyDisplay(cmd.displayHandle);
            }
            break;
            case ERendererCommand_MapSceneToDisplay:
            {
                const SceneMappingCommand& cmd = commands.getCommandData<SceneMappingCommand>(i);
                mapSceneToDisplay(cmd.sceneId, cmd.displayHandle, cmd.sceneRenderOrder);
            }
            break;
            case ERendererCommand_UnmapSceneFromDisplays:
            {
                const SceneMappingCommand& cmd = commands.getCommandData<SceneMappingCommand>(i);
                unmapScene(cmd.sceneId);
            }
            break;
            case ERendererCommand_ShowScene:
            {
                const SceneRenderCommand& cmd = commands.getCommandData<SceneRenderCommand>(i);
                showScene(cmd.sceneId);
            }
            break;
            case ERendererCommand_HideScene:
            {
                const SceneRenderCommand& cmd = commands.getCommandData<SceneRenderCommand>(i);
                hideScene(cmd.sceneId);
            }
            break;
            case ERendererCommand_RelativeTranslation:
            {
                const RendererViewCommand& cmd = commands.getCommandData<RendererViewCommand>(i);
                moveView(cmd.displayMovement);
            }
            break;
            case ERendererCommand_AbsoluteTranslation:
            {
                const RendererViewCommand& cmd = commands.getCommandData<RendererViewCommand>(i);
                setViewPosition(cmd.displayMovement);
            }
            break;
            case ERendererCommand_RelativeRotation:
            {
                const RendererViewCommand& cmd = commands.getCommandData<RendererViewCommand>(i);
                rotateView(cmd.displayMovement);
            }
            break;
            case ERendererCommand_AbsoluteRotation:
            {
                const RendererViewCommand& cmd = commands.getCommandData<RendererViewCommand>(i);
                setViewRotation(cmd.displayMovement);
            }
            break;
            case ERendererCommand_ResetRenderView:
            {
                resetView();
            }
            break;
            case ERendererCommand_UpdateWarpingData:
            {
                const WarpingDataCommand& cmd = commands.getCommandData<WarpingDataCommand>(i);
                updateWarpingData(cmd.displayHandle, cmd.warpingData);
            }
            break;
            case ERendererCommand_ReadPixels:
            {
                const ReadPixelsCommand& cmd = commands.getCommandData<ReadPixelsCommand>(i);
                readPixels(cmd.displayHandle, cmd.filename, cmd.fullScreen, cmd.x, cmd.y, cmd.width, cmd.height, cmd.sendViaDLT);
            }
            break;
            case ERendererCommand_LinkSceneData:
            {
                const DataLinkCommand& cmd = commands.getCommandData<DataLinkCommand>(i);
                linkSceneData(cmd.providerScene, cmd.providerData, cmd.consumerScene, cmd.consumerData);
            }
            break;
            case ERendererCommand_LinkBufferToSceneData:
            {
                const DataLinkCommand& cmd = commands.getCommandData<DataLinkCommand>(i);
                linkBufferToSceneData(cmd.providerBuffer, cmd.consumerScene, cmd.consumerData);
            }
            break;
            case ERendererCommand_UnlinkSceneData:
            {
                const DataLinkCommand& cmd = commands.getCommandData<DataLinkCommand>(i);
                unlinkSceneData(cmd.consumerScene, cmd.consumerData);
            }
            break;
            case ERendererCommand_CreateOffscreenBuffer:
            {
                const OffscreenBufferCommand& cmd = commands.getCommandData<OffscreenBufferCommand>(i);
                createOffscreenBuffer(cmd.bufferHandle, cmd.displayHandle, cmd.bufferWidth, cmd.bufferHeight, cmd.interruptible);
            }
            break;
            case ERendererCommand_DestroyOffscreenBuffer:
            {
                const OffscreenBufferCommand& cmd = commands.getCommandData<OffscreenBufferCommand>(i);
                destroyOffscreenBuffer(cmd.bufferHandle, cmd.displayHandle);
            }
            break;
            case ERendererCommand_AssignSceneToOffscreenBuffer:
            {
                const OffscreenBufferCommand& cmd = commands.getCommandData<OffscreenBufferCommand>(i);
                assignSceneToOffscreenBuffer(cmd.assignedScene, cmd.bufferHandle);
            }
            break;
            case ERendererCommand_AssignSceneToFramebuffer:
            {
                const OffscreenBufferCommand& cmd = commands.getCommandData<OffscreenBufferCommand>(i);
                assignSceneToFramebuffer(cmd.assignedScene);
            }
            break;
            case ERendererCommand_LogRendererStatistics:
            {
                logStatistics();
            }
            break;
            case ERendererCommand_LogRendererInfo:
            {
                const LogCommand& cmd = commands.getCommandData<LogCommand>(i);
                logRendererInfo(cmd.topic, cmd.verbose, cmd.nodeHandleFilter);
            }
            break;
            case ERendererCommand_SystemCompositorControllerListIviSurfaces:
            {
                systemCompositorControllerListIviSurfaces();
            }
            break;
            case ERendererCommand_SystemCompositorControllerSetIviSurfaceVisibility:
            {
                const CompositorCommand& cmd = commands.getCommandData<CompositorCommand>(i);
                systemCompositorControllerSetIviSurfaceVisibility(cmd.waylandIviSurfaceId, cmd.visibility);
            }
            break;
            case ERendererCommand_SystemCompositorControllerSetIviSurfaceOpacity:
            {
                const CompositorCommand& cmd = commands.getCommandData<CompositorCommand>(i);
                systemCompositorControllerSetIviSurfaceOpacity(cmd.waylandIviSurfaceId, cmd.opacity);
            }
            break;
            case ERendererCommand_SystemCompositorControllerSetIviSurfaceDestRectangle:
            {
                const CompositorCommand& cmd = commands.getCommandData<CompositorCommand>(i);
                systemCompositorControllerSetIviSurfaceDestRectangle(cmd.waylandIviSurfaceId,cmd.x, cmd.y, cmd.width, cmd.height);
            }
            break;
            case ERendererCommand_SystemCompositorControllerSetIviLayerVisibility:
            {
                const CompositorCommand& cmd = commands.getCommandData<CompositorCommand>(i);
                systemCompositorControllerSetIviLayerVisibility(cmd.waylandIviLayerId, cmd.visibility);
            }
            break;
            case ERendererCommand_SystemCompositorControllerScreenshot:
            {
                const CompositorCommand& cmd = commands.getCommandData<CompositorCommand>(i);
                systemCompositorControllerScreenshot(cmd.fileName, cmd.screenIviId);
            }
            break;
            case ERendererCommand_SystemCompositorControllerAddIviSurfaceToIviLayer:
            {
                const CompositorCommand& cmd = commands.getCommandData<CompositorCommand>(i);
                systemCompositorControllerAddIviSurfaceToIviLayer(cmd.waylandIviSurfaceId, cmd.waylandIviLayerId);
            }
            break;
            case ERendererCommand_SystemCompositorControllerRemoveIviSurfaceFromIviLayer:
            {
                const CompositorCommand& cmd = commands.getCommandData<CompositorCommand>(i);
                systemCompositorControllerRemoveIviSurfaceFromIviLayer(cmd.waylandIviSurfaceId, cmd.waylandIviLayerId);
            }
            break;
            case ERendererCommand_SystemCompositorControllerDestroyIviSurface:
            {
                const CompositorCommand& cmd = commands.getCommandData<CompositorCommand>(i);
                systemCompositorControllerDestroyIviSurface(cmd.waylandIviSurfaceId);
            }
            break;
            case ERendererCommand_ConfirmationEcho:
            {
                const ConfirmationEchoCommand& cmd = commands.getCommandData<ConfirmationEchoCommand>(i);
                confirmationEcho(cmd.text);
            }
            break;
            case ERendererCommand_FrameProfiler_Toggle:
            {
                const auto& cmd = commands.getCommandData<UpdateFrameProfilerCommand>(i);
                toggleFrameProfilerVisibility(cmd.toggleVisibility);
            }
            break;
            case ERendererCommand_FrameProfiler_TimingGraphHeight:
            {
                const auto& cmd = commands.getCommandData<UpdateFrameProfilerCommand>(i);
                setFrameProfilerTimingGraphHeight(cmd.newTimingGraphHeight);
            }
            break;
            case ERendererCommand_FrameProfiler_CounterGraphHeight:
            {
                const auto& cmd = commands.getCommandData<UpdateFrameProfilerCommand>(i);
                setFrameProfilerCounterGraphHeight(cmd.newCounterGraphHeight);
            }
            break;
            case ERendererCommand_FrameProfiler_RegionFilterFlags:
            {
                const auto& cmd = commands.getCommandData<UpdateFrameProfilerCommand>(i);
                setFrameProfilerFilteredRegionFlags(cmd.newRegionFilterFlags);
            }
            break;
            case ERendererCommand_SetClearColor:
            {
                const auto& cmd = commands.getCommandData<SetClearColorCommand>(i);
                setClearColor(cmd.displayHandle, cmd.clearColor);
            }
            break;
            case ERendererCommand_SetFrameTimerLimits:
            {
                const auto& cmd = commands.getCommandData<SetFrameTimerLimitsCommmand>(i);
                setFrameTimerLimits(cmd.limitForSceneResourcesUploadMicrosec, cmd.limitForClientResourcesUploadMicrosec, cmd.limitForSceneActionsApplyMicrosec, cmd.limitForOffscreenBufferRenderMicrosec);
            }
            break;
            case ERendererCommand_SetLimits_FlushesForceApply:
            {
                const auto& cmd = commands.getCommandData<SetFrameTimerLimitsCommmand>(i);
                setLimitsFlushesForceApply(cmd.limitForPendingFlushesForceApply);
            }
            break;
            case ERendererCommand_SetLimits_FlushesForceUnsubscribe:
            {
                const auto& cmd = commands.getCommandData<SetFrameTimerLimitsCommmand>(i);
                setLimitsFlushesForceUnsubscribe(cmd.limitForPendingFlushesForceUnsubscribe);
            }
            break;
            case ERendererCommand_SetSkippingOfUnmodifiedBuffers:
            {
                const auto& cmd = commands.getCommandData<SetFeatureCommand>(i);
                setSkippingOfUnmodifiedBuffers(cmd.enable);
            }
            break;
            default:
            {
                assert(false);
            }
            break;
            }
        }
    }

}
