//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENEDISPLAYTRACKER_H
#define RAMSES_SCENEDISPLAYTRACKER_H

#include "SceneAPI/SceneId.h"
#include "RendererAPI/Types.h"
#include "RendererLib/RendererCommands.h"
#include "RendererLib/RendererEvent.h"
#include "absl/types/optional.h"
#include <unordered_map>

namespace ramses_internal
{
    class SceneDisplayTracker
    {
    public:
        void setSceneOwnership(SceneId scene, DisplayHandle display);
        DisplayHandle getSceneOwnership(SceneId scene) const;

        // 3 return types - valid display, invalid display (error), no value (broadcast)
        absl::optional<DisplayHandle> determineDisplayFromRendererCommand(const RendererCommand::Variant& cmd) const;

        // Some commands (e.g. publish/unpublish) are broadcast to all displays (there is no relevant scene ownership),
        // this is to query if an event is a result of such command
        static bool IsEventResultOfBroadcastCommand(ERendererEventType eventType);

    private:
        std::unordered_map<SceneId, DisplayHandle> m_sceneToDisplay;

        // scene commands
        absl::optional<DisplayHandle> getDisplayOf(const RendererCommand::ReceiveScene& cmd) const { return getSceneOwnership(cmd.info.sceneID); }
        absl::optional<DisplayHandle> getDisplayOf(const RendererCommand::UpdateScene& cmd) const { return getSceneOwnership(cmd.scene); }
        absl::optional<DisplayHandle> getDisplayOf(const RendererCommand::SetSceneState& cmd) const { return getSceneOwnership(cmd.scene); }
        absl::optional<DisplayHandle> getDisplayOf(const RendererCommand::SetSceneDisplayBufferAssignment& cmd) const { return getSceneOwnership(cmd.scene); }
        absl::optional<DisplayHandle> getDisplayOf(const RendererCommand::PickEvent& cmd) const { return getSceneOwnership(cmd.scene); }
        // link commands
        absl::optional<DisplayHandle> getDisplayOf(const RendererCommand::LinkData& cmd) const { return getSceneOwnership(cmd.consumerScene); }
        absl::optional<DisplayHandle> getDisplayOf(const RendererCommand::LinkOffscreenBuffer& cmd) const { return getSceneOwnership(cmd.consumerScene); }
        absl::optional<DisplayHandle> getDisplayOf(const RendererCommand::LinkStreamBuffer& cmd) const { return getSceneOwnership(cmd.consumerScene); }
        absl::optional<DisplayHandle> getDisplayOf(const RendererCommand::UnlinkData& cmd) const { return getSceneOwnership(cmd.consumerScene); }
        // display commands
        absl::optional<DisplayHandle> getDisplayOf(const RendererCommand::SetSceneMapping& cmd) const { return cmd.display; }
        absl::optional<DisplayHandle> getDisplayOf(const RendererCommand::CreateDisplay& cmd) const { return cmd.display; }
        absl::optional<DisplayHandle> getDisplayOf(const RendererCommand::DestroyDisplay& cmd) const { return cmd.display; }
        absl::optional<DisplayHandle> getDisplayOf(const RendererCommand::CreateOffscreenBuffer& cmd) const { return cmd.display; }
        absl::optional<DisplayHandle> getDisplayOf(const RendererCommand::DestroyOffscreenBuffer& cmd) const { return cmd.display; }
        absl::optional<DisplayHandle> getDisplayOf(const RendererCommand::CreateStreamBuffer& cmd) const { return cmd.display; }
        absl::optional<DisplayHandle> getDisplayOf(const RendererCommand::DestroyStreamBuffer& cmd) const { return cmd.display; }
        absl::optional<DisplayHandle> getDisplayOf(const RendererCommand::SetStreamBufferState& cmd) const { return cmd.display; }
        absl::optional<DisplayHandle> getDisplayOf(const RendererCommand::SetClearFlags& cmd) const { return cmd.display; }
        absl::optional<DisplayHandle> getDisplayOf(const RendererCommand::SetClearColor& cmd) const { return cmd.display; }
        absl::optional<DisplayHandle> getDisplayOf(const RendererCommand::UpdateWarpingData& cmd) const { return cmd.display; }
        absl::optional<DisplayHandle> getDisplayOf(const RendererCommand::ReadPixels& cmd) const { return cmd.display; }
        absl::optional<DisplayHandle> getDisplayOf(const RendererCommand::ConfirmationEcho& cmd) const { return cmd.display; }
        // broadcast commands
        absl::optional<DisplayHandle> getDisplayOf(const RendererCommand::ScenePublished&) const { return {}; }
        absl::optional<DisplayHandle> getDisplayOf(const RendererCommand::SceneUnpublished&) const { return {}; }
        absl::optional<DisplayHandle> getDisplayOf(const RendererCommand::SetSkippingOfUnmodifiedBuffers&) const { return {}; }
        absl::optional<DisplayHandle> getDisplayOf(const RendererCommand::LogStatistics&) const { return {}; }
        absl::optional<DisplayHandle> getDisplayOf(const RendererCommand::LogInfo&) const { return {}; }
        absl::optional<DisplayHandle> getDisplayOf(const RendererCommand::SCListIviSurfaces&) const { return {}; }
        absl::optional<DisplayHandle> getDisplayOf(const RendererCommand::SCSetIviSurfaceVisibility&) const { return {}; }
        absl::optional<DisplayHandle> getDisplayOf(const RendererCommand::SCSetIviSurfaceOpacity&) const { return {}; }
        absl::optional<DisplayHandle> getDisplayOf(const RendererCommand::SCSetIviSurfaceDestRectangle&) const { return {}; }
        absl::optional<DisplayHandle> getDisplayOf(const RendererCommand::SCScreenshot&) const { return {}; }
        absl::optional<DisplayHandle> getDisplayOf(const RendererCommand::SCAddIviSurfaceToIviLayer&) const { return {}; }
        absl::optional<DisplayHandle> getDisplayOf(const RendererCommand::SCSetIviLayerVisibility&) const { return {}; }
        absl::optional<DisplayHandle> getDisplayOf(const RendererCommand::SCRemoveIviSurfaceFromIviLayer&) const { return {}; }
        absl::optional<DisplayHandle> getDisplayOf(const RendererCommand::SCDestroyIviSurface&) const { return {}; }
        absl::optional<DisplayHandle> getDisplayOf(const RendererCommand::SetLimits_FrameBudgets&) const { return {}; }
        absl::optional<DisplayHandle> getDisplayOf(const RendererCommand::SetLimits_FlushesForceApply&) const { return {}; }
        absl::optional<DisplayHandle> getDisplayOf(const RendererCommand::SetLimits_FlushesForceUnsubscribe&) const { return {}; }
        absl::optional<DisplayHandle> getDisplayOf(const RendererCommand::FrameProfiler_Toggle&) const { return {}; }
        absl::optional<DisplayHandle> getDisplayOf(const RendererCommand::FrameProfiler_TimingGraphHeight&) const { return {}; }
        absl::optional<DisplayHandle> getDisplayOf(const RendererCommand::FrameProfiler_CounterGraphHeight&) const { return {}; }
        absl::optional<DisplayHandle> getDisplayOf(const RendererCommand::FrameProfiler_RegionFilterFlags&) const { return {}; }
    };

    inline absl::optional<DisplayHandle> SceneDisplayTracker::determineDisplayFromRendererCommand(const RendererCommand::Variant& cmd) const
    {
        return absl::visit([&](const auto& c) { return getDisplayOf(c); }, cmd);
    }
}

#endif
