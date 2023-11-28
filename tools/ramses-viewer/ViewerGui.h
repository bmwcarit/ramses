//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "SceneViewerGui.h"
#include "LogicViewerGui.h"
#include <cstdint>

namespace ramses
{
    class TextureSampler;
}

namespace ramses::internal
{
    class ViewerGuiApp;
    struct ViewerSettings;

    class ViewerGui
    {
    public:
        explicit ViewerGui(ViewerGuiApp& app);
        void draw();

        void setSceneTexture(ramses::TextureSampler* sampler, uint32_t width, uint32_t height);

        void setRendererInfo(ramses::RamsesRenderer& renderer, ramses::displayId_t displayId, ramses::displayBufferId_t displayBufferId);

        void openErrorPopup(const std::string& message);

    private:
        void drawWindow();

        void drawMenuBar();
        void drawMenuItemShowWindow();
        void drawMenuItemDisplaySettings();

        static bool DrawColorSetting(const char* name, glm::vec4& color);

        void drawErrorPopup();
        void drawProgressPopup();

        void zoomIn();
        void zoomOut();

        void drawSceneTexture();

        ViewerGuiApp& m_app;
        ViewerSettings& m_settings;

        std::unique_ptr<SceneViewerGui> m_sceneGui;
        std::unique_ptr<LogicViewerGui> m_logicGui;

        ramses::TextureSampler* m_sceneTexture = nullptr;
        ImVec2 m_sceneTextureSize;

        std::string m_lastErrorMessage;

        ramses::RamsesRenderer* m_renderer = nullptr;
        ramses::displayId_t m_displayId;
        ramses::displayBufferId_t m_displayBufferId;
        bool m_skipUnmodifiedBuffers = true;

        ProgressMonitor m_progress;
    };
}

