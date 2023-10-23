//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/SceneAPI/SceneId.h"
#include "internal/RendererLib/Types.h"
#include "internal/RendererLib/RendererCommands.h"
#include "internal/RendererLib/RendererEvent.h"
#include <optional>
#include <unordered_map>

namespace ramses::internal
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
        [[nodiscard]] static std::optional<DisplayHandle> getDisplayOf(const RendererCommand::SetSceneMapping& cmd) { return cmd.display; }
        [[nodiscard]] static std::optional<DisplayHandle> getDisplayOf(const RendererCommand::CreateDisplay& cmd) { return cmd.display; }
        [[nodiscard]] static std::optional<DisplayHandle> getDisplayOf(const RendererCommand::DestroyDisplay& cmd) { return cmd.display; }
        [[nodiscard]] static std::optional<DisplayHandle> getDisplayOf(const RendererCommand::CreateOffscreenBuffer& cmd) { return cmd.display; }
        [[nodiscard]] static std::optional<DisplayHandle> getDisplayOf(const RendererCommand::CreateDmaOffscreenBuffer& cmd) { return cmd.display; }
        [[nodiscard]] static std::optional<DisplayHandle> getDisplayOf(const RendererCommand::DestroyOffscreenBuffer& cmd) { return cmd.display; }
        [[nodiscard]] static std::optional<DisplayHandle> getDisplayOf(const RendererCommand::CreateStreamBuffer& cmd) { return cmd.display; }
        [[nodiscard]] static std::optional<DisplayHandle> getDisplayOf(const RendererCommand::DestroyStreamBuffer& cmd) { return cmd.display; }
        [[nodiscard]] static std::optional<DisplayHandle> getDisplayOf(const RendererCommand::CreateExternalBuffer& cmd) { return cmd.display; }
        [[nodiscard]] static std::optional<DisplayHandle> getDisplayOf(const RendererCommand::DestroyExternalBuffer& cmd) { return cmd.display; }
        [[nodiscard]] static std::optional<DisplayHandle> getDisplayOf(const RendererCommand::SetClearFlags& cmd) { return cmd.display; }
        [[nodiscard]] static std::optional<DisplayHandle> getDisplayOf(const RendererCommand::SetClearColor& cmd) { return cmd.display; }
        [[nodiscard]] static std::optional<DisplayHandle> getDisplayOf(const RendererCommand::SetExterallyOwnedWindowSize& cmd) { return cmd.display; }
        [[nodiscard]] static std::optional<DisplayHandle> getDisplayOf(const RendererCommand::ReadPixels& cmd) { return cmd.display; }
        [[nodiscard]] static std::optional<DisplayHandle> getDisplayOf(const RendererCommand::ConfirmationEcho& cmd) { return cmd.display; }
        // broadcast commands
        [[nodiscard]] static std::optional<DisplayHandle> getDisplayOf(const RendererCommand::ScenePublished& /*unused*/) { return {}; }
        [[nodiscard]] static std::optional<DisplayHandle> getDisplayOf(const RendererCommand::SceneUnpublished& /*unused*/) { return {}; }
        [[nodiscard]] static std::optional<DisplayHandle> getDisplayOf(const RendererCommand::SetSkippingOfUnmodifiedBuffers& /*unused*/) { return {}; }
        [[nodiscard]] static std::optional<DisplayHandle> getDisplayOf(const RendererCommand::LogStatistics& /*unused*/) { return {}; }
        [[nodiscard]] static std::optional<DisplayHandle> getDisplayOf(const RendererCommand::LogInfo& /*unused*/) { return {}; }
        [[nodiscard]] static std::optional<DisplayHandle> getDisplayOf(const RendererCommand::SCListIviSurfaces& /*unused*/) { return {}; }
        [[nodiscard]] static std::optional<DisplayHandle> getDisplayOf(const RendererCommand::SCSetIviSurfaceVisibility& /*unused*/) { return {}; }
        [[nodiscard]] static std::optional<DisplayHandle> getDisplayOf(const RendererCommand::SCSetIviSurfaceOpacity& /*unused*/) { return {}; }
        [[nodiscard]] static std::optional<DisplayHandle> getDisplayOf(const RendererCommand::SCSetIviSurfaceDestRectangle& /*unused*/) { return {}; }
        [[nodiscard]] static std::optional<DisplayHandle> getDisplayOf(const RendererCommand::SCScreenshot& /*unused*/) { return {}; }
        [[nodiscard]] static std::optional<DisplayHandle> getDisplayOf(const RendererCommand::SCAddIviSurfaceToIviLayer& /*unused*/) { return {}; }
        [[nodiscard]] static std::optional<DisplayHandle> getDisplayOf(const RendererCommand::SCSetIviLayerVisibility& /*unused*/) { return {}; }
        [[nodiscard]] static std::optional<DisplayHandle> getDisplayOf(const RendererCommand::SCRemoveIviSurfaceFromIviLayer& /*unused*/) { return {}; }
        [[nodiscard]] static std::optional<DisplayHandle> getDisplayOf(const RendererCommand::SCDestroyIviSurface& /*unused*/) { return {}; }
        [[nodiscard]] static std::optional<DisplayHandle> getDisplayOf(const RendererCommand::SetLimits_FrameBudgets& /*unused*/) { return {}; }
        [[nodiscard]] static std::optional<DisplayHandle> getDisplayOf(const RendererCommand::SetLimits_FlushesForceApply& /*unused*/) { return {}; }
        [[nodiscard]] static std::optional<DisplayHandle> getDisplayOf(const RendererCommand::SetLimits_FlushesForceUnsubscribe& /*unused*/) { return {}; }

        std::unordered_map<SceneId, DisplayHandle> m_sceneToDisplay;
    };

    // NOLINTNEXTLINE(readability-convert-member-functions-to-static): false positive
    inline std::optional<DisplayHandle> SceneDisplayTracker::determineDisplayFromRendererCommand(const RendererCommand::Variant& cmd) const
    {
        return std::visit([&](const auto& c) { return getDisplayOf(c); }, cmd);
    }
}
