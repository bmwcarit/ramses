//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/client/RamsesClient.h"
#include "ramses/client/Scene.h"
#include "TestScenes/IntegrationScene.h"
#include "internal/PlatformAbstraction/Collections/HashMap.h"
#include <cassert>

namespace ramses::internal
{
    class TestScenes
    {
    public:
        explicit TestScenes(ramses::RamsesClient& client);
        ~TestScenes();

        template <typename INTEGRATION_SCENE>
        void createScene(
            uint32_t state,
            ramses::sceneId_t sceneId,
            const glm::vec3& cameraPosition = glm::vec3(0.0f),
            const ramses::SceneConfig& sceneConfig = {})
        {
            SceneData data;
            SceneConfig config = sceneConfig;
            config.setSceneId(sceneId);

            data.clientScene = m_client.createScene(config);
            data.integrationScene = new INTEGRATION_SCENE(*data.clientScene, state, cameraPosition);
            m_scenes.put(sceneId, data);

        }
        template <typename INTEGRATION_SCENE>
        ramses::sceneId_t createScene(
            uint32_t state,
            const glm::vec3& cameraPosition = glm::vec3(0.0f),
            const ramses::SceneConfig& sceneConfig = {})
        {
            const ramses::sceneId_t sceneId = m_nextSceneId;
            m_nextSceneId.getReference()++;
            createScene<INTEGRATION_SCENE>(state, sceneId, cameraPosition, sceneConfig);
            return sceneId;
        }

        template <typename INTEGRATION_SCENE>
        void createScene(
            uint32_t state,
            ramses::sceneId_t sceneId,
            const glm::vec3& cameraPosition,
            uint32_t vpWidth,
            uint32_t vpHeight)
        {
            SceneData data;
            data.clientScene = m_client.createScene(SceneConfig(sceneId));
            data.integrationScene = new INTEGRATION_SCENE(*data.clientScene, state, cameraPosition, vpWidth, vpHeight);
            m_scenes.put(sceneId, data);
        }
        template <typename INTEGRATION_SCENE>
        ramses::sceneId_t createScene(
            uint32_t state,
            const glm::vec3& cameraPosition,
            uint32_t vpWidth,
            uint32_t vpHeight)
        {
            const ramses::sceneId_t sceneId = m_nextSceneId;
            m_nextSceneId.getReference()++;
            createScene<INTEGRATION_SCENE>(state, sceneId, cameraPosition, vpWidth, vpHeight);
            return sceneId;
        }

        void createFileLoadingScene(ramses::sceneId_t sceneId, const glm::vec3& cameraPosition, const ramses::RamsesFrameworkConfig& config, uint32_t sceneState, uint32_t vpWidth, uint32_t vpHeight);

        [[nodiscard]] const ramses::Scene& getScene(ramses::sceneId_t sceneId) const;
        ramses::Scene& getScene(ramses::sceneId_t sceneId);

        template <typename INTEGRATION_SCENE>
        void setSceneState(ramses::sceneId_t sceneId, uint32_t state)
        {
            assert(m_scenes.contains(sceneId));
            assert(m_scenes.get(sceneId)->integrationScene);
            auto& scene = static_cast<INTEGRATION_SCENE&>(*m_scenes.get(sceneId)->integrationScene);
            scene.setState(state);
        }

        void destroyScenes();
        void destroyScene(ramses::sceneId_t sceneId);

    private:
        struct SceneData
        {
            ramses::Scene* clientScene = nullptr;
            ramses::internal::IntegrationScene* integrationScene = nullptr;
        };
        using SceneMap = ramses::internal::HashMap<ramses::sceneId_t, SceneData>;

        ramses::RamsesClient& m_client;
        ramses::sceneId_t m_nextSceneId{1u};
        SceneMap m_scenes;
    };
}
