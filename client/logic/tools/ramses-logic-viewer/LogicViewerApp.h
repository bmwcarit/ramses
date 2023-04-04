//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses-client.h"
#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "Result.h"
#include "LogicViewer.h"
#include <array>

class Arguments;
struct ImGuiContext;

namespace rlogic
{
    struct LogicViewerSettings;

    class LogicViewerApp
    {
    public:
        enum class ExitCode
        {
            Ok                  = 0,
            ErrorRamsesClient   = 1,
            ErrorRamsesRenderer = 2,
            ErrorSceneControl   = 3,
            ErrorLoadScene      = 4,
            ErrorLoadLogic      = 5,
            ErrorLoadLua        = 6,
            ErrorNoDisplay      = 7,
            ErrorUnknown        = -1,
        };

        LogicViewerApp();

        virtual ~LogicViewerApp();

        LogicViewerApp(const LogicViewerApp&) = delete;

        LogicViewerApp& operator=(const LogicViewerApp&) = delete;

        [[nodiscard]] virtual bool doOneLoop() = 0;

        [[nodiscard]] int exitCode() const;

        [[nodiscard]] int run();

        [[nodiscard]] rlogic::LogicViewer* getViewer();

        [[nodiscard]] const rlogic::LogicViewerSettings* getSettings() const;

    protected:
        [[nodiscard]] int loadScene(const Arguments& args);
        [[nodiscard]] int createViewer(const Arguments& args, LogicViewer::ScreenshotFunc&& fScreenshot);

        ImGuiContext*                                m_imguiContext = nullptr;
        std::unique_ptr<ramses::RamsesFramework>     m_framework;
        std::unique_ptr<rlogic::LogicViewerSettings> m_settings;
        std::unique_ptr<rlogic::LogicViewer>         m_viewer;

        ramses::RamsesClient* m_client = nullptr;
        ramses::Scene* m_scene = nullptr;
        rlogic::Result m_loadLuaStatus;

        int m_exitCode = -1;
        bool m_interactive = false;
    };

    inline int LogicViewerApp::exitCode() const
    {
        return m_exitCode;
    }

    inline int LogicViewerApp::run()
    {
        while (doOneLoop())
            ;
        return m_exitCode;
    }

    inline rlogic::LogicViewer* LogicViewerApp::getViewer()
    {
        return m_viewer.get();
    }

    inline const rlogic::LogicViewerSettings* LogicViewerApp::getSettings() const
    {
        return m_settings.get();
    }
}

