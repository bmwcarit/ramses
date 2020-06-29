//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "framework_common_gmock_header.h"
#include "PlatformAbstraction/PlatformTypes.h"
#include "Animation/Interpolator.h"
#include "AnimationTestUtils.h"

namespace ramses_internal
{
    class InterpolatorTest : public testing::Test
    {
    public:
    };

    TEST_F(InterpolatorTest, CanInterpolateUnsignedDataTypeCorrectly)
    {
        const UInt32 start = 360u;
        const UInt32 end = 20u;
        const UInt32 interp = Interpolator::InterpolateLinear(start, end, 0.5f);
        EXPECT_EQ(190u, interp);
    }

    TEST_F(InterpolatorTest, FindFractionBezierLinearSpline)
    {
        const Float p0 = 0.f;
        const Float p1 = 0.25f;
        const Float p2 = 0.5f;
        const Float p3 = 1.f;

        for (int i = 0; i <= 100; i += 5)
        {
            const float frac = 0.01f * i;
            const Float fracRes = Interpolator::FindFractionForGivenXOnBezierSpline(p0, p1, p2, p3, frac);
            EXPECT_TRUE(AnimationTestUtils::AreEqual(frac, fracRes));
        }
    }

    TEST_F(InterpolatorTest, FindFractionBezierOutOfRangeClampsValues)
    {
        const Float p0 = 0.f;
        const Float p1 = 0.25f;
        const Float p2 = 0.5f;
        const Float p3 = 1.f;

        Float fracRes = Interpolator::FindFractionForGivenXOnBezierSpline(p0, p1, p2, p3, -0.5f);
        EXPECT_FLOAT_EQ(0.f, fracRes);

        fracRes = Interpolator::FindFractionForGivenXOnBezierSpline(p0, p1, p2, p3, 1.5f);
        EXPECT_FLOAT_EQ(1.f, fracRes);
    }
}
