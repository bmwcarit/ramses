//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "PlatformAbstraction/PlatformStringUtils.h"
#include "gtest/gtest.h"

namespace ramses_internal
{
TEST(APlatformStringUtils, CopyWithEnoughBuffer)
{
    char string1[20] = "My String";
    char string2[30];
    PlatformStringUtils::Copy(string2, sizeof(string2), string1);
    EXPECT_STREQ(string1, string2);
}

TEST(APlatformStringUtils, CopyWithTruncation)
{
    char string1[20] = "My String";
    char string2[20];
    std::memset(string2, 42, sizeof(string2));
    PlatformStringUtils::Copy(string2, 4, string1);
    EXPECT_STREQ("My ", string2);

    // check that only 4 bytes are written, an no more
    for (size_t i = 4; i < sizeof(string2); ++i)
        EXPECT_EQ(42, string2[i]);
}
}
