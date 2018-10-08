//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>

#include "ramses-client-api/SplineStepBool.h"
#include "ramses-client-api/SplineStepInt32.h"
#include "ramses-client-api/SplineStepFloat.h"
#include "ramses-client-api/SplineStepVector2f.h"
#include "ramses-client-api/SplineStepVector3f.h"
#include "ramses-client-api/SplineStepVector4f.h"
#include "ramses-client-api/SplineStepVector2i.h"
#include "ramses-client-api/SplineStepVector3i.h"
#include "ramses-client-api/SplineStepVector4i.h"
#include "ramses-client-api/SplineLinearInt32.h"
#include "ramses-client-api/SplineLinearFloat.h"
#include "ramses-client-api/SplineLinearVector2f.h"
#include "ramses-client-api/SplineLinearVector3f.h"
#include "ramses-client-api/SplineLinearVector4f.h"
#include "ramses-client-api/SplineLinearVector2i.h"
#include "ramses-client-api/SplineLinearVector3i.h"
#include "ramses-client-api/SplineLinearVector4i.h"
#include "ramses-client-api/SplineBezierInt32.h"
#include "ramses-client-api/SplineBezierFloat.h"
#include "ramses-client-api/SplineBezierVector2f.h"
#include "ramses-client-api/SplineBezierVector3f.h"
#include "ramses-client-api/SplineBezierVector4f.h"
#include "ramses-client-api/SplineBezierVector2i.h"
#include "ramses-client-api/SplineBezierVector3i.h"
#include "ramses-client-api/SplineBezierVector4i.h"
#include "SplineImpl.h"

#include "ClientTestUtils.h"
#include "SplineTestMacros.h"
#include "Math3d/Vector2.h"
#include "RamsesObjectTestTypes.h"

using namespace testing;

namespace ramses
{
    class SplineTest : public LocalTestClientWithSceneAndAnimationSystem, public testing::Test
    {
    public:
        SplineTest()
            : LocalTestClientWithSceneAndAnimationSystem(), testing::Test()
            , m_tan1(-1.f, 1.f)
            , m_tan2(-1.f, 1.f)
        {
        }

    protected:
        const ramses_internal::Vector2 m_tan1;
        const ramses_internal::Vector2 m_tan2;
    };

    template <typename SplineType>
    class SplineInitTest : public LocalTestClientWithSceneAndAnimationSystem, public testing::Test {};

    TYPED_TEST_CASE(SplineInitTest, SplineTypes);

    TYPED_TEST(SplineInitTest, splineCreatedWithNoKeys)
    {
        const Spline& spline = this->template createObject<TypeParam>("spline");
        EXPECT_EQ(0u, spline.getNumberOfKeys());
        EXPECT_NE(ramses_internal::SplineHandle::Invalid(), spline.impl.getSplineHandle());
    }

    TYPED_TEST(SplineInitTest, splineCreatedWithNoKeysGivesValidationWarning)
    {
        const Spline& spline = this->template createObject<TypeParam>("spline");
        EXPECT_NE(StatusOK, spline.validate());
    }

    TEST_F(SplineTest, splineWithKeyCanBeValidated)
    {
        SplineStepBool& spline = *this->animationSystem.createSplineStepBool();
        CHECK_SET_GET_KEY1(spline, bool, 0u, 1000u, true);
        EXPECT_EQ(StatusOK, spline.validate());
    }

    TEST_F(SplineTest, setGetKey_SplineStepBool)
    {
        SplineStepBool& spline = *this->animationSystem.createSplineStepBool();
        CHECK_SET_GET_KEY1(spline, bool, 0u, 1000u, true);
        EXPECT_EQ(1u, spline.getNumberOfKeys());
    }

    TEST_F(SplineTest, setGetKey_SplineStepFloat)
    {
        SplineStepFloat& spline = *this->animationSystem.createSplineStepFloat();
        CHECK_SET_GET_KEY1(spline, float, 0u, 1000u, 99.f);
        EXPECT_EQ(1u, spline.getNumberOfKeys());
    }

    TEST_F(SplineTest, setGetKey_SplineStepInt32)
    {
        SplineStepInt32& spline = *this->animationSystem.createSplineStepInt32();
        CHECK_SET_GET_KEY1(spline, int32_t, 0u, 1000u, 99);
        EXPECT_EQ(1u, spline.getNumberOfKeys());
    }

    TEST_F(SplineTest, setGetKey_SplineStepVector2f)
    {
        SplineStepVector2f& spline = *this->animationSystem.createSplineStepVector2f();
        CHECK_SET_GET_KEY2(spline, float, 0u, 1000u, 99.f, -111.f);
        EXPECT_EQ(1u, spline.getNumberOfKeys());
    }

    TEST_F(SplineTest, setGetKey_SplineStepVector3f)
    {
        SplineStepVector3f& spline = *this->animationSystem.createSplineStepVector3f();
        CHECK_SET_GET_KEY3(spline, float, 0u, 1000u, 99.f, 111.f, -111.f);
        EXPECT_EQ(1u, spline.getNumberOfKeys());
    }

    TEST_F(SplineTest, setGetKey_SplineStepVector4f)
    {
        SplineStepVector4f& spline = *this->animationSystem.createSplineStepVector4f();
        CHECK_SET_GET_KEY4(spline, float, 0u, 1000u, 99.f, 111.f, -111.f, -999.f);
        EXPECT_EQ(1u, spline.getNumberOfKeys());
    }

    TEST_F(SplineTest, setGetKey_SplineStepVector2i)
    {
        SplineStepVector2i& spline = *this->animationSystem.createSplineStepVector2i();
        CHECK_SET_GET_KEY2(spline, int32_t, 0u, 1000u, 99, -111);
        EXPECT_EQ(1u, spline.getNumberOfKeys());
    }

    TEST_F(SplineTest, setGetKey_SplineStepVector3i)
    {
        SplineStepVector3i& spline = *this->animationSystem.createSplineStepVector3i();
        CHECK_SET_GET_KEY3(spline, int32_t, 0u, 1000u, 111, -111, -999);
        EXPECT_EQ(1u, spline.getNumberOfKeys());
    }

    TEST_F(SplineTest, setGetKey_SplineStepVector4i)
    {
        SplineStepVector4i& spline = *this->animationSystem.createSplineStepVector4i();
        CHECK_SET_GET_KEY4(spline, int32_t, 0u, 1000u, 99, 111, -111, -999);
        EXPECT_EQ(1u, spline.getNumberOfKeys());
    }

    TEST_F(SplineTest, setGetKey_SplineLinearFloat)
    {
        SplineLinearFloat& spline = *this->animationSystem.createSplineLinearFloat();
        CHECK_SET_GET_KEY1(spline, float, 0u, 1000u, 99.f);
        EXPECT_EQ(1u, spline.getNumberOfKeys());
    }

    TEST_F(SplineTest, setGetKey_SplineLinearInt32)
    {
        SplineLinearInt32& spline = *this->animationSystem.createSplineLinearInt32();
        CHECK_SET_GET_KEY1(spline, int32_t, 0u, 1000u, 99);
        EXPECT_EQ(1u, spline.getNumberOfKeys());
    }

    TEST_F(SplineTest, setGetKey_SplineLinearVector2f)
    {
        SplineLinearVector2f& spline = *this->animationSystem.createSplineLinearVector2f();
        CHECK_SET_GET_KEY2(spline, float, 0u, 1000u, 99.f, -111.f);
        EXPECT_EQ(1u, spline.getNumberOfKeys());
    }

    TEST_F(SplineTest, setGetKey_SplineLinearVector3f)
    {
        SplineLinearVector3f& spline = *this->animationSystem.createSplineLinearVector3f();
        CHECK_SET_GET_KEY3(spline, float, 0u, 1000u, 99.f, 111.f, -111.f);
        EXPECT_EQ(1u, spline.getNumberOfKeys());
    }

    TEST_F(SplineTest, setGetKey_SplineLinearVector4f)
    {
        SplineLinearVector4f& spline = *this->animationSystem.createSplineLinearVector4f();
        CHECK_SET_GET_KEY4(spline, float, 0u, 1000u, 99.f, 111.f, -111.f, -999.f);
        EXPECT_EQ(1u, spline.getNumberOfKeys());
    }

    TEST_F(SplineTest, setGetKey_SplineLinearVector2i)
    {
        SplineLinearVector2i& spline = *this->animationSystem.createSplineLinearVector2i();
        CHECK_SET_GET_KEY2(spline, int32_t, 0u, 1000u, 99, -111);
        EXPECT_EQ(1u, spline.getNumberOfKeys());
    }

    TEST_F(SplineTest, setGetKey_SplineLinearVector3i)
    {
        SplineLinearVector3i& spline = *this->animationSystem.createSplineLinearVector3i();
        CHECK_SET_GET_KEY3(spline, int32_t, 0u, 1000u, 99, 111, -111);
        EXPECT_EQ(1u, spline.getNumberOfKeys());
    }

    TEST_F(SplineTest, setGetKey_SplineLinearVector4i)
    {
        SplineLinearVector4i& spline = *this->animationSystem.createSplineLinearVector4i();
        CHECK_SET_GET_KEY4(spline, int32_t, 0u, 1000u, 99, 111, -111, -999);
        EXPECT_EQ(1u, spline.getNumberOfKeys());
    }

    TEST_F(SplineTest, setGetKey_SplineBezierFloat)
    {
        SplineBezierFloat& spline = *this->animationSystem.createSplineBezierFloat();
        CHECK_SET_GET_KEY_TANGENTS1(spline, float, 0u, 1000u, 99.f, m_tan1, m_tan2);
        EXPECT_EQ(1u, spline.getNumberOfKeys());
    }

    TEST_F(SplineTest, setGetKey_SplineBezierInt32)
    {
        SplineBezierInt32& spline = *this->animationSystem.createSplineBezierInt32();
        CHECK_SET_GET_KEY_TANGENTS1(spline, int32_t, 0u, 1000u, 99, m_tan1, m_tan2);
        EXPECT_EQ(1u, spline.getNumberOfKeys());
    }

    TEST_F(SplineTest, setGetKey_SplineBezierVector2f)
    {
        SplineBezierVector2f& spline = *this->animationSystem.createSplineBezierVector2f();
        CHECK_SET_GET_KEY_TANGENTS2(spline, float, 0u, 1000u, 99.f, -111.f, m_tan1, m_tan2);
        EXPECT_EQ(1u, spline.getNumberOfKeys());
    }

    TEST_F(SplineTest, setGetKey_SplineBezierVector3f)
    {
        SplineBezierVector3f& spline = *this->animationSystem.createSplineBezierVector3f();
        CHECK_SET_GET_KEY_TANGENTS3(spline, float, 0u, 1000u, 99.f, 111.f, -111.f, m_tan1, m_tan2);
        EXPECT_EQ(1u, spline.getNumberOfKeys());
    }

    TEST_F(SplineTest, setGetKey_SplineBezierVector4f)
    {
        SplineBezierVector4f& spline = *this->animationSystem.createSplineBezierVector4f();
        CHECK_SET_GET_KEY_TANGENTS4(spline, float, 0u, 1000u, 99.f, 111.f, -111.f, -999.f, m_tan1, m_tan2);
        EXPECT_EQ(1u, spline.getNumberOfKeys());
    }

    TEST_F(SplineTest, setGetKey_SplineBezierVector2i)
    {
        SplineBezierVector2i& spline = *this->animationSystem.createSplineBezierVector2i();
        CHECK_SET_GET_KEY_TANGENTS2(spline, int32_t, 0u, 1000u, 99, -111, m_tan1, m_tan2);
        EXPECT_EQ(1u, spline.getNumberOfKeys());
    }

    TEST_F(SplineTest, setGetKey_SplineBezierVector3i)
    {
        SplineBezierVector3i& spline = *this->animationSystem.createSplineBezierVector3i();
        CHECK_SET_GET_KEY_TANGENTS3(spline, int32_t, 0u, 1000u, 99, 111, -111, m_tan1, m_tan2);
        EXPECT_EQ(1u, spline.getNumberOfKeys());
    }

    TEST_F(SplineTest, setGetKey_SplineBezierVector4i)
    {
        SplineBezierVector4i& spline = *this->animationSystem.createSplineBezierVector4i();
        CHECK_SET_GET_KEY_TANGENTS4(spline, int32_t, 0u, 1000u, 99, 111, -111, -999, m_tan1, m_tan2);
        EXPECT_EQ(1u, spline.getNumberOfKeys());
    }
}
