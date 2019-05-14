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

ramses_internal::String getUserGroupName()
{
    passwd* pws = getpwuid(geteuid());
    if (pws)
    {
        group* group = getgrgid(pws->pw_gid);
        if (group)
        {
            return group->gr_name;
        }
    }
    return "";
}

int main(int argc, const char *argv[])
{
    ramses_internal::String defaultGroup = getUserGroupName();

    ramses_internal::CommandLineParser parser(argc, argv);
    ramses_internal::ArgumentBool generateBitmaps(parser, "gb", "generate-bitmaps", false);
    ramses_internal::ArgumentString filterInTest(parser, "fi", "filterIn", "*");
    ramses_internal::ArgumentString filterOutTest(parser, "fo", "filterOut", "");
    ramses_internal::ArgumentUInt32 repeatTestCount(parser, "rc", "repeatCount", 1);
    ramses_internal::ArgumentUInt32 waylandIviLayerId(parser, "lid", "waylandIviLayerId", 3);
    ramses_internal::ArgumentString waylandSocketEmbedded(parser, "wse", "wayland-socket-embedded", "wayland-100");
    ramses_internal::ArgumentString waylandSocketEmbeddedGroup(parser, "wsegn", "wayland-socket-embedded-groupname", defaultGroup);

    // It is not allowed to call fork after DLT_REGISTER_APP.
    // For the compositing tests, we don't need DLT at all, so just disable DLT.
    ramses_internal::GetRamsesLogger().initialize(parser, ramses_internal::String(), ramses_internal::String(), true);

    //The creation of the forking controller MUST happen before doing anything!!!
    //Do not move this from here, and do not do anything meaningful before it!!!
    ramses_internal::TestForkingController forkingController(waylandSocketEmbedded);

    std::vector<ramses_internal::String>  filterInTestStrings;
    std::vector<ramses_internal::String>  filterOutTestStrings;
    ramses_internal::StringUtils::Tokenize(filterInTest, filterInTestStrings, ':');
    ramses_internal::StringUtils::Tokenize(filterOutTest, filterOutTestStrings, ':');

    RendererTestUtils::SetWaylandIviLayerID(waylandIviLayerId);
    RendererTestUtils::SetWaylandSocketEmbedded(waylandSocketEmbedded);
    RendererTestUtils::SetWaylandSocketEmbeddedGroup(waylandSocketEmbeddedGroup);
    RendererTestUtils::SetMaxFrameCallbackPollingTime(std::chrono::microseconds{10000000});

    ramses::RamsesFrameworkConfig config(argc, argv);

    ramses_internal::EmbeddedCompositingTests embeddedCompositingTests(forkingController, waylandSocketEmbedded, filterInTestStrings, filterOutTestStrings, generateBitmaps, config);

    for (ramses_internal::UInt32 i = 0; i < repeatTestCount; ++i)
    {
        const auto success = embeddedCompositingTests.runTests();
        embeddedCompositingTests.logReport();

        if (!success)
            return 1;
    }

    return 0;
}
