//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "LogicViewerGuiApp.h"

#include "Arguments.h"
#include "SceneSetup.h"
#include "ImguiClientHelper.h"
#include "LogicViewerGui.h"
#include "LogicViewer.h"
#include "LogicViewerSettings.h"
#include "ramses-logic/Logger.h"
#include "ramses-cli.h"

namespace ramses
{
    namespace
    {
        bool getPreferredSize(const ramses::Scene& scene, uint32_t& width, uint32_t& height)
        {
            ramses::SceneObjectIterator it(scene, ramses::ERamsesObjectType_RenderPass);
            ramses::RamsesObject*       obj = nullptr;
            while (nullptr != (obj = it.getNext()))
            {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
                auto* rp = static_cast<ramses::RenderPass*>(obj);
                if (!rp->getRenderTarget())
                {
                    auto* camera = rp->getCamera();
                    if (camera)
                    {
                        width = camera->getViewportWidth();
                        height = camera->getViewportHeight();
                        return true;
                    }
                }
            }
            return false;
        }
    } // namespace

    LogicViewerGuiApp::LogicViewerGuiApp(int argc, const char * const * argv)
    {
        m_exitCode = init(argc, argv);
    }

    LogicViewerGuiApp::~LogicViewerGuiApp() = default;

    int LogicViewerGuiApp::init(int argc, char const* const* argv)
    {
        if (argc == 0 || argv == nullptr)
        {
            return static_cast<int>(ExitCode::ErrorUnknown);
        }

        CLI::App                      cli;
        Arguments                     args;
        ramses::DisplayConfig  displayConfig;
        displayConfig.setResizable(true);

        try
        {
            args.registerOptions(cli);
            cli.add_flag("--headless", m_headless, "Runs without user interface and renderer. Disables screenshots.");
            auto setClearColor = [&](const auto& clearColor) { m_defaultClearColor = clearColor; };
            cli.add_option_function<std::array<float, 4u>>("--clear-color", setClearColor, "Background clear color as RGBA (ex. --clearColor 0 0.5 0.8 1");
            ramses::registerOptions(cli, displayConfig);
        }
        catch (const CLI::Error& err)
        {
            // catch configuration errors
            std::cerr << err.what() << std::endl;
            return -1;
        }

        CLI11_PARSE(cli, argc, argv);

        const bool customWidth = cli.get_option("--width")->count() > 0;
        const bool customHeight = cli.get_option("--width")->count() > 0;
        const bool autoDetectViewportSize = !customHeight && !customWidth;

        auto exitCode = LogicViewerApp::loadScene(args);
        if (exitCode != 0)
        {
            return exitCode;
        }

        const auto guiSceneId = ramses::sceneId_t(m_scene->getSceneId().getValue() + 1);

        if (autoDetectViewportSize && getPreferredSize(*m_scene, m_width, m_height))
        {
            displayConfig.setWindowRectangle(0, 0, m_width, m_height);
        }

        int32_t winX = 0;
        int32_t winY = 0;
        displayConfig.getWindowRectangle(winX, winY, m_width, m_height);
        m_imguiHelper = std::make_unique<ramses::ImguiClientHelper>(*m_client, m_width, m_height, guiSceneId);

        ramses::RamsesRenderer* renderer = nullptr;
        ramses::displayId_t displayId;
        if (!m_headless)
        {
            renderer = m_framework->createRenderer({});
            if (!renderer)
            {
                std::cerr << "Could not create ramses renderer" << std::endl;
                return static_cast<int>(ExitCode::ErrorRamsesRenderer);
            }

            displayId = initDisplay(args, *renderer, displayConfig);
            if (!displayId.isValid())
            {
                std::cerr << "Could not create ramses display" << std::endl;
                return static_cast<int>(ExitCode::ErrorNoDisplay);
            }
        }

        auto takeScreenshot = [&](const std::string& filename) {
            static ramses::sceneVersionTag_t ver = 42;
            ++ver;
            m_scene->flush(ver);
            if (m_imguiHelper->waitForSceneVersion(m_scene->getSceneId(), ver))
            {
                if (m_imguiHelper->saveScreenshot(filename, m_sceneSetup->getOffscreenBuffer(), 0, 0, m_sceneSetup->getWidth(), m_sceneSetup->getHeight()))
                {
                    if (m_imguiHelper->waitForScreenshot())
                    {
                        return true;
                    }
                }
            }
            return false;
        };

        if (m_headless)
        {
            exitCode = LogicViewerApp::createViewer(args, LogicViewer::ScreenshotFunc());
        }
        else
        {
            m_sceneSetup->apply();
            exitCode = LogicViewerApp::createViewer(args, takeScreenshot);
        }

        if (exitCode != 0)
        {
            return exitCode;
        }

        m_gui = std::make_unique<ramses::LogicViewerGui>(*m_viewer, *m_settings, args.luaFile());
        if (!m_headless)
        {
            m_gui->setSceneTexture(m_sceneSetup->getTextureSampler(), m_width, m_height);
            m_gui->setRendererInfo(*renderer, displayId, m_sceneSetup->getOffscreenBuffer(), m_defaultClearColor);
        }

        return 0;
    }

    ramses::displayId_t LogicViewerGuiApp::initDisplay(const Arguments& args, ramses::RamsesRenderer& renderer, const ramses::DisplayConfig& displayConfig)
    {
        renderer.startThread();
        m_imguiHelper->setRenderer(&renderer);

        const auto display = renderer.createDisplay(displayConfig);
        m_imguiHelper->setDisplayId(display);
        renderer.flush();

        if (!m_imguiHelper->waitForDisplay(display))
        {
            return {};
        }

        if (args.noOffscreen())
        {
            m_sceneSetup = std::make_unique<FramebufferSetup>(*m_imguiHelper, renderer, m_scene, display);
        }
        else
        {
            m_sceneSetup = std::make_unique<OffscreenSetup>(*m_imguiHelper, renderer, m_scene, display, m_width, m_height);
        }

        renderer.setDisplayBufferClearColor(display, m_sceneSetup->getOffscreenBuffer(), {m_defaultClearColor[0], m_defaultClearColor[1], m_defaultClearColor[2], m_defaultClearColor[3]});
        renderer.flush();

        return display;
    }

    bool LogicViewerGuiApp::doOneLoop()
    {
        const auto isRunning = m_interactive && (m_exitCode == 0) && m_imguiHelper && m_imguiHelper->isRunning();
        if (isRunning)
        {
            const auto updateStatus = m_viewer->update();
            m_scene->flush();
            m_imguiHelper->dispatchEvents();
            ImGui::NewFrame();
            m_gui->draw();
            if (!m_loadLuaStatus.ok())
            {
                m_gui->openErrorPopup(m_loadLuaStatus.getMessage());
                m_loadLuaStatus = ramses::Result();
            }
            if (!updateStatus.ok())
                m_gui->openErrorPopup(updateStatus.getMessage());
            ImGui::EndFrame();
            m_imguiHelper->draw();
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
        return isRunning;
    }
}

