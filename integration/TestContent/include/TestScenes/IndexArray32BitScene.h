//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_INDEXARRAY32BITSCENE_H
#define RAMSES_INDEXARRAY32BITSCENE_H

#include "IntegrationScene.h"
#include "SceneAPI/Handles.h"
#include "ramses-client-api/MeshNode.h"

namespace ramses_internal
{
    class IndexArray32BitScene : public IntegrationScene
    {
    public:
        IndexArray32BitScene(ramses::Scene& scene, uint32_t state, const glm::vec3& cameraPosition);

        void create16BitIndexArray(ramses::GeometryBinding* geometry);
        void create32BitIndexArray(ramses::GeometryBinding* geometry);

        enum
        {
            NO_OFFSET_16BIT_INDICES = 0,
            OFFSET_16BIT_INDICES = 1,
            NO_OFFSET_32BIT_INDICES = 2,
            OFFSET_32BIT_INDICES = 3
        };

    private:
        void createGeometry(ramses::Effect& effect,uint32_t state);

        ramses::MeshNode* m_meshNode;
    };
}

#endif
