//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "ramses-text/Quad.h"

namespace ramses
{
    TEST(AQuadSize, KeepsParametersProvidedToConstructor)
    {
        {
            QuadSize test(0, 0);
            EXPECT_EQ(test.x, 0u);
            EXPECT_EQ(test.y, 0u);
        }
        {
            QuadSize test(426, 837);
            EXPECT_EQ(test.x, 426u);
            EXPECT_EQ(test.y, 837u);
        }
    }

    TEST(AQuadSize, IsEqualToOtherQuadSizeIFFSizeValuesAreEqual)
    {
        std::vector<QuadSize> vec;
        vec.emplace_back(2412, 2093);
        vec.emplace_back(2938, 2093);
        vec.emplace_back(2412, 9354);
        vec.emplace_back(0, 0);

        for (auto const& first : vec)
        {
            for (auto const& second : vec)
            {
                EXPECT_TRUE(&first == &second ? first == second : !(first == second));
            }
        }
    }

    TEST(AQuadSize, ComputesAreaWhichIsEqualToWidthTimesHeight)
    {
        std::vector<QuadSize> vec;
        vec.emplace_back(2412, 2093);
        vec.emplace_back(0, 2093);
        vec.emplace_back(2412, 0);
        vec.emplace_back(0, 0);

        for (auto const& entry : vec)
        {
            EXPECT_EQ(entry.getArea(), entry.x * entry.y);
        }
    }

    TEST(AQuadOffset, KeepsParametersProvidedToConstructor)
    {
        {
            QuadOffset test(0, 0);
            EXPECT_EQ(test.x, 0u);
            EXPECT_EQ(test.y, 0u);
        }
        {
            QuadOffset test(426, 837);
            EXPECT_EQ(test.x, 426u);
            EXPECT_EQ(test.y, 837u);
        }
    }

    TEST(AQuadOffset, IsEqualToOtherQuadSizeIFFPositionValuesAreEqual)
    {
        std::vector<QuadOffset> vec;
        vec.emplace_back(2412, 2093);
        vec.emplace_back(2938, 2093);
        vec.emplace_back(2412, 9354);
        vec.emplace_back(0, 0);

        for (auto const& first : vec)
        {
            for (auto const& second : vec)
            {
                EXPECT_TRUE(&first == &second ? first == second : !(first == second));
            }
        }
    }

    TEST(QuadConfidenceTest, QuadHasCommonEdgeReturnsFalseForIntersectingQuads_mergeDoesNothing_intersectSucceeds)
    {
        std::vector<Quad> vec;
        vec.emplace_back(QuadOffset(1, 1), QuadSize(3, 5));
        vec.emplace_back(QuadOffset(0, 0), QuadSize(3, 5));
        vec.emplace_back(QuadOffset(0, 1), QuadSize(3, 5));
        vec.emplace_back(QuadOffset(1, 0), QuadSize(3, 5));
        vec.emplace_back(QuadOffset(1, 1), QuadSize(4, 6));
        vec.emplace_back(QuadOffset(1, 1), QuadSize(3, 6));
        vec.emplace_back(QuadOffset(1, 1), QuadSize(4, 5));

        for (auto const& entry : vec)
        {
            EXPECT_FALSE(vec[0].hasCommonEdge(entry));
            Quad backup(vec[0].getOrigin(), vec[0].getSize());
            EXPECT_FALSE(vec[0].merge(entry));
            EXPECT_EQ(vec[0].getSize(), backup.getSize());
            EXPECT_TRUE(vec[0].intersects(entry));
        }
    }

    TEST(QuadConfidenceTest, QuadHasCommonEdgeReturnsFalseForQuadsTooFarApart_mergeDoesNothing_intersectFails)
    {
        std::vector<Quad> vec;
        Quad test(QuadOffset(3, 3), QuadSize(2, 2));

        vec.emplace_back(QuadOffset(3, 0), QuadSize(2, 2));
        vec.emplace_back(QuadOffset(0, 3), QuadSize(2, 2));
        vec.emplace_back(QuadOffset(0, 0), QuadSize(2, 2));
        vec.emplace_back(QuadOffset(6, 0), QuadSize(2, 2));
        vec.emplace_back(QuadOffset(0, 6), QuadSize(2, 2));
        vec.emplace_back(QuadOffset(6, 6), QuadSize(2, 2));

        for (auto const& entry : vec)
        {
            EXPECT_FALSE(test.hasCommonEdge(entry));
            Quad backup(test.getOrigin(), test.getSize());
            EXPECT_FALSE(test.merge(entry));
            EXPECT_EQ(test.getSize(), backup.getSize());
            EXPECT_FALSE(test.intersects(entry));
        }
    }

    TEST(QuadConfidenceTest, QuadHasCommonEdgeReturnsFalseForQuadsTouchingButOff_mergeDoesNothing_intersectFails)
    {
        std::vector<Quad> vec;
        Quad test(QuadOffset(2, 2), QuadSize(2, 2));

        vec.emplace_back(QuadOffset(0, 0), QuadSize(2, 2));
        vec.emplace_back(QuadOffset(1, 0), QuadSize(2, 2));
        vec.emplace_back(QuadOffset(0, 1), QuadSize(2, 2));
        vec.emplace_back(QuadOffset(4, 3), QuadSize(2, 2));
        vec.emplace_back(QuadOffset(4, 4), QuadSize(2, 2));
        vec.emplace_back(QuadOffset(3, 4), QuadSize(2, 2));

        for (auto const& entry : vec)
        {
            EXPECT_FALSE(test.hasCommonEdge(entry));
            Quad backup(test.getOrigin(), test.getSize());
            EXPECT_FALSE(test.merge(entry));
            EXPECT_EQ(test.getSize(), backup.getSize());
            EXPECT_FALSE(test.intersects(entry));
        }
    }

    TEST(QuadConfidenceTest, QuadHasCommonEdgeReturnsFalseForQuadsDifferentEdgeSize_mergeDoesNothing_intersectFails)
    {
        std::vector<Quad> vec;
        Quad test(QuadOffset(2, 2), QuadSize(2, 2));

        vec.emplace_back(QuadOffset(0, 0), QuadSize(3, 2));
        vec.emplace_back(QuadOffset(0, 0), QuadSize(2, 3));
        vec.emplace_back(QuadOffset(3, 4), QuadSize(3, 2));
        vec.emplace_back(QuadOffset(4, 1), QuadSize(2, 3));

        for (auto const& entry : vec)
        {
            EXPECT_FALSE(test.hasCommonEdge(entry));
            Quad backup(test.getOrigin(), test.getSize());
            EXPECT_FALSE(test.merge(entry));
            EXPECT_EQ(test.getSize(), backup.getSize());
            EXPECT_FALSE(test.intersects(entry));
        }
    }

    TEST(QuadConfidenceTest, QuadHasCommonEdgeReturnsTrueInCaseEdgeIsCommon_mergeMerges_intersectFails)
    {
        std::vector<Quad> vec;
        Quad test(QuadOffset(3, 3), QuadSize(3, 3));

        vec.emplace_back(QuadOffset(3, 0), QuadSize(3, 3));
        vec.emplace_back(QuadOffset(0, 3), QuadSize(3, 3));
        vec.emplace_back(QuadOffset(3, 1), QuadSize(3, 2));
        vec.emplace_back(QuadOffset(1, 3), QuadSize(2, 3));
        vec.emplace_back(QuadOffset(6, 3), QuadSize(20, 3));
        vec.emplace_back(QuadOffset(3, 6), QuadSize(3, 1));

        for (auto const& entry : vec)
        {
            EXPECT_TRUE(test.hasCommonEdge(entry));
            EXPECT_FALSE(test.intersects(entry));
            Quad toMerge(test.getOrigin(), test.getSize());
            EXPECT_TRUE(toMerge.merge(entry));
            EXPECT_EQ(toMerge.getSize().getArea(), test.getSize().getArea() + entry.getSize().getArea());
            EXPECT_NE(toMerge.getSize().x == test.getSize().x, toMerge.getSize().y == test.getSize().y);
        }
    }
}
