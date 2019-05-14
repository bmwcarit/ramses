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
#include "SceneAPI/Handles.h"
#include "Math3d/Vector3.h"
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
        CubeTextureScene(ramses::RamsesClient& ramsesClient, ramses::Scene& scene, UInt32 state, const Vector3& cameraPosition);

        enum EState
        {
            EState_RGBA8 = 0,
            EState_Float
        };

    private:
        void init(EState state);
        void initializeUnitSphere();
        void divideUnitSphereTriangle(Vector3 p1, Vector3 p2, Vector3 p3, long depth);
        ramses::TextureCube* createTextureCube(EState state);

        ramses::Effect* m_effect;
        ramses::MeshNode* m_sphereMesh;
        ramses::Node* m_transformNode;

        std::vector<Vector3> m_spherePositions;
        std::vector<Vector3> m_sphereNormals;
        std::vector<UInt16> m_sphereIndices;
    };
}

#endif
