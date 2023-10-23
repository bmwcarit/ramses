//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "IntegrationScene.h"

namespace ramses
{
    class Effect;
    class ArrayResource;
    class Texture3D;
}

namespace ramses::internal
{
    class Texture3DScene : public IntegrationScene
    {
    public:
        Texture3DScene(ramses::Scene& scene, uint32_t state, const glm::vec3& cameraPosition);

        enum
        {
            SLICES_4 = 0
        };

    protected:
        void createQuad(float x, float y, float depth, float texCoordMagnifier = 1.f);

        ramses::Effect* m_effect;
        ramses::Node* m_groupNode;
        const ramses::ArrayResource* m_vertexPositions;
        const ramses::ArrayResource* m_indices;
        ramses::Texture3D* m_texture;
    };
}
