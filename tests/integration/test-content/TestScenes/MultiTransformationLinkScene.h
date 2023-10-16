//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "IntegrationScene.h"
#include "internal/SceneGraph/SceneAPI/Handles.h"
#include "Triangle.h"

namespace ramses::internal
{
    class MultiTransformationLinkScene : public IntegrationScene
    {
    public:
        MultiTransformationLinkScene(ramses::Scene& scene, uint32_t state, const glm::vec3& cameraPosition);

        enum
        {
            PROVIDER_SCENE = 0,
            PROVIDER_CONSUMER_SCENE,
            CONSUMER_SCENE
        };

        static const uint32_t NumMeshes = 16u;
        static const uint32_t NumRows   = 4u;

        static const uint32_t DataIdRowStart = 0u;
        static const uint32_t DataIdMeshStart = 1000u;

    private:
        void createSceneProvider();
        void createSceneProviderConsumer();
        void createSceneConsumer();

        const float m_scaleFactor;

        ramses::Effect& m_dummyEffect;
        Triangle m_redTriangle;
        Triangle m_greenTriangle;
        Triangle m_blueTriangle;
    };
}
