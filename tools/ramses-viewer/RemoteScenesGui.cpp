//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RemoteScenesGui.h"
#include "ImguiWrapper.h"
#include "ViewerGuiApp.h"

namespace ramses::internal
{
    RemoteScenesGui::RemoteScenesGui(ViewerGuiApp& app)
        : m_rendererControl(app.getRendererControl())
        , m_imguiScene(app.getImguiClientHelper()->getScene()->getSceneId())
    {
        if (app.getScene())
            m_loadedScene = app.getScene()->getSceneId();
    }

    void RemoteScenesGui::drawContents(bool open)
    {
        const auto& sceneSet = m_rendererControl->getRendererScenes();
        if (m_loadedScene.isValid() && sceneSet.size() == 2)
            return; // no remote scenes - simplify ui

        if (ImGui::CollapsingHeader("Remote Scenes", open ? ImGuiTreeNodeFlags_DefaultOpen : 0))
        {
            int availableScenes = 0;
            for (auto& it : sceneSet)
            {
                if (it.first == m_imguiScene)
                    continue;
                if (it.second.state == RendererSceneState::Unavailable)
                    continue;
                ++availableScenes;
                ImGui::PushID(&it.second);
                if (it.first == m_loadedScene)
                {
                    ImGui::TextUnformatted(fmt::format("Scene:{} (local scene)", it.first, it.second.buffer.getValue()).c_str());
                }
                else
                {
                    ImGui::TextUnformatted(fmt::format("Scene:{}", it.first).c_str());
                }
                auto       newState           = it.second.state;
                int        renderOrder        = it.second.renderOrder;

                bool renderOrderChanged = false;
                if (!it.second.buffer.isValid())
                {
                    renderOrderChanged = ImGui::DragInt("Render order", &renderOrder, 1.f, 0, 255);
                    ImGui::SameLine();
                    imgui::HelpMarker("Changing the render order may not have a visual effect, if the remote scenes use depth testing.");
                }
                else
                {
                    ImGui::TextUnformatted(fmt::format("Offscreen buffer: {}", it.second.buffer.getValue()).c_str());
                    ImGui::SameLine();
                    imgui::HelpMarker("The offscreen buffer is part of the gui scene, i.e. it will cover the other scenes (even if the scene is in Ready or Available state).");
                }

                if (ImGui::RadioButton("Available", it.second.state == RendererSceneState::Available))
                    newState = RendererSceneState::Available;
                if (ImGui::RadioButton("Ready", it.second.state == RendererSceneState::Ready))
                    newState = RendererSceneState::Ready;
                if (ImGui::RadioButton("Rendered", it.second.state == RendererSceneState::Rendered))
                    newState = RendererSceneState::Rendered;

                if (renderOrderChanged || newState != it.second.state)
                    m_rendererControl->setupSceneState(it.first, newState, renderOrder);
                ImGui::PopID();
            }
            if (availableScenes == 0)
                ImGui::TextWrapped("No remote scenes available.");
        }
    }
}
