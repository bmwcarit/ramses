//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "LogicViewerApp.h"

#include "Arguments.h"
#include "LogicViewer.h"
#include "LogicViewerSettings.h"
#include "ImguiWrapper.h"

namespace ramses
{
    LogicViewerApp::LogicViewerApp()
    {
        m_imguiContext = ImGui::CreateContext();
    }

    LogicViewerApp::~LogicViewerApp()
    {
        ImGui::DestroyContext(m_imguiContext);
    }

    int LogicViewerApp::GetFeatureLevelFromFile(const std::string& sceneFilename, EFeatureLevel& featureLevel)
    {
        if (!RamsesClient::GetFeatureLevelFromFile(sceneFilename, featureLevel))
        {
            std::cerr << "Could not parse feature level from scene file " << sceneFilename << std::endl;
            return static_cast<int>(ExitCode::ErrorLoadScene);
        }
        return 0;
    }

    int LogicViewerApp::loadScene(const Arguments& args, EFeatureLevel featureLevel)
    {
        ramses::RamsesFrameworkConfig frameworkConfig{featureLevel};
        frameworkConfig.setPeriodicLogInterval(std::chrono::seconds(0));
        frameworkConfig.setLogLevelConsole(args.ramsesLogLevel());
        m_framework = std::make_unique<ramses::RamsesFramework>(frameworkConfig);
        m_client = m_framework->createClient("ramses-logic-viewer");
        if (!m_client)
        {
            std::cerr << "Could not create ramses client" << std::endl;
            return static_cast<int>(ExitCode::ErrorRamsesClient);
        }

        m_scene = m_client->loadSceneFromFile(args.sceneFile());
        if (!m_scene)
        {
            std::cerr << "Failed to load scene: " << args.sceneFile() << std::endl;
            return static_cast<int>(ExitCode::ErrorLoadScene);
        }
        m_scene->publish();
        m_scene->flush();

        m_settings = std::make_unique<ramses::LogicViewerSettings>();
        return 0;
    }

    int LogicViewerApp::createViewer(const Arguments& args, LogicViewer::ScreenshotFunc&& fScreenshot)
    {
        assert(m_scene);

        // find logic engine
        SceneObjectIterator iter{*m_scene, ERamsesObjectType::LogicEngine};

        m_logic = object_cast<LogicEngine*>(iter.getNext());
        if (!m_logic)
        {
            std::cerr << "Failed to find logic" << std::endl;
            return static_cast<int>(ExitCode::ErrorLoadLogic);
        }

        m_viewer = std::make_unique<ramses::LogicViewer>(*m_logic, fScreenshot);

        if (args.writeConfig())
        {
            ImGui::NewFrame();
            const auto status = m_viewer->saveDefaultLuaFile(args.luaFile(), *m_settings);
            ImGui::EndFrame();
            if (!status.ok())
            {
                std::cerr << status.getMessage() << std::endl;
                return static_cast<int>(ExitCode::ErrorUnknown);
            }
        }
        else if (!args.luaFunction().empty())
        {
            m_loadLuaStatus = m_viewer->loadLuaFile(args.luaFile());

            if (m_loadLuaStatus.ok())
            {
                m_loadLuaStatus = m_viewer->call(args.luaFunction());
            }
            if (!m_loadLuaStatus.ok())
            {
                std::cerr << m_loadLuaStatus.getMessage() << std::endl;
                return static_cast<int>(ExitCode::ErrorLoadLua);
            }
        }
        else if (!args.exec().empty())
        {
            // default lua file may be missing (explicit lua file is checked by CLI11 before)
            m_loadLuaStatus = m_viewer->loadLuaFile(fs::exists(args.luaFile()) ? args.luaFile() : "");
            if (m_loadLuaStatus.ok())
            {
                m_loadLuaStatus = m_viewer->exec(args.exec());
            }
            if (!m_loadLuaStatus.ok())
            {
                std::cerr << m_loadLuaStatus.getMessage() << std::endl;
                return static_cast<int>(ExitCode::ErrorLoadLua);
            }
        }
        else
        {
            // interactive mode
            m_interactive = true;
            // default lua file may be missing (explicit lua file is checked by CLI11 before)
            if (fs::exists(args.luaFile()))
            {
                m_loadLuaStatus = m_viewer->loadLuaFile(args.luaFile());
            }
        }
        return 0;
    }
}

