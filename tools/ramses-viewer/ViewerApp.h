//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/RamsesFrameworkConfig.h"
#include "ramses/framework/RamsesFramework.h"
#include "ramses/framework/ValidationReport.h"

#include "LogicViewer.h"
#include "ViewerSettings.h"

#include <memory>
#include <vector>
#include <string>

struct ImGuiContext;

namespace ramses
{
    class RamsesClient;
    class Scene;
}

namespace CLI
{
    class App;
    class Option;
}

namespace ramses::internal
{
    class ViewerApp
    {
    public:
        enum class ExitCode
        {
            Ok,
            ErrorUsage      = 1,
            ErrorClient     = 2,
            ErrorRenderer   = 3,
            ErrorScene      = 4,
            ErrorScreenshot = 5,
            ErrorDisplay    = 6,
            ErrorLoadLua    = 7,
            ErrorUnknown    = -1,
        };

        ViewerApp();
        ~ViewerApp();

        [[nodiscard]] LogicViewer* getLogicViewer();

        [[nodiscard]] const ViewerSettings* getSettings() const;

        [[nodiscard]] ViewerSettings* getSettings();

        [[nodiscard]] const ramses::ValidationReport& getValidationReport() const;

        [[nodiscard]] const std::string& getSceneFile(std::size_t i = 0) const;
        [[nodiscard]] const std::vector<std::string>& getSceneFiles() const;

        [[nodiscard]] const std::string& getLuaFile() const;

        [[nodiscard]] ramses::Scene* getScene();

    protected:
        void registerOptions(CLI::App& cli);

        [[nodiscard]] ExitCode loadScene();
        [[nodiscard]] ExitCode createLogicViewer(LogicViewer::ScreenshotFunc&& fScreenshot);

        [[nodiscard]] ramses::RamsesFramework* getFramework();
        [[nodiscard]] ramses::RamsesClient*    getClient();

        void setInteractive(bool interactive);
        [[nodiscard]] bool isInteractive() const;

        [[nodiscard]] Result popLoadLuaStatus();

    private:
        [[nodiscard]] ExitCode runLogicCliCommands();

        void printValidationReport() const;
        void printValidationReport(std::ostream& os) const;

        std::vector<std::string> m_sceneFiles;
        std::string m_validationReportFile;
        std::string m_luaFile;
        std::string m_luaFunction;
        std::string m_luaExec;
        bool        m_noValidation = false;
        bool        m_writeConfig  = false;

        bool m_interactive = true;

        ImGuiContext* m_imguiContext = nullptr;

        ramses::RamsesFrameworkConfig  m_frameworkConfig{ramses::EFeatureLevel_Latest};

        std::unique_ptr<ramses::RamsesFramework> m_framework;
        std::unique_ptr<ViewerSettings>          m_settings;
        std::unique_ptr<LogicViewer>             m_logicViewer;

        ramses::RamsesClient* m_client = nullptr;
        ramses::Scene* m_scene = nullptr;
        ramses::ValidationReport m_validationReport;

        Result m_loadLuaStatus;

        CLI::Option* m_report = nullptr;
    };

    inline ramses::RamsesFramework* ViewerApp::getFramework()
    {
        return m_framework.get();
    }

    inline ramses::Scene* ViewerApp::getScene()
    {
        return m_scene;
    }

    inline ramses::RamsesClient* ViewerApp::getClient()
    {
        return m_client;
    }

    inline LogicViewer* ViewerApp::getLogicViewer()
    {
        return m_logicViewer.get();
    }

    inline const ramses::ValidationReport& ViewerApp::getValidationReport() const
    {
        return m_validationReport;
    }

    inline const std::string& ViewerApp::getSceneFile(std::size_t i) const
    {
        assert(i < m_sceneFiles.size());
        return m_sceneFiles[i];
    }

    inline const std::vector<std::string>& ViewerApp::getSceneFiles() const
    {
        return m_sceneFiles;
    }

    inline const std::string& ViewerApp::getLuaFile() const
    {
        return m_luaFile;
    }

    inline const ViewerSettings* ViewerApp::getSettings() const
    {
        return m_settings.get();
    }

    inline ViewerSettings* ViewerApp::getSettings()
    {
        return m_settings.get();
    }

    inline void ViewerApp::setInteractive(bool interactive)
    {
        m_interactive = interactive;
    }

    inline bool ViewerApp::isInteractive() const
    {
        return m_interactive;
    }

    inline Result ViewerApp::popLoadLuaStatus()
    {
        return std::move(m_loadLuaStatus);
    }
}
