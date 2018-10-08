//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Box2dTest.h"
#include "Math3d/Box2d.h"

namespace ramses_internal
{
    Box2dTest::Box2dTest()
    {
    }

    TEST_F(Box2dTest, DefaultConstructor)
    {
        Box2d<Int32> box;
        EXPECT_EQ(box.position(), Measurement2d<Int32>(0, 0));
        EXPECT_EQ(box.size(), Measurement2d<Int32>(0, 0));
    }

    TEST_F(Box2dTest, Constructor)
    {
        Box2d<Int32> box(Measurement2d<Int32>(12, 34), Measurement2d<Int32>(56, 78));
        EXPECT_EQ(box.position(), Measurement2d<Int32>(12, 34));
        EXPECT_EQ(box.size(), Measurement2d<Int32>(56, 78));
    }

    TEST_F(Box2dTest, MaxPosition)
    {
        Box2d<Int32> box(Measurement2d<Int32>(12, 34), Measurement2d<Int32>(56, 78));
        EXPECT_EQ(box.maxPosition(), Measurement2d<Int32>(12 + 56, 34 + 78));
    }

    TEST_F(Box2dTest, SetPosition)
    {
        Box2d<Int32> box(Measurement2d<Int32>(12, 34), Measurement2d<Int32>(56, 78));
        box.setPosition(Measurement2d<Int32>(-30, -40));
        EXPECT_EQ(box.position(), Measurement2d<Int32>(-30, -40));
        EXPECT_EQ(box.size(), Measurement2d<Int32>(56, 78));
        EXPECT_EQ(box.maxPosition(), Measurement2d<Int32>(-30 + 56, -40 + 78));
    }

    TEST_F(Box2dTest, SetSize)
    {
        Box2d<Int32> box(Measurement2d<Int32>(12, 34), Measurement2d<Int32>(56, 78));
        box.setSize(Measurement2d<Int32>(30, 40));
        EXPECT_EQ(box.position(), Measurement2d<Int32>(12, 34));
        EXPECT_EQ(box.size(), Measurement2d<Int32>(30, 40));
        EXPECT_EQ(box.maxPosition(), Measurement2d<Int32>(12 + 30, 34 + 40));
    }

    TEST_F(Box2dTest, IsEmpty)
    {
        Box2d<Int32> box;
        EXPECT_TRUE(box.isEmpty());
        box.setSize(Measurement2d<Int32>(30, 40));
        EXPECT_FALSE(box.isEmpty());
    }

    TEST_F(Box2dTest, MergeWith)
    {
        Box2d<Int32> box1(Measurement2d<Int32>(-10, -15), Measurement2d<Int32>(20, 30));
        Box2d<Int32> box2(Measurement2d<Int32>(5, 10), Measurement2d<Int32>(7, 6));
        box1.mergeWith(box2);
        EXPECT_EQ(box1.position(), Measurement2d<Int32>(-10, -15));
        EXPECT_EQ(box1.size(), Measurement2d<Int32>(22, 31));

        Box2d<Int32> box3(Measurement2d<Int32>(-11, 20), Measurement2d<Int32>(3, 3));
        box1.mergeWith(box3);
        EXPECT_EQ(box1.position(), Measurement2d<Int32>(-11, -15));
        EXPECT_EQ(box1.size(), Measurement2d<Int32>(23, 38));
    }

    TEST_F(Box2dTest, MergeWithEmpty)
    {
        Box2d<Int32> box1(Measurement2d<Int32>(-10, -15), Measurement2d<Int32>(20, 30));
        Box2d<Int32> box2(Measurement2d<Int32>(50, 60), Measurement2d<Int32>(0, 0));
        box1.mergeWith(box2);
        EXPECT_EQ(box1.position(), Measurement2d<Int32>(-10, -15));
        EXPECT_EQ(box1.size(), Measurement2d<Int32>(20, 30));
    }

    TEST_F(Box2dTest, MergeEmptyWithNonEmpty)
    {
        Box2d<Int32> box1(Measurement2d<Int32>(-10, -15), Measurement2d<Int32>(0, 0));
        Box2d<Int32> box2(Measurement2d<Int32>(50, 60), Measurement2d<Int32>(20, 30));
        box1.mergeWith(box2);
        EXPECT_EQ(box1.position(), Measurement2d<Int32>(50, 60));
        EXPECT_EQ(box1.size(), Measurement2d<Int32>(20, 30));
    }

    TEST_F(Box2dTest, HasCommonEdge)
    {
        Box2d<Int32> box1(Measurement2d<Int32>(10, 15), Measurement2d<Int32>(20, 30));
        {
            Box2d<Int32> box2(Measurement2d<Int32>(30, 15), Measurement2d<Int32>(30, 30));
            EXPECT_TRUE(box1.hasCommonEdge(box2));
        }
        {
            Box2d<Int32> box2(Measurement2d<Int32>(30, 15), Measurement2d<Int32>(30, 31));
            EXPECT_FALSE(box1.hasCommonEdge(box2));
        }

        {
            Box2d<Int32> box2(Measurement2d<Int32>(29, 15), Measurement2d<Int32>(30, 30));
            EXPECT_FALSE(box1.hasCommonEdge(box2));
        }
        {
            Box2d<Int32> box2(Measurement2d<Int32>(30, 14), Measurement2d<Int32>(30, 31));
            EXPECT_FALSE(box1.hasCommonEdge(box2));
        }

        {
            Box2d<Int32> box2(Measurement2d<Int32>(0, 15), Measurement2d<Int32>(10, 30));
            EXPECT_TRUE(box1.hasCommonEdge(box2));
        }

        {
            Box2d<Int32> box2(Measurement2d<Int32>(0, 15), Measurement2d<Int32>(10, 31));
            EXPECT_FALSE(box1.hasCommonEdge(box2));
        }

        {
            Box2d<Int32> box2(Measurement2d<Int32>(-1, 15), Measurement2d<Int32>(10, 30));
            EXPECT_FALSE(box1.hasCommonEdge(box2));
        }

        {
            Box2d<Int32> box2(Measurement2d<Int32>(0, 14), Measurement2d<Int32>(10, 30));
            EXPECT_FALSE(box1.hasCommonEdge(box2));
        }

        {
            Box2d<Int32> box2(Measurement2d<Int32>(10, 0), Measurement2d<Int32>(20, 15));
            EXPECT_TRUE(box1.hasCommonEdge(box2));
        }

        {
            Box2d<Int32> box2(Measurement2d<Int32>(9, 0), Measurement2d<Int32>(20, 15));
            EXPECT_FALSE(box1.hasCommonEdge(box2));
        }

        {
            Box2d<Int32> box2(Measurement2d<Int32>(11, 0), Measurement2d<Int32>(20, 15));
            EXPECT_FALSE(box1.hasCommonEdge(box2));
        }

        {
            Box2d<Int32> box2(Measurement2d<Int32>(10, 0), Measurement2d<Int32>(20, 16));
            EXPECT_FALSE(box1.hasCommonEdge(box2));
        }

        {
            Box2d<Int32> box2(Measurement2d<Int32>(10, 0), Measurement2d<Int32>(20, 14));
            EXPECT_FALSE(box1.hasCommonEdge(box2));
        }

        {
            Box2d<Int32> box2(Measurement2d<Int32>(10, 45), Measurement2d<Int32>(20, 25));
            EXPECT_TRUE(box1.hasCommonEdge(box2));
        }

        {
            Box2d<Int32> box2(Measurement2d<Int32>(9, 45), Measurement2d<Int32>(21, 25));
            EXPECT_FALSE(box1.hasCommonEdge(box2));
        }

        {
            Box2d<Int32> box2(Measurement2d<Int32>(11, 45), Measurement2d<Int32>(19, 25));
            EXPECT_FALSE(box1.hasCommonEdge(box2));
        }

        {
            Box2d<Int32> box2(Measurement2d<Int32>(10, 44), Measurement2d<Int32>(20, 25));
            EXPECT_FALSE(box1.hasCommonEdge(box2));
        }

        {
            Box2d<Int32> box2(Measurement2d<Int32>(10, 46), Measurement2d<Int32>(20, 25));
            EXPECT_FALSE(box1.hasCommonEdge(box2));
        }

    }

    TEST_F(Box2dTest, EqualOperator)
    {
        const Box2d<Int32> box1(Measurement2d<Int32>(12, 34), Measurement2d<Int32>(56, 78));
        const Box2d<Int32> box2(Measurement2d<Int32>(12, 34), Measurement2d<Int32>(56, 78));
        const Box2d<Int32> box3(Measurement2d<Int32>(12, 34), Measurement2d<Int32>(56, 79));
        const Box2d<Int32> box4(Measurement2d<Int32>(13, 34), Measurement2d<Int32>(56, 78));
        EXPECT_EQ(box1, box2);
        EXPECT_FALSE(box1 ==  box3);
        EXPECT_FALSE(box1 == box4);
    }
}
