//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-framework-api/DcsmApiTypes.h"
#include "RamsesFrameworkTypesImpl.h"
#include "fmt/format.h"
#include "gtest/gtest.h"
#include <unordered_set>

namespace ramses
{
    TEST(DcsmApiTypes, hasRectComparison)
    {
        Rect r1{ 1,2,3,4};
        Rect r2{ 1,2,3,4 };
        Rect r3{ 1,2,4,3 };

        EXPECT_TRUE(r1 == r1);
        EXPECT_FALSE(r1 != r1);

        EXPECT_TRUE(r1 == r2);
        EXPECT_FALSE(r1 != r2);

        EXPECT_FALSE(r1 == r3);
        EXPECT_TRUE(r1 != r3);
    }

    TEST(DcsmApiTypes, canPrintRect)
    {
        EXPECT_EQ("2/3/6/7", fmt::to_string(Rect{2, 3, 6, 7}));
    }

    TEST(DcsmApiTypes, hasSizeInfoComparison)
    {
        SizeInfo si1{1, 2};
        SizeInfo si2{1, 2};
        SizeInfo si3{2, 1};

        EXPECT_TRUE(si1 == si1);
        EXPECT_FALSE(si1 != si1);

        EXPECT_TRUE(si1 == si2);
        EXPECT_FALSE(si1 != si2);

        EXPECT_FALSE(si1 == si3);
        EXPECT_TRUE(si1 != si3);
    }

    TEST(DcsmApiTypes, canPrintSizeInfo)
    {
        EXPECT_EQ("10/20", fmt::to_string(SizeInfo{10, 20}));
    }

    TEST(DcsmApiTypes, hasAnimationInformationComparison)
    {
        AnimationInformation ai1{1, 2};
        AnimationInformation ai2{1, 2};
        AnimationInformation ai3{2, 1};

        EXPECT_TRUE(ai1 == ai1);
        EXPECT_FALSE(ai1 != ai1);

        EXPECT_TRUE(ai1 == ai2);
        EXPECT_FALSE(ai1 != ai2);

        EXPECT_FALSE(ai1 == ai3);
        EXPECT_TRUE(ai1 != ai3);
    }

    TEST(DcsmApiTypes, canPrintAnimationInformation)
    {
        EXPECT_EQ("[200; 300]", fmt::to_string(AnimationInformation{200, 300}));
    }
}
