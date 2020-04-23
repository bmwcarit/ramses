//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Math3d/Vector3i.h"
#include "framework_common_gmock_header.h"
#include "PlatformAbstraction/PlatformMath.h"
#include "gmock/gmock.h"

class Vector3iTest: public testing::Test
{
public:
    ramses_internal::Vector3i vec1{1, 2, 3};
};

TEST_F(Vector3iTest, DefaultConstructor)
{
    ramses_internal::Vector3i vec2;
    EXPECT_EQ(0, vec2.x);
    EXPECT_EQ(0, vec2.y);
    EXPECT_EQ(0, vec2.z);
}

TEST_F(Vector3iTest, CopyConstructor)
{
    ramses_internal::Vector3i vec2 = vec1;
    EXPECT_EQ(1, vec2.x);
    EXPECT_EQ(2, vec2.y);
    EXPECT_EQ(3, vec2.z);
}

TEST_F(Vector3iTest, ValueConstructor)
{
    EXPECT_EQ(1, vec1.x);
    EXPECT_EQ(2, vec1.y);
    EXPECT_EQ(3, vec1.z);
}

TEST_F(Vector3iTest, ScalarValueConstructor)
{
    ramses_internal::Vector3i vec2(2);
    EXPECT_EQ(2, vec2.x);
    EXPECT_EQ(2, vec2.y);
    EXPECT_EQ(2, vec2.z);
}

TEST_F(Vector3iTest, AssignmentOperator)
{
    ramses_internal::Vector3i vec2;
    vec2 = vec1;
    EXPECT_EQ(1, vec2.x);
    EXPECT_EQ(2, vec2.y);
    EXPECT_EQ(3, vec2.z);
}

TEST_F(Vector3iTest, AddOperator)
{
    ramses_internal::Vector3i vec2(4, 5, 6);
    ramses_internal::Vector3i vec3 = vec1 + vec2;
    EXPECT_EQ(5, vec3.x);
    EXPECT_EQ(7, vec3.y);
    EXPECT_EQ(9, vec3.z);
}

TEST_F(Vector3iTest, AddAssignOperator)
{
    ramses_internal::Vector3i vec2(4, 5, 6);
    vec2 += vec1;
    EXPECT_EQ(5, vec2.x);
    EXPECT_EQ(7, vec2.y);
    EXPECT_EQ(9, vec2.z);
}

TEST_F(Vector3iTest, SubOperator)
{
    ramses_internal::Vector3i vec2(4, 5, 6);
    ramses_internal::Vector3i vec3 = vec1 - vec2;
    EXPECT_EQ(-3, vec3.x);
    EXPECT_EQ(-3, vec3.y);
    EXPECT_EQ(-3, vec3.z);
}

TEST_F(Vector3iTest, SubAssignOperator)
{
    ramses_internal::Vector3i vec2(4, 5, 6);
    vec1 -= vec2;
    EXPECT_EQ(-3, vec1.x);
    EXPECT_EQ(-3, vec1.y);
    EXPECT_EQ(-3, vec1.z);
}

TEST_F(Vector3iTest, MulOperator)
{
    ramses_internal::Vector3i vec2 = vec1 * 2;
    EXPECT_EQ(2, vec2.x);
    EXPECT_EQ(4, vec2.y);
    EXPECT_EQ(6, vec2.z);
}

TEST_F(Vector3iTest, MulFriendOperator)
{
    ramses_internal::Vector3i vec2 = 2 * vec1;
    EXPECT_EQ(2, vec2.x);
    EXPECT_EQ(4, vec2.y);
    EXPECT_EQ(6, vec2.z);
}

TEST_F(Vector3iTest, MulAssignOperator)
{
    vec1 *= 2;
    EXPECT_EQ(2, vec1.x);
    EXPECT_EQ(4, vec1.y);
    EXPECT_EQ(6, vec1.z);
}

TEST_F(Vector3iTest, MulVector)
{
    ramses_internal::Vector3i vec2(1, 2, 3);
    ramses_internal::Vector3i vec3 = vec1 * vec2;

    EXPECT_EQ(1, vec3.x);
    EXPECT_EQ(4, vec3.y);
    EXPECT_EQ(9, vec3.z);
}

TEST_F(Vector3iTest, MulAssignVector)
{
    ramses_internal::Vector3i vec2(1, 2, 3);
    vec1 *= vec2;

    EXPECT_EQ(1, vec1.x);
    EXPECT_EQ(4, vec1.y);
    EXPECT_EQ(9, vec1.z);
}

TEST_F(Vector3iTest, Dot)
{
    ramses_internal::Vector3i vec2(4, 5, 6);
    ramses_internal::Int32 scalar = vec1.dot(vec2);
    EXPECT_EQ(32, scalar);
}

TEST_F(Vector3iTest, Cross)
{
    ramses_internal::Vector3i vec2(-7, 8, 9);
    ramses_internal::Vector3i vec3 = vec1.cross(vec2);
    EXPECT_EQ(-6 , vec3.x);
    EXPECT_EQ(-30, vec3.y);
    EXPECT_EQ(22 , vec3.z);
}

TEST_F(Vector3iTest, Length)
{
    ramses_internal::Vector3i vec2(2, 4, 4);
    ramses_internal::Float length = vec2.length();
    EXPECT_FLOAT_EQ(6, length);
}

TEST_F(Vector3iTest, Angle)
{
    ramses_internal::Vector3i vec2(1, 0, 0);
    ramses_internal::Vector3i vec3(0, 1, 0);
    ramses_internal::Float angle = ramses_internal::PlatformMath::Rad2Deg(vec2.angle(vec3));
    EXPECT_FLOAT_EQ(90, angle);
}

TEST_F(Vector3iTest, Equality)
{
    ramses_internal::Vector3i vec2(1, 2, 3);
    bool equal = vec1 == vec2;

    EXPECT_EQ(true, equal);
}

TEST_F(Vector3iTest, UnEquality)
{
    ramses_internal::Vector3i vec2(0, 2, 3);
    bool unequal = vec1 != vec2;

    EXPECT_EQ(true, unequal);
}

TEST_F(Vector3iTest, SetSingleValues)
{
    vec1.set(3, 4, 7);
    ramses_internal::Vector3i vec2(3, 4, 7);

    EXPECT_EQ(vec2, vec1);
}

TEST_F(Vector3iTest, SetAllValues)
{
    vec1.set(5);
    ramses_internal::Vector3i vec2(5, 5, 5);

    EXPECT_EQ(vec2, vec1);
}

TEST_F(Vector3iTest, CanPrintToString)
{
    EXPECT_EQ("[1 2 3]", fmt::to_string(vec1));
    EXPECT_EQ("[1 2 3]", ramses_internal::StringOutputStream::ToString(vec1));
}
