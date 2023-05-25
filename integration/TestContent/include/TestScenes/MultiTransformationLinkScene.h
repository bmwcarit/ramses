//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_MULTITRANSFORMATIONLINKSCENE_H
#define RAMSES_MULTITRANSFORMATIONLINKSCENE_H

#include "IntegrationScene.h"
#include "SceneAPI/Handles.h"
#include "Triangle.h"

namespace ramses_internal
{
    class MultiTransformationLinkScene : public IntegrationScene
    {
    public:
        MultiTransformationLinkScene(ramses::Scene& scene, UInt32 state, const glm::vec3& cameraPosition);

        enum
        {
            PROVIDER_SCENE = 0,
            PROVIDER_CONSUMER_SCENE,
            CONSUMER_SCENE
        };

        static const UInt32 NumMeshes = 16u;
        static const UInt32 NumRows   = 4u;

        static const UInt32 DataIdRowStart = 0u;
        static const UInt32 DataIdMeshStart = 1000u;

    private:
        void createSceneProvider();
        void createSceneProviderConsumer();
        void createSceneConsumer();

        const float m_scaleFactor;

        ramses::Effect& m_dummyEffect;
        ramses::Triangle m_redTriangle;
        ramses::Triangle m_greenTriangle;
        ramses::Triangle m_blueTriangle;
    };
}

#endif
