//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_QUAD_H
#define RAMSES_QUAD_H

#include "Vector4i.h"
#include <algorithm>

namespace ramses_internal
{
    class Quad
    {
    public:
        Quad();
        Quad(Int32 _x, Int32 _y, Int32 _width, Int32 _height);

        bool operator==(const Quad& q) const;
        bool operator!=(const Quad& q) const;

        [[nodiscard]] Quad getBoundingQuad(const Quad& other) const;
        [[nodiscard]] Int32 getArea() const;

        Int32 x = 0;
        Int32 y = 0;
        Int32 width = 0;
        Int32 height = 0;
    };
}

#endif
