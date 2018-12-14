//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "framework_common_gmock_header.h"
#include "Math3d/Quad.h"
#include "gtest/gtest.h"

namespace ramses_internal
{
    TEST(QuadTest, CanCreateQuad_DefaultConstructor)
    {
        const Quad quad;
        EXPECT_EQ(0, quad.x);
        EXPECT_EQ(0, quad.y);
        EXPECT_EQ(0, quad.width);
        EXPECT_EQ(0, quad.height);
    }

    TEST(QuadTest, CanCreateQuad_AllParamsSet)
    {
        const Quad quad{ 1, 2, 3, 4 };
        EXPECT_EQ(1, quad.x);
        EXPECT_EQ(2, quad.y);
        EXPECT_EQ(3, quad.width);
        EXPECT_EQ(4, quad.height);
    }

    TEST(QuadTest, CanCompareQuads)
    {
        const Quad quad1{ 1, 2, 3, 4 };
        const Quad quad2{ 1, 2, 3, 4 };
        const Quad quad3;
        EXPECT_TRUE(quad1 == quad2);
        EXPECT_TRUE(quad1 != quad3);
    }

    TEST(QuadTest, CanGetArea)
    {
        const Quad quad{ 1, 2, 3, 4 };
        EXPECT_EQ(3 * 4, quad.getArea());
    }

    TEST(QuadTest, CanGetBoundingQuad_EmptyQuad)
    {
        const Quad empty;
        const Quad quad{ 1, 2, 3, 4 };
        const auto result = empty.getBoundingQuad(quad);
        EXPECT_EQ(1, result.x);
        EXPECT_EQ(2, result.y);
        EXPECT_EQ(3, result.width);
        EXPECT_EQ(4, result.height);

        const auto result2 = quad.getBoundingQuad(empty);
        EXPECT_EQ(result, result2);
    }

    TEST(QuadTest, CanGetBoundingQuad_ZeroWidth)
    {
        const Quad empty{ 10, 10, 0, 10 };
        const Quad quad{ 1, 2, 3, 4 };
        const auto result = empty.getBoundingQuad(quad);
        EXPECT_EQ(1, result.x);
        EXPECT_EQ(2, result.y);
        EXPECT_EQ(3, result.width);
        EXPECT_EQ(4, result.height);

        const auto result2 = quad.getBoundingQuad(empty);
        EXPECT_EQ(result, result2);
    }

    TEST(QuadTest, CanGetBoundingQuad_ZeroHeight)
    {
        const Quad empty{ 10, 10, 10, 0 };
        const Quad quad{ 1, 2, 3, 4 };
        const auto result = empty.getBoundingQuad(quad);
        EXPECT_EQ(1, result.x);
        EXPECT_EQ(2, result.y);
        EXPECT_EQ(3, result.width);
        EXPECT_EQ(4, result.height);

        const auto result2 = quad.getBoundingQuad(empty);
        EXPECT_EQ(result, result2);
    }

    TEST(QuadTest, CanGetBoundingQuad_OneInsideOther)
    {
        const Quad quad1{ 1, 1, 2, 2 };
        const Quad quad2{ 0, 0, 5, 5 };
        const auto result = quad1.getBoundingQuad(quad2);
        EXPECT_EQ(0, result.x);
        EXPECT_EQ(0, result.y);
        EXPECT_EQ(5, result.width);
        EXPECT_EQ(5, result.height);

        const auto result2 = quad2.getBoundingQuad(quad1);
        EXPECT_EQ(result, result2);
    }

    TEST(QuadTest, CanGetBoundingQuad_OverlappingQuads)
    {
        const Quad quad1{ 1, 1, 3, 3 };
        const Quad quad2{ 2, 2, 5, 5 };
        const auto result = quad1.getBoundingQuad(quad2);
        EXPECT_EQ(1, result.x);
        EXPECT_EQ(1, result.y);
        EXPECT_EQ(6, result.width);
        EXPECT_EQ(6, result.height);

        const auto result2 = quad2.getBoundingQuad(quad1);
        EXPECT_EQ(result, result2);
    }

    TEST(QuadTest, CanGetBoundingQuad_NonOverlappingQuads)
    {
        const Quad quad1{ 1, 1, 3, 3 };
        const Quad quad2{ 5, 0, 5, 5 };
        const auto result = quad1.getBoundingQuad(quad2);
        EXPECT_EQ(1, result.x);
        EXPECT_EQ(0, result.y);
        EXPECT_EQ(9, result.width);
        EXPECT_EQ(5, result.height);

        const auto result2 = quad2.getBoundingQuad(quad1);
        EXPECT_EQ(result, result2);
    }
}
