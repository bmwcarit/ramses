//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERPASSSCENE_H
#define RAMSES_RENDERPASSSCENE_H

#include "IntegrationScene.h"
#include "Triangle.h"
#include "SceneAPI/Handles.h"
#include "ramses-client-api/MeshNode.h"

namespace ramses_internal
{
    class RenderPassScene : public IntegrationScene
    {
    public:
        RenderPassScene(ramses::Scene& scene, uint32_t state, const glm::vec3& cameraPosition);

        enum
        {
            MESHES_NOT_IN_PASS = 0,
            ONE_MESH_PER_PASS,
            MESH_IN_MULTIPLE_PASSES,
            GROUPS_WITH_DIFFERENT_RENDER_ORDER,
            NESTED_GROUPS,
            PASSES_WITH_DIFFERENT_RENDER_ORDER,
            PASSES_WITH_LEFT_AND_RIGHT_VIEWPORT
        };

    private:
        ramses::Effect& m_effect;
        ramses::Triangle m_blueTriangle;
        ramses::Triangle m_whiteTriangle;
    };
}

#endif //RAMSES_RENDERPASSSCENE_H

