//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererTestUtils.h"
#include "Utils/CommandLineParser.h"
#include "Utils/Argument.h"
#include "Utils/StringUtils.h"
#include "DmaOffscreenBufferRenderingTests.h"
#include "ramses-framework-api/RamsesFrameworkConfig.h"

using namespace ramses_internal;
using namespace ramses;

int main(int argc, const char *argv[])
{
    CommandLineParser parser(argc, argv);
    ArgumentBool generateBitmaps(parser, "gb", "generate-bitmaps");
    ArgumentString filterInTest(parser, "fi", "filterIn", "*");
    ArgumentString filterOutTest(parser, "fo", "filterOut", "");
    ArgumentUInt32 repeatTestCount(parser, "rc", "repeatCount", 1);

    std::vector<ramses_internal::String> filterInTestStrings = StringUtils::Tokenize(filterInTest, ':');
    std::vector<ramses_internal::String> filterOutTestStrings = StringUtils::Tokenize(filterOutTest, ':');

    RendererTestUtils::SetCommandLineParamsForAllTests(argc, argv);
    const ramses::RamsesFrameworkConfig config(argc, argv);
    DmaOffscreenBufferRenderingTests renderingTests(filterInTestStrings, filterOutTestStrings, generateBitmaps, config);

    for (ramses_internal::UInt32 i = 0; i < repeatTestCount; ++i)
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
