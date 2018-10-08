//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "PerformanceTestBase.h"
#include "ConsoleResultPrinter.h"
#include "TablePrinter.h"
#include "Utils/LogMacros.h"

namespace ramses_internal {

ConsoleResultPrinter::ConsoleResultPrinter(const ramses_internal::LogContext& logContext)
    : m_logContext(logContext)
{ }

void ConsoleResultPrinter::printTestResult(const PerformanceTestBaseVector& tests)
{
    const char* header = "******************************************************* SCENE PERFORMANCE TESTS ******************************************************";
    const UInt32 headerLength = static_cast<UInt32>(strlen(header));

    StringOutputStream s;
    s << '\n';
    s << header;
    s << '\n';

    TablePrinter printer(headerLength, s);

    // Define & print row with all column labels
    printer.nextLine();
    printer.addColumnDefinition("Test name", 0.5f);
    printer.addColumnDefinition("Loop count", 0.09f);
    printer.addColumnDefinition("Per-Loop-Mem-Alloc (count/size/min/max)", 0.32f);
    printer.addColumnDefinition("Asserts", 0.09f);
    printer.nextLine();
    printer.nextLine();

    bool testsSucceeded = true;

    for (uint32_t i = 0; i < tests.size(); i++)
    {
        PerformanceTestBase& test = *tests[i];

        if (test.getFailedAssertsCount() > 0)
        {
            testsSucceeded = false;
        }

        const String& testName = test.getTestName();

        printer.printValue(testName, 0u);
        printer.printValue(test.getLoopCount(), 1u);

        String memoryInfo = GetMemoryDisplayString(test);
        printer.printValue(memoryInfo.c_str(), 2u);

        printer.printValue(GetAssertDisplayString(test), 3u);
        printer.nextLine();
    }

    // Append footer
    for (uint32_t i = 0; i < headerLength; i++)
    {
        s << '*';
    }

    s << '\n';

    LOG_INFO(m_logContext, s.c_str());

    if (!testsSucceeded)
    {
        LOG_ERROR(m_logContext, "One or more tests failed!");
    }
}

const char* ConsoleResultPrinter::GetAssertDisplayString(const PerformanceTestBase& test)
{
    if (test.getFailedAssertsCount() == 0 &&
        test.getPassedAssertsCount() == 0)
    {
        // No asserts run for this test
        return "-";
    }
    else if (test.getFailedAssertsCount() > 0)
    {
        return "FAILED!!";
    }
    else
    {
        assert(test.getPassedAssertsCount() > 0);
        return "Passed";
    }
}

String ConsoleResultPrinter::GetMemoryDisplayString(const PerformanceTestBase& test)
{
    // Just a sanity check
    if (test.getLoopCount() == 0)
    {
        return String("-");
    }

    StringOutputStream s;

    const uint64_t allocationCount = test.getAllocationCount();
    const uint64_t allocationCountPerRun = allocationCount / test.getLoopCount();

    s << allocationCountPerRun;
    s << '/';

    if (allocationCountPerRun > 0) // Avoid divide by zero
    {
        AppendByteSizeFormat(test.getTotalAllocationsInBytes() / allocationCountPerRun, s);
    }
    else
    {
        AppendByteSizeFormat(0ULL, s);
    }

    s << '/';

    if (allocationCount > 0) // min/max only makes sense if there has been allocations
    {
        AppendByteSizeFormat(test.getSmallestAllocationInBytes(), s);
        s << '/';
        AppendByteSizeFormat(test.getLargestAllocationInBytes(), s);
    }
    else
    {
        AppendByteSizeFormat(0ULL, s);
        s << '/';
        AppendByteSizeFormat(0ULL, s);
    }

    return String(s.c_str());
}

void ConsoleResultPrinter::AppendByteSizeFormat(uint64_t byteCount, StringOutputStream& stream)
{
    if (byteCount > 1000 * 1000)
    {
        stream << (byteCount / (1000 * 1000));
        stream << "MB";
    }
    else if (byteCount > 1000)
    {
        stream << (byteCount / 1000);
        stream << "KB";
    }
    else
    {
        stream << byteCount;
        stream << 'b';
    }
}
}
