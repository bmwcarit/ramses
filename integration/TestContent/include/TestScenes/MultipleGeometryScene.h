//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_MULTIPLEGEOMETRYSCENE_H
#define RAMSES_MULTIPLEGEOMETRYSCENE_H

#include "IntegrationScene.h"
#include "SceneAPI/Handles.h"
#include "ramses-client-api/MeshNode.h"


namespace ramses_internal
{
    class MultipleGeometryScene : public IntegrationScene
    {
    public:
        MultipleGeometryScene(ramses::RamsesClient& ramsesClient, ramses::Scene& scene, UInt32 state, const Vector3& cameraPosition);

        enum
        {
            MULTI_TRIANGLE_LIST_GEOMETRY_WITH_INDEX_ARRAY = 0,
            MULTI_TRIANGLE_LIST_GEOMETRY_WITHOUT_INDEX_ARRAY,
            MULTI_TRIANGLE_STRIP_GEOMETRY_WITHOUT_INDEX_ARRAY,
            VERTEX_ARRAYS_WITH_OFFSET
        };

    private:
        void createGeometries(ramses::Effect& effect, UInt32 state);

        static const UInt32 NumMeshes = 4u;
        ramses::MeshNode* m_meshNode[NumMeshes];
    };
}

#endif
