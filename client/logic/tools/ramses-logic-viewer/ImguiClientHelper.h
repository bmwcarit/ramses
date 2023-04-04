//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ImguiWrapper.h"
#include "ramses-client.h"
#include "ramses-utils.h"
#include "ramses-renderer-api/RamsesRenderer.h"
#include "ramses-renderer-api/RendererSceneControl.h"
#include "ramses-renderer-api/IRendererEventHandler.h"
#include "ramses-renderer-api/IRendererSceneControlEventHandler.h"

#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <thread>

namespace rlogic
{
    class ImguiClientHelper : public ramses::RendererEventHandlerEmpty, public ramses::RendererSceneControlEventHandlerEmpty
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

        void setRenderer(ramses::RamsesRenderer* renderer);

        void dispatchEvents();

        void dispatchClickEvent(std::pair<uint32_t, uint32_t>& clickEventOut);

        void draw();

        bool isRunning() const;
        ramses::Scene* getScene();
        uint32_t getViewportWidth() const;
        uint32_t getViewportHeight() const;

        bool saveScreenshot(const std::string& filename);
        bool saveScreenshot(const std::string& filename, ramses::displayBufferId_t screenshotBuf, uint32_t x, uint32_t y, uint32_t width, uint32_t height);

        bool waitForDisplay(ramses::displayId_t displayId);
        bool waitForSceneState(ramses::sceneId_t sceneId, ramses::RendererSceneState state);
        bool waitForSceneVersion(ramses::sceneId_t sceneId, ramses::sceneVersionTag_t version);
        bool waitForOffscreenBufferCreated(const ramses::displayBufferId_t offscreenBufferId);
        bool waitForOffscreenBufferLinked(const ramses::sceneId_t sceneId);
        bool waitForScreenshot();

        // SceneControlEvents
        void sceneStateChanged(ramses::sceneId_t sceneId, ramses::RendererSceneState state) override;
        void sceneFlushed(ramses::sceneId_t sceneId, ramses::sceneVersionTag_t sceneVersion) override;
        void offscreenBufferCreated(ramses::displayId_t displayId_t, ramses::displayBufferId_t offscreenBufferId, ramses::ERendererEventResult result) override;
        void offscreenBufferLinked(ramses::displayBufferId_t offscreenBufferId, ramses::sceneId_t consumerScene, ramses::dataConsumerId_t consumerId, bool success) override;

        // Renderer events
        void displayCreated(ramses::displayId_t displayId, ramses::ERendererEventResult result) override;
        void displayDestroyed(ramses::displayId_t displayId, ramses::ERendererEventResult result) override;
        void mouseEvent(ramses::displayId_t displayId, ramses::EMouseEvent eventType, int32_t mousePosX, int32_t mousePosY) override;
        void keyEvent(ramses::displayId_t displayId, ramses::EKeyEvent eventType, uint32_t keyModifiers, ramses::EKeyCode keyCode) override;
        void windowResized(ramses::displayId_t displayId, uint32_t width, uint32_t height) override;
        void windowClosed(ramses::displayId_t displayId) override;
        void framebufferPixelsRead(const uint8_t*               pixelData,
                                   const uint32_t               pixelDataSize,
                                   ramses::displayId_t          displayId,
                                   ramses::displayBufferId_t    displayBuffer,
                                   ramses::ERendererEventResult result) override;

    private:
        bool waitUntil(const std::function<bool()>& conditionFunction);

        struct SceneInfo
        {
            ramses::RendererSceneState state   = ramses::RendererSceneState::Unavailable;
            ramses::sceneVersionTag_t  version = ramses::InvalidSceneVersionTag;
        };

        using SceneSet = std::unordered_map<ramses::sceneId_t, SceneInfo>;
        using OffscreenBufferSet = std::unordered_set<ramses::displayBufferId_t>;
        using DisplaySet = std::unordered_set<ramses::displayId_t>;

        SceneSet m_scenes;
        SceneSet m_scenesAssignedToOffscreenBuffer;
        SceneSet m_scenesConsumingOffscreenBuffer;
        DisplaySet m_displays;
        OffscreenBufferSet m_offscreenBuffers;
        ramses::RamsesRenderer* m_renderer = nullptr;
        ramses::displayId_t m_displayId;
        ramses::Scene* m_imguiscene = nullptr;
        ramses::OrthographicCamera* imguicamera = nullptr;
        ramses::TextureSampler* sampler = nullptr;
        ramses::Effect* effect = nullptr;
        ramses::RenderGroup* renderGroup = nullptr;
        ramses::UniformInput textureInput;
        ramses::AttributeInput inputPosition;
        ramses::AttributeInput inputUV;
        ramses::AttributeInput inputColor;
        std::vector<ramses::SceneObject*> todeleteMeshes;
        std::vector<ramses::Resource*> todeleteRes;
        std::pair<uint32_t, uint32_t> m_clickEvent;
        std::string m_screenshot;
        uint32_t m_screenshotWidth = 0U;
        uint32_t m_screenshotHeight = 0U;
        bool m_screenshotSaved  = false;
        bool m_isRunning = true;
    };

    inline ramses::Scene* ImguiClientHelper::getScene()
    {
        return m_imguiscene;
    }

    inline bool ImguiClientHelper::isRunning() const
    {
        return m_isRunning;
    }

    inline void ImguiClientHelper::setRenderer(ramses::RamsesRenderer* renderer)
    {
        m_renderer = renderer;
    }

    inline uint32_t ImguiClientHelper::getViewportWidth() const
    {
        return imguicamera->getViewportWidth();
    }

    inline uint32_t ImguiClientHelper::getViewportHeight() const
    {
        return imguicamera->getViewportHeight();
    }
}
