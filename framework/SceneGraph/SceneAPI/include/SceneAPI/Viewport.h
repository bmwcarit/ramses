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
        Viewport(Int32 x = 0, Int32 y = 0, UInt32 w = 0, UInt32 h = 0)
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

        Bool operator==(const Viewport& other) const
        {
            return  posX == other.posX &&
                    posY == other.posY &&
                    width == other.width &&
                    height == other.height;
        }

        Bool operator!=(const Viewport& other) const
        {
            return !operator==(other);
        }
    };
}

#endif
