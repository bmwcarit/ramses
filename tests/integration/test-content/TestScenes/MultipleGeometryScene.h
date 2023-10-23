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
#include "ramses/client/MeshNode.h"
#include <array>

namespace ramses::internal
{
    class MultipleGeometryScene : public IntegrationScene
    {
    public:
        MultipleGeometryScene(ramses::Scene& scene, uint32_t state, const glm::vec3& cameraPosition);

        enum
        {
            MULTI_TRIANGLE_LIST_GEOMETRY_WITH_INDEX_ARRAY = 0,
            MULTI_TRIANGLE_LIST_GEOMETRY_WITHOUT_INDEX_ARRAY,
            MULTI_TRIANGLE_STRIP_GEOMETRY_WITHOUT_INDEX_ARRAY,
            VERTEX_ARRAYS_WITH_OFFSET
        };

    private:
        void createGeometries(ramses::Effect& effect, uint32_t state);

        static const uint32_t NumMeshes = 4u;
        std::array<ramses::MeshNode*, NumMeshes> m_meshNode{};
    };
}
