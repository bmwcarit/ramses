//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ViewerGuiApp.h"
#include "ramses-cli.h"
#include "ramses/client/ramses-client.h"
#include "ramses/client/ramses-utils.h"
#include "ramses/renderer/RamsesRenderer.h"
#include "ramses/renderer/IRendererSceneControlEventHandler.h"
#include "impl/DisplayConfigImpl.h"
#include "internal/PlatformAbstraction/PlatformThread.h"
#include "internal/Core/Utils/LogMacros.h"
#include <map>

namespace ramses::internal
{
    namespace
    {
        void SetPreferredSize(ramses::DisplayConfig& cfg, const ramses::Scene& scene)
        {
            ramses::SceneObjectIterator it(scene, ramses::ERamsesObjectType::RenderPass);
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

    ViewerGuiApp::ViewerGuiApp()
    {
        m_displayConfig.setResizable(true);
    }

    void ViewerGuiApp::registerOptions(CLI::App& cli)
    {
        ViewerApp::registerOptions(cli);

        const std::map<std::string, GuiMode> modes{{"on", GuiMode::On}, {"overlay", GuiMode::Overlay}, {"off", GuiMode::Off}, {"only", GuiMode::Only}};
        auto gui = cli.add_option("--gui", m_guiMode, "Inspection Gui display mode")->transform(CLI::CheckedTransformer(modes));
        cli.add_flag("--no-skub", m_noSkub, "Disable skub (skip unmodified buffers). Render unchanged frames.");
        cli.add_flag("--headless", m_headless, "Runs without user interface and renderer. Disables screenshots.");
        auto screenshot = cli.add_option("-x,--screenshot", m_screenshotFile, "Stores a screenshot of the scene to the given filename and exits");
        ramses::registerOptions(cli, m_rendererConfig);
        ramses::registerOptions(cli, m_displayConfig);
        m_width = cli.get_option("--width");
        m_height = cli.get_option("--height");
        screenshot->excludes(gui);
    }

    ViewerGuiApp::ExitCode ViewerGuiApp::setup()
    {
        if (m_imguiHelper)
            return ExitCode::ErrorUsage; // double setup is not allowed

        auto exitCode = ViewerApp::loadScene();
        if (exitCode != ExitCode::Ok)
            return exitCode;

        const bool customWidth = m_width ? (m_width->count() > 0) : false;
        const bool customHeight = m_height ? (m_height->count() > 0) : false;
        if (!customHeight && !customWidth)
            SetPreferredSize(m_displayConfig, *getScene());
        int32_t winX = 0;
        int32_t winY = 0;
        uint32_t winWidth = 0u;
        uint32_t winHeight = 0u;
        m_displayConfig.getWindowRectangle(winX, winY, winWidth, winHeight);

        // avoid sceneId collision
        const auto imguiSceneId = ramses::sceneId_t(getScene()->getSceneId().getValue() + 1);
        m_imguiHelper = std::make_unique<ImguiClientHelper>(*getClient(), winWidth, winHeight, imguiSceneId);

        if (m_headless)
            return createLogicViewer(LogicViewer::ScreenshotFunc());

        auto renderer = getFramework()->createRenderer(m_rendererConfig);
        if (!renderer)
        {
            LOG_ERROR(CONTEXT_CLIENT, "Creation of renderer failed");
            return ExitCode::ErrorRenderer;
        }
        renderer->setSkippingOfUnmodifiedBuffers(!m_noSkub);
        renderer->startThread();

        getFramework()->connect();

        const ramses::displayId_t displayId = renderer->createDisplay(m_displayConfig);
        renderer->flush();

        m_imguiHelper->setDisplayId(displayId);
        m_imguiHelper->setRenderer(renderer);

        if (!m_imguiHelper->waitForDisplay(displayId))
        {
            return ExitCode::ErrorDisplay;
        }

        switch (m_guiMode)
        {
        case GuiMode::On:
            m_sceneSetup = std::make_unique<OffscreenSetup>(*m_imguiHelper, renderer, getScene(), displayId, winWidth, winHeight);
            break;
        case GuiMode::Only:
            m_sceneSetup = std::make_unique<FramebufferSetup>(*m_imguiHelper, renderer, nullptr, displayId);
            break;
        case GuiMode::Overlay:
        case GuiMode::Off:
            m_sceneSetup = std::make_unique<FramebufferSetup>(*m_imguiHelper, renderer, getScene(), displayId);
            break;
        }
        getScene()->flush();
        getSettings()->clearColor = m_displayConfig.impl().getInternalDisplayConfig().getClearColor();
        renderer->setDisplayBufferClearColor(displayId, m_sceneSetup->getOffscreenBuffer(), getSettings()->clearColor);
        m_sceneSetup->apply();

        auto takeScreenshot = [&](const std::string& filename) {
            static ramses::sceneVersionTag_t ver = 42;
            ++ver;
            getScene()->flush(ver);
            if (m_imguiHelper->waitForSceneVersion(getScene()->getSceneId(), ver))
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
        exitCode = createLogicViewer(takeScreenshot);
        if (exitCode != ExitCode::Ok)
            return exitCode;

        m_gui = std::make_unique<ViewerGui>(*this);
        m_gui->setSceneTexture(m_sceneSetup->getTextureSampler(), winWidth, winHeight);
        m_gui->setRendererInfo(*renderer, displayId, m_sceneSetup->getOffscreenBuffer());

        if (!m_screenshotFile.empty())
        {
            if (!m_imguiHelper->saveScreenshot(m_screenshotFile, m_sceneSetup->getOffscreenBuffer(), 0, 0, winWidth, winHeight))
            {
                LOG_ERROR(CONTEXT_CLIENT, "Failure when saving screenshot");
                return ExitCode::ErrorScreenshot;
            }
            if (!m_imguiHelper->waitForScreenshot())
            {
                LOG_ERROR(CONTEXT_CLIENT, "Screenshot not saved");
                return ExitCode::ErrorScreenshot;
            }
            setInteractive(false);
        }

        return ExitCode::Ok;
    }

    ViewerGuiApp::ExitCode ViewerGuiApp::run()
    {
        const auto exitCode = setup();
        if (exitCode != ExitCode::Ok)
            return exitCode;

        while (doOneLoop())
        {
            ramses::internal::PlatformThread::Sleep(40u);
        }
        return ExitCode::Ok;
    }

    bool ViewerGuiApp::doOneLoop()
    {
        const bool isRunning = isInteractive() && m_imguiHelper->isRunning();
        if (!isRunning)
            return false;

        Result updateStatus;
        if (getLogicViewer())
            updateStatus = getLogicViewer()->update();
        getScene()->flush();
        m_imguiHelper->dispatchEvents();
        if (m_guiMode != GuiMode::Off)
        {
            ImGui::NewFrame();
            m_gui->draw();
            auto loadLuaStatus = popLoadLuaStatus();
            if (!loadLuaStatus.ok())
                m_gui->openErrorPopup(loadLuaStatus.getMessage());
            if (!updateStatus.ok())
                m_gui->openErrorPopup(updateStatus.getMessage());
            ImGui::EndFrame();
            m_imguiHelper->draw();
        }
        return true;
    }
}
