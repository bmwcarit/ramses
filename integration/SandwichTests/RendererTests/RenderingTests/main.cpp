//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererTestUtils.h"
#include "Utils/CommandLineParser.h"
#include "Utils/Argument.h"
#include "Utils/StringUtils.h"
#include "RenderingTests.h"
#include "ramses-framework-api/RamsesFrameworkConfig.h"

using namespace ramses_internal;
using namespace ramses;

int main(int argc, const char *argv[])
{
    CommandLineParser parser(argc, argv);
    ArgumentBool generateBitmaps(parser, "gb", "generate-bitmaps", false);
    ArgumentString filterInTest(parser, "fi", "filterIn", "*");
    ArgumentString filterOutTest(parser, "fo", "filterOut", "");
    ArgumentUInt32 repeatTestCount(parser, "rc", "repeatCount", 1);
    ArgumentUInt32 waylandIviLayerId(parser, "lid", "waylandIviLayerId", 3);

    std::vector<ramses_internal::String>  filterInTestStrings;
    std::vector<ramses_internal::String>  filterOutTestStrings;
    StringUtils::Tokenize(filterInTest, filterInTestStrings, ':');
    StringUtils::Tokenize(filterOutTest, filterOutTestStrings, ':');

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

    RendererTestUtils::SetWaylandIviLayerID(waylandIviLayerId);
    const ramses::RamsesFrameworkConfig config(argc, argv);
    RenderingTests renderingTests(filterInTestStrings, filterOutTestStrings, generateBitmaps, config);

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
