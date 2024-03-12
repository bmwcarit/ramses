//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TESTSCENES_H
#define RAMSES_TESTSCENES_H

#include "ramses-client-api/RamsesClient.h"
#include "ramses-client-api/Scene.h"
#include "TestScenes/IntegrationScene.h"
#include "Collections/HashMap.h"
#include "Math3d/Vector3.h"
#include <cassert>

class TestScenes
{
public:
    explicit TestScenes(ramses::RamsesClient& client);
    ~TestScenes();

    template <typename INTEGRATION_SCENE>
    void createScene(
        uint32_t state,
        ramses::sceneId_t sceneId,
        const ramses_internal::Vector3& cameraPosition = ramses_internal::Vector3(0.0f),
        const ramses::SceneConfig& sceneConfig = {})
    {
        SceneData data;
        data.clientScene = m_client.createScene(sceneId, sceneConfig);
        data.integrationScene = new INTEGRATION_SCENE(*data.clientScene, state, cameraPosition);
        m_scenes.put(sceneId, data);

    }
    template <typename INTEGRATION_SCENE>
    ramses::sceneId_t createScene(
        uint32_t state,
        const ramses_internal::Vector3& cameraPosition = ramses_internal::Vector3(0.0f),
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
        const ramses_internal::Vector3& cameraPosition,
        uint32_t vpWidth,
        uint32_t vpHeight,
        const ramses::SceneConfig& config = {})
    {
        SceneData data;
        data.clientScene = m_client.createScene(sceneId, config);
        data.integrationScene = new INTEGRATION_SCENE(*data.clientScene, state, cameraPosition, vpWidth, vpHeight);
        m_scenes.put(sceneId, data);
    }

    template <typename INTEGRATION_SCENE>
    ramses::sceneId_t createScene(
        uint32_t state,
        const ramses_internal::Vector3& cameraPosition,
        uint32_t vpWidth,
        uint32_t vpHeight,
        const ramses::SceneConfig& config = {})
    {
        const ramses::sceneId_t sceneId = m_nextSceneId;
        m_nextSceneId.getReference()++;
        createScene<INTEGRATION_SCENE>(state, sceneId, cameraPosition, vpWidth, vpHeight, config);
        return sceneId;
    }

    void createFileLoadingScene(ramses::sceneId_t sceneId, const ramses_internal::Vector3& cameraPosition, const ramses::RamsesFrameworkConfig& config, uint32_t sceneState, uint32_t vpWidth, uint32_t vpHeight);

    const ramses::Scene& getScene(ramses::sceneId_t sceneId) const;
    ramses::Scene& getScene(ramses::sceneId_t sceneId);

    template <typename INTEGRATION_SCENE>
    void setSceneState(ramses::sceneId_t sceneId, uint32_t state)
    {
        assert(m_scenes.contains(sceneId));
        assert(m_scenes.get(sceneId)->integrationScene);
        INTEGRATION_SCENE& scene = static_cast<INTEGRATION_SCENE&>(*m_scenes.get(sceneId)->integrationScene);
        scene.setState(state);
    }

    void destroyScenes();
    void destroyScene(ramses::sceneId_t sceneId);

private:
    struct SceneData
    {
        ramses::Scene* clientScene = nullptr;
        ramses_internal::IntegrationScene* integrationScene = nullptr;
    };
    using SceneMap = ramses_internal::HashMap<ramses::sceneId_t, SceneData>;

    ramses::RamsesClient& m_client;
    ramses::sceneId_t m_nextSceneId{1u};
    SceneMap m_scenes;
};

#endif
