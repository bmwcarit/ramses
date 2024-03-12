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

#include <fcntl.h>
#if _WIN32
#  include <io.h>
#else
#  include <unistd.h>
#endif

namespace
{
    inline int OpenFileDescriptor(const char* path)
    {
#if _WIN32
        return ::open(path, O_RDONLY | O_BINARY);
#else
        return ::open(path, O_RDONLY);
#endif
    }
}

namespace ramses_internal
{
    std::istream& operator>>(std::istream& is, SceneViewerDcsmHandler::Config& cfg)
    {
        uint64_t tmp;
        char     separator;
        is >> tmp >> separator;
        cfg.content = ramses::ContentID(tmp);
        is >> tmp >> separator;
        cfg.category = ramses::Category(tmp);
        is >> tmp;
        cfg.scene = ramses::sceneId_t(tmp);
        return is;
    }

    namespace
    {
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

    SceneViewer::SceneViewer(int argc, char* argv[])
        : m_cli("Ramses SceneViewer")
        , m_frameworkConfig(argc, argv)
        , m_rendererConfig(argc, argv)
        , m_displayConfig(argc, argv)
    {
        registerOptions(m_cli);

        m_frameworkConfig.setPeriodicLogsEnabled(false);
        m_frameworkConfig.setRequestedRamsesShellType(ramses::ERamsesShellType_Console);
        m_displayConfig.setResizable(true);

        try
        {
            m_cli.parse(argc, argv);
        }
        catch (const CLI::ParseError& e)
        {
            const auto err = m_cli.exit(e);
            exit(err);
        }
    }

    void SceneViewer::registerOptions(CLI::App& cli)
    {
        const std::map<std::string, GuiMode> modes{{"on", GuiMode::On}, {"overlay", GuiMode::Overlay}, {"off", GuiMode::Off}};
        cli.add_option("--width");
        cli.add_option("--height");
        cli.add_flag("-f,--fullscreen", "Opens a fullscreen window");
        cli.add_option("-s,scenefile", m_scenePath, "RAMSES scene file")->check(CLI::ExistingFile)->required();
        cli.add_option("--sceneid", m_sceneId.getReference(), "Overrides the stored scene id in the loaded scene file");
        cli.add_option("--gui", m_guiMode, "inspection gui display mode")->transform(CLI::CheckedTransformer(modes))->default_val(m_guiMode);
        cli.add_option("--dcsm", m_dcsmConfig, "Offer scene(s) on DCSM. Format: ContentID,CategoryID,SceneID")->type_name("DCSM");
        cli.add_flag("-n,--no-validation", m_noValidation, "disable scene validation (faster startup for complex scenes)");
        cli.add_option("--output-dir", m_validationOutputDirectory, "directory path were validation output should be saved")->type_name("DIR");
        cli.add_option("-x,--screenshot", m_screenshotFile, "screenshot filename. Renders the scene to a screenshot and exits.")->excludes("--fullscreen")->type_name("FILE");
        cli.add_flag("--no-skub", m_noSkub, "disable skub (skip unmodified buffers), re-render unmodified scene");
        cli.add_option_function<uint32_t>(
            "--ivi-surface", [&](auto value) { m_displayConfig.setWaylandIviSurfaceID(ramses::waylandIviSurfaceId_t(value)); }, "Wayland-IVI surface");
        cli.add_option_function<uint32_t>(
            "--ivi-layer", [&](auto value) { m_displayConfig.setWaylandIviLayerID(ramses::waylandIviLayerId_t(value)); }, "Wayland-IVI layer");
    }

    int SceneViewer::run()
    {
        const File sceneFile(m_scenePath);
        m_sceneName = sceneFile.getFileName().stdRef();
        return loadAndRenderScene(m_scenePath);
    }

    SceneViewer::GuiMode SceneViewer::getGuiMode() const
    {
        if (!m_screenshotFile.empty())
        {
            return GuiMode::Off;
        }
        return m_guiMode;
    }

    ramses::Scene* SceneViewer::loadScene(ramses::RamsesClient &client, const String& filename)
    {
        size_t fileSize = 0;
        {
            ramses_internal::File inFile(filename);
            if (!inFile.getSizeInBytes(fileSize))
                return nullptr;
        }

        const auto fd = OpenFileDescriptor(filename.c_str());
        if (m_sceneId.isValid())
        {
            return client.loadSceneFromFileDescriptor(m_sceneId, fd, 0, fileSize);
        }
        else
        {
            return client.loadSceneFromFileDescriptor(fd, 0, fileSize);
        }
    }

    int SceneViewer::loadAndRenderScene(const String& sceneFile)
    {
        const GuiMode guiMode = getGuiMode();
        ramses::RamsesFramework framework(m_frameworkConfig);

        SceneViewerDcsmHandler dcsm(*framework.createDcsmProvider(), m_dcsmConfig);

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
        auto loadedScene = loadScene(*client, sceneFile);
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

        const auto customWidth = m_cli.get_option("--width")->count();
        const auto customHeight = m_cli.get_option("--height")->count();
        if (!customWidth && !customHeight)
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
        switch (guiMode)
        {
        case GuiMode::On:
            sceneSetup = std::make_unique<OffscreenSetup>(imguiHelper, renderer, loadedScene, displayId, winWidth, winHeight);
            break;
        case GuiMode::Only:
            sceneSetup = std::make_unique<FramebufferSetup>(imguiHelper, renderer, nullptr, displayId);
            break;
        case GuiMode::Overlay:
        case GuiMode::Off:
        case GuiMode::Invalid:
            sceneSetup = std::make_unique<FramebufferSetup>(imguiHelper, renderer, loadedScene, displayId);
            break;
        }
        sceneSetup->apply();

        SceneViewerGui gui(*loadedScene, sceneFile.stdRef(), imguiHelper);
        gui.setSceneTexture(sceneSetup->getTextureSampler(), winWidth, winHeight);

        if (!m_screenshotFile.empty())
        {
            if (!imguiHelper.saveScreenshot(m_screenshotFile.stdRef(), sceneSetup->getOffscreenBuffer(), 0, 0, winWidth, winHeight))
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
                dcsm.dispatchEvents();
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

        if (!m_validationOutputDirectory.empty())
        {
            // dump scene verification
            const std::string basePath = m_validationOutputDirectory;
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
