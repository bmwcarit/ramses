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
#include "ramses-framework-api/RamsesFrameworkTypes.h"

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
        void printUsage() const;
        void loadAndRenderScene(int argc, char* argv[], const String& sceneFile, const String& resFile);
        ramses::Scene* loadSceneWithResources(ramses::RamsesClient& client, const String& sceneFile, const String& resFile);
        void validateContent(const ramses::RamsesClient& client, const ramses::Scene& scene) const;

        static std::string getFileName(std::string path);
        std::string m_sceneName;
        CommandLineParser m_parser;
        ArgumentBool   m_helpArgument;
        ArgumentString m_scenePathAndFileArgument;
        ArgumentString m_optionalResFileArgument;
        ArgumentString m_validationUnrequiredObjectsDirectoryArgument;
    };
}

#endif
