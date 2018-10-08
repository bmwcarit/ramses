//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PIXELRECTANGLE_H
#define RAMSES_PIXELRECTANGLE_H

#include "PlatformAbstraction/PlatformTypes.h"

namespace ramses_internal
{
    struct PixelRectangle
    {
        UInt32 x;
        UInt32 y;
        Int32 width;
        Int32 height;

        static Bool IsSameSizeAs(const PixelRectangle& first, const PixelRectangle& second)
        {
            return first.width == second.width && first.height == second.height;
        }
    };
}

#endif
