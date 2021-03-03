//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererTestUtils.h"
#include "Utils/CommandLineParser.h"
#include "Utils/Argument.h"
#include "Utils/StringUtils.h"
#include "EmbeddedCompositingTests.h"
#include "Utils/RamsesLogger.h"
#include "EmbeddedCompositingTestFramework/TestForkingController.h"
#include "ramses-framework-api/RamsesFrameworkConfig.h"
#include <pwd.h>
#include <grp.h>

int main(int argc, const char *argv[])
{
    ramses_internal::CommandLineParser parser(argc, argv);
    ramses_internal::ArgumentBool generateBitmaps(parser, "gb", "generate-bitmaps");
    ramses_internal::ArgumentString filterInTest(parser, "fi", "filterIn", "*");
    ramses_internal::ArgumentString filterOutTest(parser, "fo", "filterOut", "");
    ramses_internal::ArgumentUInt32 repeatTestCount(parser, "rc", "repeatCount", 1);


    // It is not allowed to call fork after DLT_REGISTER_APP.
    // For the compositing tests, we don't need DLT at all, so just disable DLT.
    ramses_internal::GetRamsesLogger().initialize(parser, ramses_internal::String(), ramses_internal::String(), true, true);

    //create renderer config to get EC display name from cmd line args
    const ramses::RendererConfig dummyRendererConfig(argc, argv);
    const ramses_internal::String embeddedCompositingSocketName = dummyRendererConfig.getWaylandEmbeddedCompositingSocketName();
    //The creation of the forking controller MUST happen before doing anything!!!
    //Do not move this from here, and do not do anything meaningful before it!!!
    ramses_internal::TestForkingController forkingController(embeddedCompositingSocketName);

    RendererTestUtils::SetCommandLineParamsForAllTests(argc, argv);

    std::vector<ramses_internal::String>  filterInTestStrings;
    std::vector<ramses_internal::String>  filterOutTestStrings;
    ramses_internal::StringUtils::Tokenize(filterInTest, filterInTestStrings, ':');
    ramses_internal::StringUtils::Tokenize(filterOutTest, filterOutTestStrings, ':');

    RendererTestUtils::SetMaxFrameCallbackPollingTimeForAllTests(std::chrono::microseconds{10000000});

    ramses::RamsesFrameworkConfig config(argc, argv);

    ramses_internal::EmbeddedCompositingTests embeddedCompositingTests(forkingController, embeddedCompositingSocketName, filterInTestStrings, filterOutTestStrings, generateBitmaps, config);

    for (ramses_internal::UInt32 i = 0; i < repeatTestCount; ++i)
    {
        const auto success = embeddedCompositingTests.runTests();
        embeddedCompositingTests.logReport();

        if (!success)
            return 1;
    }

    return 0;
}
