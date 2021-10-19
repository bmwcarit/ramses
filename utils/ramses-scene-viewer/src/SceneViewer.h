//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENE_VIEWER_SCENEVIEWER_H
#define RAMSES_SCENE_VIEWER_SCENEVIEWER_H

#include <memory>
#include "Utils/CommandLineParser.h"
#include "Utils/Argument.h"
#include "Collections/String.h"
#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "ramses-renderer-api/IRendererEventHandler.h"
#include "ramses-renderer-api/RamsesRenderer.h"

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

    protected:
        enum class GuiMode
        {
            Off,
            Overlay, ///< Debugging gui overlaps the rendered scene
            Window   ///< Separate window for the debugging gui
        };

        GuiMode getGuiMode() const;
        void printUsage() const;
        void loadAndRenderScene(int argc, char* argv[], const String& sceneFile);
        ramses::Scene* loadScene(ramses::RamsesClient& client, const String& sceneFile);
        void validateContent(const ramses::RamsesClient& client, const ramses::Scene& scene) const;

        std::string m_sceneName;
        CommandLineParser m_parser;
        ArgumentBool   m_helpArgument;
        ArgumentString m_scenePathAndFileArgument;
        ArgumentBool   m_noValidation;
        ArgumentString m_validationUnrequiredObjectsDirectoryArgument;
        ArgumentString m_screenshotFile;
        ArgumentBool   m_noSkub;
        ArgumentString m_guiModeArgument;
    };
}

#endif
