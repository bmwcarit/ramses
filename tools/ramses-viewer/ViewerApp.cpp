//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  Copyright (C) 2019 Daniel Werner Lima Souza de Almeida (dwlsalmeida@gmail.com)
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ViewerApp.h"

#include "ramses-cli.h"
#include "ramses/client/ramses-client.h"
#include "ramses/client/ramses-utils.h"

#include "impl/ValidationReportImpl.h"

#include "internal/Core/Utils/File.h"
#include "internal/Core/Utils/LogMacros.h"
#include "ramses-sdk-build-config.h"
#include "ImguiWrapper.h"
#include "ramses/framework/EFeatureLevel.h"

#include <filesystem>
#include <fstream>
#include <optional>

namespace fs = std::filesystem;

namespace ramses::internal
{
    ViewerApp::ViewerApp()
    {
        m_imguiContext = ImGui::CreateContext();
        m_settings     = std::make_unique<ViewerSettings>();
        // set default configuration
        m_frameworkConfig.setPeriodicLogInterval(std::chrono::seconds(0));
        m_frameworkConfig.setRequestedRamsesShellType(ramses::ERamsesShellType::Console);
        m_frameworkConfig.setLogLevelConsole(ELogLevel::Error);
    }

    ViewerApp::~ViewerApp()
    {
        ImGui::DestroyContext(m_imguiContext);
    }

    void ViewerApp::registerOptions(CLI::App& cli)
    {
        cli.description(R"(
Loads and shows a ramses scene from the <ramsesfile>.
<luafile> is auto-resolved if matching file with *.lua extension is found in the same path as <ramsesfile>. (Explicit argument overrides autodetection.)
)");
        cli.set_version_flag("--version", ramses_sdk::RAMSES_SDK_RAMSES_VERSION);
        auto* cliScene = cli.add_option("-s,--scene,scene", m_sceneFiles, "Scene files to load and merge")->check(CLI::ExistingFile);
        cli.add_option("--lua", m_luaFile, "Lua configuration file")->check(CLI::ExistingFile)->needs(cliScene);
        auto exec = cli.add_option("--exec", m_luaFunction, "Calls the given lua function and exits.");
        cli.add_option("--exec-lua", m_luaExec, "Calls the given lua code and exits.")->excludes(exec)->needs(cliScene);
        auto setWriteConfig = [&](const std::string& filename) {
            m_luaFile     = filename;
            m_writeConfig = true;
        };
        cli.add_option_function<std::string>("--write-config", setWriteConfig, "Writes the default lua configuration file and exits")
            ->expected(0, 1)
            ->type_name("[FILE]")
            ->excludes(exec)
            ->needs(cliScene);
        auto nv = cli.add_flag("--no-validation", m_noValidation, "Disable scene validation (faster startup for complex scenes)");
        m_report = cli.add_option("--report", m_validationReportFile, "Prints the validation report to the given file (or stdout if no filename is given)")->expected(0, 1)->excludes(nv);

        ramses::registerOptions(cli, m_frameworkConfig);
    }

    ViewerApp::ExitCode ViewerApp::loadScene()
    {
        std::optional<ramses::EFeatureLevel> featureLevel;
        if (!m_sceneFiles.empty())
        {
            for (const auto& sceneFile: m_sceneFiles)
            {
                if (sceneFile.empty())
                {
                    return ExitCode::ErrorUsage;
                }
                ramses::EFeatureLevel fileFeatureLevel = ramses::EFeatureLevel_Latest;
                if (!ramses::RamsesClient::GetFeatureLevelFromFile(sceneFile, fileFeatureLevel))
                {
                    LOG_ERROR(CONTEXT_CLIENT, "File does not exist: {}", sceneFile);
                    return ExitCode::ErrorUsage;
                }
                if (featureLevel.has_value() && featureLevel.value() != fileFeatureLevel)
                {
                    LOG_ERROR(CONTEXT_CLIENT, "Feature Level of files to merge have to match. {} != {}", featureLevel.value(), fileFeatureLevel);
                    return ExitCode::ErrorUsage;
                }
                featureLevel = fileFeatureLevel;
            }
        }

        m_frameworkConfig.setFeatureLevel(featureLevel.value_or(EFeatureLevel::EFeatureLevel_Latest));
        m_framework = std::make_unique<ramses::RamsesFramework>(m_frameworkConfig);

        m_client = m_framework->createClient("ramses-viewer");
        if (!m_client)
        {
            LOG_ERROR(CONTEXT_CLIENT, "Creation of client failed");
            return ExitCode::ErrorClient;
        }

        if (!m_sceneFiles.empty())
        {
            auto sceneIt = m_sceneFiles.begin();
            auto const & sceneFile = *sceneIt;

            // scene viewer relies on resources being kept in memory (e.g. to query size),
            // load scene as 'remote' which uses a shadowcopy scene and guarantees to keep resources in memory
            m_scene = m_client->loadSceneFromFile(sceneFile, SceneConfig({}, EScenePublicationMode::LocalAndRemote));
            if (!m_scene)
            {
                LOG_ERROR(CONTEXT_CLIENT, "Loading scene failed! ({})", sceneFile);
                return ExitCode::ErrorScene;
            }

            while ((++sceneIt) != m_sceneFiles.end())
            {
                auto const & sceneFileMerge = *sceneIt;
                if (!m_client->mergeSceneFromFile(*m_scene, sceneFileMerge))
                {
                    LOG_ERROR(CONTEXT_CLIENT, "Merging scene failed! ({})", sceneFileMerge);
                    return ExitCode::ErrorScene;
                }
            }

            m_scene->publish();
            m_scene->flush();

            if (!m_noValidation)
            {
                m_scene->validate(m_validationReport);
                if (m_report->count() > 0)
                {
                    printValidationReport();
                }
            }
        }

        return ExitCode::Ok;
    }

    ViewerApp::ExitCode ViewerApp::createLogicViewer(LogicViewer::ScreenshotFunc&& fScreenshot)
    {
        m_logicViewer = std::make_unique<LogicViewer>(*m_scene, fScreenshot);
        if (!m_logicViewer->getLogicEngines().empty())
        {
            return runLogicCliCommands();
        }
        m_logicViewer.reset();
        return ExitCode::Ok;
    }

    ViewerApp::ExitCode ViewerApp::runLogicCliCommands()
    {
        m_interactive = false;
        assert(m_logicViewer);
        assert(!m_logicViewer->getLogicEngines().empty());
        if (m_luaFile.empty())
        {
            m_luaFile = fs::path(getSceneFile()).replace_extension("lua").string();
        }

        if (m_writeConfig)
        {
            ImGui::NewFrame();
            const auto status = m_logicViewer->saveDefaultLuaFile(m_luaFile, *m_settings);
            ImGui::EndFrame();
            if (!status.ok())
            {
                std::cerr << status.getMessage() << std::endl;
                return ExitCode::ErrorUnknown;
            }
        }
        else if (!m_luaFunction.empty())
        {
            auto loadLuaStatus = m_logicViewer->loadLuaFile(m_luaFile);

            if (loadLuaStatus.ok())
            {
                loadLuaStatus = m_logicViewer->call(m_luaFunction);
            }
            if (!loadLuaStatus.ok())
            {
                std::cerr << loadLuaStatus.getMessage() << std::endl;
                return ExitCode::ErrorLoadLua;
            }
        }
        else if (!m_luaExec.empty())
        {
            // default lua file may be missing (explicit lua file is checked by CLI11 before)
            auto loadLuaStatus = m_logicViewer->loadLuaFile(fs::exists(m_luaFile) ? m_luaFile : "");
            if (loadLuaStatus.ok())
            {
                loadLuaStatus = m_logicViewer->exec(m_luaExec);
            }
            if (!loadLuaStatus.ok())
            {
                std::cerr << loadLuaStatus.getMessage() << std::endl;
                return ExitCode::ErrorLoadLua;
            }
        }
        else
        {
            // interactive mode
            m_interactive = true;
            // default lua file may be missing (explicit lua file is checked by CLI11 before)
            if (fs::exists(m_luaFile))
            {
                m_loadLuaStatus = m_logicViewer->loadLuaFile(m_luaFile);
            }
        }
        return ExitCode::Ok;
    }

    void ViewerApp::printValidationReport() const
    {
        if (m_validationReportFile.empty())
        {
            printValidationReport(std::cout);
        }
        else
        {
            std::ofstream os(m_validationReportFile);
            printValidationReport(os);
        }
    }

    void ViewerApp::printValidationReport(std::ostream& os) const
    {
        os << m_validationReport.impl().toString() << std::endl;
        ramses::RamsesUtils::DumpUnrequiredSceneObjectsToFile(*m_scene, os);
    }
}
