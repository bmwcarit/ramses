//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "SceneFactory.h"
#include "Scene/ClientScene.h"
#include "Utils/LogMacros.h"

namespace ramses_internal
{
    ClientScene* SceneFactory::createScene(const SceneInfo& sceneInfo)
    {
        if (m_scenes.count(sceneInfo.sceneID) != 0)
        {
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "SceneFactory::createScene: scene with id " << sceneInfo.sceneID << " already exists, scene ID must be unique!");
            return nullptr;
        }

        auto newScene = std::make_unique<ClientScene>(sceneInfo);
        auto* newScenePtr = newScene.get();
        m_scenes.insert({ newScene->getSceneId(), std::move(newScene) });

        return newScenePtr;
    }

    InternalSceneOwningPtr SceneFactory::releaseScene(SceneId id)
    {
        InternalSceneOwningPtr ret;
        auto it = m_scenes.find(id);
        if (it != m_scenes.end())
        {
            ret = std::move(it->second);
            m_scenes.erase(it);
        }

        return ret;
    }
}
