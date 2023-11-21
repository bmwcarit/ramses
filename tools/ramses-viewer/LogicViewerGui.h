//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ImguiClientHelper.h"
#include "ramses/client/logic/Collection.h"
#include <array>

namespace ramses
{
    class LogicEngine;
    class DataArray;
}

namespace ramses::internal
{
    class ViewerGuiApp;
    class LogicViewer;
    struct ViewerSettings;

    class LogicViewerGui
    {
    public:
        /**
        * @param luafile the luafile argument provided by command line
        *        If it does not exist the LogicViewerGui will provide a UI to save the default settings
        */
        explicit LogicViewerGui(ViewerGuiApp& app, std::string& errorMessage);
        void draw();

        void drawGlobalContextMenuItems();

        void handleShortcuts();

    private:
        void drawMenuItemReload();

        void drawWindow();
        void drawMenuBar();
        void drawCurrentView();
        void drawUpdateReport();
        void drawSaveDefaultLuaFile();

        static void DrawDataArray(const ramses::DataArray* obj, std::string_view context = std::string_view());

        void reloadConfiguration();
        void loadLuaFile(const std::string& filename);
        void saveDefaultLuaFile();

        ViewerSettings& m_settings;
        LogicViewer& m_viewer;
        ramses::LogicEngine& m_logicEngine;

        std::string& m_lastErrorMessage;
        std::string m_filename;
        size_t m_updateReportInterval = 60u;
    };
}

