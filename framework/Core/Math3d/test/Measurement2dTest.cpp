//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Measurement2dTest.h"

namespace ramses_internal
{
    Measurement2dTest::Measurement2dTest()
    {
    }

    Measurement2dTest::~Measurement2dTest()
    {
    }

    void Measurement2dTest::SetUp()
    {
    }

    void Measurement2dTest::TearDown()
    {
    }

    TEST_F(Measurement2dTest, DefaultConstructor)
    {
        EXPECT_FLOAT_EQ(0.0f, mMeasurement.width);
        EXPECT_FLOAT_EQ(0.0f, mMeasurement.height);
    }

    TEST_F(Measurement2dTest, WidthHeightConstructor)
    {
        Measurement2d<Float> measurement(20.f, 30.f);

        EXPECT_FLOAT_EQ(20.0f, measurement.width);
        EXPECT_FLOAT_EQ(30.0f, measurement.height);
    }

    TEST_F(Measurement2dTest, ConvertConstructor)
    {
        Measurement2d<Float> floatMeasurement(20.f, 30.f);
        Measurement2d<UInt32> intMeasurement(floatMeasurement);
        EXPECT_EQ(20U, intMeasurement.width);
        EXPECT_EQ(30U, intMeasurement.height);
    }

    TEST_F(Measurement2dTest, GetAspect)
    {
        Measurement2d<Float> floatMeasurement(10.f, 20.f);
        EXPECT_FLOAT_EQ(0.5f, floatMeasurement.getAspect());
    }

    TEST_F(Measurement2dTest, Equal)
    {
        Measurement2d<Float> floatMeasurement1(20.f, 30.f);
        Measurement2d<Float> floatMeasurement2(20.f, 30.f);

        EXPECT_TRUE(floatMeasurement1 == floatMeasurement2);
    }
}
