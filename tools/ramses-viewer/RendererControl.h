//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/renderer/RamsesRenderer.h"
#include "ramses/renderer/RendererSceneControl.h"
#include "ramses/renderer/IRendererEventHandler.h"
#include "ramses/renderer/IRendererSceneControlEventHandler.h"

#include <unordered_set>

namespace ramses
{
    class Scene;
}

namespace ramses::internal
{
    class RendererControl : public RendererEventHandlerEmpty, public RendererSceneControlEventHandlerEmpty
    {
    public:
        RendererControl(RamsesRenderer* renderer, displayId_t display, uint32_t displayWidth, uint32_t displayHeight,
                        IRendererEventHandler* rendererEventHandler = nullptr,
                        IRendererSceneControlEventHandler* sceneControlEventHandler = nullptr);

        void setupSceneState(sceneId_t sceneId, RendererSceneState state, displayBufferId_t buffer, int32_t renderOrder);

        void setupSceneState(sceneId_t sceneId, RendererSceneState state, int32_t renderOrder);

        void setAutoShow(bool autoShow)
        {
            m_autoShow = autoShow;
        }

        void dispatchEvents();

        struct SceneInfo
        {
            ramses::RendererSceneState state   = ramses::RendererSceneState::Unavailable;
            ramses::sceneVersionTag_t  version = ramses::InvalidSceneVersionTag;
            ramses::displayBufferId_t  buffer = {};
            int32_t                    renderOrder = 0;
            bool                       configuredManually = false; // won't be auto-shown if true
        };

        using SceneSet = std::unordered_map<ramses::sceneId_t, SceneInfo>;

        const SceneSet& getRendererScenes() const
        {
            return m_scenes;
        }

        RamsesRenderer* getRenderer()
        {
            return m_renderer;
        }

        displayId_t getDisplayId() const
        {
            return m_displayId;
        }

        uint32_t getDisplayWidth() const
        {
            return m_displayWidth;
        }

        uint32_t getDisplayHeight() const
        {
            return m_displayHeight;
        }

        [[nodiscard]] bool isRunning() const
        {
            return m_isRunning;
        }

        bool saveScreenshot(const std::string& filename);
        bool saveScreenshot(const std::string& filename, ramses::displayBufferId_t screenshotBuf, uint32_t x, uint32_t y, uint32_t width, uint32_t height);

        bool waitForDisplay(ramses::displayId_t displayId);
        bool waitForSceneState(ramses::Scene& scene, ramses::RendererSceneState state);
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
        void windowResized(ramses::displayId_t displayId, uint32_t width, uint32_t height) override;
        void windowClosed(ramses::displayId_t displayId) override;
        void framebufferPixelsRead(const uint8_t*               pixelData,
                                   const uint32_t               pixelDataSize,
                                   ramses::displayId_t          displayId,
                                   ramses::displayBufferId_t    displayBuffer,
                                   ramses::ERendererEventResult result) override;

    private:
        bool waitUntil(const std::function<bool()>& conditionFunction, const std::function<void()>& dispatch = {});

        using OffscreenBufferSet = std::unordered_set<ramses::displayBufferId_t>;
        using DisplaySet = std::unordered_set<ramses::displayId_t>;

        SceneSet                              m_scenes;
        SceneSet                              m_scenesAssignedToOffscreenBuffer;
        SceneSet                              m_scenesConsumingOffscreenBuffer;
        DisplaySet                            m_displays;
        OffscreenBufferSet                    m_offscreenBuffers;
        ramses::RamsesRenderer*               m_renderer;
        IRendererEventHandler*                m_rendererEventHandler;
        IRendererSceneControlEventHandler*    m_sceneControlEventHandler;
        ramses::displayId_t                   m_displayId;

        uint32_t m_displayWidth;
        uint32_t m_displayHeight;

        std::string                           m_screenshot;
        uint32_t                              m_screenshotWidth  = 0U;
        uint32_t                              m_screenshotHeight = 0U;
        bool                                  m_screenshotSaved  = false;
        bool                                  m_isRunning        = true;
        bool                                  m_autoShow         = true;
    };
}

