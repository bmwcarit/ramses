//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <cassert>
#include <algorithm>
#include <vector>

namespace ramses
{
    struct Vec2i
    {
        Vec2i() = default;
        Vec2i(uint32_t x_, uint32_t y_)
            : x(x_), y(y_)
        {
        }

        bool operator==(const Vec2i& other) const
        {
            return x == other.x && y == other.y;
        }

        bool operator!=(const Vec2i& other) const
        {
            return !operator==(other);
        }

        uint32_t x = 0u;
        uint32_t y = 0u;
    };

    using QuadOffset = Vec2i;

    struct QuadSize : Vec2i
    {
        QuadSize() = default;
        QuadSize(uint32_t x_, uint32_t y_)
            : Vec2i(x_, y_)
        {
        }

        [[nodiscard]] uint32_t getArea() const
        {
            return x * y;
        }
    };

    class Quad
    {
    public:
        Quad(const QuadOffset& offset, const QuadSize& size)
            : m_offset(offset)
            , m_size(size)
        {
        }

        [[nodiscard]] const QuadSize& getSize() const
        {
            return m_size;
        }

        [[nodiscard]] const QuadOffset& getOrigin() const
        {
            return m_offset;
        }

        [[nodiscard]] bool hasCommonEdge(const Quad& other) const
        {
            const uint32_t min1x = m_offset.x;
            const uint32_t min1y = m_offset.y;
            const uint32_t min2x = other.m_offset.x;
            const uint32_t min2y = other.m_offset.y;

            const uint32_t max1x = min1x + m_size.x;
            const uint32_t max1y = min1y + m_size.y;
            const uint32_t max2x = min2x + other.m_size.x;
            const uint32_t max2y = min2y + other.m_size.y;

            if ((min1y == min2y) && (max1y == max2y))
            {
                return (max1x == min2x) || (max2x == min1x);
            }

            if ((min1x == min2x) && (max1x == max2x))
            {
                return (max1y == min2y) || (max2y == min1y);
            }
            return false;
        }

        bool merge(const Quad& other)
        {
            if (!hasCommonEdge(other))
                return false;

            assert(0 != m_size.getArea());
            assert(0 != other.m_size.getArea());


            const uint32_t max1x = m_offset.x + m_size.x;
            const uint32_t max1y = m_offset.y + m_size.y;
            const uint32_t max2x = other.m_offset.x + other.m_size.x;
            const uint32_t max2y = other.m_offset.y + other.m_size.y;

            m_offset = QuadOffset
            {
                std::min(m_offset.x, other.m_offset.x),
                std::min(m_offset.y, other.m_offset.y)
            };

            m_size = QuadSize
            {
                std::max(max1x, max2x) - m_offset.x,
                std::max(max1y, max2y) - m_offset.y
            };

            return true;
        }

        [[nodiscard]] bool intersects(Quad const& other) const
        {
            return !(m_offset.x >= other.m_offset.x + other.m_size.x ||
                     m_offset.y >= other.m_offset.y + other.m_size.y ||
                     other.m_offset.x >= m_offset.x + m_size.x ||
                     other.m_offset.y >= m_offset.y + m_size.y);
        }

    private:
        QuadOffset m_offset;
        QuadSize m_size;
    };

    using Quads = std::vector<Quad>;
}
