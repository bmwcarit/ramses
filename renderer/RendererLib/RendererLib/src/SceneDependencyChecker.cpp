//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "SceneDependencyChecker.h"
#include "RendererAPI/Types.h"

namespace ramses_internal
{
    SceneDependencyChecker::SceneDependencyChecker()
        : m_dirty(false)
    {
    }

    Bool SceneDependencyChecker::addDependency(SceneId providerScene, SceneId consumerScene)
    {
        if (hasDependencyAsConsumerToProvider(providerScene, consumerScene))
        {
            return false;
        }

        auto providers = m_consumerToProvidersMap.get(consumerScene);
        if (providers != nullptr)
        {
            providers->push_back(providerScene);
        }
        else
        {
            m_consumerToProvidersMap.put(consumerScene, SceneIdVector(1u, providerScene));
        }

        m_dirty = true;
        return true;
    }

    void SceneDependencyChecker::removeDependency(SceneId providerScene, SceneId consumerScene)
    {
        assert(m_consumerToProvidersMap.contains(consumerScene));
        auto& providers = *m_consumerToProvidersMap.get(consumerScene);
        assert(contains_c(providers, providerScene));
        providers.erase(find_c(providers, providerScene));
        if (providers.empty())
        {
            m_consumerToProvidersMap.remove(consumerScene);
        }

        m_dirty = true;
    }

    Bool SceneDependencyChecker::hasDependencyAsConsumer(SceneId scene) const
    {
        return m_consumerToProvidersMap.contains(scene);
    }

    const SceneIdVector& SceneDependencyChecker::getDependentScenesInOrder() const
    {
        if (m_dirty)
        {
            updateSceneOrder();
        }

        return m_sceneOrderList;
    }

    void SceneDependencyChecker::removeScene(SceneId scene)
    {
        m_consumerToProvidersMap.remove(scene);

        SceneIdVector consumersToRemove;
        for (auto& it : m_consumerToProvidersMap)
        {
            auto& providers = it.value;
            while (contains_c(providers, scene))
            {
                providers.erase(find_c(providers, scene));
                if (providers.empty())
                {
                    consumersToRemove.push_back(it.key);
                }
            }
        }

        for (const auto it : consumersToRemove)
        {
            m_consumerToProvidersMap.remove(it);
        }

        m_dirty = true;
    }

    Bool SceneDependencyChecker::hasDependencyAsConsumerToProvider(SceneId consumerScene, SceneId providerScene) const
    {
        SceneIdVector scenesQueue;
        scenesQueue.reserve(m_sceneOrderList.size());
        scenesQueue.push_back(consumerScene);

        while (!scenesQueue.empty())
        {
            const SceneId currScene = scenesQueue.back();
            scenesQueue.pop_back();

            if (currScene == providerScene)
            {
                return true;
            }

            if (m_consumerToProvidersMap.contains(currScene))
            {
                const auto& providers = *m_consumerToProvidersMap.get(currScene);
                scenesQueue.insert(scenesQueue.end(), providers.begin(), providers.end());
            }
        }

        return false;
    }

    void SceneDependencyChecker::updateSceneOrder() const
    {
        // collect all scenes
        SceneIdVector scenes;
        for (const auto& it : m_consumerToProvidersMap)
        {
            if (!contains_c(scenes, it.key))
            {
                scenes.push_back(it.key);
            }

            for (const auto& provider : it.value)
            {
                if (!contains_c(scenes, provider))
                {
                    scenes.push_back(provider);
                }
            }
        }

        m_sceneOrderList.clear();
        m_sceneOrderList.reserve(scenes.size());

        while (!scenes.empty())
        {
            // pop scene from front
            const SceneId currScene = scenes.front();
            scenes.erase(scenes.begin());

            // check if has any dependency to the rest of the scenes
            // (intersection of its providers list and rest of scenes is not empty)
            Bool dependsOnRestOfTheSet = false;
            if (m_consumerToProvidersMap.contains(currScene))
            {
                const auto& providers = *m_consumerToProvidersMap.get(currScene);
                for (const auto provider : providers)
                {
                    if (contains_c(scenes, provider))
                    {
                        dependsOnRestOfTheSet = true;
                        break;
                    }
                }
            }

            if (dependsOnRestOfTheSet)
            {
                // add back to scenes if has dependency - to be processed again later
                scenes.push_back(currScene);
            }
            else
            {
                // has no dependency -> move to ordered list
                m_sceneOrderList.push_back(currScene);
            }
        }

        m_dirty = false;
    }

    Bool SceneDependencyChecker::isEmpty() const
    {
        return m_consumerToProvidersMap.count() == 0u;
    }
}
