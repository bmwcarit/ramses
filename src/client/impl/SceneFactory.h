//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/SceneAPI/IScene.h"
#include "ramses/framework/EFeatureLevel.h"
#include <unordered_map>

namespace ramses::internal
{
    class ClientScene;
    using InternalSceneOwningPtr = std::unique_ptr<ClientScene>;

    class SceneFactory
    {
    public:
        ClientScene* createScene(const SceneInfo& sceneInfo, EFeatureLevel featureLevel);
        InternalSceneOwningPtr releaseScene(SceneId id);

    private:
        using SceneMap = std::unordered_map<SceneId, InternalSceneOwningPtr>;
        SceneMap m_scenes;
    };
}
