//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ViewerGuiApp.h"
#include "fmt/format.h"
#include "ramses-cli.h"
#include "ramses/client/ramses-client.h"
#include "ramses/client/ramses-utils.h"
#include "ramses/renderer/RamsesRenderer.h"
#include "ramses/renderer/IRendererSceneControlEventHandler.h"
#include "impl/DisplayConfigImpl.h"
#include "internal/PlatformAbstraction/PlatformThread.h"
#include "internal/Core/Utils/LogMacros.h"
#include <map>
#include "roboto.h"

namespace ramses::internal
{
    namespace
    {
        void SetPreferredSize(ramses::DisplayConfig& cfg, const ramses::Scene* scene)
        {
            if (!scene)
                return;
            ramses::SceneObjectIterator it(*scene, ramses::ERamsesObjectType::RenderPass);
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
        m_displayConfig.setWindowTitle("RAMSES Viewer");

        auto* fontAtlas = ImGui::GetIO().Fonts;
        fontAtlas->AddFontDefault();
        for (int fontSize = 16; fontSize <= 22; fontSize += 2)
        {
            ImFontConfig fontConfig;
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) no suitable replacement
            snprintf(fontConfig.Name, sizeof(fontConfig.Name), "Roboto Regular, %d px", fontSize);
            fontAtlas->AddFontFromMemoryCompressedTTF(roboto_ttf_compressed_data, roboto_ttf_compressed_size, static_cast<float>(fontSize), &fontConfig);
        }

        if (getSettings()->font < fontAtlas->Fonts.Size)
            ImGui::GetIO().FontDefault = fontAtlas->Fonts[getSettings()->font];
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

        if (!getSceneFiles().empty())
        {
            const auto& sceneFile = getSceneFile();
            if (!sceneFile.empty())
            {

                m_displayConfig.setWindowTitle(fmt::format("{} - RAMSES Viewer", fmt::join(getSceneFiles(), " + ")));
            }
        }

        const bool customWidth = m_width ? (m_width->count() > 0) : false;
        const bool customHeight = m_height ? (m_height->count() > 0) : false;
        if (!customHeight && !customWidth)
            SetPreferredSize(m_displayConfig, getScene());
        int32_t winX = 0;
        int32_t winY = 0;
        uint32_t winWidth = 0u;
        uint32_t winHeight = 0u;
        m_displayConfig.getWindowRectangle(winX, winY, winWidth, winHeight);

        // avoid sceneId collision
        const auto imguiSceneId = getScene() ? ramses::sceneId_t(getScene()->getSceneId().getValue() + 1) : ramses::sceneId_t(std::numeric_limits<sceneId_t::BaseType>::max() - 1);
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

        m_streamViewer = std::make_unique<StreamViewer>(*m_imguiHelper, *renderer, displayId);
        m_streamViewer->setAutoShow(getScene() == nullptr);
        m_rendererControl = std::make_unique<RendererControl>(renderer, displayId, winWidth, winHeight, m_imguiHelper.get(), m_streamViewer.get());
        m_rendererControl->setAutoShow(getScene() == nullptr);
        m_imguiHelper->setDisplayId(displayId);

        switch (getScene() ? m_guiMode : GuiMode::Only)
        {
        case GuiMode::On:
            m_sceneSetup = std::make_unique<OffscreenSetup>(*m_rendererControl, m_imguiHelper->getScene(), getScene());
            break;
        case GuiMode::Only:
            m_sceneSetup = std::make_unique<FramebufferSetup>(*m_rendererControl, m_imguiHelper->getScene(), nullptr);
            break;
        case GuiMode::Overlay:
        case GuiMode::Off:
            m_sceneSetup = std::make_unique<FramebufferSetup>(*m_rendererControl, m_imguiHelper->getScene(), getScene());
            break;
        }

        if (!m_rendererControl->waitForDisplay(displayId))
        {
            return ExitCode::ErrorDisplay;
        }

        getSettings()->clearColor = m_displayConfig.impl().getInternalDisplayConfig().getClearColor();
        renderer->setDisplayBufferClearColor(displayId, m_sceneSetup->getOffscreenBuffer(), getSettings()->clearColor);
        m_sceneSetup->apply();

        if (getScene())
        {
            auto takeScreenshot = [&](const std::string& filename) {
                static ramses::sceneVersionTag_t ver = 42;
                ++ver;
                getScene()->flush(ver);
                if (m_rendererControl->waitForSceneVersion(getScene()->getSceneId(), ver))
                {
                    if (m_rendererControl->saveScreenshot(filename, m_sceneSetup->getOffscreenBuffer(), 0, 0, m_sceneSetup->getWidth(), m_sceneSetup->getHeight()))
                    {
                        if (m_rendererControl->waitForScreenshot())
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
        }

        m_gui = std::make_unique<ViewerGui>(*this);
        m_gui->setSceneTexture(m_sceneSetup->getTextureSampler(), winWidth, winHeight);
        m_gui->setRendererInfo(*renderer, displayId, m_sceneSetup->getOffscreenBuffer());

        if (!m_screenshotFile.empty())
        {
            if (!m_rendererControl->saveScreenshot(m_screenshotFile, m_sceneSetup->getOffscreenBuffer(), 0, 0, winWidth, winHeight))
            {
                LOG_ERROR(CONTEXT_CLIENT, "Failure when saving screenshot");
                return ExitCode::ErrorScreenshot;
            }
            if (!m_rendererControl->waitForScreenshot())
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
        const bool isRunning = isInteractive() && m_rendererControl->isRunning();
        if (!isRunning)
            return false;

        Result updateStatus;
        if (getLogicViewer())
            updateStatus = getLogicViewer()->update();
        if (getScene())
            getScene()->flush();
        m_rendererControl->dispatchEvents();
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
