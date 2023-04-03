//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

struct ImGuiSettingsHandler;
struct ImGuiContext;
struct ImGuiTextBuffer;

namespace rlogic
{
    struct LogicViewerSettings
    {
        bool showWindow         = true;
        bool showOutputs        = true;
        bool showLinkedInputs   = true;
        bool showInterfaces     = true;
        bool showScripts        = false;
        bool showAnimationNodes = false;
        bool showTimerNodes     = false;
        bool showDataArrays     = false;
        bool showRamsesBindings = false;
        bool showUpdateReport   = true;

        bool luaPreferObjectIds   = false;
        bool luaPreferIdentifiers = false;

        bool showDisplaySettings = false;

        LogicViewerSettings();

        static void* IniReadOpen(ImGuiContext* context, ImGuiSettingsHandler* handler, const char* name);
        static void IniReadLine(ImGuiContext* context, ImGuiSettingsHandler* handler, void* entry, const char* line);
        static void IniWriteAll(ImGuiContext* context, ImGuiSettingsHandler* handler, ImGuiTextBuffer* buf);
    };
}
