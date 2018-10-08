//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Utils/CommandLineParser.h"
#include "Utils/Argument.h"
#include "Utils/StringUtils.h"
#include "Utils/LogMacros.h"
#include "Utils/File.h"
#include "Utils/FileUtils.h"
#include "ConsoleLogCapture.h"
#include "StressTestFactory.h"
#include "StressTest.h"

#include <numeric>
#include <cstdlib>
#include <cstdio>
#include <utility>
#include <algorithm>

using namespace ramses_internal;

using TestVector = Vector<UInt32>;

void PrintTests(int32_t argc, const char* argv[])
{
    CommandLineParser parser(argc, argv);
    if( ArgumentBool(parser, "lt", "list-tests", false) )
    {
        for(UInt32 t = 0; t < StressTestFactory::GetNumberOfTests(); ++t)
        {
            printf("Test index %u - %s\n", t, StressTestFactory::GetNameOfTest(t));
        }
        exit(0);
    }
}

TestVector GetTestsToRun(int32_t argc, const char* argv[])
{
    CommandLineParser parser(argc, argv);
    const String testNumberString = ArgumentString(parser, "tn", "test-number", "");
    // The test-number might offer a complicated structure. It may consist of a
    // comma separated list of values. These values could be integers or two
    // integeres separated by a dash. While the first version marks a specific test
    // number, the second version (incl. the dash) marks a range of the numbers
    // including both integers
    // e.g: 1,3,6-11,19 would yield the tests [1,3,6,7,8,9,10,11,19]

    // First check whether the argument was set at all
    if (testNumberString.getLength() == 0)
    {
        // The default is to use all tests
        TestVector tests(StressTestFactory::GetNumberOfTests());
        std::iota(tests.begin(), tests.end(), 0);
        return tests;
    }
    else
    {
        // Some argument has been provided
        TestVector tests;
        StringVector tokens;
        // first split up the comma separated values
        StringUtils::Tokenize(testNumberString, tokens, ',');
        for(auto token: tokens)
        {
            // for each of these values check whether there is a dash present
            auto rangeDelimiterIndex = token.indexOf('-');
            if(rangeDelimiterIndex < 0)
            {
                // so there is no dash, then interpret the value as integer
                tests.push_back(static_cast<UInt32>(std::atoi(token.c_str())));
            }
            else
            {
                // with a dash get the two values and interpret them as range ends
                UInt32 rangeStart = static_cast<UInt32>(std::atoi(token.substr(0, rangeDelimiterIndex).c_str()));
                UInt32 rangeEnd   = static_cast<UInt32>(std::atoi(token.substr(rangeDelimiterIndex+1, token.getLength()).c_str()));

                // if the first value has been omited (e.g. "-7"), take everything from the start
                if(rangeDelimiterIndex==0)
                {
                    rangeStart = 0;
                }

                // if the second value has been omited (e.g."7-"), take everything until the end
                if(static_cast<UInt>(rangeDelimiterIndex+1) == token.getLength())
                {
                    // as rangeEnd is gets a +1 later we need to -1 it here, or we add a non
                    // existing test
                    rangeEnd = StressTestFactory::GetNumberOfTests() - 1;
                }

                if(rangeStart>rangeEnd)
                {
                    std::swap(rangeStart, rangeEnd);
                }

                // increase end by 1 to also include this test number
                ++rangeEnd;

                TestVector rangeToAdd( rangeEnd - rangeStart );
                std::iota(rangeToAdd.begin(), rangeToAdd.end(), rangeStart);
                tests.insert(tests.end(), rangeToAdd.begin(), rangeToAdd.end());
            }
        }
        return tests;
    }
}

Int32 RunTest(StressTestPtr& testObject)
{
    ConsoleLogCapture logCapture(ELogLevel::Error);

    Int32 returnValue = testObject->run();

    const float runningTimeSec = std::min(static_cast<float>(testObject->runningTimeMs())/1000.f, 99999999.f);  // max running time is ~3 years, this should be enough
    // Report string is length of characters (22) + max number of digits for running loops (uint32-> max 10 digits) + max number of digits for running
    // time (3 years -> 8+3 digits, with decimal '.' and two digits after the decimal "%.2f") + one stop char '\0'  ==> 44 chars
    char runningTimeReport[44];
    sprintf(runningTimeReport, "(%u iterations in %.2f sec.)", testObject->runningLoops(), runningTimeSec);

    if (logCapture.hasMessages() || returnValue != 0)
    {
        returnValue = -1;
        LOG_ERROR(CONTEXT_TEST, "Test " << testObject->name() << " finished with errors. " << runningTimeReport);
    }
    else
    {
        GetRamsesLogger().setLogLevelForContexts(ELogLevel::Info);
        LOG_INFO(CONTEXT_TEST, "Test " << testObject->name() << " finished successfully. " << runningTimeReport);
        GetRamsesLogger().setLogLevelForContexts(ELogLevel::Error);
    }
    return returnValue;
}

Int32 RunTests(TestVector testsToRun, int32_t argc, const char* argv[])
{
    File resultDirectory("StressTestResults");
    FileUtils::RemoveDirectory(resultDirectory);
    FileUtils::CreateDirectories(resultDirectory);

    StringOutputStream verboseTestResults;
    Int32 returnValue = 0;
    for (auto testNumber : testsToRun)
    {
        StressTestPtr testObject = StressTestFactory::CreateTest(testNumber, argc, argv);
        testObject->init();

        if (RunTest(testObject) != 0)
        {
            returnValue -= (1 << testNumber);
            verboseTestResults << "Test " << testNumber << " [" << testObject->name() << "] failed!\n";
        }
        else
        {
            verboseTestResults << "Test " << testNumber << " [" << testObject->name() << "] passed!\n";
        }
    }

    StringOutputStream memoryResults;
    memoryResults << testsToRun.size() << "\n";
    File resultNumTests(resultDirectory, "ClientStressTests-NumTestResults");
    FileUtils::WriteAllText(resultNumTests, memoryResults.c_str());

    printf("Test results:\n%s", verboseTestResults.c_str());

    return returnValue;
}

int main(int argc, const char *argv[])
{
    PrintTests(argc, argv);

    TestVector testsToRun = GetTestsToRun(argc, argv);
    return RunTests(testsToRun, argc, argv);
}
