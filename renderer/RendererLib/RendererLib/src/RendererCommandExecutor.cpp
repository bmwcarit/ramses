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
#include "RendererLib/IRendererSceneUpdater.h"
#include "RendererLib/SceneLinksManager.h"
#include "RendererLib/RendererSceneControlLogic.h"
#include "RendererLib/FrameTimer.h"
#include "RendererLib/RendererSceneUpdater.h"
#include "RendererLib/RendererCommandUtils.h"
#include "RendererAPI/IDisplayController.h"
#include "RendererEventCollector.h"
#include "Utils/Image.h"
#include "Utils/ThreadLocalLogForced.h"

namespace ramses_internal
{
    RendererCommandExecutor::RendererCommandExecutor(Renderer& renderer, RendererCommandBuffer& rendererCommandBuffer, IRendererSceneUpdater& sceneUpdater, IRendererSceneControlLogic& sceneControlLogic, RendererEventCollector& rendererEventCollector, FrameTimer& frameTimer)
        : m_renderer(renderer)
        , m_sceneUpdater(sceneUpdater)
        , m_sceneControlLogic(sceneControlLogic)
        , m_rendererCommandBuffer(rendererCommandBuffer)
        , m_rendererEventCollector(rendererEventCollector)
        , m_frameTimer(frameTimer)
    {
    }

    void RendererCommandExecutor::executePendingCommands()
    {
        FRAME_PROFILER_REGION(FrameProfilerStatistics::ERegion::ExecuteRendererCommands);

        m_tmpCommands.clear();
        m_rendererCommandBuffer.swapCommands(m_tmpCommands);

        const auto numCommandsToLog = std::count_if(m_tmpCommands.cbegin(), m_tmpCommands.cend(), [](const auto& cmd) {
            return !absl::holds_alternative<RendererCommand::UpdateScene>(cmd) && !absl::holds_alternative<RendererCommand::LogInfo>(cmd);
        });
        if (numCommandsToLog > 0)
            LOG_INFO_P(CONTEXT_RENDERER, "RendererCommandExecutor executing {} commands, {} commands will be logged, rest is flush/sceneupdate commands", m_tmpCommands.size(), numCommandsToLog);

        for (auto& cmd : m_tmpCommands)
            absl::visit(*this, cmd);
    }

    void RendererCommandExecutor::operator()(const RendererCommand::ScenePublished& cmd)
    {
        LOG_INFO(CONTEXT_RENDERER, " - executing " << RendererCommandUtils::ToString(cmd));
        m_sceneUpdater.handleScenePublished(cmd.scene, cmd.publicationMode);
    }

    void RendererCommandExecutor::operator()(const RendererCommand::SceneUnpublished& cmd)
    {
        LOG_INFO(CONTEXT_RENDERER, " - executing " << RendererCommandUtils::ToString(cmd));
        m_sceneUpdater.handleSceneUnpublished(cmd.scene);
    }

    void RendererCommandExecutor::operator()(const RendererCommand::ReceiveScene& cmd)
    {
        LOG_INFO(CONTEXT_RENDERER, " - executing " << RendererCommandUtils::ToString(cmd));
        m_sceneUpdater.handleSceneReceived(cmd.info);
    }

    void RendererCommandExecutor::operator()(RendererCommand::UpdateScene& cmd)
    {
        // log debug only to reduce spam
        LOG_DEBUG(CONTEXT_RENDERER, " - executing " << RendererCommandUtils::ToString(cmd));
        m_sceneUpdater.handleSceneUpdate(cmd.scene, std::move(cmd.updateData));
    }

    void RendererCommandExecutor::operator()(const RendererCommand::SetSceneState& cmd)
    {
        LOG_INFO(CONTEXT_RENDERER, " - executing " << RendererCommandUtils::ToString(cmd));
        m_sceneControlLogic.setSceneState(cmd.scene, cmd.state);
    }

    void RendererCommandExecutor::operator()(const RendererCommand::SetSceneMapping&)
    {
        // command sets scene ownership on dispatcher level, noop here
    }

    void RendererCommandExecutor::operator()(const RendererCommand::SetSceneDisplayBufferAssignment& cmd)
    {
        LOG_INFO(CONTEXT_RENDERER, " - executing " << RendererCommandUtils::ToString(cmd));
        m_sceneControlLogic.setSceneDisplayBufferAssignment(cmd.scene, cmd.buffer, cmd.renderOrder);
    }

    void RendererCommandExecutor::operator()(const RendererCommand::LinkData& cmd)
    {
        LOG_INFO(CONTEXT_RENDERER, " - executing " << RendererCommandUtils::ToString(cmd));
        m_sceneUpdater.handleSceneDataLinkRequest(cmd.providerScene, cmd.providerData, cmd.consumerScene, cmd.consumerData);
    }

    void RendererCommandExecutor::operator()(const RendererCommand::LinkOffscreenBuffer& cmd)
    {
        LOG_INFO(CONTEXT_RENDERER, " - executing " << RendererCommandUtils::ToString(cmd));
        m_sceneUpdater.handleBufferToSceneDataLinkRequest(cmd.providerBuffer, cmd.consumerScene, cmd.consumerData);
    }

    void RendererCommandExecutor::operator()(const RendererCommand::LinkStreamBuffer& cmd)
    {
        LOG_INFO(CONTEXT_RENDERER, " - executing " << RendererCommandUtils::ToString(cmd));
        m_sceneUpdater.handleBufferToSceneDataLinkRequest(cmd.providerBuffer, cmd.consumerScene, cmd.consumerData);
    }

    void RendererCommandExecutor::operator()(const RendererCommand::UnlinkData& cmd)
    {
        LOG_INFO(CONTEXT_RENDERER, " - executing " << RendererCommandUtils::ToString(cmd));
        m_sceneUpdater.handleDataUnlinkRequest(cmd.consumerScene, cmd.consumerData);
    }

    void RendererCommandExecutor::operator()(const RendererCommand::PickEvent& cmd)
    {
        LOG_INFO(CONTEXT_RENDERER, " - executing " << RendererCommandUtils::ToString(cmd));
        m_sceneUpdater.handlePickEvent(cmd.scene, cmd.coordsNormalizedToBufferSize);
    }

    void RendererCommandExecutor::operator()(const RendererCommand::CreateDisplay& cmd)
    {
        LOG_INFO(CONTEXT_RENDERER, " - executing " << RendererCommandUtils::ToString(cmd));
        m_sceneUpdater.createDisplayContext(cmd.config, cmd.binaryShaderCache);
    }

    void RendererCommandExecutor::operator()(const RendererCommand::DestroyDisplay& cmd)
    {
        LOG_INFO(CONTEXT_RENDERER, " - executing " << RendererCommandUtils::ToString(cmd));
        m_sceneUpdater.destroyDisplayContext();
    }

    void RendererCommandExecutor::operator()(const RendererCommand::CreateOffscreenBuffer& cmd)
    {
        LOG_INFO(CONTEXT_RENDERER, " - executing " << RendererCommandUtils::ToString(cmd));
        const bool succeeded = m_sceneUpdater.handleBufferCreateRequest(cmd.offscreenBuffer, cmd.width, cmd.height, cmd.sampleCount, cmd.interruptible, cmd.depthStencilBufferType);
        m_rendererEventCollector.addOBEvent((succeeded ? ERendererEventType::OffscreenBufferCreated : ERendererEventType::OffscreenBufferCreateFailed), cmd.offscreenBuffer, cmd.display);
    }

    void RendererCommandExecutor::operator()(const RendererCommand::DestroyOffscreenBuffer& cmd)
    {
        LOG_INFO(CONTEXT_RENDERER, " - executing " << RendererCommandUtils::ToString(cmd));
        const bool succeeded = m_sceneUpdater.handleBufferDestroyRequest(cmd.offscreenBuffer);
        m_rendererEventCollector.addOBEvent((succeeded ? ERendererEventType::OffscreenBufferDestroyed : ERendererEventType::OffscreenBufferDestroyFailed), cmd.offscreenBuffer, cmd.display);
    }

    void RendererCommandExecutor::operator()(const RendererCommand::CreateStreamBuffer& cmd)
    {
        LOG_INFO(CONTEXT_RENDERER, " - executing " << RendererCommandUtils::ToString(cmd));
        m_sceneUpdater.handleBufferCreateRequest(cmd.streamBuffer, cmd.source);
    }

    void RendererCommandExecutor::operator()(const RendererCommand::DestroyStreamBuffer& cmd)
    {
        LOG_INFO(CONTEXT_RENDERER, " - executing " << RendererCommandUtils::ToString(cmd));
        m_sceneUpdater.handleBufferDestroyRequest(cmd.streamBuffer);
    }

    void RendererCommandExecutor::operator()(const RendererCommand::SetStreamBufferState& cmd)
    {
        LOG_INFO(CONTEXT_RENDERER, " - executing " << RendererCommandUtils::ToString(cmd));
        m_sceneUpdater.setStreamBufferState(cmd.streamBuffer, cmd.newState);
    }

    void RendererCommandExecutor::operator()(const RendererCommand::SetClearFlags& cmd)
    {
        LOG_INFO(CONTEXT_RENDERER, " - executing " << RendererCommandUtils::ToString(cmd));
        m_sceneUpdater.handleSetClearFlags(cmd.offscreenBuffer, cmd.clearFlags);
    }

    void RendererCommandExecutor::operator()(const RendererCommand::SetClearColor& cmd)
    {
        LOG_INFO(CONTEXT_RENDERER, " - executing " << RendererCommandUtils::ToString(cmd));
        m_sceneUpdater.handleSetClearColor(cmd.offscreenBuffer, cmd.clearColor);
    }

    void RendererCommandExecutor::operator()(RendererCommand::UpdateWarpingData& cmd)
    {
        LOG_INFO(CONTEXT_RENDERER, " - executing " << RendererCommandUtils::ToString(cmd));
        if (m_renderer.hasDisplayController() && m_renderer.getDisplayController().isWarpingEnabled())
        {
            m_renderer.setWarpingMeshData(std::move(cmd.data));
            m_rendererEventCollector.addDisplayEvent(ERendererEventType::WarpingDataUpdated, cmd.display);
        }
        else
            m_rendererEventCollector.addDisplayEvent(ERendererEventType::WarpingDataUpdateFailed, cmd.display);
    }

    void RendererCommandExecutor::operator()(RendererCommand::ReadPixels& cmd)
    {
        LOG_INFO(CONTEXT_RENDERER, " - executing " << RendererCommandUtils::ToString(cmd));
        ScreenshotInfo screenshot;
        screenshot.rectangle = { cmd.offsetX, cmd.offsetY, cmd.width, cmd.height };
        screenshot.filename = std::move(cmd.filename);
        screenshot.sendViaDLT = cmd.sendViaDLT;
        screenshot.fullScreen = cmd.fullScreen;
        m_sceneUpdater.handleReadPixels(cmd.offscreenBuffer, std::move(screenshot));
    }

    void RendererCommandExecutor::operator()(const RendererCommand::SetSkippingOfUnmodifiedBuffers& cmd)
    {
        LOG_INFO(CONTEXT_RENDERER, " - executing " << RendererCommandUtils::ToString(cmd));
        m_sceneUpdater.setSkippingOfUnmodifiedScenes(cmd.enable);
    }

    void RendererCommandExecutor::operator()(const RendererCommand::LogStatistics& cmd)
    {
        LOG_INFO(CONTEXT_RENDERER, " - executing " << RendererCommandUtils::ToString(cmd));
        LOG_INFO_F(CONTEXT_RENDERER, ([&](StringOutputStream& sos) { m_renderer.getStatistics().writeStatsToStream(sos); }));
        LOG_INFO_F(CONTEXT_RENDERER, ([&](StringOutputStream& sos) { m_renderer.getProfilerStatistics().writeLongestFrameTimingsToStream(sos); }));
    }

    void RendererCommandExecutor::operator()(const RendererCommand::LogInfo& cmd)
    {
        LOG_INFO(CONTEXT_RENDERER, " - executing " << RendererCommandUtils::ToString(cmd));
        m_sceneUpdater.logRendererInfo(cmd.topic, cmd.verbose, cmd.nodeFilter);
    }

    void RendererCommandExecutor::operator()(const RendererCommand::SCListIviSurfaces& cmd)
    {
        LOG_INFO(CONTEXT_RENDERER, " - executing " << RendererCommandUtils::ToString(cmd));
        m_renderer.systemCompositorListIviSurfaces();
    }

    void RendererCommandExecutor::operator()(const RendererCommand::SCSetIviSurfaceVisibility& cmd)
    {
        LOG_INFO(CONTEXT_RENDERER, " - executing " << RendererCommandUtils::ToString(cmd));
        m_renderer.systemCompositorSetIviSurfaceVisibility(cmd.surface, cmd.visibility);
    }

    void RendererCommandExecutor::operator()(const RendererCommand::SCSetIviSurfaceOpacity& cmd)
    {
        LOG_INFO(CONTEXT_RENDERER, " - executing " << RendererCommandUtils::ToString(cmd));
        m_renderer.systemCompositorSetIviSurfaceOpacity(cmd.surface, cmd.opacity);
    }

    void RendererCommandExecutor::operator()(const RendererCommand::SCSetIviSurfaceDestRectangle& cmd)
    {
        LOG_INFO(CONTEXT_RENDERER, " - executing " << RendererCommandUtils::ToString(cmd));
        m_renderer.systemCompositorSetIviSurfaceDestRectangle(cmd.surface, cmd.x, cmd.y, cmd.width, cmd.height);
    }

    void RendererCommandExecutor::operator()(const RendererCommand::SCScreenshot& cmd)
    {
        LOG_INFO(CONTEXT_RENDERER, " - executing " << RendererCommandUtils::ToString(cmd));
        m_renderer.systemCompositorScreenshot(cmd.filename, cmd.screenId);
    }

    void RendererCommandExecutor::operator()(const RendererCommand::SCAddIviSurfaceToIviLayer& cmd)
    {
        LOG_INFO(CONTEXT_RENDERER, " - executing " << RendererCommandUtils::ToString(cmd));
        m_renderer.systemCompositorAddIviSurfaceToIviLayer(cmd.surface, cmd.layer);
    }

    void RendererCommandExecutor::operator()(const RendererCommand::SCSetIviLayerVisibility& cmd)
    {
        LOG_INFO(CONTEXT_RENDERER, " - executing " << RendererCommandUtils::ToString(cmd));
        m_renderer.systemCompositorSetIviLayerVisibility(cmd.layer, cmd.visibility);
    }

    void RendererCommandExecutor::operator()(const RendererCommand::SCRemoveIviSurfaceFromIviLayer& cmd)
    {
        LOG_INFO(CONTEXT_RENDERER, " - executing " << RendererCommandUtils::ToString(cmd));
        m_renderer.systemCompositorRemoveIviSurfaceFromIviLayer(cmd.surface, cmd.layer);
    }

    void RendererCommandExecutor::operator()(const RendererCommand::SCDestroyIviSurface& cmd)
    {
        LOG_INFO(CONTEXT_RENDERER, " - executing " << RendererCommandUtils::ToString(cmd));
        m_renderer.systemCompositorDestroyIviSurface(cmd.surface);
    }

    void RendererCommandExecutor::operator()(const RendererCommand::SetLimits_FrameBudgets& cmd)
    {
        LOG_INFO(CONTEXT_RENDERER, " - executing " << RendererCommandUtils::ToString(cmd));
        m_frameTimer.setSectionTimeBudget(EFrameTimerSectionBudget::SceneResourcesUpload, cmd.limitForSceneResourcesUploadMicrosec);
        m_frameTimer.setSectionTimeBudget(EFrameTimerSectionBudget::ResourcesUpload, cmd.limitForResourcesUploadMicrosec);
        m_frameTimer.setSectionTimeBudget(EFrameTimerSectionBudget::OffscreenBufferRender, cmd.limitForOffscreenBufferRenderMicrosec);
    }

    void RendererCommandExecutor::operator()(const RendererCommand::SetLimits_FlushesForceApply& cmd)
    {
        LOG_INFO(CONTEXT_RENDERER, " - executing " << RendererCommandUtils::ToString(cmd));
        m_sceneUpdater.setLimitFlushesForceApply(cmd.limitForPendingFlushesForceApply);
    }

    void RendererCommandExecutor::operator()(const RendererCommand::SetLimits_FlushesForceUnsubscribe& cmd)
    {
        LOG_INFO(CONTEXT_RENDERER, " - executing " << RendererCommandUtils::ToString(cmd));
        m_sceneUpdater.setLimitFlushesForceUnsubscribe(cmd.limitForPendingFlushesForceUnsubscribe);
    }

    void RendererCommandExecutor::operator()(const RendererCommand::FrameProfiler_Toggle& cmd)
    {
        LOG_INFO(CONTEXT_RENDERER, " - executing " << RendererCommandUtils::ToString(cmd));
        if (m_renderer.hasDisplayController())
            m_renderer.getFrameProfileRenderer().enable(cmd.toggle ? !m_renderer.getFrameProfileRenderer().isEnabled() : true);
    }

    void RendererCommandExecutor::operator()(const RendererCommand::FrameProfiler_TimingGraphHeight& cmd)
    {
        LOG_INFO(CONTEXT_RENDERER, " - executing " << RendererCommandUtils::ToString(cmd));
        if (m_renderer.hasDisplayController())
            m_renderer.getFrameProfileRenderer().setTimingGraphHeight(cmd.height);
    }

    void RendererCommandExecutor::operator()(const RendererCommand::FrameProfiler_CounterGraphHeight& cmd)
    {
        LOG_INFO(CONTEXT_RENDERER, " - executing " << RendererCommandUtils::ToString(cmd));
        if (m_renderer.hasDisplayController())
            m_renderer.getFrameProfileRenderer().setCounterGraphHeight(cmd.height);
    }

    void RendererCommandExecutor::operator()(const RendererCommand::FrameProfiler_RegionFilterFlags& cmd)
    {
        LOG_INFO(CONTEXT_RENDERER, " - executing " << RendererCommandUtils::ToString(cmd));
        m_renderer.getProfilerStatistics().setFilteredRegionFlags(cmd.flags);
    }

    void RendererCommandExecutor::operator()(const RendererCommand::ConfirmationEcho& cmd)
    {
        LOG_INFO(CONTEXT_RENDERER, " - executing " << RendererCommandUtils::ToString(cmd));
        LOG_INFO(CONTEXT_RAMSH, "confirmation: " << cmd.text);
    }
}
