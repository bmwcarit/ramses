//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PERFORMANCETESTBASE_H
#define RAMSES_PERFORMANCETESTBASE_H

#include "Collections/String.h"
#include "Collections/Vector.h"
#include "ramses-client-api/RamsesClient.h"
#include "ramses-client-api/Scene.h"

class PerformanceTestBase
{
public:

    // Executed once per test case
    virtual void initTest(ramses::RamsesClient& client, ramses::Scene& scene) = 0;
    // Executed at the beginning of every loop iteration before update()
    virtual void preUpdate() {}
    // Executed once per loop iteration
    virtual void update() = 0;

    virtual ~PerformanceTestBase() {}

    PerformanceTestBase(ramses_internal::String testName, uint32_t testState)
        : m_testState(testState)
        , m_testName(testName)
        , m_loopCount(0)
        , m_assertsPassed(0)
        , m_assertsFailed(0)
    {
    }

    void setLoopCount(uint64_t loopCount)
    {
        m_loopCount = loopCount;
    }

    uint64_t getLoopCount() const
    {
        return m_loopCount;
    }

    const ramses_internal::String& getTestName() const
    {
        return m_testName;
    };

    uint32_t getPassedAssertsCount() const
    {
        return m_assertsPassed;
    };

    uint32_t getFailedAssertsCount() const
    {
        return m_assertsFailed;
    };

    void addPassedAssert()
    {
        m_assertsPassed++;
    };

    void addFailedAssert()
    {
        m_assertsFailed++;
    };

    void setMemoryAllocationStats(uint64_t allocationCount, uint64_t totalAllocationsInBytes, uint64_t smallestAllocationInBytes, uint64_t largestAllocationInBytes)
    {
        m_allocationCount = allocationCount;
        m_totalAllocationsInBytes = totalAllocationsInBytes;
        m_smallestAllocationInBytes = smallestAllocationInBytes;
        m_largestAllocationInBytes = largestAllocationInBytes;
    };

    uint64_t getAllocationCount() const
    {
        return m_allocationCount;
    };

    uint64_t getTotalAllocationsInBytes() const
    {
        return m_totalAllocationsInBytes;
    };

    uint64_t getSmallestAllocationInBytes() const
    {
        return m_smallestAllocationInBytes;
    };

    uint64_t getLargestAllocationInBytes() const
    {
        return m_largestAllocationInBytes;
    };

protected:
    const uint32_t m_testState;

private:
    ramses_internal::String m_testName;
    uint64_t m_loopCount;

    uint32_t m_assertsPassed;
    uint32_t m_assertsFailed;

    // Memory allocation stats. These are total sum values for all runs, so need to be divided by m_loopCount.
    uint64_t m_allocationCount;
    uint64_t m_totalAllocationsInBytes;
    uint64_t m_smallestAllocationInBytes;
    uint64_t m_largestAllocationInBytes;
};

typedef std::vector<PerformanceTestBase*> PerformanceTestBaseVector;

#endif
