//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "IntegrationScene.h"
#include "internal/SceneGraph/SceneAPI/Handles.h"
#include "ramses/client/MeshNode.h"


namespace ramses::internal
{
    class GeometryInstanceScene : public IntegrationScene
    {
    public:
        GeometryInstanceScene(ramses::Scene& scene, uint32_t state, const glm::vec3& cameraPosition);

        enum
        {
            GEOMETRY_INSTANCE_UNIFORM = 0,
            GEOMETRY_INSTANCE_VERTEX,
            GEOMETRY_INSTANCE_AND_NOT_INSTANCE,
            GEOMETRY_INSTANCE_ZERO_INSTANCE_COUNT
        };

    private:
        static void SetInstancedUniforms(ramses::Appearance& appearance);
        void setInstancedAttributes(const ramses::Effect& effect, ramses::Geometry* geometry, uint32_t instancingDivisor);
        ramses::Geometry* createGeometry(const ramses::Effect& effect);

        static constexpr uint32_t NumInstances = 3u;
    };
}
