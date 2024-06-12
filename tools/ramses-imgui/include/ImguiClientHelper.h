//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#endif

#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif

#include "ramses/client/ramses-client.h"
#include "ramses/client/ramses-utils.h"
#include "ramses/renderer/IRendererEventHandler.h"

#include <vector>

namespace ramses::internal
{
    class ImguiClientHelper : public ramses::RendererEventHandlerEmpty
    {
    public:
        ImguiClientHelper(ramses::RamsesClient& client, uint32_t width, uint32_t height, ramses::sceneId_t sceneid);
        ImguiClientHelper(const ImguiClientHelper&) = delete;
        ImguiClientHelper& operator=(const ImguiClientHelper&) = delete;
        ~ImguiClientHelper() override;

        /**
         * @brief Used to filter input events for a certain display only
         * @param[in] displayId The display to receive events for - invalid displayId handles all events
         */
        void setDisplayId(ramses::displayId_t displayId);

        void draw();

        ramses::Scene* getScene();

        // Renderer events
        void mouseEvent(ramses::displayId_t displayId, ramses::EMouseEvent eventType, int32_t mousePosX, int32_t mousePosY) override;
        void keyEvent(ramses::displayId_t displayId, ramses::EKeyEvent eventType, ramses::KeyModifiers keyModifiers, ramses::EKeyCode keyCode) override;
        void windowResized(ramses::displayId_t displayId, uint32_t width, uint32_t height) override;

        std::pair<dataConsumerId_t, TextureSampler*> createTextureConsumer();

        ramses::RenderGroup* getBackgroundGroup();

    private:
        ramses::displayId_t                   m_displayId;
        ramses::Scene*                        m_imguiscene = nullptr;
        ramses::OrthographicCamera*           imguicamera  = nullptr;
        ramses::TextureSampler*               sampler      = nullptr;
        ramses::Effect*                       effect       = nullptr;
        ramses::RenderGroup*                  renderGroup  = nullptr;
        ramses::RenderGroup*                  m_backgroundGroup = nullptr;
        ramses::Texture2D*                    m_placeholderTexture = nullptr;
        uint32_t m_nextTextureConsumerId = 700;
        std::optional<ramses::UniformInput>   textureInput{std::nullopt};
        std::optional<ramses::AttributeInput> inputPosition{std::nullopt};
        std::optional<ramses::AttributeInput> inputUV{std::nullopt};
        std::optional<ramses::AttributeInput> inputColor{std::nullopt};
        std::vector<ramses::SceneObject*>     todeleteMeshes;
        std::vector<ramses::Resource*>        todeleteRes;
        ImGuiContext*                         m_context          = nullptr;
    };

    inline ramses::Scene* ImguiClientHelper::getScene()
    {
        return m_imguiscene;
    }

    inline void ImguiClientHelper::setDisplayId(ramses::displayId_t displayId)
    {
        m_displayId = displayId;
    }

    inline ramses::RenderGroup* ImguiClientHelper::getBackgroundGroup()
    {
        return m_backgroundGroup;
    }
}
