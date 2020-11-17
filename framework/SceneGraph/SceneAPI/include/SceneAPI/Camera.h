//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENEAPI_CAMERA_H
#define RAMSES_SCENEAPI_CAMERA_H

#include "SceneAPI/Handles.h"
#include "SceneAPI/Viewport.h"
#include "SceneAPI/ECameraProjectionType.h"

namespace ramses_internal
{
    struct Camera
    {
        ECameraProjectionType projectionType = ECameraProjectionType::Perspective;
        NodeHandle node;
        DataInstanceHandle dataInstance;

        static constexpr DataFieldHandle ViewportOffsetField{ 0 };
        static constexpr DataFieldHandle ViewportSizeField{ 1 };
        static constexpr DataFieldHandle FrustumPlanesField{ 2 };
        static constexpr DataFieldHandle FrustumNearFarPlanesField{ 3 };
    };
}

#endif
