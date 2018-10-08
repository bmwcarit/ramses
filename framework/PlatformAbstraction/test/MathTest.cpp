//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "PlatformAbstraction/PlatformMath.h"
#include <gtest/gtest.h>

namespace ramses_internal
{
    TEST(PlatformMath, abs1)
    {
        Int val = 5;
        EXPECT_EQ(5, PlatformMath::Abs(val));
        val = -5;
        EXPECT_EQ(5, PlatformMath::Abs(val));
    }

    TEST(PlatformMath, abs2)
    {
        double val = 6;
        EXPECT_EQ(6, PlatformMath::Abs(val));
        val = -6;
        EXPECT_EQ(6, PlatformMath::Abs(val));
    }

    TEST(PlatformMath, abs3)
    {
        float val = 7;
        EXPECT_EQ(7, PlatformMath::Abs(val));
        val = -7;
        EXPECT_EQ(7, PlatformMath::Abs(val));
    }

    TEST(PlatformMath, PI)
    {
        float  val = PlatformMath::PI_f;
        double val2 = PlatformMath::PI_d;
        EXPECT_TRUE(val > 3);
        EXPECT_TRUE(val2 > 3);
    }

    TEST(PlatformMath, Log2)
    {
        EXPECT_FLOAT_EQ(3.321928f, PlatformMath::Log2(10.f));
        EXPECT_NEAR(3.3219280948873644, PlatformMath::Log2(10.0), 0.0000000000001);
    }

    TEST(PlatformMath, Exp)
    {
        EXPECT_FLOAT_EQ(200.33685f, PlatformMath::Exp(5.3f));
        EXPECT_NEAR(200.33680997479166, PlatformMath::Exp(5.3), 0.0000000000001);
    }

    TEST(PlatformMath, Ceil)
    {
        EXPECT_FLOAT_EQ(3.0f, PlatformMath::Ceil(2.3f));
        EXPECT_FLOAT_EQ(3.0f, PlatformMath::Ceil(2.8f));
        EXPECT_FLOAT_EQ(-2.0f, PlatformMath::Ceil(-2.8f));
        EXPECT_DOUBLE_EQ(3.0, PlatformMath::Ceil(2.3));
        EXPECT_DOUBLE_EQ(3.0, PlatformMath::Ceil(2.8));
        EXPECT_DOUBLE_EQ(-2.0, PlatformMath::Ceil(-2.8));
    }

    TEST(PlatformMath, Floor)
    {
        EXPECT_FLOAT_EQ(2.0f, PlatformMath::Floor(2.3f));
        EXPECT_FLOAT_EQ(2.0f, PlatformMath::Floor(2.8f));
        EXPECT_FLOAT_EQ(-3.0f, PlatformMath::Floor(-2.3f));
        EXPECT_DOUBLE_EQ(2.0, PlatformMath::Floor(2.3));
        EXPECT_DOUBLE_EQ(2.0, PlatformMath::Floor(2.8));
        EXPECT_DOUBLE_EQ(-3.0, PlatformMath::Floor(-2.3));
    }

    TEST(PlatformMath, Sqrt)
    {
        EXPECT_FLOAT_EQ(3.0f, PlatformMath::Sqrt(9.0f));
        EXPECT_DOUBLE_EQ(3.0, PlatformMath::Sqrt(9.0));
    }

    TEST(PlatformMath, Pow)
    {
        EXPECT_FLOAT_EQ(4.0f, PlatformMath::Pow2(2.0f));
        EXPECT_DOUBLE_EQ(4.0, PlatformMath::Pow2(2.0));

        EXPECT_FLOAT_EQ(8.0f, PlatformMath::Pow(2.0f, 3.0f));
        EXPECT_DOUBLE_EQ(8.0, PlatformMath::Pow(2.0, 3.0));

        EXPECT_FLOAT_EQ(PlatformMath::Pow(4.0f, 2.0f), PlatformMath::Pow2(4.0f));
        EXPECT_DOUBLE_EQ(PlatformMath::Pow(4.0, 2.0), PlatformMath::Pow2(4.0));
    }

    TEST(PlatformMath, Cos)
    {
        double pid = PlatformMath::PI_d;
        float pif = PlatformMath::PI_f;

        EXPECT_FLOAT_EQ(-1.0f, PlatformMath::Cos(pif));
        EXPECT_DOUBLE_EQ(-1.0, PlatformMath::Cos(pid));
    }

    TEST(PlatformMath, Sin)
    {
        double pid = PlatformMath::PI_d;
        float pif = PlatformMath::PI_f;

        EXPECT_FLOAT_EQ(1.0f, PlatformMath::Sin(pif/2.0f));
        EXPECT_DOUBLE_EQ(1.0, PlatformMath::Sin(pid/2.0));
    }

    TEST(PlatformMath, Tan)
    {
        double pid = PlatformMath::PI_d;
        float pif = PlatformMath::PI_f;

        EXPECT_FLOAT_EQ(1.0f, PlatformMath::Tan(pif/4.0f));
        EXPECT_DOUBLE_EQ(1.0, PlatformMath::Tan(pid/4.0));
    }

    TEST(PlatformMath, ArcCos)
    {
        double pid = PlatformMath::PI_d;
        float pif = PlatformMath::PI_f;

        EXPECT_FLOAT_EQ(pif, PlatformMath::ArcCos(-1.0f));
        EXPECT_DOUBLE_EQ(pid, PlatformMath::ArcCos(-1.0));
    }

    TEST(PlatformMath, ArcSin)
    {
        double pid = PlatformMath::PI_d;
        float pif = PlatformMath::PI_f;

        EXPECT_FLOAT_EQ(pif/2.0f, PlatformMath::ArcSin(1.0f));
        EXPECT_DOUBLE_EQ(pid/2.0, PlatformMath::ArcSin(1.0));
    }

    TEST(PlatformMath, ArcTan)
    {
        double pid = PlatformMath::PI_d;
        float pif = PlatformMath::PI_f;

        EXPECT_FLOAT_EQ(pif/4.0f, PlatformMath::ArcTan(1.0f));
        EXPECT_DOUBLE_EQ(pid/4.0, PlatformMath::ArcTan(1.0));
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
