//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ViewerGui.h"
#include "ViewerSettings.h"
#include "ViewerGuiApp.h"
#include "ImguiWrapper.h"
#include "glm/gtc/type_ptr.hpp"

namespace ramses::internal
{
    ViewerGui::ViewerGui(ViewerGuiApp& app)
        : m_app(app)
        , m_settings(*app.getSettings())
    {
        auto* scene = app.getScene();
        if (scene)
            m_sceneGui = std::make_unique<SceneViewerGui>(app, *app.getScene(), m_lastErrorMessage, m_progress);

        if (app.getLogicViewer())
            m_logicGui = std::make_unique<LogicViewerGui>(app, m_lastErrorMessage);
    }

    void ViewerGui::setRendererInfo(ramses::RamsesRenderer& renderer, ramses::displayId_t displayId, ramses::displayBufferId_t displayBufferId)
    {
        m_renderer = &renderer;
        m_displayId = displayId;
        m_displayBufferId = displayBufferId;
    }

    void ViewerGui::openErrorPopup(const std::string& message)
    {
        // ImGui::OpenPopup("Error") does not work in all cases (The calculated ID seems to be context related)
        // The popup will be opened in ViewerGui::draw() instead
        m_lastErrorMessage = message;
    }

    void ViewerGui::draw()
    {
        if (ImGui::IsKeyPressed(ramses::EKeyCode_F11))
        {
            m_settings.showWindow = !m_settings.showWindow;
        }

        if (ImGui::IsKeyPressed(ramses::EKeyCode_F10) && m_app.getLogicViewer())
        {
            m_settings.showLogicWindow = !m_settings.showLogicWindow;
        }

        if (m_sceneGui)
            m_sceneGui->handleShortcuts();
        if (m_logicGui)
            m_logicGui->handleShortcuts();

        if (ImGui::BeginPopupContextVoid("GlobalContextMenu"))
        {
            drawMenuItemShowWindow();
            if (m_sceneGui)
            {
                ImGui::Separator();
                m_sceneGui->drawGlobalContextMenuItems();
            }
            if (m_logicGui)
            {
                ImGui::Separator();
                m_logicGui->drawGlobalContextMenuItems();
            }
            ImGui::EndPopup();
        }

        drawSceneTexture();
        drawErrorPopup();
        drawProgressPopup();

        if (m_settings.showWindow)
            drawWindow();
        if (m_logicGui && m_settings.showLogicWindow)
            m_logicGui->drawWindow();
    }

    void ViewerGui::drawWindow()
    {
        const auto featureLevel = m_app.getScene()->getRamsesClient().getRamsesFramework().getFeatureLevel();
        const std::string windowTitle =
            m_app.getScene()->getName().empty()
                ? fmt::format("Scene[{}] (FeatureLevel 0{})", m_app.getScene()->getSceneId().getValue(), featureLevel)
                : fmt::format("Scene[{}]: {} (FeatureLevel 0{})", m_app.getScene()->getSceneId().getValue(), m_app.getScene()->getName(), featureLevel);

        if (!ImGui::Begin(windowTitle.c_str(), &m_settings.showWindow, ImGuiWindowFlags_MenuBar))
        {
            ImGui::End();
            return;
        }

        drawMenuBar();
        if (m_logicGui && !m_settings.showLogicWindow)
        {
            ImGui::SetNextItemOpen(true, ImGuiCond_Once);
            bool showInline = true;
            if (ImGui::CollapsingHeader("Logic", &showInline))
                m_logicGui->drawContents();
            m_settings.showLogicWindow = !showInline;
        }

        if (m_sceneGui)
            m_sceneGui->drawContents();
        ImGui::End();
    }

    void ViewerGui::setSceneTexture(ramses::TextureSampler* sampler, uint32_t width, uint32_t height)
    {
        m_sceneTexture       = sampler;
        m_sceneTextureSize.x = static_cast<float>(width);
        m_sceneTextureSize.y = static_cast<float>(height);
    }

    void ViewerGui::drawMenuBar()
    {
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (m_sceneGui)
                    m_sceneGui->drawFileMenuItems();
                if (m_logicGui)
                {
                    ImGui::Separator();
                    m_logicGui->drawMenuItemReload();
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Settings"))
            {
                drawMenuItemShowWindow();
                drawMenuItemDisplaySettings();
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
    }

    void ViewerGui::drawMenuItemShowWindow()
    {
        ImGui::MenuItem("Show Scene Window", "F11", &m_settings.showWindow);
        if (m_app.getLogicViewer())
            ImGui::MenuItem("Show Logic Window", "F10", &m_settings.showLogicWindow);
        if (m_app.getGuiMode() == ViewerGuiApp::GuiMode::On)
        {
            ImGui::MenuItem("Show Scene in Window", nullptr, &m_settings.showSceneInWindow);
        }
        if (m_app.getLogicViewer())
        {
            if (ImGui::MenuItem("Show LogicEngine Update Report", nullptr, &m_settings.showUpdateReport))
            {
                m_app.getLogicViewer()->enableUpdateReport(m_settings.showUpdateReport, m_settings.updateReportInterval);
            }
        }
    }

    void ViewerGui::drawMenuItemDisplaySettings()
    {
        if (m_renderer)
        {
            ImGui::Separator();
            if (DrawColorSetting("Clear color", m_settings.clearColor))
            {
                m_renderer->setDisplayBufferClearColor(m_displayId, m_displayBufferId, m_settings.clearColor);
                m_renderer->flush();
            }

            if (ImGui::MenuItem("Skip rendering of unmodified buffers", nullptr, &m_skipUnmodifiedBuffers))
            {
                m_renderer->setSkippingOfUnmodifiedBuffers(m_skipUnmodifiedBuffers);
                m_renderer->flush();
            }

            float fps = m_renderer->getFramerateLimit(m_displayId);
            if (ImGui::DragFloat("Maximum FPS", &fps, 1.f, 1.f, 1000.f))
            {
                m_renderer->setFramerateLimit(m_displayId, fps);
                m_renderer->flush();
            }

            ImGui::Separator();
            ImGui::MenuItem("Lua: prefer identifiers (scripts.foo)", nullptr, &m_settings.luaPreferIdentifiers);
            ImGui::MenuItem("Lua: prefer object ids (scripts[1])", nullptr, &m_settings.luaPreferObjectIds);
        }
    }

    bool ViewerGui::DrawColorSetting(const char* name, glm::vec4& color)
    {
        bool modified = false;
        const auto flags = ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel;
        if (ImGui::ColorEdit4(name, glm::value_ptr(color), flags))
        {
            modified = true;
        }
        ImGui::SameLine();
        ImGui::TextUnformatted(name);
        return modified;
    }

    void ViewerGui::drawErrorPopup()
    {
        if (!m_lastErrorMessage.empty())
            ImGui::OpenPopup("Error");

        if (ImGui::BeginPopupModal("Error", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::TextUnformatted(m_lastErrorMessage.c_str());
            ImGui::Separator();

            if (ImGui::Button("OK", ImVec2(120, 0)))
            {
                m_lastErrorMessage.clear();
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Copy Message", ImVec2(120, 0)))
            {
                ImGui::SetClipboardText(m_lastErrorMessage.c_str());
            }
            ImGui::EndPopup();
        }
    }

    void ViewerGui::drawProgressPopup()
    {
        if (m_progress.isRunning() && !m_progress.canceled)
        {
            ImGui::OpenPopup("Progress");
        }

        if (ImGui::BeginPopupModal("Progress", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            const uint32_t current = m_progress.current;
            ImGui::TextUnformatted(m_progress.getDescription().c_str());
            ImGui::TextUnformatted(fmt::format("{} of {}", current, m_progress.getTotal()).c_str());
            ImGui::Separator();
            if (ImGui::Button("Cancel", ImVec2(120,0)))
            {
                m_progress.canceled = true;
                ImGui::CloseCurrentPopup();
            }
            if (!m_progress.isRunning())
            {
                ImGui::CloseCurrentPopup();
                auto result = m_progress.getResult();
                if (!result.empty())
                {
                    m_lastErrorMessage = fmt::format("{}", fmt::join(result, "\n"));
                }
            }
            ImGui::EndPopup();
        }
    }

    void ViewerGui::zoomIn()
    {
        if (m_settings.zoomIx < (static_cast<int>(m_settings.zoomLevels.size()) - 1))
        {
            ++m_settings.zoomIx;
        }
    }

    void ViewerGui::zoomOut()
    {
        if (m_settings.zoomIx > 0)
        {
            --m_settings.zoomIx;
        }
    }

    void ViewerGui::drawSceneTexture()
    {
        if (m_sceneTexture)
        {
            if (m_settings.showSceneInWindow)
            {
                if (ImGui::Begin("Preview", &m_settings.showSceneInWindow, ImGuiWindowFlags_AlwaysAutoResize))
                {
                    if (ImGui::GetIO().KeyCtrl)
                    {
                        if (ImGui::GetIO().MouseWheel >= 1.f)
                        {
                            zoomIn();
                        }
                        if (ImGui::GetIO().MouseWheel <= -1.f)
                        {
                            zoomOut();
                        }
                    }
                    const auto f = m_settings.zoomLevels[m_settings.zoomIx];
                    if (ImGui::SmallButton("-"))
                        zoomOut();
                    ImGui::SameLine();
                    if (ImGui::SmallButton("+"))
                        zoomIn();
                    ImGui::SameLine();
                    ImGui::TextUnformatted(fmt::format("Zoom {}%", static_cast<int>(f * 100)).c_str());
                    ImVec2 size(m_sceneTextureSize.x * f, m_sceneTextureSize.y * f);
                    ImGui::Image(m_sceneTexture, size, ImVec2(0, 1), ImVec2(1, 0));
                    ImGui::End();
                }
                else
                {
                    ImGui::End();
                }
            }
            else
            {
                ImGui::GetBackgroundDrawList()->AddImage(m_sceneTexture, ImVec2(0, 0), m_sceneTextureSize, ImVec2(0, 1), ImVec2(1, 0));
            }
        }
    }

}
