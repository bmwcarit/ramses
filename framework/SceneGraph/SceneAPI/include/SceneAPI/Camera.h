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
    struct Frustum
    {
        Frustum()
        {
        }

        Frustum(Float _leftPlane, Float _rightPlane, Float _bottomPlane, Float _topPlane, Float _nearPlane, Float _farPlane)
            : leftPlane(_leftPlane)
            , rightPlane(_rightPlane)
            , bottomPlane(_bottomPlane)
            , topPlane(_topPlane)
            , nearPlane(_nearPlane)
            , farPlane(_farPlane)
        {
        }

        Float leftPlane = -1.f;
        Float rightPlane = 1.f;
        Float bottomPlane = -1.f;
        Float topPlane = 1.f;
        Float nearPlane = 0.1f;
        Float farPlane = 1.f;
    };

    struct Camera
    {
        ECameraProjectionType projectionType = ECameraProjectionType_Renderer;
        Frustum frustum;
        NodeHandle node;
        DataInstanceHandle viewportDataInstance;

        static constexpr DataFieldHandle ViewportOffsetField{ 0 };
        static constexpr DataFieldHandle ViewportSizeField{ 1 };
    };
}

#endif
