//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Math3d/Vector3.h"
#include "Math3d/Vector4.h"
#include "framework_common_gmock_header.h"
#include "PlatformAbstraction/PlatformMath.h"
#include "gtest/gtest.h"

class Vector3Test: public testing::Test
{
public:
    ramses_internal::Vector3 vec1{1.f, 2.f, 3.f};
};

TEST_F(Vector3Test, DefaultConstructor)
{
    ramses_internal::Vector3 vec2;
    EXPECT_EQ(0.f, vec2.x);
    EXPECT_EQ(0.f, vec2.y);
    EXPECT_EQ(0.f, vec2.z);
}

TEST_F(Vector3Test, CopyConstructor)
{
    ramses_internal::Vector3 vec2 = vec1;
    EXPECT_EQ(1.f, vec2.x);
    EXPECT_EQ(2.f, vec2.y);
    EXPECT_EQ(3.f, vec2.z);
}

TEST_F(Vector3Test, ValueConstructor)
{
    EXPECT_EQ(1.f, vec1.x);
    EXPECT_EQ(2.f, vec1.y);
    EXPECT_EQ(3.f, vec1.z);
}

TEST_F(Vector3Test, ScalarValueConstructor)
{
    ramses_internal::Vector3 vec2(2.f);
    EXPECT_EQ(2.f, vec2.x);
    EXPECT_EQ(2.f, vec2.y);
    EXPECT_EQ(2.f, vec2.z);
}

TEST_F(Vector3Test, ConstructorFromvector4)
{
    ramses_internal::Vector4 vec4(1.0f, 2.0f, 3.0f, 4.0f);
    ramses_internal::Vector3 vec3(vec4);
    EXPECT_EQ(1.f, vec3.x);
    EXPECT_EQ(2.f, vec3.y);
    EXPECT_EQ(3.f, vec3.z);
}

TEST_F(Vector3Test, AssignmentOperator)
{
    ramses_internal::Vector3 vec2;
    vec2 = vec1;
    EXPECT_EQ(1.f, vec2.x);
    EXPECT_EQ(2.f, vec2.y);
    EXPECT_EQ(3.f, vec2.z);
}

TEST_F(Vector3Test, AddOperator)
{
    ramses_internal::Vector3 vec2(4.f, 5.f, 6.f);
    ramses_internal::Vector3 vec3 = vec1 + vec2;
    EXPECT_EQ(5.f, vec3.x);
    EXPECT_EQ(7.f, vec3.y);
    EXPECT_EQ(9.f, vec3.z);
}

TEST_F(Vector3Test, AddAssignOperator)
{
    ramses_internal::Vector3 vec2(4.f, 5.f, 6.f);
    vec2 += vec1;
    EXPECT_EQ(5.f, vec2.x);
    EXPECT_EQ(7.f, vec2.y);
    EXPECT_EQ(9.f, vec2.z);
}

TEST_F(Vector3Test, SubOperator)
{
    ramses_internal::Vector3 vec2(4.f, 5.f, 6.f);
    ramses_internal::Vector3 vec3 = vec1 - vec2;
    EXPECT_EQ(-3.f, vec3.x);
    EXPECT_EQ(-3.f, vec3.y);
    EXPECT_EQ(-3.f, vec3.z);
}

TEST_F(Vector3Test, SubAssignOperator)
{
    ramses_internal::Vector3 vec2(4.f, 5.f, 6.f);
    vec1 -= vec2;
    EXPECT_EQ(-3.f, vec1.x);
    EXPECT_EQ(-3.f, vec1.y);
    EXPECT_EQ(-3.f, vec1.z);
}

TEST_F(Vector3Test, MulOperator)
{
    ramses_internal::Vector3 vec2 = vec1 * 2.f;
    EXPECT_EQ(2.f, vec2.x);
    EXPECT_EQ(4.f, vec2.y);
    EXPECT_EQ(6.f, vec2.z);
}

TEST_F(Vector3Test, MulFriendOperator)
{
    ramses_internal::Vector3 vec2 = 2.f * vec1;
    EXPECT_EQ(2.f, vec2.x);
    EXPECT_EQ(4.f, vec2.y);
    EXPECT_EQ(6.f, vec2.z);
}

TEST_F(Vector3Test, MulAssignOperator)
{
    vec1 *= 2.f;
    EXPECT_EQ(2.f, vec1.x);
    EXPECT_EQ(4.f, vec1.y);
    EXPECT_EQ(6.f, vec1.z);
}

TEST_F(Vector3Test, MulVector)
{
    ramses_internal::Vector3 vec2(1.f, 2.f, 3.f);
    ramses_internal::Vector3 vec3 = vec1 * vec2;

    EXPECT_EQ(1.f, vec3.x);
    EXPECT_EQ(4.f, vec3.y);
    EXPECT_EQ(9.f, vec3.z);
}

TEST_F(Vector3Test, MulAssignVector)
{
    ramses_internal::Vector3 vec2(1.f, 2.f, 3.f);
    vec1 *= vec2;

    EXPECT_EQ(1.f, vec1.x);
    EXPECT_EQ(4.f, vec1.y);
    EXPECT_EQ(9.f, vec1.z);
}

TEST_F(Vector3Test, Dot)
{
    ramses_internal::Vector3 vec2(4.f, 5.f, 6.f);
    ramses_internal::Float scalar = vec1.dot(vec2);
    EXPECT_EQ(32.f, scalar);
}

TEST_F(Vector3Test, Cross)
{
    ramses_internal::Vector3 vec2(-7.f, 8.f, 9.f);
    ramses_internal::Vector3 vec3 = vec1.cross(vec2);
    EXPECT_EQ(-6.f , vec3.x);
    EXPECT_EQ(-30.f, vec3.y);
    EXPECT_EQ(22.f , vec3.z);
}

TEST_F(Vector3Test, Length)
{
    ramses_internal::Vector3 vec2(2.f, 4.f, 4.f);
    ramses_internal::Float length = vec2.length();
    EXPECT_FLOAT_EQ(6.f, length);
}

TEST_F(Vector3Test, Angle)
{
    ramses_internal::Vector3 vec2(1.f, 0.f, 0.f);
    ramses_internal::Vector3 vec3(0.f, 1.f, 0.f);
    ramses_internal::Float angle = ramses_internal::PlatformMath::Rad2Deg(vec2.angle(vec3));
    EXPECT_FLOAT_EQ(90.f, angle);
}

TEST_F(Vector3Test, Equality)
{
    ramses_internal::Vector3 vec2(1.f, 2.f, 3.f);
    bool equal = vec1 == vec2;

    EXPECT_EQ(true, equal);
}

TEST_F(Vector3Test, UnEquality)
{
    ramses_internal::Vector3 vec2(0.f, 2.f, 3.f);
    bool unequal = vec1 != vec2;

    EXPECT_EQ(true, unequal);
}

TEST_F(Vector3Test, SetSingleValues)
{
    vec1.set(3.0f, 4.0f, 7.0f);
    ramses_internal::Vector3 vec2(3.0f, 4.0f, 7.0f);

    EXPECT_EQ(vec2, vec1);
}

TEST_F(Vector3Test, SetAllValues)
{
    vec1.set(5.0f);
    ramses_internal::Vector3 vec2(5.0f, 5.0f, 5.0f);

    EXPECT_EQ(vec2, vec1);
}

TEST_F(Vector3Test, Normalize)
{
    vec1.set(1.0f, 1.0f, 1.0f);
    ramses_internal::Vector3 normalized = vec1.normalize();

    EXPECT_FLOAT_EQ(0.57735026f, normalized.x);
    EXPECT_FLOAT_EQ(0.57735026f, normalized.y);
    EXPECT_FLOAT_EQ(0.57735026f, normalized.z);

    vec1.set(1.0f, 2.0f, 3.0f);
    normalized = vec1.normalize();

    EXPECT_FLOAT_EQ( 0.26726124f, normalized.x);
    EXPECT_FLOAT_EQ( 0.53452247f, normalized.y);
    EXPECT_FLOAT_EQ( 0.80178368f, normalized.z);
}

TEST_F(Vector3Test, CanPrintToString)
{
    EXPECT_EQ("[1.0 2.0 3.0]", fmt::to_string(vec1));
    EXPECT_EQ("[1.0 2.0 3.0]", ramses_internal::StringOutputStream::ToString(vec1));
}
