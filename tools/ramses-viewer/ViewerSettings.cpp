//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ViewerSettings.h"

#include"ImguiWrapper.h"

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#endif
#include "imgui_internal.h" // for ImGuiSettingsHandler
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif

namespace ramses::internal
{
    namespace
    {
        bool IniReadFlag(const char* line, const char* fmt, int* flag)
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg, cert-err34-c) no suitable replacement
            return (sscanf(line, fmt, flag) == 1);
        }
    }

    ViewerSettings::ViewerSettings()
    {
        auto* ctx = ImGui::GetCurrentContext();
        assert(ctx != nullptr);
        ImGuiSettingsHandler ini_handler;
        ini_handler.TypeName = "ramses-viewer";
        ini_handler.TypeHash = ImHashStr("ramses-viewer");
        ini_handler.ReadOpenFn = IniReadOpen;
        ini_handler.ReadLineFn = IniReadLine;
        ini_handler.WriteAllFn = IniWriteAll;
        ini_handler.UserData   = this;
        ctx->SettingsHandlers.push_back(ini_handler);
        ImGui::LoadIniSettingsFromDisk(ImGui::GetIO().IniFilename);
    }

    void* ViewerSettings::IniReadOpen(ImGuiContext* /*context*/, ImGuiSettingsHandler* handler, const char* /*name*/)
    {
        return handler->UserData;
    }

    void ViewerSettings::IniReadLine(ImGuiContext* /*context*/, ImGuiSettingsHandler* handler, void* /*entry*/, const char* line)
    {
        auto* gui = static_cast<ViewerSettings*>(handler->UserData);
        int flag = 0;
        if (IniReadFlag(line, "ShowSceneViewerWindow=%d", &flag))
        {
            gui->showSceneViewerWindow = (flag != 0);
        }
        else if (IniReadFlag(line, "ShowLogicWindow=%d", &flag))
        {
            gui->showLogicWindow = (flag != 0);
        }
        else if (IniReadFlag(line, "ShowUpdateReport=%d", &flag))
        {
            gui->showUpdateReport = (flag != 0);
        }
        else if (IniReadFlag(line, "LuaPreferObjectIds=%d", &flag))
        {
            gui->luaPreferObjectIds = (flag != 0);
        }
        else if (IniReadFlag(line, "LuaPreferIdentifiers=%d", &flag))
        {
            gui->luaPreferIdentifiers = (flag != 0);
        }
    }

    void ViewerSettings::IniWriteAll(ImGuiContext* /*context*/, ImGuiSettingsHandler* handler, ImGuiTextBuffer* buf)
    {
        auto* gui = static_cast<ViewerSettings*>(handler->UserData);
        buf->reserve(buf->size() + 400);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) 3rd party interface
        buf->appendf("[%s][Settings]\n", handler->TypeName);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) 3rd party interface
        buf->appendf("ShowSceneViewerWindow=%d\n", gui->showSceneViewerWindow ? 1 : 0);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) 3rd party interface
        buf->appendf("ShowLogicWindow=%d\n", gui->showLogicWindow ? 1 : 0);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) 3rd party interface
        buf->appendf("ShowUpdateReport=%d\n", gui->showUpdateReport ? 1 : 0);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) 3rd party interface
        buf->appendf("LuaPreferObjectIds=%d\n", gui->luaPreferObjectIds ? 1 : 0);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) 3rd party interface
        buf->appendf("LuaPreferIdentifiers=%d\n", gui->luaPreferIdentifiers ? 1 : 0);
        buf->append("\n");
    }
}
