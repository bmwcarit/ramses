//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererTestUtils.h"
#include "Utils/StringUtils.h"
#include "RenderingTests.h"
#include "ramses-framework-api/RamsesFrameworkConfig.h"
#include "CLI/CLI.hpp"

using namespace ramses_internal;
using namespace ramses;

int main(int argc, const char *argv[])
{
    CLI::App cli;

    bool generateBitmaps = false;
    std::string filterIn = "*";
    std::string filterOut;
    uint32_t    repeatCount = 1u;
    ramses::RamsesFrameworkConfig config;
    ramses::RendererConfig        rendererConfig;
    ramses::DisplayConfig         displayConfig;

    try
    {
        cli.add_flag("--gb,--generate-bitmaps", generateBitmaps);
        cli.add_option("--fi,--filter-in", filterIn);
        cli.add_option("--fo,--filter-out", filterOut);
        cli.add_option("--rc,--repeat", repeatCount);
        cli.add_option("--gtest_output"); // added by cmake/ctest
        config.registerOptions(cli);
        rendererConfig.registerOptions(cli);
        displayConfig.registerOptions(cli);
    }
    catch (const CLI::Error& error)
    {
        std::cerr << error.what();
        return -1;
    }
    CLI11_PARSE(cli, argc, argv);

    std::vector<ramses_internal::String> filterInTestStrings = StringUtils::Tokenize(filterIn.c_str(), ':');
    std::vector<ramses_internal::String> filterOutTestStrings = StringUtils::Tokenize(filterOut.c_str(), ':');

    // TODO Violin try to find better solution than this
    // Convenience filtering for desktop debugging
    // Disable all tests which are known to have issues on desktop/dev machines
    // Tests are filtered properly in tests scripts
#ifdef WIN32
    if (filterOutTestStrings.size() == 0)
    {
        filterOutTestStrings.push_back("RenderTarget_Format_RGB16F");
        filterOutTestStrings.push_back("RenderTarget_Format_RGB32F");
    }
#endif

    RendererTestUtils::SetDefaultConfigForAllTests(rendererConfig, displayConfig);
    RenderingTests renderingTests(filterInTestStrings, filterOutTestStrings, generateBitmaps, config);

    for (ramses_internal::UInt32 i = 0; i < repeatCount; ++i)
    {
        const bool renderingTestsSuccess = renderingTests.runTests();
        renderingTests.logReport();

        if (!renderingTestsSuccess)
        {
            printf("Some rendering tests failed! Look above for more detailed info.\n");
            return 1;
        }
    }

    return 0;
}
