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
    SceneFactory::SceneFactory()
    {
    }

    SceneFactory::~SceneFactory()
    {
        for (const auto& item : m_scenes)
        {
            if(nullptr != item.value)
            {
                delete item.value;
            }
        }
    }

    ClientScene* SceneFactory::createScene(const SceneInfo& sceneInfo)
    {
        if (m_scenes.contains(sceneInfo.sceneID))
        {
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "SceneFactory::createScene: scene with id " << sceneInfo.sceneID.getValue() << " already exists, scene ID must be unique!");
            return nullptr;
        }

        ClientScene* newScene = new ClientScene(sceneInfo);
        m_scenes.put(newScene->getSceneId(), newScene);

        return newScene;
    }

    ClientScene* SceneFactory::releaseScene(SceneId id)
    {
        ClientScene* ret = nullptr;
        SceneMap::Iterator it = m_scenes.find(id);
        if (it != m_scenes.end())
        {
            ret = it->value;
            m_scenes.remove(it);
        }
        return ret;
    }
}
