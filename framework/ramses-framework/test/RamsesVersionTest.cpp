//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-framework-api/RamsesVersion.h"
#include "ramses-sdk-build-config.h"
#include "gtest/gtest.h"

namespace ramses
{
    TEST(ARamsesVersion, hasExpectedValues)
    {
        const RamsesVersion version = GetRamsesVersion();
        EXPECT_STREQ(ramses_sdk::RAMSES_SDK_RAMSES_VERSION, version.string);
        EXPECT_EQ(ramses_sdk::RAMSES_SDK_PROJECT_VERSION_MAJOR_INT, version.major);
        EXPECT_EQ(ramses_sdk::RAMSES_SDK_PROJECT_VERSION_MINOR_INT, version.minor);
        EXPECT_EQ(ramses_sdk::RAMSES_SDK_PROJECT_VERSION_PATCH_INT, version.patch);
    }
}
