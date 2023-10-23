//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include <cstdint>

namespace ramses::internal
{
    struct Viewport
    {
        Viewport()
            : Viewport(0, 0, 0, 0)
        {}

        Viewport(int32_t x, int32_t y, uint32_t w, uint32_t h)
            : posX(x)
            , posY(y)
            , width(w)
            , height(h)
        {
        }

        int32_t posX;
        int32_t posY;
        uint32_t width;
        uint32_t height;

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
