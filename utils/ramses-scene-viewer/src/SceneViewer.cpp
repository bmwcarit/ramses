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

#include "PlatformAbstraction/PlatformThread.h"
#include "RendererLib/RendererConfigUtils.h"
#include "ramses-hmi-utils.h"
#include <fstream>
#include "Utils/Image.h"
#include "Utils/File.h"
#include "SceneSetup.h"


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

        void setPreferredSize(ramses::DisplayConfig& cfg, const ramses::Scene& scene, const std::vector<const char*>& args)
        {
            const auto custom = std::find_if(args.begin(), args.end(), [](const std::string& arg) { return arg == "-w" || arg == "-h"; });
            if (custom == args.end())
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
    }

    SceneViewer::SceneViewer(int argc, char* argv[])
        : m_parser(argc, argv)
        , m_helpArgument(m_parser, "help", "help", "Print this help")
        , m_scenePathAndFileArgument(m_parser, "s", "scene", String(), "Scene path+file")
        , m_noValidation(m_parser, "nv", "no-validation", "Disable scene validation")
        , m_validationUnrequiredObjectsDirectoryArgument(m_parser, "vd", "validation-output-directory", String(), "Directory Path were validation output should be saved")
        , m_screenshotFile(m_parser, "x", "screenshot-file", {}, "Screenshot filename. Setting to non-empty enables screenshot capturing after the scene is shown")
        , m_noSkub(m_parser, "ns", "no-skub", "Disable skub, render also when no changes")
        , m_guiModeArgument(m_parser, "gui", "gui", "on", "Inspection Gui display mode [off|on|overlay]")
        , m_frameworkConfig(argc, argv)
        , m_rendererConfig(argc, argv)
        , m_displayConfig(argc, argv)
        , m_args(argv, argv + argc)
    {
        GetRamsesLogger().initialize(m_parser, String(), String(), false, true);
        m_frameworkConfig.setPeriodicLogsEnabled(false);
        m_frameworkConfig.setRequestedRamsesShellType(ramses::ERamsesShellType_Console);
        m_displayConfig.setResizable(true);
    }

    int SceneViewer::run()
    {
        const bool helpRequested = m_helpArgument;
        String scenePathAndFile = m_scenePathAndFileArgument;

        if (helpRequested)
        {
            printUsage();
            return 0;
        }

        if (scenePathAndFile.empty())
        {
            LOG_ERROR(CONTEXT_CLIENT,
                      "A scene file including path has to be specified by option "
                      << m_scenePathAndFileArgument.getHelpString());
            return ErrorUsage;
        }

        if (m_screenshotFile.hasValue() && m_displayConfig.isWindowFullscreen())
        {
            LOG_ERROR(CONTEXT_CLIENT, "Screenshot in fullscreen mode is not supported");
            return ErrorUsage;
        }

        const File sceneFile(scenePathAndFile);
        if (!sceneFile.exists())
        {
            // try with extension
            scenePathAndFile += ".ramses";
        }

        m_sceneName = sceneFile.getFileName().stdRef();
        return loadAndRenderScene(scenePathAndFile);
    }

    void SceneViewer::printUsage() const
    {
        const std::string argumentHelpString = m_helpArgument.getHelpString() + m_scenePathAndFileArgument.getHelpString() + m_validationUnrequiredObjectsDirectoryArgument.getHelpString() + m_screenshotFile.getHelpString() + m_guiModeArgument.getHelpString();
        const String& programName = m_parser.getProgramName();
        LOG_INFO(CONTEXT_CLIENT,
                "\nUsage: " << programName << " [options] -s <sceneFileName>\n"
                "Loads and views a RAMSES scene from the files <sceneFileName>.ramses / <sceneFileName>.ramres\n"
                "Arguments:\n" << argumentHelpString);

        ramses_internal::RendererConfigUtils::PrintCommandLineOptions();
    }

    SceneViewer::GuiMode SceneViewer::getGuiMode() const
    {
        if (m_screenshotFile.hasValue())
        {
            return GuiMode::Off;
        }

        const String guiMode = m_guiModeArgument;
        GuiMode retval = GuiMode::Invalid;
        if (guiMode == "on")
            retval = GuiMode::On;
        else if (guiMode == "overlay")
            retval = GuiMode::Overlay;
        else if (guiMode == "off")
            retval = GuiMode::Off;
        else
            LOG_ERROR_P(CONTEXT_CLIENT, "Invalid Gui mode: {}", guiMode);
        return retval;
    }

    int SceneViewer::loadAndRenderScene(const String& sceneFile)
    {
        const GuiMode guiMode = getGuiMode();
        if (guiMode == GuiMode::Invalid)
        {
            return ErrorUsage;
        }
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

        setPreferredSize(m_displayConfig, *loadedScene, m_args);
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
        if (guiMode == GuiMode::On)
        {
            sceneSetup = std::make_unique<OffscreenSetup>(imguiHelper, renderer, loadedScene, displayId, winWidth, winHeight);
        }
        else
        {
            sceneSetup = std::make_unique<FramebufferSetup>(imguiHelper, renderer, loadedScene, displayId);
        }
        sceneSetup->apply();

        SceneViewerGui gui(*loadedScene, sceneFile.stdRef(), imguiHelper);
        gui.setSceneTexture(sceneSetup->getTextureSampler(), winWidth, winHeight);

        const String screenshotFile = m_screenshotFile;
        if (!screenshotFile.empty())
        {
            if (!imguiHelper.saveScreenshot(screenshotFile.stdRef(), sceneSetup->getOffscreenBuffer(), 0, 0, winWidth, winHeight))
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
                if (guiMode != GuiMode::Off)
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

        const auto guiMode = getGuiMode();
        const auto reportLevel = (guiMode == GuiMode::Off) ? ramses::EValidationSeverity_Info : ramses::EValidationSeverity_Warning;
        LOG_INFO(CONTEXT_CLIENT, "Scene validation report: " << scene.getValidationReport(reportLevel));

        if (m_validationUnrequiredObjectsDirectoryArgument.wasDefined() && m_validationUnrequiredObjectsDirectoryArgument.hasValue())
        {
            // dump scene verification
            const std::string basePath = ramses_internal::String(m_validationUnrequiredObjectsDirectoryArgument).c_str();
            const std::string validationFilePath = basePath + m_sceneName + "_validationReport.txt";
            std::ofstream validationFile(validationFilePath);
            validationFile << scene.getValidationReport(ramses::EValidationSeverity_Info) << std::endl;
            // dump unused objects
            const std::string unrequiredObjectsReportFilePath = basePath + m_sceneName + "_unrequObjsReport.txt";
            std::ofstream unrequObjsOfstream(unrequiredObjectsReportFilePath);
            ramses::RamsesHMIUtils::DumpUnrequiredSceneObjectsToFile(scene, unrequObjsOfstream);
        }

        if (guiMode == GuiMode::Off)
        {
            ramses::RamsesHMIUtils::DumpUnrequiredSceneObjects(scene);
        }
    }
}
