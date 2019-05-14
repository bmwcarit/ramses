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
#include "RendererLifecycleTests.h"

using namespace ramses_internal;
using namespace ramses;

int main(int argc, const char *argv[])
{
    CommandLineParser parser(argc, argv);
    ArgumentString filterInTest(parser, "fi", "filterIn", "*");
    ArgumentString filterOutTest(parser, "fo", "filterOut", "");
    ArgumentUInt32 repeatTestCount(parser, "rc", "repeatCount", 1);
    ArgumentUInt32 waylandIviLayerId(parser, "lid", "waylandIviLayerId", 3);

    std::vector<ramses_internal::String>  filterInTestStrings;
    std::vector<ramses_internal::String>  filterOutTestStrings;
    StringUtils::Tokenize(filterInTest, filterInTestStrings, ':');
    StringUtils::Tokenize(filterOutTest, filterOutTestStrings, ':');

    RendererTestUtils::SetWaylandIviLayerID(waylandIviLayerId);
    ramses_internal::RendererLifecycleTests lifecycleTests(filterInTestStrings, filterOutTestStrings, argc, argv);

    for (ramses_internal::UInt32 i = 0; i < repeatTestCount; ++i)
    {
        const bool successLifecycleTests = lifecycleTests.runTests();

        lifecycleTests.logReport();

        if (!successLifecycleTests)
        {
            printf("Some lifecycle tests failed! Look above for more detailed info.\n");
            return 1;
        }
    }

    return 0;
}
