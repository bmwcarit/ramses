//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Math3d/Vector4.h"
#include "framework_common_gmock_header.h"
#include "PlatformAbstraction/PlatformMath.h"
#include "IOStreamTester.h"
#include "gmock/gmock.h"

class Vector4Test: public testing::Test
{
public:
    ramses_internal::Vector4 vec1{1.f, 2.f, 3.f, 4.f};
};

TEST_F(Vector4Test, DefaultConstructor)
{
    ramses_internal::Vector4 vec2;
    EXPECT_EQ(0.f, vec2.x);
    EXPECT_EQ(0.f, vec2.y);
    EXPECT_EQ(0.f, vec2.z);
    EXPECT_EQ(0.f, vec2.w);
}

TEST_F(Vector4Test, CopyConstructor)
{
    ramses_internal::Vector4 vec2 = vec1;
    EXPECT_EQ(1.f, vec2.x);
    EXPECT_EQ(2.f, vec2.y);
    EXPECT_EQ(3.f, vec2.z);
    EXPECT_EQ(4.f, vec2.w);
}

TEST_F(Vector4Test, ValueConstructor)
{
    EXPECT_EQ(1.f, vec1.x);
    EXPECT_EQ(2.f, vec1.y);
    EXPECT_EQ(3.f, vec1.z);
    EXPECT_EQ(4.f, vec1.w);
}

TEST_F(Vector4Test, ScalarValueConstructor)
{
    ramses_internal::Vector4 vec2(2.f);
    EXPECT_EQ(2.f, vec2.x);
    EXPECT_EQ(2.f, vec2.y);
    EXPECT_EQ(2.f, vec2.z);
    EXPECT_EQ(2.f, vec2.w);
}

TEST_F(Vector4Test, AssignmentOperator)
{
    ramses_internal::Vector4 vec2;
    vec2 = vec1;
    EXPECT_EQ(1.f, vec2.x);
    EXPECT_EQ(2.f, vec2.y);
    EXPECT_EQ(3.f, vec2.z);
    EXPECT_EQ(4.f, vec2.w);
}

TEST_F(Vector4Test, AddOperator)
{
    ramses_internal::Vector4 vec2(4.f, 5.f, 6.f, 7.f);
    ramses_internal::Vector4 vec3 = vec1 + vec2;
    EXPECT_EQ(5.f, vec3.x);
    EXPECT_EQ(7.f, vec3.y);
    EXPECT_EQ(9.f, vec3.z);
    EXPECT_EQ(11.f, vec3.w);
}

TEST_F(Vector4Test, AddAssignOperator)
{
    ramses_internal::Vector4 vec2(4.f, 5.f, 6.f, 7.f);
    vec2 += vec1;
    EXPECT_EQ(5.f, vec2.x);
    EXPECT_EQ(7.f, vec2.y);
    EXPECT_EQ(9.f, vec2.z);
    EXPECT_EQ(11.f, vec2.w);
}

TEST_F(Vector4Test, SubOperator)
{
    ramses_internal::Vector4 vec2(4.f, 5.f, 6.f, 7.f);
    ramses_internal::Vector4 vec3 = vec1 - vec2;
    EXPECT_EQ(-3.f, vec3.x);
    EXPECT_EQ(-3.f, vec3.y);
    EXPECT_EQ(-3.f, vec3.z);
    EXPECT_EQ(-3.f, vec3.w);
}

TEST_F(Vector4Test, SubAssignOperator)
{
    ramses_internal::Vector4 vec2(4.f, 5.f, 6.f, 7.f);
    vec1 -= vec2;
    EXPECT_EQ(-3.f, vec1.x);
    EXPECT_EQ(-3.f, vec1.y);
    EXPECT_EQ(-3.f, vec1.z);
    EXPECT_EQ(-3.f, vec1.w);
}

TEST_F(Vector4Test, MulOperator)
{
    ramses_internal::Vector4 vec2 = vec1 * 2.f;

    EXPECT_EQ(2.f, vec2.x);
    EXPECT_EQ(4.f, vec2.y);
    EXPECT_EQ(6.f, vec2.z);
    EXPECT_EQ(8.f, vec2.w);
}

TEST_F(Vector4Test, DivOperator)
{
    ramses_internal::Vector4 vec2 = vec1 / 2.f;

    EXPECT_FLOAT_EQ(.5f, vec2.x);
    EXPECT_FLOAT_EQ(1.f, vec2.y);
    EXPECT_FLOAT_EQ(1.5f, vec2.z);
    EXPECT_FLOAT_EQ(2.f, vec2.w);
}

TEST_F(Vector4Test, MulFriendOperator)
{
    ramses_internal::Vector4 vec2 = 2.f * vec1;

    EXPECT_EQ(2.f, vec2.x);
    EXPECT_EQ(4.f, vec2.y);
    EXPECT_EQ(6.f, vec2.z);
    EXPECT_EQ(8.f, vec2.w);
}

TEST_F(Vector4Test, MulAssignOperator)
{
    vec1 *= 2.f;

    EXPECT_EQ(2.f, vec1.x);
    EXPECT_EQ(4.f, vec1.y);
    EXPECT_EQ(6.f, vec1.z);
    EXPECT_EQ(8.f, vec1.w);
}

TEST_F(Vector4Test, MulVector)
{
    ramses_internal::Vector4 vec2(1.f, 2.f, 3.f, 4.f);
    ramses_internal::Vector4 vec3 = vec1 * vec2;

    EXPECT_EQ(1.f, vec3.x);
    EXPECT_EQ(4.f, vec3.y);
    EXPECT_EQ(9.f, vec3.z);
    EXPECT_EQ(16.f, vec3.w);
}

TEST_F(Vector4Test, MulAssignVector)
{
    ramses_internal::Vector4 vec2(1.f, 2.f, 3.f, 4.f);
    vec1 *= vec2;

    EXPECT_EQ(1.f, vec1.x);
    EXPECT_EQ(4.f, vec1.y);
    EXPECT_EQ(9.f, vec1.z);
    EXPECT_EQ(16.f, vec1.w);
}

TEST_F(Vector4Test, DivAssignVectorByScalar)
{
    ramses_internal::Vector4 vecTest(2.f, 6.f, 12.f, 14.f);
    vecTest /= 2.f;
    EXPECT_EQ(1.f, vecTest.x);
    EXPECT_EQ(3.f, vecTest.y);
    EXPECT_EQ(6.f, vecTest.z);
    EXPECT_EQ(7.f, vecTest.w);
}

TEST_F(Vector4Test, Equality)
{
    ramses_internal::Vector4 vec2(1.f, 2.f, 3.f, 4.f);
    bool equal = vec1 == vec2;

    EXPECT_EQ(true, equal);
}

TEST_F(Vector4Test, UnEquality)
{
    ramses_internal::Vector4 vec2(0.f, 2.f, 3.f, 4.f);
    bool unequal = vec1 != vec2;

    EXPECT_EQ(true, unequal);
}

TEST_F(Vector4Test, Dot)
{
    ramses_internal::Vector4 vec2(4.f, 5.f, 6.f, 7.f);
    ramses_internal::Float scalar = vec1.dot(vec2);
    EXPECT_EQ(60.f, scalar);
}

TEST_F(Vector4Test, Cross)
{
    ramses_internal::Vector4 vec2(4.f, 3.f, 2.f, 1.f);

    ramses_internal::Vector4 vec3 = vec1.cross(vec2);
    EXPECT_EQ(-20.f , vec3.x);
    EXPECT_EQ(20.f , vec3.y);
    EXPECT_EQ(20.f , vec3.z);
    EXPECT_EQ(-20.f , vec3.w);
}

TEST_F(Vector4Test, Length)
{
    ramses_internal::Vector4 vec2(2.f, 2.f, 2.f, 2.f);
    ramses_internal::Float length = vec2.length();
    EXPECT_EQ(4.f, length);
}

TEST_F(Vector4Test, Angle)
{
    ramses_internal::Vector4 vec2(1.f, 0.f, 0.f, 0.f);
    ramses_internal::Vector4 vec3(0.f, 1.f, 0.f, 0.f);
    ramses_internal::Float angle = ramses_internal::PlatformMath::Rad2Deg(vec2.angle(vec3));
    EXPECT_FLOAT_EQ(90.f, angle);
}

TEST_F(Vector4Test, SetSingleValues)
{
    vec1.set(3.0f, 4.0f, 7.0f, 5.0f);
    ramses_internal::Vector4 vec2(3.0f, 4.0f, 7.0f, 5.0f);

    EXPECT_EQ(vec2, vec1);
}

TEST_F(Vector4Test, SetAllValues)
{
    vec1.set(5.0f);
    ramses_internal::Vector4 vec2(5.0f, 5.0f, 5.0f, 5.0f);

    EXPECT_EQ(vec2, vec1);
}

TEST_F(Vector4Test, CanPrintToString)
{
    EXPECT_EQ("[1.0 2.0 3.0 4.0]", fmt::to_string(vec1));
    EXPECT_EQ("[1.0 2.0 3.0 4.0]", ramses_internal::StringOutputStream::ToString(vec1));
}

TEST_F(Vector4Test, canBinarySerializeDeserialize)
{
    ramses_internal::IOStreamTesterBase::expectSame(ramses_internal::Vector4());
    ramses_internal::IOStreamTesterBase::expectSame(ramses_internal::Vector4(1.f, 2.f, 3.f, 4.f));
    ramses_internal::IOStreamTesterBase::expectSame(ramses_internal::Vector4(std::numeric_limits<float>::max(), std::numeric_limits<float>::min(), std::numeric_limits<float>::max()-0.1f, std::numeric_limits<float>::min()+0.1f));
}
