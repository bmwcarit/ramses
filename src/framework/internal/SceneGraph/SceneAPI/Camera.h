//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/SceneAPI/Handles.h"
#include "internal/SceneGraph/SceneAPI/Viewport.h"
#include "internal/SceneGraph/SceneAPI/ECameraProjectionType.h"

namespace ramses::internal
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
