//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TESTRUNNER_H
#define RAMSES_TESTRUNNER_H

#include "PlatformAbstraction/PlatformTypes.h"
#include "Collections/Vector.h"

#include "Utils/LogContext.h"
#include "PerformanceTestBase.h"
#include "PerformanceTestAssert.h"
#include "PerformanceTestData.h"

namespace ramses_internal {

class TestRunner
{
public:
    TestRunner(PerformanceTestData& data, const LogContext& logContext);
    ~TestRunner();

    void run(UInt32 testTimeSec, bool asTotalTime);
    bool hasErrors() const;

private:
    void runTest(PerformanceTestBase& test, UInt64 durationMS);

    const PerformanceTestData& m_data;
    const LogContext& m_logContext;
    ramses::RamsesFramework& m_framework;
    ramses::RamsesClient& m_client;
    bool m_hasErrors;
};
}
#endif
