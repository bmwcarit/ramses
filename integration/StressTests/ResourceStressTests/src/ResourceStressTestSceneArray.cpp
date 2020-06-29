//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ResourceStressTestSceneArray.h"
#include "StressTestRenderer.h"
#include <unordered_set>

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
            m_renderer.setSceneDisplayAndBuffer(sceneConfig.sceneId, sceneConfig.displayId, sceneConfig.offscreenBufferId);
        }
    }

    void ResourceStressTestSceneArray::getAllToState(ramses::RendererSceneState state)
    {
        // request change for all scenes first
        for (const auto& sceneConfig : m_sceneConfigs)
            m_renderer.setSceneState(sceneConfig.sceneId, state);

        // wait for all scenes to reach their state
        for (const auto& sceneConfig : m_sceneConfigs)
            m_renderer.waitForSceneState(sceneConfig.sceneId, state);

        // Link OBs to consumers
        if (state == ramses::RendererSceneState::Rendered)
        {
            std::unordered_set<ramses::dataConsumerId_t> linkedTextureConsumers;

            for (const auto& sceneConfig : m_sceneConfigs)
            {
                // If two OB scenes are in the same OB, should link the OB to the scene consumer only once
                if (sceneConfig.offscreenBufferId.isValid() && linkedTextureConsumers.count(sceneConfig.consumerSceneDataId) == 0)
                {
                    m_renderer.linkOffscreenBufferToSceneTexture(sceneConfig.consumerSceneId, sceneConfig.offscreenBufferId, sceneConfig.consumerSceneDataId);
                    linkedTextureConsumers.insert(sceneConfig.consumerSceneDataId);
                }
            }
        }
    }

    void ResourceStressTestSceneArray::doExpensiveFlushOnAll(ramses::sceneVersionTag_t flushName)
    {
        for (const auto& scene : m_scenes)
        {
            scene->recreateResources();
            scene->flush(flushName);
        }
    }

    void ResourceStressTestSceneArray::waitForFlushOnAll(ramses::sceneVersionTag_t flushName)
    {
        for (const auto& sceneConfig : m_sceneConfigs)
            m_renderer.waitForFlush(sceneConfig.sceneId, flushName);
    }
}
