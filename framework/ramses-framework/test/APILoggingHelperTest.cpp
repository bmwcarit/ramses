//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "APILoggingHelper.h"

namespace ramses_internal
{
    TEST(APILoggingHelperTest, emptyArgumentsTest)
    {
        int32_t argc = 0;
        const char* argv[1] = {};
        EXPECT_EQ(APILoggingHelper::MakeLoggingString(argc, argv), "0 arguments: [  ]");
    }

    TEST(APILoggingHelperTest, singleArgumentTest)
    {
        int32_t argc = 1;
        const char* argv[1] = {"foo"};
        EXPECT_EQ(APILoggingHelper::MakeLoggingString(argc, argv), "1 arguments: [ foo ]");
    }

    TEST(APILoggingHelperTest, multiArgumentsTest)
    {
        int32_t argc = 3;
        const char* argv[3] = {"foo", "bar", "huh"};
        EXPECT_EQ(APILoggingHelper::MakeLoggingString(argc, argv), "3 arguments: [ foo ; bar ; huh ]");
    }
}
