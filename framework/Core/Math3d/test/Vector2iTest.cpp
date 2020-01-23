//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Vector2iTest.h"

void Vector2iTest::SetUp()
{
    vec1 = ramses_internal::Vector2i(1, 2);
}

void Vector2iTest::TearDown()
{
}

TEST_F(Vector2iTest, DefaultConstructor)
{
    ramses_internal::Vector2i vec2;
    EXPECT_EQ(0, vec2.x);
    EXPECT_EQ(0, vec2.y);
}

TEST_F(Vector2iTest, CopyConstructor)
{
    ramses_internal::Vector2i vec2 = vec1;
    EXPECT_EQ(1, vec2.x);
    EXPECT_EQ(2, vec2.y);
}

TEST_F(Vector2iTest, ValueConstructor)
{
    EXPECT_EQ(1, vec1.x);
    EXPECT_EQ(2, vec1.y);
}

TEST_F(Vector2iTest, ScalarValueConstructor)
{
    ramses_internal::Vector2i vec2(2);
    EXPECT_EQ(2, vec2.x);
    EXPECT_EQ(2, vec2.y);
}

TEST_F(Vector2iTest, AssignmentOperator)
{
    ramses_internal::Vector2i vec2;
    vec2 = vec1;
    EXPECT_EQ(1, vec2.x);
    EXPECT_EQ(2, vec2.y);
}

TEST_F(Vector2iTest, AddOperator)
{
    ramses_internal::Vector2i vec2(4, 5);
    ramses_internal::Vector2i vec3 = vec1 + vec2;
    EXPECT_EQ(5, vec3.x);
    EXPECT_EQ(7, vec3.y);
}

TEST_F(Vector2iTest, AddAssignOperator)
{
    ramses_internal::Vector2i vec2(4, 5);
    vec2 += vec1;
    EXPECT_EQ(5, vec2.x);
    EXPECT_EQ(7, vec2.y);
}

TEST_F(Vector2iTest, SubOperator)
{
    ramses_internal::Vector2i vec2(4, 5);
    ramses_internal::Vector2i vec3 = vec1 - vec2;
    EXPECT_EQ(-3, vec3.x);
    EXPECT_EQ(-3, vec3.y);
}

TEST_F(Vector2iTest, SubAssignOperator)
{
    ramses_internal::Vector2i vec2(4, 5);
    vec1 -= vec2;
    EXPECT_EQ(-3, vec1.x);
    EXPECT_EQ(-3, vec1.y);
}

TEST_F(Vector2iTest, MulOperator)
{
    ramses_internal::Vector2i vec2 = vec1 * 2;
    EXPECT_EQ(2, vec2.x);
    EXPECT_EQ(4, vec2.y);
}

TEST_F(Vector2iTest, MulFriendOperator)
{
    ramses_internal::Vector2i vec2 = 2 * vec1;
    EXPECT_EQ(2, vec2.x);
    EXPECT_EQ(4, vec2.y);
}

TEST_F(Vector2iTest, MulAssignOperator)
{
    vec1 *= 2;
    EXPECT_EQ(2, vec1.x);
    EXPECT_EQ(4, vec1.y);
}

TEST_F(Vector2iTest, MulVector)
{
    ramses_internal::Vector2i vec2(1, 2);
    ramses_internal::Vector2i vec3 = vec1 * vec2;

    EXPECT_EQ(1, vec3.x);
    EXPECT_EQ(4, vec3.y);
}

TEST_F(Vector2iTest, MulAssignVector)
{
    ramses_internal::Vector2i vec2(1, 2);
    vec1 *= vec2;

    EXPECT_EQ(1, vec1.x);
    EXPECT_EQ(4, vec1.y);
}

TEST_F(Vector2iTest, Dot)
{
    ramses_internal::Vector2i vec2(4, 5);
    ramses_internal::Int32 scalar = vec1.dot(vec2);
    EXPECT_EQ(14, scalar);
}

TEST_F(Vector2iTest, Length)
{
    ramses_internal::Vector2i vec2(3, 4);
    ramses_internal::Float length = vec2.length();
    EXPECT_FLOAT_EQ(5, length);
}

TEST_F(Vector2iTest, Angle)
{
    ramses_internal::Vector2i vec2(1, 0);
    ramses_internal::Vector2i vec3(0, 1);
    ramses_internal::Float angle = ramses_internal::PlatformMath::Rad2Deg(vec2.angle(vec3));
    EXPECT_FLOAT_EQ(90, angle);
}

TEST_F(Vector2iTest, Equality)
{
    ramses_internal::Vector2i vec2(1, 2);
    bool equal = vec1 == vec2;

    EXPECT_EQ(true, equal);
}

TEST_F(Vector2iTest, UnEquality)
{
    ramses_internal::Vector2i vec2(0, 2);
    bool unequal = vec1 != vec2;

    EXPECT_EQ(true, unequal);
}

TEST_F(Vector2iTest, Empty)
{
    ramses_internal::Vector2i vec2(0, 0);

    EXPECT_EQ(vec2, ramses_internal::Vector2i::Empty);
}

TEST_F(Vector2iTest, SetSingleValues)
{
    vec1.set(3, 4);
    ramses_internal::Vector2i vec2(3, 4);

    EXPECT_EQ(vec2, vec1);
}

TEST_F(Vector2iTest, SetAllValues)
{
    vec1.set(5);
    ramses_internal::Vector2i vec2(5, 5);

    EXPECT_EQ(vec2, vec1);
}
