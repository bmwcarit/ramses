//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes.h"
#include "TestScenes/FileLoadingScene.h"
#include <cassert>

namespace ramses::internal
{
    TestScenes::TestScenes(ramses::RamsesClient& client)
        : m_client(client)
    {
    }

    TestScenes::~TestScenes()
    {
        for (const auto& sceneEntry : m_scenes)
        {
            delete sceneEntry.value.integrationScene;
        }
    }

    void TestScenes::createFileLoadingScene(ramses::sceneId_t sceneId, const glm::vec3& cameraPosition, const ramses::RamsesFrameworkConfig& config, uint32_t sceneState, uint32_t vpWidth, uint32_t vpHeight)
    {
        // This creates the scene internally inside the instance
        ramses::internal::FileLoadingScene fileLoadingScene(m_client, sceneState, sceneId, cameraPosition, ".", config, vpWidth, vpHeight);
        SceneData data;
        data.clientScene = fileLoadingScene.getCreatedScene();
        m_scenes.put(sceneId, data);
    }

    const ramses::Scene& TestScenes::getScene(ramses::sceneId_t sceneId) const
    {
        assert(m_scenes.contains(sceneId));
        return *m_scenes.get(sceneId)->clientScene;
    }

    ramses::Scene& TestScenes::getScene(ramses::sceneId_t sceneId)
    {
        assert(m_scenes.contains(sceneId));
        return *m_scenes.get(sceneId)->clientScene;
    }

    void TestScenes::destroyScenes()
    {
        while (m_scenes.size() != 0u)
        {
            destroyScene(m_scenes.begin()->key);
        }
    }

    void TestScenes::destroyScene(ramses::sceneId_t sceneId)
    {
        auto it = m_scenes.find(sceneId);
        assert(it != m_scenes.end());

        auto& clientScene = *it->value.clientScene;
        delete it->value.integrationScene;
        m_client.destroy(clientScene);
        m_scenes.remove(it);
    }
}
