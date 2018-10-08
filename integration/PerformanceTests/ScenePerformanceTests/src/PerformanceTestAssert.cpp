//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "PerformanceTestAssert.h"
#include "PlatformAbstraction/PlatformMath.h"

namespace ramses_internal {

PerformanceTestAssert::PerformanceTestAssert(PerformanceTestBase* first)
    : m_first(first)
    , m_comparision(EComparison_Undefined)
{

}

void PerformanceTestAssert::isFasterThan(PerformanceTestBase* other)
{
    setValues(EComparison_ThisIsFaster, other, 1.0f);
}

void PerformanceTestAssert::isSlowerThan(PerformanceTestBase* other)
{
    setValues(EComparison_ThisIsSlower, other, 1.0f);
}

void PerformanceTestAssert::isMuchFasterThan(PerformanceTestBase* other, float factor)
{
    setValues(EComparison_ThisIsFaster, other, factor);
}

void PerformanceTestAssert::isMuchSlowerThan(PerformanceTestBase* other, float factor)
{
    setValues(EComparison_ThisIsSlower, other, factor);
}

void PerformanceTestAssert::isSameSpeedAs(PerformanceTestBase* other, float allowedDeviationFactor)
{
    setValues(EComparison_SameSpeed, other, allowedDeviationFactor);
}

void PerformanceTestAssert::setValues(EComparison type, PerformanceTestBase* other, float factor)
{
    m_second = other;
    m_factor = factor;
    m_comparision = type;
}

bool PerformanceTestAssert::validateAssert() const
{
    assert(m_first);
    assert(m_second);

    // High loop count => The method managed to run many times and is therefore fast

    switch (m_comparision)
    {
    case EComparison_ThisIsFaster:
        return m_first->getLoopCount() > (m_second->getLoopCount() * m_factor);
    case EComparison_ThisIsSlower:
        return m_first->getLoopCount() < (m_second->getLoopCount() * m_factor);
    case EComparison_SameSpeed:
    {
        const Double first = static_cast<Double>(m_first->getLoopCount());
        const Double second = static_cast<Double>(m_second->getLoopCount());
        const Double min = ramses_internal::min(first, second);
        const Double max = ramses_internal::max(first, second);

        return (max / min) <= m_factor;
    }
    case EComparison_Undefined:
    default:
    {
        assert(false);
        return false;
    }
    }
}

PerformanceTestBase* PerformanceTestAssert::getFirst() const
{
    return m_first;
}

PerformanceTestBase* PerformanceTestAssert::getSecond() const
{
    return m_second;
}
}
