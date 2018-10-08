//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PERFORMANCETESTASSERT_H
#define RAMSES_PERFORMANCETESTASSERT_H

#include "PlatformAbstraction/PlatformTypes.h"
#include "Collections/Vector.h"
#include "Utils/LogContext.h"
#include "PerformanceTestBase.h"

namespace ramses_internal {

class PerformanceTestAssert
{
public:
    PerformanceTestAssert(PerformanceTestBase* first);

    void isFasterThan(PerformanceTestBase* other);
    void isSlowerThan(PerformanceTestBase* other);

    void isMuchFasterThan(PerformanceTestBase* other, float factor);
    void isMuchSlowerThan(PerformanceTestBase* other, float factor);

    void isSameSpeedAs(PerformanceTestBase* other, float allowedDeviationFactor);

    bool validateAssert() const;

    PerformanceTestBase* getFirst() const;
    PerformanceTestBase* getSecond() const;

private:

    enum EComparison
    {
        EComparison_Undefined = 0,
        EComparison_ThisIsFaster,
        EComparison_ThisIsSlower,
        EComparison_SameSpeed
    };

    void setValues(EComparison type, PerformanceTestBase* other, float factor);

    PerformanceTestBase* m_first;
    PerformanceTestBase* m_second;
    float m_factor;
    EComparison m_comparision;
};
}
#endif
