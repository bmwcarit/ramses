//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "LogicViewerSettings.h"
#include "ImguiWrapper.h"

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#endif
#include "imgui_internal.h" // for ImGuiSettingsHandler
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif

namespace rlogic
{
    namespace
    {
        bool IniReadFlag(const char* line, const char* fmt, int* flag)
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg, cert-err34-c) no suitable replacement
            return (sscanf(line, fmt, flag) == 1);
        }
    }

    LogicViewerSettings::LogicViewerSettings()
    {
        auto* ctx = ImGui::GetCurrentContext();
        assert(ctx != nullptr);
        ImGuiSettingsHandler ini_handler;
        ini_handler.TypeName = "LogicViewerGui";
        ini_handler.TypeHash = ImHashStr("LogicViewerGui");
        ini_handler.ReadOpenFn = IniReadOpen;
        ini_handler.ReadLineFn = IniReadLine;
        ini_handler.WriteAllFn = IniWriteAll;
        ini_handler.UserData   = this;
        ctx->SettingsHandlers.push_back(ini_handler);
        ImGui::LoadIniSettingsFromDisk(ImGui::GetIO().IniFilename);
    }

    void* LogicViewerSettings::IniReadOpen(ImGuiContext* /*context*/, ImGuiSettingsHandler* handler, const char* /*name*/)
    {
        return handler->UserData;
    }

    void LogicViewerSettings::IniReadLine(ImGuiContext* /*context*/, ImGuiSettingsHandler* handler, void* /*entry*/, const char* line)
    {
        auto* gui = static_cast<LogicViewerSettings*>(handler->UserData);
        int flag = 0;
        if (IniReadFlag(line, "ShowWindow=%d", &flag))
        {
            gui->showWindow = (flag != 0);
        }
        else if (IniReadFlag(line, "ShowInterfaces=%d", &flag))
        {
            gui->showInterfaces = (flag != 0);
        }
        else if (IniReadFlag(line, "ShowScripts=%d", &flag))
        {
            gui->showScripts = (flag != 0);
        }
        else if (IniReadFlag(line, "ShowAnimationNodes=%d", &flag))
        {
            gui->showAnimationNodes = (flag != 0);
        }
        else if (IniReadFlag(line, "ShowTimerNodes=%d", &flag))
        {
            gui->showTimerNodes = (flag != 0);
        }
        else if (IniReadFlag(line, "ShowDataArrays=%d", &flag))
        {
            gui->showDataArrays = (flag != 0);
        }
        else if (IniReadFlag(line, "ShowRamsesBindings=%d", &flag))
        {
            gui->showRamsesBindings = (flag != 0);
        }
        else if (IniReadFlag(line, "ShowUpdateReport=%d", &flag))
        {
            gui->showUpdateReport = (flag != 0);
        }
        else if (IniReadFlag(line, "ShowLinkedInputs=%d", &flag))
        {
            gui->showLinkedInputs = (flag != 0);
        }
        else if (IniReadFlag(line, "ShowOutputs=%d", &flag))
        {
            gui->showOutputs = (flag != 0);
        }
        else if (IniReadFlag(line, "LuaPreferObjectIds=%d", &flag))
        {
            gui->luaPreferObjectIds = (flag != 0);
        }
        else if (IniReadFlag(line, "LuaPreferIdentifiers=%d", &flag))
        {
            gui->luaPreferIdentifiers = (flag != 0);
        }
        else if (IniReadFlag(line, "ShowDisplaySettings=%d", &flag))
        {
            gui->showDisplaySettings = (flag != 0);
        }
    }

    void LogicViewerSettings::IniWriteAll(ImGuiContext* /*context*/, ImGuiSettingsHandler* handler, ImGuiTextBuffer* buf)
    {
        auto* gui = static_cast<LogicViewerSettings*>(handler->UserData);
        buf->reserve(buf->size() + 400);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) 3rd party interface
        buf->appendf("[%s][Settings]\n", handler->TypeName);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) 3rd party interface
        buf->appendf("ShowWindow=%d\n", gui->showWindow ? 1 : 0);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) 3rd party interface
        buf->appendf("ShowInterfaces=%d\n", gui->showInterfaces ? 1 : 0);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) 3rd party interface
        buf->appendf("ShowScripts=%d\n", gui->showScripts ? 1 : 0);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) 3rd party interface
        buf->appendf("ShowAnimationNodes=%d\n", gui->showAnimationNodes ? 1 : 0);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) 3rd party interface
        buf->appendf("ShowTimerNodes=%d\n", gui->showTimerNodes ? 1 : 0);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) 3rd party interface
        buf->appendf("ShowDataArrays=%d\n", gui->showDataArrays ? 1 : 0);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) 3rd party interface
        buf->appendf("ShowRamsesBindings=%d\n", gui->showRamsesBindings ? 1 : 0);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) 3rd party interface
        buf->appendf("ShowUpdateReport=%d\n", gui->showUpdateReport ? 1 : 0);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) 3rd party interface
        buf->appendf("ShowLinkedInputs=%d\n", gui->showLinkedInputs ? 1 : 0);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) 3rd party interface
        buf->appendf("ShowOutputs=%d\n", gui->showOutputs ? 1 : 0);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) 3rd party interface
        buf->appendf("LuaPreferObjectIds=%d\n", gui->luaPreferObjectIds ? 1 : 0);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) 3rd party interface
        buf->appendf("LuaPreferIdentifiers=%d\n", gui->luaPreferIdentifiers ? 1 : 0);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) 3rd party interface
        buf->appendf("ShowDisplaySettings=%d\n", gui->showDisplaySettings ? 1 : 0);
        buf->append("\n");
    }
}
