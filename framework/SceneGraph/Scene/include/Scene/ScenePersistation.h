//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENEPERSISTATION_H
#define RAMSES_SCENEPERSISTATION_H

#include "SceneAPI/SceneSizeInformation.h"
#include "SceneAPI/IScene.h"

namespace ramses_internal
{
    class ClientScene;
    class String;
    class IOutputStream;
    class IInputStream;
    class AnimationSystemFactory;
    struct SceneCreationInformation;

    class ScenePersistation
    {
    public:
        static void WriteSceneMetadataToStream(IOutputStream& outStream, const IScene& scene);
        static void WriteSceneToStream(IOutputStream& outStream, const ClientScene& scene);
        static void WriteSceneToFile(const String& filename, const ClientScene& scene);

        static void ReadSceneMetadataFromStream(IInputStream& inStream, SceneCreationInformation& createInfo);
        static void ReadSceneFromStream(IInputStream& inStream, IScene& scene, AnimationSystemFactory* animSystemFactory = NULL);
        static void ReadSceneFromFile(const String& filename, IScene& scene, AnimationSystemFactory* animSystemFactory = NULL);
    };
}

#endif
