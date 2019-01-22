//  -------------------------------------------------------------------------
//  Copyright (C) 2018 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-citymodel/BoundingBox.h"
#include "ramses-framework-api/RamsesFrameworkTypes.h"

#include "limits"
#include "algorithm"

BoundingBox::BoundingBox()
    : m_min(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max())
    , m_max(std::numeric_limits<float>::lowest(),
            std::numeric_limits<float>::lowest(),
            std::numeric_limits<float>::lowest())
{
}

BoundingBox::BoundingBox(const ramses_internal::Vector3& min, const ramses_internal::Vector3& max)
    : m_min(min)
    , m_max(max)
{
}

void BoundingBox::set(const ramses_internal::Vector3& min, const ramses_internal::Vector3& max)
{
    m_min = min;
    m_max = max;
}

void BoundingBox::add(const ramses_internal::Vector3& p)
{
    for (int i = 0; i < 3; i++)
    {
        m_min.data[i] = ramses_internal::min(m_min.data[i], p.data[i]);
        m_max.data[i] = ramses_internal::max(m_max.data[i], p.data[i]);
    }
}

void BoundingBox::add(const BoundingBox& box)
{
    for (int i = 0; i < 3; i++)
    {
        m_min.data[i] = ramses_internal::min(m_min.data[i], box.getMinimumBoxCorner()[i]);
        m_max.data[i] = ramses_internal::max(m_max.data[i], box.getMaximumBoxCorner()[i]);
    }
}

const ramses_internal::Vector3& BoundingBox::getMinimumBoxCorner() const
{
    return m_min;
}

const ramses_internal::Vector3& BoundingBox::getMaximumBoxCorner() const
{
    return m_max;
}

void BoundingBox::reset()
{
    m_min.set(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    m_max.set(std::numeric_limits<float>::lowest(),
              std::numeric_limits<float>::lowest(),
              std::numeric_limits<float>::lowest());
}

bool BoundingBox::checkOverlap(const BoundingBox& other) const
{
    for (uint32_t i = 0; i < 3; i++)
    {
        if ((m_min.data[i] > other.m_max.data[i]) || (m_max.data[i] < other.m_min.data[i]))
        {
            return false;
        }
    }
    return true;
}

ramses_internal::Vector3 BoundingBox::getPoint(uint32_t index) const
{
    return ramses_internal::Vector3(
        (index & 1) ? m_max.x : m_min.x, (index & 2) ? m_max.y : m_min.y, (index & 4) ? m_max.z : m_min.z);
}

BoundingBox BoundingBox::intersect(const BoundingBox& other) const
{
    ramses_internal::Vector3 min;
    ramses_internal::Vector3 max;
    for (int i = 0; i < 3; i++)
    {
        min.data[i] = ramses_internal::max(m_min.data[i], other.getMinimumBoxCorner()[i]);
        max.data[i] = ramses_internal::min(m_max.data[i], other.getMaximumBoxCorner()[i]);
    }

    return BoundingBox(min, max);
}

bool BoundingBox::isEmpty() const
{
    return (m_min.x > m_max.x) || (m_min.y > m_max.y) || (m_min.z > m_max.z);
}
