//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERCOMMANDUTILS_H
#define RAMSES_RENDERERCOMMANDUTILS_H

#include "RendererLib/RendererCommands.h"
#include "RendererLib/RendererEvent.h"

namespace ramses_internal
{
    namespace RendererCommandUtils
    {
        inline std::string ToString(const RendererCommand::ScenePublished& cmd) { return fmt::format("ScenePublished (sceneId={} publication={})", cmd.scene, cmd.publicationMode); }
        inline std::string ToString(const RendererCommand::SceneUnpublished& cmd) { return fmt::format("SceneUnpublished (sceneId={})", cmd.scene); }
        inline std::string ToString(const RendererCommand::ReceiveScene& cmd) { return fmt::format("ReceiveScene (sceneId={} name={})", cmd.info.sceneID, cmd.info.friendlyName); }
        inline std::string ToString(const RendererCommand::UpdateScene& cmd) { return fmt::format("UpdateScene (sceneId={} sceneactions={} resources={})", cmd.scene, cmd.updateData.actions.numberOfActions(), cmd.updateData.resources.size()); }
        inline std::string ToString(const RendererCommand::SetSceneState& cmd) { return fmt::format("SetSceneState (sceneId={} state={})", cmd.scene, cmd.state); }
        inline std::string ToString(const RendererCommand::SetSceneMapping& cmd) { return fmt::format("SetSceneMapping (sceneId={} display={})", cmd.scene, cmd.display); }
        inline std::string ToString(const RendererCommand::SetSceneDisplayBufferAssignment& cmd) { return fmt::format("SetSceneDisplayBufferAssignment (sceneId={} OB={} renderorder={})", cmd.scene, cmd.buffer, cmd.renderOrder); }
        inline std::string ToString(const RendererCommand::LinkData& cmd) { return fmt::format("LinkData (providerSceneId={} providerDataId={} consumerSceneId={} consumerDataId={})", cmd.providerScene, cmd.providerData, cmd.consumerScene, cmd.consumerData); }
        inline std::string ToString(const RendererCommand::LinkOffscreenBuffer& cmd) { return fmt::format("LinkOffscreenBuffer (providerOB={} consumerSceneId={} consumerDataId={})", cmd.providerBuffer, cmd.consumerScene, cmd.consumerData); }
        inline std::string ToString(const RendererCommand::LinkStreamBuffer& cmd) { return fmt::format("LinkStreamBuffer (providerSB={} consumerSceneId={} consumerDataId={})", cmd.providerBuffer, cmd.consumerScene, cmd.consumerData); }
        inline std::string ToString(const RendererCommand::UnlinkData& cmd) { return fmt::format("UnlinkData (consumerSceneId={} consumerDataId={})", cmd.consumerScene, cmd.consumerData); }
        inline std::string ToString(const RendererCommand::PickEvent& cmd) { return fmt::format("PickEvent (sceneId={} coords={})", cmd.scene, cmd.coordsNormalizedToBufferSize); }
        inline std::string ToString(const RendererCommand::CreateDisplay& cmd) { return fmt::format("CreateDisplay (displayId={})", cmd.display); }
        inline std::string ToString(const RendererCommand::DestroyDisplay& cmd) { return fmt::format("DestroyDisplay (displayId={})", cmd.display); }
        inline std::string ToString(const RendererCommand::CreateOffscreenBuffer& cmd) { return fmt::format("CreateOffscreenBuffer (displayId={} OB={} wh={}x{} msaa={} interruptible={})", cmd.display, cmd.offscreenBuffer, cmd.width, cmd.height, cmd.sampleCount, cmd.interruptible); }
        inline std::string ToString(const RendererCommand::DestroyOffscreenBuffer& cmd) { return fmt::format("DestroyOffscreenBuffer (displayId={} OB={})", cmd.display, cmd.offscreenBuffer); }
        inline std::string ToString(const RendererCommand::CreateStreamBuffer& cmd) { return fmt::format("CreateStreamBuffer (displayId={} SB={})", cmd.display, cmd.streamBuffer); }
        inline std::string ToString(const RendererCommand::DestroyStreamBuffer& cmd) { return fmt::format("DestroyStreamBuffer (displayId={} SB={})", cmd.display, cmd.streamBuffer); }
        inline std::string ToString(const RendererCommand::SetStreamBufferState& cmd) { return fmt::format("SetStreamBufferState (displayId={} SB={} state={})", cmd.display, cmd.streamBuffer, cmd.newState); }
        inline std::string ToString(const RendererCommand::SetClearColor& cmd) { return fmt::format("SetClearColor (displayId={} OB={} color={})", cmd.display, cmd.offscreenBuffer, cmd.clearColor); }
        inline std::string ToString(const RendererCommand::UpdateWarpingData& cmd) { return fmt::format("UpdateWarpingData (displayId={})", cmd.display); }
        inline std::string ToString(const RendererCommand::ReadPixels& cmd) { return fmt::format("ReadPixels (displayId={} OB={})", cmd.display, cmd.offscreenBuffer); }
        inline std::string ToString(const RendererCommand::SetSkippingOfUnmodifiedBuffers& cmd) { return fmt::format("SetSkippingOfUnmodifiedBuffers (enable={})", cmd.enable); }
        inline std::string ToString(const RendererCommand::LogStatistics&) { return "LogStatistics"; }
        inline std::string ToString(const RendererCommand::LogInfo&) { return "LogInfo"; }
        inline std::string ToString(const RendererCommand::SCListIviSurfaces&) { return "SCListIviSurfaces"; }
        inline std::string ToString(const RendererCommand::SCSetIviSurfaceVisibility& cmd) { return fmt::format("SCSetIviSurfaceVisibility (surfaceId={} visibility={})", cmd.surface, cmd.visibility); }
        inline std::string ToString(const RendererCommand::SCSetIviSurfaceOpacity& cmd) { return fmt::format("SCSetIviSurfaceOpacity (surfaceId={} opacity={})", cmd.surface, cmd.opacity); }
        inline std::string ToString(const RendererCommand::SCSetIviSurfaceDestRectangle& cmd) { return fmt::format("SCSetIviSurfaceDestRectangle (surfaceId={} x={} y={} w={} h={})", cmd.surface, cmd.x, cmd.y, cmd.width, cmd.height); }
        inline std::string ToString(const RendererCommand::SCScreenshot& cmd) { return fmt::format("SCScreenshot (screenId={} file={})", cmd.screenId, cmd.filename); }
        inline std::string ToString(const RendererCommand::SCAddIviSurfaceToIviLayer& cmd) { return fmt::format("SCAddIviSurfaceToIviLayer (surfaceId={} layerId={})", cmd.surface, cmd.layer); }
        inline std::string ToString(const RendererCommand::SCSetIviLayerVisibility& cmd) { return fmt::format("SCSetIviLayerVisibility (layerId={} visibility={})", cmd.layer, cmd.visibility); }
        inline std::string ToString(const RendererCommand::SCRemoveIviSurfaceFromIviLayer& cmd) { return fmt::format("SCRemoveIviSurfaceFromIviLayer (surfaceId={} layerId={})", cmd.surface, cmd.layer); }
        inline std::string ToString(const RendererCommand::SCDestroyIviSurface& cmd) { return fmt::format("SCDestroyIviSurface (surfaceId={})", cmd.surface); }
        inline std::string ToString(const RendererCommand::SetLimits_FrameBudgets& cmd) { return fmt::format("SetLimits_FrameBudgets (dynResources={} resources={} obRender={})", cmd.limitForSceneResourcesUploadMicrosec, cmd.limitForResourcesUploadMicrosec, cmd.limitForOffscreenBufferRenderMicrosec); }
        inline std::string ToString(const RendererCommand::SetLimits_FlushesForceApply& cmd) { return fmt::format("SetLimits_FlushesForceApply (numFlushes={})", cmd.limitForPendingFlushesForceApply); }
        inline std::string ToString(const RendererCommand::SetLimits_FlushesForceUnsubscribe& cmd) { return fmt::format("SetLimits_FlushesForceUnsubscribe (numFlushes={})", cmd.limitForPendingFlushesForceUnsubscribe); }
        inline std::string ToString(const RendererCommand::FrameProfiler_Toggle& cmd) { return fmt::format("FrameProfiler_Toggle (enable={})", cmd.toggle); }
        inline std::string ToString(const RendererCommand::FrameProfiler_TimingGraphHeight& cmd) { return fmt::format("FrameProfiler_TimingGraphHeight (height={})", cmd.height); }
        inline std::string ToString(const RendererCommand::FrameProfiler_CounterGraphHeight& cmd) { return fmt::format("FrameProfiler_CounterGraphHeight (height={})", cmd.height); }
        inline std::string ToString(const RendererCommand::FrameProfiler_RegionFilterFlags& cmd) { return fmt::format("FrameProfiler_RegionFilterFlags (flags={})", cmd.flags); }
        inline std::string ToString(const RendererCommand::ConfirmationEcho& cmd) { return fmt::format("ConfirmationEcho (text={})", cmd.text); }
        inline std::string ToString(const RendererCommand::Variant& var)
        {
            return absl::visit([&](const auto& c) { return RendererCommandUtils::ToString(c); }, var);
        }

        template <typename T>
        RendererCommand::Variant CreateVariantFrom(const T& cmd)
        {
            T copy{ cmd };
            return RendererCommand::Variant{ std::move(copy) };
        }
        template <>
        inline RendererCommand::Variant CreateVariantFrom<RendererCommand::UpdateScene>(const RendererCommand::UpdateScene&)
        {
            // implementation needed for visiting commands but this should never be executed
            assert(!"this command is not supposed to be copied");
            return RendererCommand::Variant{};
        }
        inline RendererCommand::Variant Copy(const RendererCommand::Variant& var)
        {
            return absl::visit([&](const auto& c) { return RendererCommandUtils::CreateVariantFrom(c); }, var);
        }

        template <typename T>
        RendererEvent GenerateFailEventForCommand(const T&)
        {
            return RendererEvent{ ERendererEventType::Invalid };
        }
        template <>
        inline RendererEvent GenerateFailEventForCommand<RendererCommand::DestroyDisplay>(const RendererCommand::DestroyDisplay& cmd)
        {
            RendererEvent evt{ ERendererEventType::DisplayDestroyFailed };
            evt.displayHandle = cmd.display;
            return evt;
        }
        template <>
        inline RendererEvent GenerateFailEventForCommand<RendererCommand::CreateOffscreenBuffer>(const RendererCommand::CreateOffscreenBuffer& cmd)
        {
            RendererEvent evt{ ERendererEventType::OffscreenBufferCreateFailed };
            evt.displayHandle = cmd.display;
            evt.offscreenBuffer = cmd.offscreenBuffer;
            return evt;
        }
        template <>
        inline RendererEvent GenerateFailEventForCommand<RendererCommand::DestroyOffscreenBuffer>(const RendererCommand::DestroyOffscreenBuffer& cmd)
        {
            RendererEvent evt{ ERendererEventType::OffscreenBufferDestroyFailed };
            evt.displayHandle = cmd.display;
            evt.offscreenBuffer = cmd.offscreenBuffer;
            return evt;
        }
        template <>
        inline RendererEvent GenerateFailEventForCommand<RendererCommand::UpdateWarpingData>(const RendererCommand::UpdateWarpingData& cmd)
        {
            RendererEvent evt{ ERendererEventType::WarpingDataUpdateFailed };
            evt.displayHandle = cmd.display;
            return evt;
        }
        template <>
        inline RendererEvent GenerateFailEventForCommand<RendererCommand::ReadPixels>(const RendererCommand::ReadPixels& cmd)
        {
            RendererEvent evt{ ERendererEventType::ReadPixelsFromFramebufferFailed };
            evt.displayHandle = cmd.display;
            evt.offscreenBuffer = cmd.offscreenBuffer;
            return evt;
        }
        inline RendererEvent GenerateFailEventForCommand(const RendererCommand::Variant& var)
        {
            return absl::visit([&](const auto& c) { return RendererCommandUtils::GenerateFailEventForCommand(c); }, var);
        }
    }
}

#endif
