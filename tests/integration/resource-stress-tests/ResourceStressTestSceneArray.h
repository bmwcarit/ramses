//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ResourceStressTestScene.h"
#include "ramses/framework/RendererSceneState.h"
#include <vector>
#include <memory>

namespace ramses::internal
{
    class StressTestRenderer;

    struct SceneConfig
    {
        ramses::sceneId_t sceneId;
        ramses::displayId_t displayId;
        ramses::displayBufferId_t offscreenBufferId;
        ScreenspaceQuad targetScreenQuad;
        TextureConsumerDataIds textureConsumerIds;
        ramses::sceneId_t consumerSceneId;
        ramses::dataConsumerId_t consumerSceneDataId;
    };

    using SceneArrayConfig = std::vector<SceneConfig>;

    class ResourceStressTestSceneArray
    {
    public:
        ResourceStressTestSceneArray(ramses::RamsesClient& client, StressTestRenderer& renderer, const SceneArrayConfig& sceneArrayConfig);

        void getAllToState(ramses::RendererSceneState state);
        void doExpensiveFlushOnAll(ramses::sceneVersionTag_t flushName);
        void waitForFlushOnAll(ramses::sceneVersionTag_t flushName);

    private:
        StressTestRenderer& m_renderer;
        SceneArrayConfig m_sceneConfigs;
        std::vector<std::unique_ptr<ResourceStressTestScene>> m_scenes;
    };
}
