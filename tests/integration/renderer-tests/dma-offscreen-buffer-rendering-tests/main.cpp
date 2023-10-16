//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererTestUtils.h"
#include "internal/Core/Utils/StringUtils.h"
#include "DmaOffscreenBufferRenderingTests.h"
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
    ramses::RendererConfig rendererConfig;
    ramses::DisplayConfig displayConfig;

    try
    {
        cli.add_flag("--gb,--generate-bitmaps", generateBitmaps);
        cli.add_option("--fi,--filter-in", filterIn);
        cli.add_option("--fo,--filter-out", filterOut);
        cli.add_option("--rc,--repeat", repeatCount);
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

    RendererTestUtils::SetDefaultConfigForAllTests(rendererConfig, displayConfig);
    DmaOffscreenBufferRenderingTests renderingTests(filterInTestStrings, filterOutTestStrings, generateBitmaps, config);

    for (uint32_t i = 0; i < repeatCount; ++i)
    {
        const bool renderingTestsSuccess = renderingTests.runTests();
        renderingTests.logReport();

        if (!renderingTestsSuccess)
        {
            fmt::print("Some rendering tests failed! Look above for more detailed info.\n");
            return 1;
        }
    }

    return 0;
}
