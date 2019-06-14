//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-framework-api/DcsmApiTypes.h"
#include "Components/DcsmComponent.h"
#include "gtest/gtest.h"
#include <unordered_set>

namespace ramses
{
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
}
