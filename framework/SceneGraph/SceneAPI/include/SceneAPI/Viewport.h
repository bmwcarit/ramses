//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENEAPI_VIEWPORT_H
#define RAMSES_SCENEAPI_VIEWPORT_H

#include "PlatformAbstraction/PlatformTypes.h"

namespace ramses_internal
{
    struct Viewport
    {
        Viewport()
            : Viewport(0, 0, 0, 0)
        {}

        Viewport(Int32 x, Int32 y, UInt32 w, UInt32 h)
            : posX(x)
            , posY(y)
            , width(w)
            , height(h)
        {
        }

        Int32 posX;
        Int32 posY;
        UInt32 width;
        UInt32 height;

        bool operator==(const Viewport& other) const
        {
            return  posX == other.posX &&
                    posY == other.posY &&
                    width == other.width &&
                    height == other.height;
        }

        bool operator!=(const Viewport& other) const
        {
            return !operator==(other);
        }
    };
}

#endif
