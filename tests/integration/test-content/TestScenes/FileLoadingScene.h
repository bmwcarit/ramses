//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "IntegrationScene.h"
#include "ramses/framework/RamsesFrameworkTypes.h"
#include "ramses/client/SceneConfig.h"

#include <string>

namespace ramses
{
    class RenderGroup;
    class RamsesFrameworkConfig;
}

namespace ramses::internal
{
    // FileLoadingScene is NO IntegrationScene (IntegrationScene needs scene and initializes it -> not wanted here)
    class FileLoadingScene
    {
    public:
        FileLoadingScene(ramses::RamsesClient& clientForLoading, uint32_t state, ramses::sceneId_t sceneId, const glm::vec3& cameraPosition, const std::string& folder, const ramses::RamsesFrameworkConfig& config, uint32_t vpWidth = IntegrationScene::DefaultViewportWidth, uint32_t vpHeight = IntegrationScene::DefaultViewportHeight);

        ramses::Scene* getCreatedScene();

        enum
        {
            CREATE_SAVE_DESTROY_LOAD_USING_SEPARATE_CLIENT = 0,
            CREATE_SAVE_DESTROY_LOAD_USING_SAME_CLIENT
        };

    private:
        void createFiles(ramses::RamsesClient& client, ramses::sceneId_t sceneId, const glm::vec3& cameraPosition, const std::string& folder);
        static void AddTriangles(ramses::Scene& scene, ramses::RenderGroup& renderGroup);
        void loadFromFiles(ramses::RamsesClient& client, const std::string& folder);
        static void CleanupFiles(const std::string& folder);

        ramses::Scene* m_createdScene = nullptr;
        uint32_t m_viewportWidth;
        uint32_t m_viewportHeight;
    };
}
