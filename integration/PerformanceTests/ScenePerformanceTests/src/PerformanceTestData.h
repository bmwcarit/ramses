//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PERFORMANCETESTDATA_H
#define RAMSES_PERFORMANCETESTDATA_H

#include "ramses-framework-api/RamsesFramework.h"
#include "PerformanceTestBase.h"

namespace ramses_internal {
class PerformanceTestAssert;

class PerformanceTestData
{
public:

    PerformanceTestData(ramses_internal::String filterIn, ramses_internal::String filterOut);
    ~PerformanceTestData();

    void init();

    uint32_t getTestCount() const;

    friend class TestRunner;

    template <typename T> T* createTest(ramses_internal::String testName, uint32_t testState);
    PerformanceTestAssert& createAssert(PerformanceTestBase* inputTest);
    bool isMatchedByFilter(ramses_internal::String testName) const;

    const PerformanceTestBaseVector& getTests() const;

private:

    typedef std::vector<PerformanceTestAssert*> PerformanceTestAssertVector;

    PerformanceTestBaseVector m_tests;
    PerformanceTestAssertVector m_testAsserts;

    ramses_internal::String m_filterIn;
    ramses_internal::String m_filterOut;
};

template <typename T>
T* PerformanceTestData::createTest(ramses_internal::String testName, uint32_t testState)
{
    if (isMatchedByFilter(testName))
    {
        T* result = new T(testName, testState);
        m_tests.push_back(result);
        return result;
    }

    return NULL;
}
}
#endif
