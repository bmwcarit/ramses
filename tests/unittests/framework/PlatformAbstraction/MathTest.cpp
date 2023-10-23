//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/PlatformAbstraction/PlatformMath.h"
#include <gtest/gtest.h>

namespace ramses::internal
{
    TEST(PlatformMath, PI)
    {
        float  val = PlatformMath::PI_f;
        double val2 = PlatformMath::PI_d;
        EXPECT_TRUE(val > 3);
        EXPECT_TRUE(val2 > 3);
    }

    TEST(PlatformMath, Rad2Deg)
    {
        EXPECT_FLOAT_EQ(180.0f, PlatformMath::Rad2Deg(PlatformMath::PI_f));
        EXPECT_DOUBLE_EQ(180.0, PlatformMath::Rad2Deg(PlatformMath::PI_d));
    }

    TEST(PlatformMath, Deg2Rad)
    {
        EXPECT_FLOAT_EQ(PlatformMath::PI_f, PlatformMath::Deg2Rad(180.0f));
        EXPECT_DOUBLE_EQ(PlatformMath::PI_d, PlatformMath::Deg2Rad(180.0));
    }
}
