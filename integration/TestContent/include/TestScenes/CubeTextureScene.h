//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CUBETEXTURESCENE_H
#define RAMSES_CUBETEXTURESCENE_H

#include "IntegrationScene.h"
#include "DataTypesImpl.h"
#include "SceneAPI/Handles.h"
#include "Collections/Vector.h"

namespace ramses
{
    class TextureCube;
    class Node;
}

namespace ramses_internal
{
    class CubeTextureScene : public IntegrationScene
    {
    public:
        CubeTextureScene(ramses::Scene& scene, uint32_t state, const glm::vec3& cameraPosition);

        enum EState
        {
            EState_RGBA8 = 0,
            EState_BGRA_Swizzled,
            EState_Float
        };

    private:
        void init(EState state);
        void initializeUnitSphere();
        void divideUnitSphereTriangle(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, long depth);
        ramses::TextureCube* createTextureCube(EState state);

        ramses::Effect* m_effect;
        ramses::MeshNode* m_sphereMesh;
        ramses::Node* m_transformNode;

        std::vector<ramses::vec3f> m_spherePositions;
        std::vector<ramses::vec3f> m_sphereNormals;
        std::vector<uint16_t> m_sphereIndices;
    };
}

#endif
