//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/SceneAPI/SceneSizeInformation.h"
#include "internal/SceneGraph/SceneAPI/IScene.h"
#include "ramses/framework/EFeatureLevel.h"

namespace ramses::internal
{
    class ClientScene;
    class IOutputStream;
    class IInputStream;
    class SceneMergeHandleMapping;
    struct SceneCreationInformation;

    class ScenePersistation
    {
    public:
        static void WriteSceneMetadataToStream(IOutputStream& outStream, const IScene& scene, EFeatureLevel featureLevel);
        static void WriteSceneToStream(IOutputStream& outStream, const ClientScene& scene, EFeatureLevel featureLevel);
        static void WriteSceneToFile(std::string_view filename, const ClientScene& scene, EFeatureLevel featureLevel);

        static void ReadSceneMetadataFromStream(IInputStream& inStream, SceneCreationInformation& createInfo, EFeatureLevel featureLevel);
        static void ReadSceneFromStream(IInputStream& inStream, IScene& scene, EFeatureLevel featureLevel, SceneMergeHandleMapping* mapping);
        static void ReadSceneFromFile(std::string_view filename, IScene& scene, EFeatureLevel featureLevel, SceneMergeHandleMapping* mapping);
    };
}
