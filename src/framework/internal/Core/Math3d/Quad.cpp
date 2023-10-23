//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Core/Math3d/Quad.h"

namespace ramses::internal
{
    Quad::Quad() = default;

    Quad::Quad(int32_t _x, int32_t _y, int32_t _width, int32_t _height)
        : x(_x)
        , y(_y)
        , width(_width)
        , height(_height)
    {
    }

    bool Quad::operator==(const Quad& q) const
    {
        return x == q.x && y == q.y && width == q.width && height == q.height;
    }

    bool Quad::operator!=(const Quad& q) const
    {
        return !(*this == q);
    }

    Quad Quad::getBoundingQuad(const Quad& other) const
    {
        if (0 == width || 0 == height)
            return other;

        if (0 == other.width || 0 == other.height)
            return *this;

        const auto minX = std::min(x, other.x);
        const auto minY = std::min(y, other.y);
        const auto maxX = std::max(x + width , other.x + other.width);
        const auto maxY = std::max(y + height, other.y + other.height);

        return { minX, minY, maxX - minX, maxY - minY };
    }

    int32_t Quad::getArea() const
    {
        return width * height;
    }
}
