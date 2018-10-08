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
#include "UnixUtilities/UnixDomainSocketHelper.h"

int main(int argc, const char *argv[])
{
    ramses_internal::CommandLineParser parser(argc, argv);
    ramses_internal::ArgumentBool generateBitmaps(parser, "gb", "generate-bitmaps", false);
    ramses_internal::ArgumentUInt32 waylandIviLayerId(parser, "lid", "waylandIviLayerId", 3);
    ramses_internal::ArgumentString waylandSocketEmbedded(parser, "wse", "wayland-socket-embedded", "wayland-100");

    // It is not allowed to call fork after DLT_REGISTER_APP.
    // For the compositing tests, we don't need DLT at all, so just disable DLT.
    ramses_internal::GetRamsesLogger().initialize(parser, ramses_internal::String(), ramses_internal::String(), true);

    //The creation of the forking controller MUST happen before doing anything!!!
    //Do not move this from here, and do not do anything meaningful before it!!!
    ramses_internal::TestForkingController forkingController(waylandSocketEmbedded);

    ramses_internal::UnixDomainSocketHelper socketHelper(waylandSocketEmbedded);

    RendererTestUtils::SetWaylandIviLayerID(waylandIviLayerId);
    RendererTestUtils::SetWaylandSocketEmbeddedFileDescriptor(socketHelper.createBoundFileDescriptor());

    ramses::RamsesFrameworkConfig config(argc, argv);

    // just want to run a single test "ShowStreamTexture", but as "ShowStreamTexture" exists as prefix in other tests
    // we have to filter out other tests by using their prefixes
    const ramses_internal::Vector<ramses_internal::String>  filterInTestStrings = { "ShowStreamTexture" };
    const ramses_internal::Vector<ramses_internal::String>  filterOutTestStrings = {"ShowStreamTextureAfter", "ShowStreamTextureOn", "ShowStreamTextureWhen"};

    ramses_internal::EmbeddedCompositingTests embeddedCompositingTests(forkingController, filterInTestStrings, filterOutTestStrings, generateBitmaps, config);

    const int returnValue = embeddedCompositingTests.runTests() ? 0 : 1;
    embeddedCompositingTests.logReport();

    return returnValue;
}
