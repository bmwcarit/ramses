//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "ramses-framework-api/RamsesFramework.h"
#include "ramses-framework-api/RamsesFramework.h"
#include "ramses-client-api/RamsesClient.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/RamsesClient.h"
#include "Collections/String.h"
#include "Collections/StringOutputStream.h"
#include "Collections/HashSet.h"
#include "PlatformAbstraction/PlatformMath.h"
#include "PlatformAbstraction/PlatformTime.h"
#include "TablePrinter.h"
#include "Utils/LogMacros.h"

#include "TestRunner.h"
#include "MemoryTracking.h"

namespace ramses_internal {

TestRunner::TestRunner(PerformanceTestData& data, const LogContext& logContext)
    : m_data(data)
    , m_logContext(logContext)
    , m_framework(*new ramses::RamsesFramework())
    , m_client(*new ramses::RamsesClient("ScenePerformanceTests-client", m_framework))
    , m_hasErrors(false)
{

}

TestRunner::~TestRunner()
{
    delete &m_client;
    delete &m_framework;
}

void TestRunner::run(UInt32 testTimeSec, bool asTotalTime)
{
    if (m_data.getTestCount() == 0)
    {
        return;
    }

    UInt64 timePerTestInMS = 0;

    if (asTotalTime)
    {
        timePerTestInMS = (testTimeSec * 1000) / m_data.getTestCount();
    }
    else
    {
        timePerTestInMS = testTimeSec * 1000;
    }

    // Run all individual tests
    for (UInt32 i = 0; i < m_data.m_tests.size(); i++)
    {
        PerformanceTestBase& test = *m_data.m_tests[i];
        const String testName = test.getTestName();
        LOG_INFO(m_logContext, "Runnning '" << testName.c_str() << "'...");

        runTest(test, timePerTestInMS);
    }

    // Check all asserts
    for (UInt32 i = 0; i < m_data.m_testAsserts.size(); i++)
    {
        PerformanceTestAssert& testAssert = *m_data.m_testAsserts[i];
        PerformanceTestBase* firstTest = testAssert.getFirst();
        PerformanceTestBase* secondTest = testAssert.getSecond();

        // Both involved tests must have been run (not filtered out)
        if (firstTest && secondTest)
        {
            const bool passed = testAssert.validateAssert();

            if (passed)
            {
                firstTest->addPassedAssert();
                secondTest->addPassedAssert();
            }
            else
            {
                firstTest->addFailedAssert();
                secondTest->addFailedAssert();
                m_hasErrors = true;
            }
        }
    }
}

void TestRunner::runTest(PerformanceTestBase& test, UInt64 durationMS)
{
    const uint32_t sceneId = 12345u;
    ramses::Scene& scene = *m_client.createScene(sceneId);

    test.initTest(m_client, scene);

    MemoryTracking& memoryTracking = MemoryTracking::GetInstance();

    // Memory tracking is global in the application, so must be reset for each test run.
    memoryTracking.reset();
    memoryTracking.setEnabled(false);

    const UInt64 TestDurationInMicroSec = durationMS * 1000;
    // The planned end time - will be advanced by the amount of time taken by preUpdate().
    UInt64 currentEndTime = ramses_internal::PlatformTime::GetMicrosecondsMonotonic() + TestDurationInMicroSec;

    // In case of a very slow preUpdate (compared to update()), this will be the guard value for how long the test should run
    const UInt64 MaximumAllowedEndTime = currentEndTime + 20 * TestDurationInMicroSec;

    UInt64 loopCount = 0;

    do
    {
        const UInt64 preUpdateBegin = ramses_internal::PlatformTime::GetMicrosecondsMonotonic();
        test.preUpdate();
        const UInt64 preUpdateEnd = ramses_internal::PlatformTime::GetMicrosecondsMonotonic();
        const UInt64 preUpdateDuration = preUpdateEnd - preUpdateBegin;

        currentEndTime += preUpdateDuration; // Add time taken
        currentEndTime = ramses_internal::min(currentEndTime, MaximumAllowedEndTime);

        memoryTracking.setEnabled(true);
        test.update();
        memoryTracking.setEnabled(false);
        loopCount++;

    } while (ramses_internal::PlatformTime::GetMicrosecondsMonotonic() < currentEndTime);

    m_client.destroy(scene);

    // Save data collected for the test run
    test.setLoopCount(loopCount);
    test.setMemoryAllocationStats(  memoryTracking.getAllocationCount(),
                                    memoryTracking.getTotalAllocationsInBytes(),
                                    memoryTracking.getSmallestAllocationInBytes(),
                                    memoryTracking.getLargestAllocationInBytes());
}

bool TestRunner::hasErrors() const
{
    return m_hasErrors;
}
}
