//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  Copyright (C) 2019 Daniel Werner Lima Souza de Almeida (dwlsalmeida@gmail.com)
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "SceneViewer.h"
#include "SceneViewerGui.h"

#include "ramses-client.h"
#include "ramses-utils.h"

#include "Utils/Argument.h"
#include "Utils/LogMacros.h"
#include "Utils/RamsesLogger.h"
#include "ImguiClientHelper.h"

#include "ramses-renderer-api/RamsesRenderer.h"
#include "ramses-renderer-api/IRendererSceneControlEventHandler.h"
#include "ramses-framework-api/RamsesFramework.h"
#include "RamsesFrameworkConfigImpl.h"

#include "PlatformAbstraction/PlatformThread.h"
#include "ramses-hmi-utils.h"
#include <fstream>
#include "Utils/Image.h"
#include "Utils/File.h"
#include "SceneSetup.h"
#include "CLI/CLI.hpp"


namespace ramses_internal
{
    namespace
    {
        const int ErrorUsage      = 1;
        const int ErrorClient     = 2;
        const int ErrorRenderer   = 3;
        const int ErrorScene      = 4;
        const int ErrorScreenshot = 5;
        const int ErrorDisplay    = 6;

        void setPreferredSize(ramses::DisplayConfig& cfg, const ramses::Scene& scene)
        {
            ramses::SceneObjectIterator it(scene, ramses::ERamsesObjectType_RenderPass);
            ramses::RamsesObject*       obj = nullptr;
            while (nullptr != (obj = it.getNext()))
            {
                auto* rp = static_cast<ramses::RenderPass*>(obj);
                if (!rp->getRenderTarget())
                {
                    auto* camera = rp->getCamera();
                    if (camera)
                    {
                        const auto width  = camera->getViewportWidth();
                        const auto height = camera->getViewportHeight();
                        cfg.setWindowRectangle(0, 0, width, height);
                    }
                }
            }
        }
    }

    SceneViewer::SceneViewer()
    {
        // set default configuration
        m_frameworkConfig.setPeriodicLogsEnabled(false);
        m_frameworkConfig.setRequestedRamsesShellType(ramses::ERamsesShellType_Console);
        m_displayConfig.setResizable(true);
    }

    void SceneViewer::registerOptions(CLI::App& cli)
    {
        cli.description("Loads and views a RAMSES scene from file");
        const std::map<std::string, GuiMode> modes{{"on", GuiMode::On}, {"overlay", GuiMode::Overlay}, {"off", GuiMode::Off}, {"only", GuiMode::Only}};
        cli.add_option("-s,--scene,scene", m_sceneName, "Scene file to load")->check(CLI::ExistingFile)->required();
        auto gui = cli.add_option("--gui", m_guiMode, "Inspection Gui display mode")->transform(CLI::CheckedTransformer(modes));
        cli.add_flag("--no-validation", m_noValidation, "Disable scene validation (faster startup for complex scenes)");
        cli.add_option("--output", m_validationOutput, "Directory Path were validation output should be saved");
        auto screenshot = cli.add_option("-x,--screenshot", m_screenshotFile, "Stores a screenshot of the scene to the given filename and exits");
        cli.add_flag("--no-skub", m_noSkub, "Disable skub (skip unmodified buffers). Render unchanged frames.");

        m_frameworkConfig.registerOptions(cli);
        m_rendererConfig.registerOptions(cli);
        m_displayConfig.registerOptions(cli);
        m_width = cli.get_option("--width");
        m_height = cli.get_option("--height");
        screenshot->excludes("--fullscreen")->excludes(gui);
    }

    int SceneViewer::run()
    {
        GetRamsesLogger().initialize(m_frameworkConfig.impl.loggerConfig, false, true);
        const String scenePathAndFile = m_sceneName.c_str();
        const File sceneFile(scenePathAndFile);
        m_sceneName = sceneFile.getFileName().stdRef();
        return loadAndRenderScene(scenePathAndFile);
    }

    int SceneViewer::loadAndRenderScene(const String& sceneFile)
    {
        ramses::EFeatureLevel featureLevel = ramses::EFeatureLevel_01;
        if (!ramses::RamsesClient::GetFeatureLevelFromFile(sceneFile.c_str(), featureLevel))
            return ErrorUsage;

        m_frameworkConfig.setFeatureLevel(featureLevel);
        ramses::RamsesFramework framework(m_frameworkConfig);

        auto client = framework.createClient("ramses-scene-viewer");
        if (!client)
        {
            LOG_ERROR(CONTEXT_CLIENT, "Creation of client failed");
            return ErrorClient;
        }

        auto renderer = framework.createRenderer(m_rendererConfig);
        if (!renderer)
        {
            LOG_ERROR(CONTEXT_CLIENT, "Creation of renderer failed");
            return ErrorRenderer;
        }
        renderer->setSkippingOfUnmodifiedBuffers(!m_noSkub);
        renderer->startThread();
        framework.connect();

        LOG_INFO(CONTEXT_CLIENT, "Load scene:" << sceneFile);
        auto loadedScene = client->loadSceneFromFile(sceneFile.c_str());
        if (loadedScene == nullptr)
        {
            LOG_ERROR(CONTEXT_CLIENT, "Loading scene failed!");
            framework.disconnect();
            return ErrorScene;
        }
        loadedScene->publish();
        loadedScene->flush();
        if (!m_noValidation)
        {
            validateContent(*loadedScene);
        }

        const bool customWidth = m_width ? (m_width->count() > 0) : false;
        const bool customHeight = m_height ? (m_height->count() > 0) : false;
        if (!customHeight && !customWidth)
        {
            setPreferredSize(m_displayConfig, *loadedScene);
        }
        int32_t winX = 0;
        int32_t winY = 0;
        uint32_t winWidth = 0u;
        uint32_t winHeight = 0u;
        m_displayConfig.getWindowRectangle(winX, winY, winWidth, winHeight);
        const ramses::displayId_t displayId = renderer->createDisplay(m_displayConfig);
        renderer->flush();

        // avoid sceneId collision
        const auto imguiSceneId = ramses::sceneId_t(loadedScene->getSceneId().getValue() + 1);
        ImguiClientHelper imguiHelper(*client, winWidth, winHeight, imguiSceneId);
        imguiHelper.setDisplayId(displayId);
        imguiHelper.setRenderer(renderer);

        if (!imguiHelper.waitForDisplay(displayId))
        {
            return ErrorDisplay;
        }

        std::unique_ptr<ISceneSetup> sceneSetup;
        switch (m_guiMode)
        {
        case GuiMode::On:
            sceneSetup = std::make_unique<OffscreenSetup>(imguiHelper, renderer, loadedScene, displayId, winWidth, winHeight);
            break;
        case GuiMode::Only:
            sceneSetup = std::make_unique<FramebufferSetup>(imguiHelper, renderer, nullptr, displayId);
            break;
        case GuiMode::Overlay:
        case GuiMode::Off:
            sceneSetup = std::make_unique<FramebufferSetup>(imguiHelper, renderer, loadedScene, displayId);
            break;
        }
        sceneSetup->apply();

        SceneViewerGui gui(*loadedScene, sceneFile.stdRef(), imguiHelper);
        gui.setSceneTexture(sceneSetup->getTextureSampler(), winWidth, winHeight);

        if (!m_screenshotFile.empty())
        {
            if (!imguiHelper.saveScreenshot(m_screenshotFile, sceneSetup->getOffscreenBuffer(), 0, 0, winWidth, winHeight))
            {
                LOG_ERROR(CONTEXT_CLIENT, "Failure when saving screenshot");
                return ErrorScreenshot;
            }
            if (!imguiHelper.waitForScreenshot())
            {
                LOG_ERROR(CONTEXT_CLIENT, "Screenshot not saved");
                return ErrorScreenshot;
            }
        }
        else
        {
            // interactive mode
            while (imguiHelper.isRunning())
            {
                loadedScene->flush();
                imguiHelper.dispatchEvents();
                if (m_guiMode != GuiMode::Off)
                {
                    ImGui::NewFrame();
                    gui.draw();
                    // ImGui::ShowDemoWindow();
                    ImGui::EndFrame();
                    imguiHelper.draw();
                }
                ramses_internal::PlatformThread::Sleep(20u);
            }
        }
        return 0;
    }

    void SceneViewer::validateContent(const ramses::Scene& scene) const
    {
        ramses::status_t validateStatus = scene.validate();
        if (validateStatus != ramses::StatusOK)
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene validate failed: " << scene.getStatusMessage(validateStatus));
        }

        const auto reportLevel = (m_guiMode == GuiMode::Off) ? ramses::EValidationSeverity_Info : ramses::EValidationSeverity_Warning;
        LOG_INFO(CONTEXT_CLIENT, "Scene validation report: " << scene.getValidationReport(reportLevel));

        if (!m_validationOutput.empty())
        {
            // dump scene verification
            const std::string validationFilePath = m_validationOutput + m_sceneName + "_validationReport.txt";
            std::ofstream validationFile(validationFilePath);
            validationFile << scene.getValidationReport(ramses::EValidationSeverity_Info) << std::endl;
            // dump unused objects
            const std::string unrequiredObjectsReportFilePath = m_validationOutput + m_sceneName + "_unrequObjsReport.txt";
            std::ofstream unrequObjsOfstream(unrequiredObjectsReportFilePath);
            ramses::RamsesHMIUtils::DumpUnrequiredSceneObjectsToFile(scene, unrequObjsOfstream);
        }

        if (m_guiMode == GuiMode::Off)
        {
            ramses::RamsesHMIUtils::DumpUnrequiredSceneObjects(scene);
        }
    }
}
