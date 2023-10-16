//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/AppearanceEnums.h"
#include "internal/SceneGraph/SceneAPI/RenderState.h"

namespace ramses::internal
{
    class AppearanceUtils
    {
    public:
        static bool GeometryShaderCompatibleWithDrawMode(EDrawMode geometryShaderInputType, EDrawMode drawMode)
        {
            // only basic 'variant' (i.e. no strip/fan) of a primitive is allowed as GS input declaration
            assert(geometryShaderInputType == EDrawMode::Points || geometryShaderInputType == EDrawMode::Lines || geometryShaderInputType == EDrawMode::Triangles);

            switch (drawMode)
            {
            case EDrawMode::Points:
                return geometryShaderInputType == EDrawMode::Points;
            case EDrawMode::Lines:
            case EDrawMode::LineStrip:
            case EDrawMode::LineLoop:
                return geometryShaderInputType == EDrawMode::Lines;
            case EDrawMode::Triangles:
            case EDrawMode::TriangleStrip:
            case EDrawMode::TriangleFan:
                return geometryShaderInputType == EDrawMode::Triangles;
            }

            assert(false);
            return false;
        }
    };
}
