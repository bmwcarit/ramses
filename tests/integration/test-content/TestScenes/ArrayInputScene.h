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
#include "internal/PlatformAbstraction/Collections/Vector.h"


namespace ramses::internal
{
    class ArrayInputScene : public IntegrationScene
    {
    public:
        ArrayInputScene(ramses::Scene& scene, uint32_t state, const glm::vec3& cameraPosition);

        enum
        {
            ARRAY_INPUT_VEC4 = 0,
            ARRAY_INPUT_INT32,
            ARRAY_INPUT_INT32_DYNAMIC_INDEX
        };
    };
}
