//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Utils/CommandLineParser.h"
#include "Utils/Argument.h"
#include "Utils/LogContext.h"
#include "Utils/LogMacros.h"
#include "Utils/RamsesLogger.h"
#include "MemoryTracking.h"
#include "TestRunner.h"
#include "ConsoleResultPrinter.h"
#include "FileResultPrinter.h"

int main(int argc, const char *argv[])
{
    using namespace ramses_internal;
    // Set up parameters
    const CommandLineParser parser(argc, argv);
    const ArgumentString filterInArg(parser, "fi", "filterIn", "*");
    const ArgumentString filterOutArg(parser, "fo", "filterOut", "*");
    const ArgumentString outputFile(parser, "o", "output", "-");

    // Only one of these should be provided
    const Int32 singleTestDurationInSeconds = ArgumentInt32(parser, "s", "singleDuration", -1);
    const Int32 totalDurationInSeconds = ArgumentInt32(parser, "t", "totalDuration", -1);
    const bool useTimePerTest = singleTestDurationInSeconds != -1;
    const bool useTotalTime = totalDurationInSeconds != -1;

    if (useTimePerTest == useTotalTime)
    {
        printf("Usage: ScenePerformanceTest <arguments>\n");
        printf("Arguments:\n\n");
        printf("--filterIn <filterString>      Only run tests where the test name matches (by substring) this filter string.\n");
        printf("--filterOut <filterString>     Skip tests where the test name matches (by substring) this filter string.\n");
        printf("--output <outputFileName>      Write results out to a file. This is optional.\n");
        printf("--totalDuration <sec>          Run the program for a given total time. The duration of each test will be split between this time.\n");
        printf("--singleDuration <sec>         Run each test for the given time. The duration of the full program will be number of tests multiplied by this time.\n");
        printf("Remark: Only use either --totalDuration or --singleDuration. Not both at the same time.\n\n");
        printf("Examples:\n");
        printf("'ScenePerformanceTest --totalDuration 400'\n");
        printf("'ScenePerformanceTest --filterIn mySpecificTests --singleDuration 10'\n");
        printf("'ScenePerformanceTest --filterOut someTestName --singleDuration 2 --output myResultsFile.txt'\n");
        return 1;
    }

    if (outputFile.hasValue() && useTotalTime)
    {
        printf("Please use an explicit time per test (--singleDuration) when saving results to a file.");
        return 1;
    }

    PerformanceTestData testData(filterInArg, filterOutArg);

    TestRunner runner(testData, CONTEXT_TEST);
    runner.run(useTotalTime ? totalDurationInSeconds : singleTestDurationInSeconds, useTotalTime);

    // Always print to console
    ConsoleResultPrinter consolePrinter(CONTEXT_TEST);
    consolePrinter.printTestResult(testData.getTests());

    if (outputFile.wasDefined())
    {
        FileResultPrinter filePrinter(outputFile);
        filePrinter.printTestResult(testData.getTests(), CONTEXT_TEST, singleTestDurationInSeconds);
    }

    return runner.hasErrors() ? 1 : 0;
}

#if _MSC_VER
// Ignores: "warning C4290: C++ exception specification ignored except to indicate a function is not __declspec(nothrow)"
#pragma warning( disable : 4290 )

// For now, only attempt this stuff on Windows. It should also be able to run on Linux in the future,
// but for now it conflicts with Valgrind and GoogleTest which both also tries to override new/delete operators.
void* operator new(std::size_t n) throw(std::bad_alloc)
{
    MemoryTracking::GetInstance().addAllocation(n);
    return malloc(n);
}

void operator delete(void* p) throw()
{
    free(p);
}

void* operator new[](std::size_t n) throw(std::bad_alloc)
{
    MemoryTracking::GetInstance().addAllocation(n);
    return malloc(n);
}
void operator delete[](void* p) throw()
{
    free(p);
}
#endif
