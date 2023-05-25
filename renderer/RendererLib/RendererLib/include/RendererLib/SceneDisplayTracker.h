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
#include <optional>
#include <unordered_map>

namespace ramses_internal
{
    class SceneDisplayTracker
    {
    public:
        void setSceneOwnership(SceneId scene, DisplayHandle display);
        [[nodiscard]] DisplayHandle getSceneOwnership(SceneId scene) const;
        void unregisterDisplay(DisplayHandle display);

        // 3 return types - valid display, invalid display (error), no value (broadcast)
        [[nodiscard]] std::optional<DisplayHandle> determineDisplayFromRendererCommand(const RendererCommand::Variant& cmd) const;

    private:
        // scene commands
        [[nodiscard]] std::optional<DisplayHandle> getDisplayOf(const RendererCommand::ReceiveScene& cmd) const { return getSceneOwnership(cmd.info.sceneID); }
        [[nodiscard]] std::optional<DisplayHandle> getDisplayOf(const RendererCommand::UpdateScene& cmd) const { return getSceneOwnership(cmd.scene); }
        [[nodiscard]] std::optional<DisplayHandle> getDisplayOf(const RendererCommand::SetSceneState& cmd) const { return getSceneOwnership(cmd.scene); }
        [[nodiscard]] std::optional<DisplayHandle> getDisplayOf(const RendererCommand::SetSceneDisplayBufferAssignment& cmd) const { return getSceneOwnership(cmd.scene); }
        [[nodiscard]] std::optional<DisplayHandle> getDisplayOf(const RendererCommand::PickEvent& cmd) const { return getSceneOwnership(cmd.scene); }
        // link commands
        [[nodiscard]] std::optional<DisplayHandle> getDisplayOf(const RendererCommand::LinkData& cmd) const { return getSceneOwnership(cmd.consumerScene); }
        [[nodiscard]] std::optional<DisplayHandle> getDisplayOf(const RendererCommand::LinkOffscreenBuffer& cmd) const { return getSceneOwnership(cmd.consumerScene); }
        [[nodiscard]] std::optional<DisplayHandle> getDisplayOf(const RendererCommand::LinkStreamBuffer& cmd) const { return getSceneOwnership(cmd.consumerScene); }
        [[nodiscard]] std::optional<DisplayHandle> getDisplayOf(const RendererCommand::LinkExternalBuffer& cmd) const { return getSceneOwnership(cmd.consumerScene); }
        [[nodiscard]] std::optional<DisplayHandle> getDisplayOf(const RendererCommand::UnlinkData& cmd) const { return getSceneOwnership(cmd.consumerScene); }
        // display commands
        [[nodiscard]] std::optional<DisplayHandle> getDisplayOf(const RendererCommand::SetSceneMapping& cmd) const { return cmd.display; }
        [[nodiscard]] std::optional<DisplayHandle> getDisplayOf(const RendererCommand::CreateDisplay& cmd) const { return cmd.display; }
        [[nodiscard]] std::optional<DisplayHandle> getDisplayOf(const RendererCommand::DestroyDisplay& cmd) const { return cmd.display; }
        [[nodiscard]] std::optional<DisplayHandle> getDisplayOf(const RendererCommand::CreateOffscreenBuffer& cmd) const { return cmd.display; }
        [[nodiscard]] std::optional<DisplayHandle> getDisplayOf(const RendererCommand::CreateDmaOffscreenBuffer& cmd) const { return cmd.display; }
        [[nodiscard]] std::optional<DisplayHandle> getDisplayOf(const RendererCommand::DestroyOffscreenBuffer& cmd) const { return cmd.display; }
        [[nodiscard]] std::optional<DisplayHandle> getDisplayOf(const RendererCommand::CreateStreamBuffer& cmd) const { return cmd.display; }
        [[nodiscard]] std::optional<DisplayHandle> getDisplayOf(const RendererCommand::DestroyStreamBuffer& cmd) const { return cmd.display; }
        [[nodiscard]] std::optional<DisplayHandle> getDisplayOf(const RendererCommand::CreateExternalBuffer& cmd) const { return cmd.display; }
        [[nodiscard]] std::optional<DisplayHandle> getDisplayOf(const RendererCommand::DestroyExternalBuffer& cmd) const { return cmd.display; }
        [[nodiscard]] std::optional<DisplayHandle> getDisplayOf(const RendererCommand::SetClearFlags& cmd) const { return cmd.display; }
        [[nodiscard]] std::optional<DisplayHandle> getDisplayOf(const RendererCommand::SetClearColor& cmd) const { return cmd.display; }
        [[nodiscard]] std::optional<DisplayHandle> getDisplayOf(const RendererCommand::SetExterallyOwnedWindowSize& cmd) const { return cmd.display; }
        [[nodiscard]] std::optional<DisplayHandle> getDisplayOf(const RendererCommand::ReadPixels& cmd) const { return cmd.display; }
        [[nodiscard]] std::optional<DisplayHandle> getDisplayOf(const RendererCommand::ConfirmationEcho& cmd) const { return cmd.display; }
        // broadcast commands
        [[nodiscard]] std::optional<DisplayHandle> getDisplayOf(const RendererCommand::ScenePublished&) const { return {}; }
        [[nodiscard]] std::optional<DisplayHandle> getDisplayOf(const RendererCommand::SceneUnpublished&) const { return {}; }
        [[nodiscard]] std::optional<DisplayHandle> getDisplayOf(const RendererCommand::SetSkippingOfUnmodifiedBuffers&) const { return {}; }
        [[nodiscard]] std::optional<DisplayHandle> getDisplayOf(const RendererCommand::LogStatistics&) const { return {}; }
        [[nodiscard]] std::optional<DisplayHandle> getDisplayOf(const RendererCommand::LogInfo&) const { return {}; }
        [[nodiscard]] std::optional<DisplayHandle> getDisplayOf(const RendererCommand::SCListIviSurfaces&) const { return {}; }
        [[nodiscard]] std::optional<DisplayHandle> getDisplayOf(const RendererCommand::SCSetIviSurfaceVisibility&) const { return {}; }
        [[nodiscard]] std::optional<DisplayHandle> getDisplayOf(const RendererCommand::SCSetIviSurfaceOpacity&) const { return {}; }
        [[nodiscard]] std::optional<DisplayHandle> getDisplayOf(const RendererCommand::SCSetIviSurfaceDestRectangle&) const { return {}; }
        [[nodiscard]] std::optional<DisplayHandle> getDisplayOf(const RendererCommand::SCScreenshot&) const { return {}; }
        [[nodiscard]] std::optional<DisplayHandle> getDisplayOf(const RendererCommand::SCAddIviSurfaceToIviLayer&) const { return {}; }
        [[nodiscard]] std::optional<DisplayHandle> getDisplayOf(const RendererCommand::SCSetIviLayerVisibility&) const { return {}; }
        [[nodiscard]] std::optional<DisplayHandle> getDisplayOf(const RendererCommand::SCRemoveIviSurfaceFromIviLayer&) const { return {}; }
        [[nodiscard]] std::optional<DisplayHandle> getDisplayOf(const RendererCommand::SCDestroyIviSurface&) const { return {}; }
        [[nodiscard]] std::optional<DisplayHandle> getDisplayOf(const RendererCommand::SetLimits_FrameBudgets&) const { return {}; }
        [[nodiscard]] std::optional<DisplayHandle> getDisplayOf(const RendererCommand::SetLimits_FlushesForceApply&) const { return {}; }
        [[nodiscard]] std::optional<DisplayHandle> getDisplayOf(const RendererCommand::SetLimits_FlushesForceUnsubscribe&) const { return {}; }

        std::unordered_map<SceneId, DisplayHandle> m_sceneToDisplay;
    };

    inline std::optional<DisplayHandle> SceneDisplayTracker::determineDisplayFromRendererCommand(const RendererCommand::Variant& cmd) const
    {
        return std::visit([&](const auto& c) { return getDisplayOf(c); }, cmd);
    }
}

#endif
