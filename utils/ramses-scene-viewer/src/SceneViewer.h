//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENE_VIEWER_SCENEVIEWER_H
#define RAMSES_SCENE_VIEWER_SCENEVIEWER_H

#include "Utils/CommandLineParser.h"
#include "Utils/Argument.h"
#include "Collections/String.h"
#include "ramses-framework-api/RamsesFrameworkConfig.h"
#include "ramses-renderer-api/RendererConfig.h"
#include "ramses-renderer-api/DisplayConfig.h"
#include <memory>
#include <vector>

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
            Invalid
        };

        GuiMode getGuiMode() const;
        void printUsage() const;
        int loadAndRenderScene(const String& sceneFile);
        void validateContent(const ramses::Scene& scene) const;

        std::string m_sceneName;
        CommandLineParser m_parser;
        ArgumentBool   m_helpArgument;
        ArgumentString m_scenePathAndFileArgument;
        ArgumentBool   m_noValidation;
        ArgumentString m_validationUnrequiredObjectsDirectoryArgument;
        ArgumentString m_screenshotFile;
        ArgumentBool   m_noSkub;
        ArgumentString m_guiModeArgument;

        ramses::RamsesFrameworkConfig  m_frameworkConfig;
        ramses::RendererConfig         m_rendererConfig;
        ramses::DisplayConfig          m_displayConfig;
        const std::vector<const char*> m_args;
    };
}

#endif
