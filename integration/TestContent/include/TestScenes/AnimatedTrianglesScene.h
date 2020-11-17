//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ANIMATEDTRIANGLESSCENE_H
#define RAMSES_ANIMATEDTRIANGLESSCENE_H

#include "IntegrationScene.h"
#include "SceneAPI/Handles.h"
#include "ramses-client-api/MeshNode.h"

namespace ramses
{
    class AnimationSystem;
    class AnimationSequence;
}

namespace ramses_internal
{
    class AnimatedTrianglesScene : public IntegrationScene
    {
    public:
        AnimatedTrianglesScene(ramses::Scene& scene, UInt32 state, const Vector3& cameraPosition, uint32_t vpWidth = IntegrationScene::DefaultViewportWidth, uint32_t vpHeight = IntegrationScene::DefaultViewportHeight);

        enum
        {
            ANIMATION_POINT0 = 0u,
            ANIMATION_POINT1,
            ANIMATION_POINT2,
            ANIMATION_POINT3,
            ANIMATION_POINT4,
            ANIMATION_RUNNING,

            NUM_STATES
        };

    private:

        ramses::AnimationSequence& createSequence(
            ramses::Scene& scene,
            ramses::AnimationSystem& animationSystem,
            ramses::Appearance& appearance,
            ramses::GeometryBinding& geometry);

        static const UInt64 StartTime = 1000u;
        static const UInt64 StopTime[NUM_STATES];
    };
}

#endif
