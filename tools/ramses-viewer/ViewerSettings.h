//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include <array>
#include <glm/vec4.hpp>

struct ImGuiSettingsHandler;
struct ImGuiContext;
struct ImGuiTextBuffer;

namespace ramses::internal
{
    struct ViewerSettings
    {
        // scene viewer settings
        bool showGui = true;

        bool showSceneViewerWindow = true;
        bool showSceneInWindow = false;

        const std::array<float, 6> zoomLevels = {(1.f / 4.f), (1.f / 3.f), (1.f / 2.f), (2.f / 3.f), (3.f / 4.f), 1.f};
        int zoomIx = 4;

        // Logic viewer settings
        bool showLogicWindow    = true;
        bool showUpdateReport   = true;

        bool luaPreferObjectIds   = false;
        bool luaPreferIdentifiers = false;

        glm::vec4 clearColor{ 0, 0, 0, 1 };

        ViewerSettings();

        static void* IniReadOpen(ImGuiContext* context, ImGuiSettingsHandler* handler, const char* name);
        static void  IniReadLine(ImGuiContext* context, ImGuiSettingsHandler* handler, void* entry, const char* line);
        static void  IniWriteAll(ImGuiContext* context, ImGuiSettingsHandler* handler, ImGuiTextBuffer* buf);
    };
}
