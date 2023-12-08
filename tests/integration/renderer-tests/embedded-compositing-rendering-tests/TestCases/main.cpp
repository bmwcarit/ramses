//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererTestUtils.h"
#include "internal/Core/Utils/StringUtils.h"
#include "EmbeddedCompositingTests.h"
#include "impl/RamsesLoggerImpl.h"
#include "EmbeddedCompositingTestFramework/TestForkingController.h"
#include "ramses/framework/RamsesFrameworkConfig.h"
#include <pwd.h>
#include <grp.h>
#include "ramses-cli.h"

int main(int argc, const char *argv[])
{
    CLI::App cli;

    bool generateBitmaps = false;
    std::string filterIn = "*";
    std::string filterOut;
    uint32_t    repeatCount = 1u;
    ramses::RamsesFrameworkConfig config{ramses::EFeatureLevel_Latest};
    ramses::RendererConfig rendererConfig;
    ramses::DisplayConfig displayConfig;

    try
    {
        cli.add_flag("--gb,--generate-bitmaps", generateBitmaps);
        cli.add_option("--fi,--filter-in", filterIn);
        cli.add_option("--fo,--filter-out", filterOut);
        cli.add_option("--rc,--repeat", repeatCount);
        cli.add_option("--gtest_output"); // added by cmake/ctest
        ramses::registerOptions(cli, config);
        ramses::registerOptions(cli, rendererConfig);
        ramses::registerOptions(cli, displayConfig);
    }
    catch (const CLI::Error& error)
    {
        std::cerr << error.what();
        return -1;
    }
    CLI11_PARSE(cli, argc, argv);

    // It is not allowed to call fork after DLT_REGISTER_APP.
    // For the compositing tests, we don't need DLT at all, so just disable DLT.
    ramses::internal::RamsesLoggerConfig loggerConfig;
    ramses::internal::GetRamsesLogger().initialize(loggerConfig, true, true);

    //The creation of the forking controller MUST happen before doing anything!!!
    //Do not move this from here, and do not do anything meaningful before it!!!
    ramses::internal::TestForkingController forkingController;

    ramses::internal::RendererTestUtils::SetDefaultConfigForAllTests(rendererConfig, displayConfig);

    auto filterInTestStrings = ramses::internal::StringUtils::Tokenize(filterIn, ':');
    auto filterOutTestStrings = ramses::internal::StringUtils::Tokenize(filterOut, ':');

    ramses::internal::RendererTestUtils::SetMaxFrameCallbackPollingTimeForAllTests(std::chrono::microseconds{10000000});

    ramses::internal::EmbeddedCompositingTests embeddedCompositingTests(forkingController, filterInTestStrings, filterOutTestStrings, generateBitmaps, config);

    for (uint32_t i = 0; i < repeatCount; ++i)
    {
        const auto success = embeddedCompositingTests.runTests();
        embeddedCompositingTests.logReport();

        if (!success)
            return 1;
    }

    return 0;
}
