//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ARRAYINPUTSCENE_H
#define RAMSES_ARRAYINPUTSCENE_H

#include "IntegrationScene.h"
#include "SceneAPI/Handles.h"
#include "ramses-client-api/MeshNode.h"
#include "Collections/Vector.h"


namespace ramses_internal
{
    class Vector4;

    class ArrayInputScene : public IntegrationScene
    {
    public:
        ArrayInputScene(ramses::Scene& scene, UInt32 state, const Vector3& cameraPosition);

        enum
        {
            ARRAY_INPUT_VEC4 = 0,
            ARRAY_INPUT_INT32,
            ARRAY_INPUT_INT32_DYNAMIC_INDEX
        };
    };
}

#endif
