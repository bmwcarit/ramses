//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_QUAD_H
#define RAMSES_QUAD_H

#include <algorithm>
#include "PlatformAbstraction/PlatformTypes.h"

namespace ramses_internal
{
    class Quad
    {
    public:
        Quad();
        Quad(int32_t _x, int32_t _y, int32_t _width, int32_t _height);

        bool operator==(const Quad& q) const;
        bool operator!=(const Quad& q) const;

        [[nodiscard]] Quad getBoundingQuad(const Quad& other) const;
        [[nodiscard]] int32_t getArea() const;

        int32_t x = 0;
        int32_t y = 0;
        int32_t width = 0;
        int32_t height = 0;
    };
}

#endif
