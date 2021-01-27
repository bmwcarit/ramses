//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERCOMMANDEXECUTOR_H
#define RAMSES_RENDERERCOMMANDEXECUTOR_H

#include "SceneAPI/SceneId.h"
#include "RendererLib/RendererCommands.h"

namespace ramses_internal
{
    class Renderer;
    class RendererCommandBuffer;
    class RendererEventCollector;
    class IRendererSceneUpdater;
    class IRendererSceneControlLogic;
    class FrameTimer;

    class RendererCommandExecutor
    {
    public:
        RendererCommandExecutor(Renderer& renderer, RendererCommandBuffer& rendererCommandBuffer, IRendererSceneUpdater& sceneUpdater, IRendererSceneControlLogic& sceneControlLogic, RendererEventCollector& rendererEventCollector, FrameTimer& frameTimer);

        void executePendingCommands();

        void operator()(const RendererCommand::ScenePublished& cmd);
        void operator()(const RendererCommand::SceneUnpublished& cmd);
        void operator()(const RendererCommand::ReceiveScene& cmd);
        void operator()(RendererCommand::UpdateScene& cmd);
        void operator()(const RendererCommand::SetSceneState& cmd);
        void operator()(const RendererCommand::SetSceneMapping& cmd);
        void operator()(const RendererCommand::SetSceneDisplayBufferAssignment& cmd);
        void operator()(const RendererCommand::LinkData& cmd);
        void operator()(const RendererCommand::LinkOffscreenBuffer& cmd);
        void operator()(const RendererCommand::LinkStreamBuffer& cmd);
        void operator()(const RendererCommand::UnlinkData& cmd);
        void operator()(const RendererCommand::PickEvent& cmd);
        void operator()(const RendererCommand::CreateDisplay& cmd);
        void operator()(const RendererCommand::DestroyDisplay& cmd);
        void operator()(const RendererCommand::CreateOffscreenBuffer& cmd);
        void operator()(const RendererCommand::DestroyOffscreenBuffer& cmd);
        void operator()(const RendererCommand::CreateStreamBuffer& cmd);
        void operator()(const RendererCommand::DestroyStreamBuffer& cmd);
        void operator()(const RendererCommand::SetStreamBufferState& cmd);
        void operator()(const RendererCommand::SetClearColor& cmd);
        void operator()(RendererCommand::UpdateWarpingData& cmd);
        void operator()(RendererCommand::ReadPixels& cmd);
        void operator()(const RendererCommand::SetSkippingOfUnmodifiedBuffers& cmd);
        void operator()(const RendererCommand::LogStatistics& cmd);
        void operator()(const RendererCommand::LogInfo& cmd);
        void operator()(const RendererCommand::SCListIviSurfaces& cmd);
        void operator()(const RendererCommand::SCSetIviSurfaceVisibility& cmd);
        void operator()(const RendererCommand::SCSetIviSurfaceOpacity& cmd);
        void operator()(const RendererCommand::SCSetIviSurfaceDestRectangle& cmd);
        void operator()(const RendererCommand::SCScreenshot& cmd);
        void operator()(const RendererCommand::SCAddIviSurfaceToIviLayer& cmd);
        void operator()(const RendererCommand::SCSetIviLayerVisibility& cmd);
        void operator()(const RendererCommand::SCRemoveIviSurfaceFromIviLayer& cmd);
        void operator()(const RendererCommand::SCDestroyIviSurface& cmd);
        void operator()(const RendererCommand::SetLimits_FrameBudgets& cmd);
        void operator()(const RendererCommand::SetLimits_FlushesForceApply& cmd);
        void operator()(const RendererCommand::SetLimits_FlushesForceUnsubscribe& cmd);
        void operator()(const RendererCommand::FrameProfiler_Toggle& cmd);
        void operator()(const RendererCommand::FrameProfiler_TimingGraphHeight& cmd);
        void operator()(const RendererCommand::FrameProfiler_CounterGraphHeight& cmd);
        void operator()(const RendererCommand::FrameProfiler_RegionFilterFlags& cmd);
        void operator()(const RendererCommand::ConfirmationEcho& cmd);

    private:
        Renderer&                       m_renderer;
        IRendererSceneUpdater&          m_sceneUpdater;
        IRendererSceneControlLogic&     m_sceneControlLogic;
        RendererCommandBuffer&          m_rendererCommandBuffer;
        RendererEventCollector&         m_rendererEventCollector;
        FrameTimer&                     m_frameTimer;

        // to avoid allocs
        RendererCommands m_tmpCommands;
    };
}

#endif
