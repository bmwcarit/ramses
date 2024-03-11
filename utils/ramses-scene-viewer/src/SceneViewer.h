//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENE_VIEWER_SCENEVIEWER_H
#define RAMSES_SCENE_VIEWER_SCENEVIEWER_H

#include "Collections/String.h"
#include "ramses-framework-api/RamsesFrameworkConfig.h"
#include "ramses-renderer-api/RendererConfig.h"
#include "ramses-renderer-api/DisplayConfig.h"
#include "SceneViewerDcsmHandler.h"
#include <memory>
#include <vector>
#include <CLI/CLI.hpp>

namespace ramses
{
    class RamsesClient;
    class Scene;
    class RamsesRenderer;
}

namespace ramses_internal
{
    class SceneViewer
    {
    public:
        SceneViewer(int argc, char* argv[]);

        int run();

    private:
        enum class GuiMode
        {
            Off,
            On,       ///< Loaded scene rendered to offscreen buffer (configurable size and position)
            Overlay,  ///< Debugging gui overlaps the rendered scene (no offscreen buffer)
            Only,     ///< Only shows the gui, not the scene itself
            Invalid
        };

        GuiMode getGuiMode() const;
        ramses::Scene *loadScene(ramses::RamsesClient& client, const String& filename);
        int loadAndRenderScene(const String& sceneFile);
        void validateContent(const ramses::Scene& scene) const;

        void registerOptions(CLI::App& cli);

        CLI::App          m_cli;
        String            m_scenePath;
        ramses::sceneId_t m_sceneId;
        bool              m_noValidation = false;
        std::string       m_validationOutputDirectory;
        String            m_screenshotFile;
        std::string m_sceneName;
        GuiMode           m_guiMode = GuiMode::Overlay;
        bool              m_noSkub  = false;

        ramses::RamsesFrameworkConfig  m_frameworkConfig;
        ramses::RendererConfig         m_rendererConfig;
        ramses::DisplayConfig          m_displayConfig;
        SceneViewerDcsmHandler::ConfigList  m_dcsmConfig;
    };
}

#endif
