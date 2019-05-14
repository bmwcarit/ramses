//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/RendererCommandExecutor.h"
#include "RendererLib/RendererCommandBuffer.h"
#include "RendererLib/Renderer.h"
#include "RendererLib/RendererSceneUpdater.h"
#include "RendererLib/SceneLinksManager.h"
#include "RendererAPI/IDisplayController.h"
#include "RendererEventCollector.h"
#include "Utils/Image.h"
#include "Utils/LogMacros.h"
#include "RendererLib/FrameTimer.h"

namespace ramses_internal
{
    RendererCommandExecutor::RendererCommandExecutor(Renderer& renderer, RendererCommandBuffer& rendererCommandBuffer, RendererSceneUpdater& rendererSceneUpdater, RendererEventCollector& rendererEventCollector, FrameTimer& frameTimer)
        : m_renderer(renderer)
        , m_rendererSceneUpdater(rendererSceneUpdater)
        , m_rendererCommandBuffer(rendererCommandBuffer)
        , m_rendererEventCollector(rendererEventCollector)
        , m_frameTimer(frameTimer)
    {
    }

    void RendererCommandExecutor::executePendingCommands()
    {
        FRAME_PROFILER_REGION(FrameProfilerStatistics::ERegion::RendererCommands);

        LOG_TRACE(CONTEXT_PROFILING, "  RendererCommandExecutor::executePendingCommands swap out commands");

        m_executedCommands.clear();
        m_rendererCommandBuffer.lock();
        m_rendererCommandBuffer.swapCommandContainer(m_executedCommands);
        m_rendererCommandBuffer.unlock();

        LOG_TRACE(CONTEXT_PROFILING, "  RendererCommandExecutor::executePendingCommands start executing commands");
        const auto numCommands = m_executedCommands.getTotalCommandCount();

        // log commands only if there are other than 'scene actions' commands to minimize log spam
        UInt32 numUnloggedCmds = 0u;
        for (UInt32 i = 0; i < numCommands; ++i)
            numUnloggedCmds += (m_executedCommands.getCommandType(i) == ERendererCommand_SceneActions ||
                                m_executedCommands.getCommandType(i) == ERendererCommand_LogRendererInfo) ?
                1 : 0;
        if (numCommands > numUnloggedCmds)
            LOG_INFO(CONTEXT_RENDERER, "RendererCommandExecutor executing " << numCommands - numUnloggedCmds << " renderer commands:");

        for (UInt32 i = 0; i < numCommands; ++i)
        {
            const ERendererCommand commandType = m_executedCommands.getCommandType(i);
            LOG_TRACE(CONTEXT_PROFILING, "    RendererCommandExecutor::executePendingCommands executing command of type " << static_cast<UInt32>(commandType));
            switch (commandType)
            {
            case ERendererCommand_PublishedScene:
            {
                const SceneInfoCommand& command = m_executedCommands.getCommandData<SceneInfoCommand>(i);
                LOG_INFO(CONTEXT_RENDERER, " - executing " << EnumToString(commandType) << " sceneId " << command.sceneInformation.sceneID);
                m_rendererSceneUpdater.handleScenePublished(command.sceneInformation.sceneID, command.clientID, command.sceneInformation.publicationMode);
                break;
            }
            case ERendererCommand_UnpublishedScene:
            {
                const SceneInfoCommand& command = m_executedCommands.getCommandData<SceneInfoCommand>(i);
                LOG_INFO(CONTEXT_RENDERER, " - executing " << EnumToString(commandType) << " sceneId " << command.sceneInformation.sceneID);
                m_rendererSceneUpdater.handleSceneUnpublished(command.sceneInformation.sceneID);
                break;
            }
            case ERendererCommand_ReceivedScene:
            {
                const SceneInfoCommand& command = m_executedCommands.getCommandData<SceneInfoCommand>(i);
                LOG_INFO(CONTEXT_RENDERER, " - executing " << EnumToString(commandType) << " sceneId " << command.sceneInformation.sceneID);
                m_rendererSceneUpdater.handleSceneReceived(command.sceneInformation);
                break;
            }
            case ERendererCommand_SubscribeScene:
            {
                const SceneInfoCommand& command = m_executedCommands.getCommandData<SceneInfoCommand>(i);
                const SceneId sceneId = command.sceneInformation.sceneID;
                LOG_INFO(CONTEXT_RENDERER, " - executing " << EnumToString(commandType) << " sceneId " << sceneId);
                m_rendererSceneUpdater.handleSceneSubscriptionRequest(sceneId);
                break;
            }
            case ERendererCommand_UnsubscribeScene:
            {
                const SceneInfoCommand& command = m_executedCommands.getCommandData<SceneInfoCommand>(i);
                const SceneId sceneId = command.sceneInformation.sceneID;
                const bool indirect = command.indirect;
                LOG_INFO(CONTEXT_RENDERER, " - executing " << EnumToString(commandType) << " sceneId " << sceneId << " indirect " << indirect);
                m_rendererSceneUpdater.handleSceneUnsubscriptionRequest(sceneId, indirect);
                break;
            }
            case ERendererCommand_CreateDisplay:
            {
                const DisplayCommand& command = m_executedCommands.getCommandData<DisplayCommand>(i);
                const DisplayHandle displayHandle = command.displayHandle;
                LOG_INFO(CONTEXT_RENDERER, " - executing " << EnumToString(commandType) << " displayId " << displayHandle);
                m_rendererSceneUpdater.createDisplayContext(command.displayConfig, *command.resourceProvider, *command.resourceUploader, displayHandle);
                break;
            }
            case ERendererCommand_DestroyDisplay:
            {
                const DisplayCommand& command = m_executedCommands.getCommandData<DisplayCommand>(i);
                LOG_INFO(CONTEXT_RENDERER, " - executing " << EnumToString(commandType) << " displayId " << command.displayHandle);
                m_rendererSceneUpdater.destroyDisplayContext(command.displayHandle);
                break;
            }
            case ERendererCommand_MapSceneToDisplay:
            {
                const SceneMappingCommand& command = m_executedCommands.getCommandData<SceneMappingCommand>(i);
                LOG_INFO(CONTEXT_RENDERER, " - executing " << EnumToString(commandType) << " sceneId " << command.sceneId << " displayId " << command.displayHandle);
                m_rendererSceneUpdater.handleSceneMappingRequest(command.sceneId, command.displayHandle, command.sceneRenderOrder);
                break;
            }
            case ERendererCommand_UnmapSceneFromDisplays:
            {
                const SceneMappingCommand& command = m_executedCommands.getCommandData<SceneMappingCommand>(i);
                LOG_INFO(CONTEXT_RENDERER, " - executing " << EnumToString(commandType) << " sceneId " << command.sceneId);
                m_rendererSceneUpdater.handleSceneUnmappingRequest(command.sceneId);
                break;
            }
            case ERendererCommand_ShowScene:
            {
                const SceneRenderCommand& command = m_executedCommands.getCommandData<SceneRenderCommand>(i);
                LOG_INFO(CONTEXT_RENDERER, " - executing " << EnumToString(commandType) << " sceneId " << command.sceneId);
                m_rendererSceneUpdater.handleSceneShowRequest(command.sceneId);
                break;
            }
            case ERendererCommand_HideScene:
            {
                const SceneRenderCommand& command = m_executedCommands.getCommandData<SceneRenderCommand>(i);
                LOG_INFO(CONTEXT_RENDERER, " - executing " << EnumToString(commandType) << " sceneId " << command.sceneId);
                m_rendererSceneUpdater.handleSceneHideRequest(command.sceneId);
                break;
            }
            case ERendererCommand_UpdateWarpingData:
            {
                const WarpingDataCommand& command = m_executedCommands.getCommandData<WarpingDataCommand>(i);
                LOG_INFO(CONTEXT_RENDERER, " - executing " << EnumToString(commandType) << " displayId " << command.displayHandle);
                if (m_renderer.hasDisplayController(command.displayHandle) && m_renderer.getDisplayController(command.displayHandle).isWarpingEnabled())
                {
                    m_renderer.getDisplayController(command.displayHandle).enableContext();
                    m_renderer.setWarpingMeshData(command.displayHandle, command.warpingData);
                    m_rendererEventCollector.addEvent(ERendererEventType_WarpingDataUpdated, command.displayHandle);
                }
                else
                {
                    m_rendererEventCollector.addEvent(ERendererEventType_WarpingDataUpdateFailed, command.displayHandle);
                }
                break;
            }
            case ERendererCommand_ReadPixels:
            {
                const ReadPixelsCommand& command = m_executedCommands.getCommandData<ReadPixelsCommand>(i);
                LOG_INFO(CONTEXT_RENDERER, " - executing " << EnumToString(commandType) << " displayId " << command.displayHandle);
                if (m_renderer.hasDisplayController(command.displayHandle))
                {

                    ScreenshotInfo screenshot;
                    screenshot.rectangle = { command.x, command.y, command.width, command.height };
                    screenshot.display = command.displayHandle;
                    screenshot.filename = command.filename;
                    screenshot.sendViaDLT = command.sendViaDLT;

                    if (command.fullScreen)
                    {
                        const IDisplayController& displayController = m_renderer.getDisplayController(command.displayHandle);
                        screenshot.rectangle = { 0u, 0u, displayController.getDisplayWidth(), displayController.getDisplayHeight() };
                    }

                    m_renderer.scheduleScreenshot(screenshot);
                }
                else
                {
                    if (command.filename.getLength() == 0u)
                    {
                        // only generate event when not saving pixels to file!
                        m_rendererEventCollector.addEvent(ERendererEventType_ReadPixelsFromFramebufferFailed, command.displayHandle);
                    }
                    LOG_ERROR(CONTEXT_RENDERER, "RendererCommandExecutor::readPixels failed, unknown display " << command.displayHandle.asMemoryHandle());
                }

                break;
            }
            case ERendererCommand_SetClearColor:
            {
                const auto& command = m_executedCommands.getCommandData<SetClearColorCommand>(i);
                LOG_INFO(CONTEXT_RENDERER, " - executing " << EnumToString(commandType) << " displayId " << command.displayHandle);
                if (m_renderer.hasDisplayController(command.displayHandle))
                {
                    m_renderer.setClearColor(command.displayHandle, command.clearColor);
                }
                else
                {
                    LOG_ERROR(CONTEXT_RENDERER, "RendererCommandExecutor::setClearColor failed, unknown display " << command.displayHandle.asMemoryHandle());
                }
                break;
            }
            case ERendererCommand_RelativeTranslation:
            {
                const RendererViewCommand& command = m_executedCommands.getCommandData<RendererViewCommand>(i);
                LOG_INFO(CONTEXT_RENDERER, " - executing " << EnumToString(commandType));
                for (DisplayHandle handle(0u); handle < m_renderer.getDisplayControllerCount(); ++handle)
                {
                    if (m_renderer.hasDisplayController(handle))
                    {
                        IDisplayController& controller = m_renderer.getDisplayController(handle);
                        controller.setViewPosition(controller.getViewPosition() + command.displayMovement);
                    }
                }
                break;
            }
            case ERendererCommand_AbsoluteTranslation:
            {
                const RendererViewCommand& command = m_executedCommands.getCommandData<RendererViewCommand>(i);
                LOG_INFO(CONTEXT_RENDERER, " - executing " << EnumToString(commandType));
                for (DisplayHandle handle(0u); handle < m_renderer.getDisplayControllerCount(); ++handle)
                {
                    if (m_renderer.hasDisplayController(handle))
                    {
                        m_renderer.getDisplayController(handle).setViewPosition(command.displayMovement);
                    }
                }
                break;
            }
            case ERendererCommand_RelativeRotation:
            {
                const RendererViewCommand& command = m_executedCommands.getCommandData<RendererViewCommand>(i);
                LOG_INFO(CONTEXT_RENDERER, " - executing " << EnumToString(commandType));
                for (DisplayHandle handle(0u); handle < m_renderer.getDisplayControllerCount(); ++handle)
                {
                    if (m_renderer.hasDisplayController(handle))
                    {
                        IDisplayController& controller = m_renderer.getDisplayController(handle);
                        controller.setViewRotation(controller.getViewRotation() + command.displayMovement);
                    }
                }
                break;
            }
            case ERendererCommand_AbsoluteRotation:
            {
                const RendererViewCommand& command = m_executedCommands.getCommandData<RendererViewCommand>(i);
                LOG_INFO(CONTEXT_RENDERER, " - executing " << EnumToString(commandType));
                for (DisplayHandle handle(0u); handle < m_renderer.getDisplayControllerCount(); ++handle)
                {
                    if (m_renderer.hasDisplayController(handle))
                    {
                        m_renderer.getDisplayController(handle).setViewRotation(command.displayMovement);
                    }
                }
                break;
            }
            case ERendererCommand_LinkSceneData:
            {
                const DataLinkCommand& command = m_executedCommands.getCommandData<DataLinkCommand>(i);
                LOG_INFO(CONTEXT_RENDERER, " - executing " << EnumToString(commandType) << " providerScene " << command.providerScene << " providerData " << command.providerData << " consumerScene " << command.consumerScene << " consumerData " << command.consumerData);
                m_rendererSceneUpdater.handleSceneDataLinkRequest(command.providerScene, command.providerData, command.consumerScene, command.consumerData);
                break;
            }
            case ERendererCommand_LinkBufferToSceneData:
            {
                const DataLinkCommand& command = m_executedCommands.getCommandData<DataLinkCommand>(i);
                LOG_INFO(CONTEXT_RENDERER, " - executing " << EnumToString(commandType) << " providerBuffer " << command.providerBuffer << " consumerScene " << command.consumerScene << " consumerData " << command.consumerData);
                m_rendererSceneUpdater.handleBufferToSceneDataLinkRequest(command.providerBuffer, command.consumerScene, command.consumerData);
                break;
            }
            case ERendererCommand_UnlinkSceneData:
            {
                const DataLinkCommand& command = m_executedCommands.getCommandData<DataLinkCommand>(i);
                LOG_INFO(CONTEXT_RENDERER, " - executing " << EnumToString(commandType) << " consumerScene " << command.consumerScene << " consumerData " << command.consumerData);
                m_rendererSceneUpdater.handleDataUnlinkRequest(command.consumerScene, command.consumerData);
                break;
            }
            case ERendererCommand_CreateOffscreenBuffer:
            {
                const OffscreenBufferCommand& command = m_executedCommands.getCommandData<OffscreenBufferCommand>(i);
                LOG_INFO(CONTEXT_RENDERER, " - executing " << EnumToString(commandType) << " displayId " << command.displayHandle << " bufferHandle " << command.bufferHandle);
                const Bool succeeded = m_rendererSceneUpdater.handleBufferCreateRequest(command.bufferHandle, command.displayHandle, command.bufferWidth, command.bufferHeight, command.interruptible);
                m_rendererEventCollector.addEvent((succeeded ? ERendererEventType_OffscreenBufferCreated : ERendererEventType_OffscreenBufferCreateFailed), command.bufferHandle, command.displayHandle);
                break;
            }
            case ERendererCommand_DestroyOffscreenBuffer:
            {
                const OffscreenBufferCommand& command = m_executedCommands.getCommandData<OffscreenBufferCommand>(i);
                LOG_INFO(CONTEXT_RENDERER, " - executing " << EnumToString(commandType) << " displayId " << command.displayHandle << " bufferHandle " << command.bufferHandle);
                const Bool succeeded = m_rendererSceneUpdater.handleBufferDestroyRequest(command.bufferHandle, command.displayHandle);
                m_rendererEventCollector.addEvent((succeeded ? ERendererEventType_OffscreenBufferDestroyed : ERendererEventType_OffscreenBufferDestroyFailed), command.bufferHandle, command.displayHandle);
                break;
            }
            case ERendererCommand_AssignSceneToOffscreenBuffer:
            {
                const OffscreenBufferCommand& command = m_executedCommands.getCommandData<OffscreenBufferCommand>(i);
                LOG_INFO(CONTEXT_RENDERER, " - executing " << EnumToString(commandType) << " sceneId " << command.assignedScene << " bufferHandle " << command.bufferHandle);
                const Bool succeeded = m_rendererSceneUpdater.handleSceneOffscreenBufferAssignmentRequest(command.assignedScene, command.bufferHandle);
                m_rendererEventCollector.addEvent((succeeded ? ERendererEventType_SceneAssignedToOffscreenBuffer : ERendererEventType_SceneAssignedToOffscreenBufferFailed), command.bufferHandle, command.assignedScene);
                break;
            }
            case ERendererCommand_AssignSceneToFramebuffer:
            {
                const OffscreenBufferCommand& command = m_executedCommands.getCommandData<OffscreenBufferCommand>(i);
                LOG_INFO(CONTEXT_RENDERER, " - executing " << EnumToString(commandType) << " sceneId " << command.assignedScene);
                const Bool succeeded = m_rendererSceneUpdater.handleSceneFramebufferAssignmentRequest(command.assignedScene);
                m_rendererEventCollector.addEvent((succeeded ? ERendererEventType_SceneAssignedToFramebuffer : ERendererEventType_SceneAssignedToFramebufferFailed), command.assignedScene);
                break;
            }
            case ERendererCommand_LogRendererInfo:
            {
                const LogCommand& command = m_executedCommands.getCommandData<LogCommand>(i);
                RendererLogger::LogTopic(m_rendererSceneUpdater, command.topic, command.verbose, command.nodeHandleFilter);
                break;
            }
            case ERendererCommand_LogRendererStatistics:
            {
                LOG_INFO(CONTEXT_RENDERER, " - executing " << EnumToString(commandType));
                LOG_INFO_F(CONTEXT_RENDERER, ([&](StringOutputStream& sos) { m_renderer.getStatistics().writeStatsToStream(sos); }));
                LOG_INFO_F(CONTEXT_RENDERER, ([&](StringOutputStream& sos) { m_renderer.getProfilerStatistics().writeLongestFrameTimingsToStream(sos); }));
                break;
            }
            case ERendererCommand_ResetRenderView:
            {
                LOG_INFO(CONTEXT_RENDERER, " - executing " << EnumToString(commandType));
                for (DisplayHandle handle(0u); handle < m_renderer.getDisplayControllerCount(); ++handle)
                {
                    if (m_renderer.hasDisplayController(handle))
                    {
                        m_renderer.getDisplayController(handle).resetView();
                    }
                }
                break;
            }
            case ERendererCommand_SystemCompositorControllerListIviSurfaces:
            {
                LOG_INFO(CONTEXT_RENDERER, " - executing " << EnumToString(commandType));
                m_renderer.systemCompositorListIviSurfaces();
                break;
            }
            case ERendererCommand_SystemCompositorControllerSetIviSurfaceVisibility:
            {
                const CompositorCommand& command = m_executedCommands.getCommandData<CompositorCommand>(i);
                LOG_INFO(CONTEXT_RENDERER, " - executing " << EnumToString(commandType) << " surfaceId " << command.waylandIviSurfaceId);
                m_renderer.systemCompositorSetIviSurfaceVisibility(command.waylandIviSurfaceId, command.visibility);
                break;
            }
            case ERendererCommand_SystemCompositorControllerSetIviSurfaceOpacity:
            {
                const CompositorCommand& command = m_executedCommands.getCommandData<CompositorCommand>(i);
                LOG_INFO(CONTEXT_RENDERER, " - executing " << EnumToString(commandType) << " surfaceId " << command.waylandIviSurfaceId);
                m_renderer.systemCompositorSetIviSurfaceOpacity(command.waylandIviSurfaceId, command.opacity);
                break;
            }
            case ERendererCommand_SystemCompositorControllerSetIviSurfaceDestRectangle:
            {
                const CompositorCommand& command = m_executedCommands.getCommandData<CompositorCommand>(i);
                LOG_INFO(CONTEXT_RENDERER, " - executing " << EnumToString(commandType) << " surfaceId " << command.waylandIviSurfaceId);
                m_renderer.systemCompositorSetIviSurfaceDestRectangle(command.waylandIviSurfaceId, command.x, command.y, command.width, command.height);
                break;
            }
            case ERendererCommand_SystemCompositorControllerAddIviSurfaceToIviLayer:
            {
                const CompositorCommand& command = m_executedCommands.getCommandData<CompositorCommand>(i);
                LOG_INFO(CONTEXT_RENDERER, " - executing " << EnumToString(commandType) << " surfaceId " << command.waylandIviSurfaceId << " layerId " << command.waylandIviLayerId);
                m_renderer.systemCompositorAddIviSurfaceToIviLayer(command.waylandIviSurfaceId, command.waylandIviLayerId);
                break;
            }
            case ERendererCommand_SystemCompositorControllerSetIviLayerVisibility:
            {
                const CompositorCommand& command = m_executedCommands.getCommandData<CompositorCommand>(i);
                LOG_INFO(CONTEXT_RENDERER, " - executing " << EnumToString(commandType) << " layerId " << command.waylandIviLayerId);
                m_renderer.systemCompositorSetIviLayerVisibility(command.waylandIviLayerId, command.visibility);
                break;
            }
            case ERendererCommand_SystemCompositorControllerRemoveIviSurfaceFromIviLayer:
            {
                const CompositorCommand& command = m_executedCommands.getCommandData<CompositorCommand>(i);
                LOG_INFO(CONTEXT_RENDERER, " - executing " << EnumToString(commandType) << " surfaceId " << command.waylandIviSurfaceId << " layerId " << command.waylandIviLayerId);
                m_renderer.systemCompositorRemoveIviSurfaceFromIviLayer(command.waylandIviSurfaceId, command.waylandIviLayerId);
                break;
            }
            case ERendererCommand_SystemCompositorControllerDestroyIviSurface:
            {
                const CompositorCommand& command = m_executedCommands.getCommandData<CompositorCommand>(i);
                LOG_INFO(CONTEXT_RENDERER, " - executing " << EnumToString(commandType) << " surfaceId " << command.waylandIviSurfaceId);
                m_renderer.systemCompositorDestroyIviSurface(command.waylandIviSurfaceId);
                break;
            }
            case ERendererCommand_SystemCompositorControllerScreenshot:
            {
                LOG_INFO(CONTEXT_RENDERER, " - executing " << EnumToString(commandType));
                const CompositorCommand& command = m_executedCommands.getCommandData<CompositorCommand>(i);
                m_renderer.systemCompositorScreenshot(command.fileName, command.screenIviId);
                break;
            }
            case ERendererCommand_ConfirmationEcho:
            {
                LOG_INFO(CONTEXT_RENDERER, " - executing " << EnumToString(commandType));
                const ConfirmationEchoCommand& command = m_executedCommands.getCommandData<ConfirmationEchoCommand>(i);
                LOG_INFO(CONTEXT_RAMSH, "confirmation: " << command.text);
                break;
            }
            case ERendererCommand_FrameProfiler_Toggle:
            {
                LOG_INFO(CONTEXT_RENDERER, " - executing " << EnumToString(commandType));
                const auto& command = m_executedCommands.getCommandData<UpdateFrameProfilerCommand>(i);
                FrameProfileRenderer::ForAllFrameProfileRenderer(m_renderer,
                    [&](FrameProfileRenderer& renderer) { renderer.enable(command.toggleVisibility ? !renderer.isEnabled() : true); });
                break;
            }
            case ERendererCommand_FrameProfiler_TimingGraphHeight:
            {
                LOG_INFO(CONTEXT_RENDERER, " - executing " << EnumToString(commandType));
                const auto& command = m_executedCommands.getCommandData<UpdateFrameProfilerCommand>(i);
                FrameProfileRenderer::ForAllFrameProfileRenderer(m_renderer,
                    [&](FrameProfileRenderer& renderer) { renderer.setTimingGraphHeight(command.newTimingGraphHeight); });
                break;
            }
            case ERendererCommand_FrameProfiler_CounterGraphHeight:
            {
                LOG_INFO(CONTEXT_RENDERER, " - executing " << EnumToString(commandType));
                const auto& command = m_executedCommands.getCommandData<UpdateFrameProfilerCommand>(i);
                FrameProfileRenderer::ForAllFrameProfileRenderer(m_renderer,
                    [&](FrameProfileRenderer& renderer) { renderer.setCounterGraphHeight(command.newCounterGraphHeight); });
                break;
            }
            case ERendererCommand_FrameProfiler_RegionFilterFlags:
            {
                LOG_INFO(CONTEXT_RENDERER, " - executing " << EnumToString(commandType));
                const auto& command = m_executedCommands.getCommandData<UpdateFrameProfilerCommand>(i);
                m_renderer.getProfilerStatistics().setFilteredRegionFlags(command.newRegionFilterFlags);
                break;
            }
            case ERendererCommand_SceneActions:
            {
                SceneActionsCommand& command = m_executedCommands.getCommandData<SceneActionsCommand>(i);
                const SceneId sceneId = command.sceneId;
                SceneActionCollection& actionsForScene = command.sceneActions;
                m_rendererSceneUpdater.handleSceneActions(sceneId, actionsForScene);
                break;
            }
            case ERendererCommand_SetFrameTimerLimits:
            {
                const SetFrameTimerLimitsCommmand& command = m_executedCommands.getCommandData<SetFrameTimerLimitsCommmand>(i);
                LOG_INFO(CONTEXT_RENDERER, " - executing " << EnumToString(commandType) << " sceneResUpload " << command.limitForSceneResourcesUploadMicrosec << " clientResUpload " << command.limitForClientResourcesUploadMicrosec << " actionApply " << command.limitForSceneActionsApplyMicrosec << " render " << command.limitForOffscreenBufferRenderMicrosec);
                m_frameTimer.setSectionTimeBudget(EFrameTimerSectionBudget::SceneResourcesUpload, command.limitForSceneResourcesUploadMicrosec);
                m_frameTimer.setSectionTimeBudget(EFrameTimerSectionBudget::ClientResourcesUpload, command.limitForClientResourcesUploadMicrosec);
                m_frameTimer.setSectionTimeBudget(EFrameTimerSectionBudget::SceneActionsApply, command.limitForSceneActionsApplyMicrosec);
                m_frameTimer.setSectionTimeBudget(EFrameTimerSectionBudget::OffscreenBufferRender, command.limitForOffscreenBufferRenderMicrosec);
                break;
            }
            case ERendererCommand_SetLimits_FlushesForceApply:
            {
                const SetFrameTimerLimitsCommmand& command = m_executedCommands.getCommandData<SetFrameTimerLimitsCommmand>(i);
                LOG_INFO(CONTEXT_RENDERER, " - executing " << EnumToString(commandType) << " max flushes before force apply: " << command.limitForPendingFlushesForceApply);
                m_rendererSceneUpdater.setLimitFlushesForceApply(command.limitForPendingFlushesForceApply);
                break;
            }
            case ERendererCommand_SetLimits_FlushesForceUnsubscribe:
            {
                const SetFrameTimerLimitsCommmand& command = m_executedCommands.getCommandData<SetFrameTimerLimitsCommmand>(i);
                LOG_INFO(CONTEXT_RENDERER, " - executing " << EnumToString(commandType) << " max flushes before force unsubscribe: " << command.limitForPendingFlushesForceUnsubscribe);
                m_rendererSceneUpdater.setLimitFlushesForceUnsubscribe(command.limitForPendingFlushesForceUnsubscribe);
                break;
            }
            case ERendererCommand_SetSkippingOfUnmodifiedBuffers:
            {
                const SetFeatureCommand& command = m_executedCommands.getCommandData<SetFeatureCommand>(i);
                LOG_INFO(CONTEXT_RENDERER, " - executing " << EnumToString(commandType) << " enable=" << command.enable);
                m_renderer.setSkippingOfUnmodifiedBuffers(command.enable);
                break;
            }
            default:
                LOG_ERROR(CONTEXT_RENDERER, "RendererCommandExecutor::executePendingCommands failed, unknown renderer command type!");
                assert(false);
                break;
            }
        }

        LOG_TRACE(CONTEXT_PROFILING, "  RendererCommandExecutor::executePendingCommands finished executing commands");
    }

}
