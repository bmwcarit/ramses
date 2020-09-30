//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PIXELRECTANGLE_H
#define RAMSES_PIXELRECTANGLE_H

#include <cstdint>

namespace ramses_internal
{
    struct PixelRectangle
    {
        uint32_t x;
        uint32_t y;
        int32_t width;
        int32_t height;
    };
}

#endif
