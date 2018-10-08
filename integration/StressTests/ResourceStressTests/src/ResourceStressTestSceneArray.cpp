//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ResourceStressTestSceneArray.h"
#include "StressTestRenderer.h"

namespace ramses_internal
{

    ResourceStressTestSceneArray::ResourceStressTestSceneArray(ramses::RamsesClient& client, StressTestRenderer& renderer, const SceneArrayConfig& sceneArrayConfig)
        : m_renderer(renderer)
        , m_sceneConfigs(sceneArrayConfig)
        , m_scenes(sceneArrayConfig.size())
    {
        uint32_t index = 0;
        for (const auto& sceneConfig : sceneArrayConfig)
        {
            m_scenes[index++].reset(new ResourceStressTestScene(client, sceneConfig.sceneId, sceneConfig.textureConsumerIds, sceneConfig.targetScreenQuad));
        }
    }

    void ResourceStressTestSceneArray::subscribeAll()
    {
        for (const auto& sceneConfig : m_sceneConfigs)
        {
            m_renderer.subscribeScene(sceneConfig.sceneId);
        }
    }

    void ResourceStressTestSceneArray::mapAndShowAll()
    {
        HashSet<ramses::dataConsumerId_t> linkedTextureConsumers;

        for (const auto& sceneConfig : m_sceneConfigs)
        {
            m_renderer.mapScene(sceneConfig.displayId, sceneConfig.sceneId);
            if (sceneConfig.offscreenBufferId == ramses::InvalidOffscreenBufferId)
            {
                m_renderer.showScene(sceneConfig.sceneId);
            }
            else
            {
                m_renderer.showSceneOnOffscreenBuffer(sceneConfig.sceneId, sceneConfig.offscreenBufferId);

                // If two OB scenes are in the same OB, should link the OB to the scene consumer only once
                if (!linkedTextureConsumers.hasElement(sceneConfig.consumerSceneDataId))
                {
                    m_renderer.linkOffscreenBufferToSceneTexture(sceneConfig.consumerSceneId, sceneConfig.offscreenBufferId, sceneConfig.consumerSceneDataId);
                    linkedTextureConsumers.put(sceneConfig.consumerSceneDataId);
                }
            }

        }
    }

    void ResourceStressTestSceneArray::hideAndUnmapAll()
    {
        for (const auto& sceneConfig : m_sceneConfigs)
        {
            m_renderer.hideAndUnmapScene(sceneConfig.sceneId);
        }
    }

    void ResourceStressTestSceneArray::doExpensiveFlushOnAll(ramses::sceneVersionTag_t flushName)
    {
        for (const auto& scene : m_scenes)
        {
            scene->recreateResources();
            scene->flush(ramses::ESceneFlushMode_SynchronizedWithResources, flushName);
        }
    }

    void ResourceStressTestSceneArray::waitForFlushOnAll(ramses::sceneVersionTag_t flushName)
    {
        for (const auto& sceneConfig : m_sceneConfigs)
        {
            m_renderer.waitForNamedFlush(sceneConfig.sceneId, flushName);
        }
    }
}
