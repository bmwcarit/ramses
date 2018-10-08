//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Utils/CommandLineParser.h"
#include "Utils/Argument.h"
#include "Utils/StringUtils.h"
#include "ResourceStressTests.h"

using namespace ramses_internal;

int main(int argc, const char *argv[])
{
    CommandLineParser parser(argc, argv);
    const Int32 testNumber          = ArgumentInt32 (parser, "tn"       , "test-number"                 , -1);
    const UInt32 displayCountNumber = ArgumentUInt32(parser, "d"        , "display-count"               , 2);
    const UInt32 scenesPerDisplay   = ArgumentUInt32(parser, "sspd"     , "scenes-sets-per-display"     , 1);
    const bool disableSkipping      = ArgumentBool  (parser, "noskip"   , "never-skip-rendering"        , false);
    const UInt32 limitClientRes     = ArgumentUInt32(parser, "lc"       , "res-upload-limit"            , 10000);
    const UInt32 limitSceneActions  = ArgumentUInt32(parser, "ls"       , "sceneactions-limit"          , 12000);
    const UInt32 limitRendering     = ArgumentUInt32(parser, "lr"       , "rendering-limit"             , 16000);
    const UInt32 renderablePerLoop  = ArgumentUInt32(parser, "rpl"      , "renderable-per-loop"         , 1);
    // Some tests don't work with duration < 15 seconds
    // TODO Violin refactor stress tests to be similar to other sandwich tests
    const UInt32 durationEachTest   = ArgumentUInt32(parser, "dur"      , "duration-each-test-seconds"  , 20);

    const StressTestConfig testConfig = {
        argc,
        argv,
        durationEachTest,
        displayCountNumber,
        scenesPerDisplay,
        disableSkipping,
        limitClientRes,
        limitSceneActions,
        limitRendering,
        renderablePerLoop
    };

    if (testNumber == -1)
    {
        return ResourceStressTests::RunAllTests(testConfig);
    }
    else
    {
        return ResourceStressTests::RunTest(static_cast<EStressTestCaseId>(testNumber), testConfig);
    }
}
