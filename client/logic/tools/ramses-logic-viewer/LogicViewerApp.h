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

namespace ramses
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

        [[nodiscard]] ramses::LogicViewer* getViewer();

        [[nodiscard]] const ramses::LogicViewerSettings* getSettings() const;

    protected:
        [[nodiscard]] static int GetFeatureLevelFromFiles(const std::string& sceneFilename, const std::string& logicFilename, EFeatureLevel& featureLevel);
        [[nodiscard]] int loadScene(const Arguments& args, EFeatureLevel featureLevel);
        [[nodiscard]] int createViewer(const Arguments& args, EFeatureLevel featureLevel, LogicViewer::ScreenshotFunc&& fScreenshot);

        ImGuiContext*                                m_imguiContext = nullptr;
        std::unique_ptr<ramses::RamsesFramework>     m_framework;
        std::unique_ptr<ramses::LogicViewerSettings> m_settings;
        std::unique_ptr<ramses::LogicViewer>         m_viewer;

        ramses::RamsesClient* m_client = nullptr;
        ramses::Scene* m_scene = nullptr;
        ramses::Result m_loadLuaStatus;

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

    inline ramses::LogicViewer* LogicViewerApp::getViewer()
    {
        return m_viewer.get();
    }

    inline const ramses::LogicViewerSettings* LogicViewerApp::getSettings() const
    {
        return m_settings.get();
    }
}

