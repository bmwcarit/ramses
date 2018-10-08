//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENEFACTORY_H
#define RAMSES_SCENEFACTORY_H

#include "Collections/HashSet.h"
#include "SceneAPI/IScene.h"

namespace ramses_internal
{
    class ClientScene;

    class SceneFactory
    {
    public:
        explicit SceneFactory();
        ~SceneFactory();

        ClientScene* createScene(const SceneInfo& sceneInfo);
        ClientScene* releaseScene(SceneId id);

    private:
        typedef HashMap<SceneId, ClientScene*> SceneMap;
        SceneMap m_scenes;

    };
}

#endif
