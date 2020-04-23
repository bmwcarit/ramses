//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#include "AnimationTimeTest.h"
#include "gtest/gtest.h"

using namespace testing;

namespace ramses_internal
{
    TEST_F(AnimationTimeTest, InitialState)
    {
        EXPECT_FALSE(m_time.isValid());
    }

    TEST_F(AnimationTimeTest, InitialiazedState)
    {
        m_time = 0u;
        EXPECT_TRUE(m_time.isValid());
    }

    TEST_F(AnimationTimeTest, ComparisonEq)
    {
        m_time = 999u;
        EXPECT_EQ(m_time, AnimationTime(999u));
        EXPECT_EQ(m_time, AnimationTime(m_time));
    }

    TEST_F(AnimationTimeTest, ComparisonLtLe)
    {
        m_time = 999u;
        EXPECT_LT(m_time, AnimationTime(1000u));
        EXPECT_LE(m_time, AnimationTime(999u));
        EXPECT_LE(m_time, AnimationTime(1000u));
    }

    TEST_F(AnimationTimeTest, ComparisonGtGe)
    {
        m_time = 999u;
        EXPECT_GT(m_time, AnimationTime(998u));
        EXPECT_GE(m_time, AnimationTime(999u));
        EXPECT_GE(m_time, AnimationTime(998u));
    }

    TEST_F(AnimationTimeTest, GetDuration)
    {
        m_time = 200u;
        AnimationTime time2 = 100u;
        EXPECT_EQ(100u, m_time.getDurationSince(time2));
    }

    TEST_F(AnimationTimeTest, GetInvalidDuration)
    {
        m_time = 200u;
        AnimationTime time2 = 300u;
        const AnimationTime invalidTime = AnimationTime::InvalidTimeStamp;
        EXPECT_EQ(invalidTime, m_time.getDurationSince(time2));
    }
}
