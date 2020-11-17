//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ARRAYRESOURCESCENE_H
#define RAMSES_ARRAYRESOURCESCENE_H

#include "IntegrationScene.h"
#include "SceneAPI/Handles.h"
#include "ramses-client-api/MeshNode.h"

namespace ramses
{
    class Effect;
    class ArrayResource;
}

namespace ramses_internal
{
    class ArrayResourceScene : public IntegrationScene
    {
    public:
        ArrayResourceScene(ramses::Scene& scene, UInt32 state, const Vector3& cameraPosition);

        enum
        {
            ARRAY_RESOURCE_INTERLEAVED,
            ARRAY_RESOURCE_INTERLEAVED_TWO_STRIDES,
            ARRAY_RESOURCE_INTERLEAVED_SINGLE_ATTRIB,
            ARRAY_RESOURCE_INTERLEAVED_START_VERTEX,
        };

    private:
        void createGeometry(ramses::Effect& effect, UInt32 state);
        ramses::Effect* createEffect(UInt32 state);

        void createVertexArrayInterleaved();
        void createVertexArrayInterleavedTwoStrides();
        void createVertexArrayInterleavedSingleAttrib();
        void createVertexArrayInterleavedStartVertex();

        ramses::MeshNode* m_meshNode = nullptr;
        ramses::Effect& m_effect;
        ramses::GeometryBinding* m_geometry = nullptr;
    };
}

#endif
