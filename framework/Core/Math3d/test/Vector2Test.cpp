//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Math3d/Vector2.h"
#include "PlatformAbstraction/PlatformMath.h"
#include "IOStreamTester.h"
#include "framework_common_gmock_header.h"
#include "gtest/gtest.h"


class Vector2Test: public testing::Test
{
public:
    ramses_internal::Vector2 vec1{1.f, 2.f};
};

TEST_F(Vector2Test, DefaultConstructor)
{
    ramses_internal::Vector2 vec2;
    EXPECT_FLOAT_EQ(0.f, vec2.x);
    EXPECT_FLOAT_EQ(0.f, vec2.y);
}

TEST_F(Vector2Test, CopyConstructor)
{
    ramses_internal::Vector2 vec2 = vec1;
    EXPECT_FLOAT_EQ(1.f, vec2.x);
    EXPECT_FLOAT_EQ(2.f, vec2.y);
}

TEST_F(Vector2Test, ValueConstructor)
{
    EXPECT_FLOAT_EQ(1.f, vec1.x);
    EXPECT_FLOAT_EQ(2.f, vec1.y);
}

TEST_F(Vector2Test, ScalarValueConstructor)
{
    ramses_internal::Vector2 vec2(2.f);
    EXPECT_FLOAT_EQ(2.f, vec2.x);
    EXPECT_FLOAT_EQ(2.f, vec2.y);
}

TEST_F(Vector2Test, AssignmentOperator)
{
    ramses_internal::Vector2 vec2;
    vec2 = vec1;
    EXPECT_FLOAT_EQ(1.f, vec2.x);
    EXPECT_FLOAT_EQ(2.f, vec2.y);
}

TEST_F(Vector2Test, AddOperator)
{
    ramses_internal::Vector2 vec2(4.f, 5.f);
    ramses_internal::Vector2 vec3 = vec1 + vec2;
    EXPECT_FLOAT_EQ(5.f, vec3.x);
    EXPECT_FLOAT_EQ(7.f, vec3.y);
}

TEST_F(Vector2Test, AddAssignOperator)
{
    ramses_internal::Vector2 vec2(4.f, 5.f);
    vec2 += vec1;
    EXPECT_FLOAT_EQ(5.f, vec2.x);
    EXPECT_FLOAT_EQ(7.f, vec2.y);
}

TEST_F(Vector2Test, SubOperator)
{
    ramses_internal::Vector2 vec2(4.f, 5.f);
    ramses_internal::Vector2 vec3 = vec1 - vec2;
    EXPECT_FLOAT_EQ(-3.f, vec3.x);
    EXPECT_FLOAT_EQ(-3.f, vec3.y);
}

TEST_F(Vector2Test, SubAssignOperator)
{
    ramses_internal::Vector2 vec2(4.f, 5.f);
    vec1 -= vec2;
    EXPECT_FLOAT_EQ(-3.f, vec1.x);
    EXPECT_FLOAT_EQ(-3.f, vec1.y);
}

TEST_F(Vector2Test, MulOperator)
{
    ramses_internal::Vector2 vec2 = vec1 * 2.f;
    EXPECT_FLOAT_EQ(2.f, vec2.x);
    EXPECT_FLOAT_EQ(4.f, vec2.y);
}

TEST_F(Vector2Test, MulFriendOperator)
{
    ramses_internal::Vector2 vec2 = 2.f * vec1;
    EXPECT_FLOAT_EQ(2.f, vec2.x);
    EXPECT_FLOAT_EQ(4.f, vec2.y);
}

TEST_F(Vector2Test, MulAssignOperator)
{
    vec1 *= 2.f;
    EXPECT_FLOAT_EQ(2.f, vec1.x);
    EXPECT_FLOAT_EQ(4.f, vec1.y);
}

TEST_F(Vector2Test, MulVector)
{
    ramses_internal::Vector2 vec2(1.f, 2.f);
    ramses_internal::Vector2 vec3 = vec1 * vec2;

    EXPECT_FLOAT_EQ(1.f, vec3.x);
    EXPECT_FLOAT_EQ(4.f, vec3.y);
}

TEST_F(Vector2Test, MulAssignVector)
{
    ramses_internal::Vector2 vec2(1.f, 2.f);
    vec1 *= vec2;

    EXPECT_FLOAT_EQ(1.f, vec1.x);
    EXPECT_FLOAT_EQ(4.f, vec1.y);
}

TEST_F(Vector2Test, Dot)
{
    ramses_internal::Vector2 vec2(4.f, 5.f);
    ramses_internal::Float scalar = vec1.dot(vec2);
    EXPECT_FLOAT_EQ(14.f, scalar);
}

TEST_F(Vector2Test, Length)
{
    ramses_internal::Vector2 vec2(3.f, 4.f);
    ramses_internal::Float length = vec2.length();
    EXPECT_FLOAT_EQ(5.f, length);
}

TEST_F(Vector2Test, Angle)
{
    ramses_internal::Vector2 vec2(1.f, 0.f);
    ramses_internal::Vector2 vec3(0.f, 1.f);
    ramses_internal::Float angle = ramses_internal::PlatformMath::Rad2Deg(vec2.angle(vec3));
    EXPECT_FLOAT_EQ(90.f, angle);
}

TEST_F(Vector2Test, Equality)
{
    ramses_internal::Vector2 vec2(1.f, 2.f);
    bool equal = vec1 == vec2;

    EXPECT_EQ(true, equal);
}

TEST_F(Vector2Test, UnEquality)
{
    ramses_internal::Vector2 vec2(0.f, 2.f);
    bool unequal = vec1 != vec2;

    EXPECT_EQ(true, unequal);
}

TEST_F(Vector2Test, SetSingleValues)
{
    vec1.set(3.0f, 4.0f);
    ramses_internal::Vector2 vec2(3.0f, 4.0f);

    EXPECT_EQ(vec2, vec1);
}

TEST_F(Vector2Test, SetAllValues)
{
    vec1.set(5.0f);
    ramses_internal::Vector2 vec2(5.0f, 5.0f);

    EXPECT_EQ(vec2, vec1);
}

TEST_F(Vector2Test, Normalize)
{
    vec1.set(1.0f, 1.0f);
    ramses_internal::Vector2 normalized = vec1.normalize();

    EXPECT_FLOAT_EQ(0.70710677f, normalized.x);
    EXPECT_FLOAT_EQ(0.70710677f, normalized.y);

    vec1.set(1.0f, 2.0f);
    normalized = vec1.normalize();

    EXPECT_FLOAT_EQ( 0.44721359f, normalized.x);
    EXPECT_FLOAT_EQ( 0.89442718f, normalized.y);
}

TEST_F(Vector2Test, CanPrintToString)
{
    EXPECT_EQ("[1 2]", fmt::to_string(vec1));
}

TEST_F(Vector2Test, canBinarySerializeDeserialize)
{
    ramses_internal::IOStreamTesterBase::expectSame(ramses_internal::Vector2());
    ramses_internal::IOStreamTesterBase::expectSame(ramses_internal::Vector2(1.f, 2.f));
    ramses_internal::IOStreamTesterBase::expectSame(ramses_internal::Vector2(std::numeric_limits<float>::max(), std::numeric_limits<float>::min()));
}
