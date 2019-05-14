//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RESOURCESTRESSTESTSCENEARRAY_H
#define RAMSES_RESOURCESTRESSTESTSCENEARRAY_H

#include "ResourceStressTestScene.h"
#include "Collections/Vector.h"
#include <memory>

namespace ramses_internal
{
    class StressTestRenderer;

    struct SceneConfig
    {
        ramses::sceneId_t sceneId;
        ramses::displayId_t displayId;
        ramses::offscreenBufferId_t offscreenBufferId;
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

        void subscribeAll();
        void mapAndShowAll();
        void hideAndUnmapAll();

        void doExpensiveFlushOnAll(ramses::sceneVersionTag_t flushName);
        void waitForFlushOnAll(ramses::sceneVersionTag_t flushName);
    private:

        StressTestRenderer& m_renderer;
        SceneArrayConfig m_sceneConfigs;
        std::vector<std::unique_ptr<ResourceStressTestScene>> m_scenes;
    };
}

#endif
