//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Utils/StringUtils.h"
#include "ResourceStressTests.h"
#include "CLI/CLI.hpp"

using namespace ramses_internal;

int main(int argc, const char *argv[])
{
    CLI::App cli;

    StressTestConfig testConfig;

    int32_t testNumber = -1;
    testConfig.durationEachTestSeconds = 20;
    testConfig.displayCount                                 = 2;
    testConfig.sceneSetsPerDisplay                          = 1;
    testConfig.disableSkippingOfFrames                      = false;
    testConfig.perFrameBudgetMSec_ClientRes                 = 10000;
    testConfig.perFrameBudgetMSec_Rendering                 = 16000;
    testConfig.renderablesBatchSizeForRenderingInterruption = 1;

    try
    {
        cli.add_option("--tn,--test-nr", testNumber);
        cli.add_option("-d,--displays", testConfig.displayCount);
        cli.add_option("--sspd,--scenes-per-display", testConfig.sceneSetsPerDisplay);
        cli.add_flag("--no-skip", testConfig.disableSkippingOfFrames);
        cli.add_option("--lc,--res-upload-limit", testConfig.perFrameBudgetMSec_ClientRes);
        cli.add_option("--lr,--rendering-limit", testConfig.perFrameBudgetMSec_Rendering);
        cli.add_option("--rpl,--renderable-per-loop", testConfig.renderablesBatchSizeForRenderingInterruption);
        cli.add_option("--duration", testConfig.durationEachTestSeconds, "test duration in seconds");
        testConfig.frameworkConfig.registerOptions(cli);
        testConfig.rendererConfig.registerOptions(cli);
        testConfig.displayConfig.registerOptions(cli);
    }
    catch (const CLI::Error& error)
    {
        std::cerr << error.what();
        return -1;
    }
    CLI11_PARSE(cli, argc, argv);

    if (testNumber == -1)
    {
        return ResourceStressTests::RunAllTests(testConfig);
    }
    else
    {
        return ResourceStressTests::RunTest(static_cast<EStressTestCaseId>(testNumber), testConfig);
    }
}
