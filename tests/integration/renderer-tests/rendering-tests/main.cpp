//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererTestUtils.h"
#include "internal/Core/Utils/StringUtils.h"
#include "RenderingTests.h"
#include "ramses/framework/RamsesFrameworkConfig.h"
#include "ramses-cli.h"

using namespace ramses::internal;
using namespace ramses;

int main(int argc, const char *argv[])
{
    CLI::App cli;

    bool generateBitmaps = false;
    std::string filterIn = "*";
    std::string filterOut;
    uint32_t    repeatCount = 1u;
    ramses::RamsesFrameworkConfig config{ramses::EFeatureLevel_Latest};
    ramses::RendererConfig        rendererConfig;
    ramses::DisplayConfig         displayConfig;

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

    auto filterInTestStrings = StringUtils::Tokenize(filterIn, ':');
    auto filterOutTestStrings = StringUtils::Tokenize(filterOut, ':');

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
    bool renderingTestsSuccess = false;
    std::array<std::string, EFeatureLevel_Latest> reports;

    for (EFeatureLevel featureLevel = EFeatureLevel_01; featureLevel <= EFeatureLevel_Latest; featureLevel = EFeatureLevel{ featureLevel + 1 })
    {
        fmt::print("\nStarting feature level 0{} rendering tests\n\n", featureLevel);
        if (!config.setFeatureLevel(featureLevel))
            return 1;

        RenderingTests renderingTests(filterInTestStrings, filterOutTestStrings, generateBitmaps, config);
        for (uint32_t i = 0; i < repeatCount; ++i)
        {
            renderingTestsSuccess = renderingTests.runTests();
            reports[featureLevel-1] = renderingTests.generateReport();
            fmt::print("\nFinished feature level 0{} rendering tests\n", featureLevel);
            fmt::print("{}\n", reports[featureLevel-1]);

            if (!renderingTestsSuccess)
            {
                fmt::print("Some rendering tests failed! Look above for more detailed info.\n");
                return 1;
            }
        }
    }

    for (EFeatureLevel featureLevel = EFeatureLevel_01; featureLevel <= EFeatureLevel_Latest; featureLevel = EFeatureLevel{ featureLevel + 1 })
    {
        fmt::print("\nFeature level 0{}:", featureLevel);
        fmt::print("{}\n", reports[featureLevel-1]);
    }

    return 0;
}
