//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "internal/Core/Utils/TextureMathUtils.h"

namespace ramses::internal
{
    TEST(TextureMathUtilsTest, getsCorrectLowerMipSize)
    {
        EXPECT_EQ(1u, TextureMathUtils::GetLowerMipSize(0u));
        EXPECT_EQ(1u, TextureMathUtils::GetLowerMipSize(1u));
        EXPECT_EQ(1u, TextureMathUtils::GetLowerMipSize(2u));
        EXPECT_EQ(1u, TextureMathUtils::GetLowerMipSize(3u));
        EXPECT_EQ(2u, TextureMathUtils::GetLowerMipSize(4u));
        EXPECT_EQ(2u, TextureMathUtils::GetLowerMipSize(5u));
        EXPECT_EQ(3u, TextureMathUtils::GetLowerMipSize(6u));
        EXPECT_EQ(3u, TextureMathUtils::GetLowerMipSize(7u));
        EXPECT_EQ(4u, TextureMathUtils::GetLowerMipSize(8u));
    }

    TEST(TextureMathUtilsTest, getsCorrectMipLevelSize)
    {
        EXPECT_EQ(8u, TextureMathUtils::GetMipSize(0u, 8u));
        EXPECT_EQ(4u, TextureMathUtils::GetMipSize(1u, 8u));
        EXPECT_EQ(2u, TextureMathUtils::GetMipSize(2u, 8u));
        EXPECT_EQ(1u, TextureMathUtils::GetMipSize(3u, 8u));
        EXPECT_EQ(1u, TextureMathUtils::GetMipSize(4u, 8u));

        EXPECT_EQ(3u, TextureMathUtils::GetMipSize(1u, 7u));
        EXPECT_EQ(1u, TextureMathUtils::GetMipSize(2u, 7u));
        EXPECT_EQ(2u, TextureMathUtils::GetMipSize(1u, 5u));
    }

    TEST(TextureMathUtilsTest, getsCorrectMipCount1D)
    {
        EXPECT_EQ(1u, TextureMathUtils::GetMipLevelCount(1u, 1u, 1u));
        EXPECT_EQ(2u, TextureMathUtils::GetMipLevelCount(2u, 1u, 1u));
        EXPECT_EQ(2u, TextureMathUtils::GetMipLevelCount(3u, 1u, 1u));
        EXPECT_EQ(3u, TextureMathUtils::GetMipLevelCount(4u, 1u, 1u));
        EXPECT_EQ(3u, TextureMathUtils::GetMipLevelCount(5u, 1u, 1u));
        EXPECT_EQ(3u, TextureMathUtils::GetMipLevelCount(6u, 1u, 1u));
        EXPECT_EQ(3u, TextureMathUtils::GetMipLevelCount(7u, 1u, 1u));
        EXPECT_EQ(4u, TextureMathUtils::GetMipLevelCount(8u, 1u, 1u));
    }

    TEST(TextureMathUtilsTest, getsCorrectMipCount2D)
    {
        EXPECT_EQ(1u, TextureMathUtils::GetMipLevelCount(1u, 1u, 1u));
        EXPECT_EQ(2u, TextureMathUtils::GetMipLevelCount(2u, 2u, 1u));
        EXPECT_EQ(2u, TextureMathUtils::GetMipLevelCount(3u, 3u, 1u));
        EXPECT_EQ(3u, TextureMathUtils::GetMipLevelCount(4u, 4u, 1u));
        EXPECT_EQ(3u, TextureMathUtils::GetMipLevelCount(5u, 5u, 1u));
        EXPECT_EQ(3u, TextureMathUtils::GetMipLevelCount(6u, 6u, 1u));
        EXPECT_EQ(3u, TextureMathUtils::GetMipLevelCount(7u, 7u, 1u));
        EXPECT_EQ(4u, TextureMathUtils::GetMipLevelCount(8u, 8u, 1u));

        EXPECT_EQ(4u, TextureMathUtils::GetMipLevelCount(1u, 8u, 1u));
        EXPECT_EQ(3u, TextureMathUtils::GetMipLevelCount(2u, 7u, 1u));
        EXPECT_EQ(3u, TextureMathUtils::GetMipLevelCount(3u, 6u, 1u));
        EXPECT_EQ(3u, TextureMathUtils::GetMipLevelCount(4u, 5u, 1u));
        EXPECT_EQ(3u, TextureMathUtils::GetMipLevelCount(5u, 4u, 1u));
        EXPECT_EQ(3u, TextureMathUtils::GetMipLevelCount(6u, 3u, 1u));
        EXPECT_EQ(3u, TextureMathUtils::GetMipLevelCount(7u, 2u, 1u));
        EXPECT_EQ(4u, TextureMathUtils::GetMipLevelCount(8u, 1u, 1u));
    }

    TEST(TextureMathUtilsTest, getsCorrectMipCount3D)
    {
        EXPECT_EQ(1u, TextureMathUtils::GetMipLevelCount(1u, 1u, 1u));
        EXPECT_EQ(2u, TextureMathUtils::GetMipLevelCount(2u, 2u, 2u));
        EXPECT_EQ(2u, TextureMathUtils::GetMipLevelCount(3u, 3u, 3u));
        EXPECT_EQ(3u, TextureMathUtils::GetMipLevelCount(4u, 4u, 4u));
        EXPECT_EQ(3u, TextureMathUtils::GetMipLevelCount(5u, 5u, 5u));
        EXPECT_EQ(3u, TextureMathUtils::GetMipLevelCount(6u, 6u, 6u));
        EXPECT_EQ(3u, TextureMathUtils::GetMipLevelCount(7u, 7u, 7u));
        EXPECT_EQ(4u, TextureMathUtils::GetMipLevelCount(8u, 8u, 8u));

        EXPECT_EQ(4u, TextureMathUtils::GetMipLevelCount(1u, 1u, 8u));
        EXPECT_EQ(3u, TextureMathUtils::GetMipLevelCount(2u, 1u, 7u));
        EXPECT_EQ(3u, TextureMathUtils::GetMipLevelCount(3u, 1u, 6u));
        EXPECT_EQ(3u, TextureMathUtils::GetMipLevelCount(4u, 1u, 5u));
        EXPECT_EQ(3u, TextureMathUtils::GetMipLevelCount(5u, 1u, 4u));
        EXPECT_EQ(3u, TextureMathUtils::GetMipLevelCount(6u, 1u, 3u));
        EXPECT_EQ(3u, TextureMathUtils::GetMipLevelCount(7u, 1u, 2u));
        EXPECT_EQ(4u, TextureMathUtils::GetMipLevelCount(8u, 1u, 1u));
    }
}
