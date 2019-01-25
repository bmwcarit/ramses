//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "Collections/Vector.h"

namespace ramses_internal
{
    TEST(AVector, isInitiallyEmpty)
    {
        Vector<int> v;
        EXPECT_EQ(0u, v.size());
    }

    TEST(AVector, canBeConstructedWithDefaultValues)
    {
        Vector<int> v(2);
        ASSERT_EQ(2u, v.size());
        EXPECT_EQ(0, v[0]);
        EXPECT_EQ(0, v[1]);
    }

    TEST(AVector, canBeConstructedWithGivenValue)
    {
        Vector<int> v(2, 1);
        ASSERT_EQ(2u, v.size());
        EXPECT_EQ(1, v[0]);
        EXPECT_EQ(1, v[1]);
    }

    TEST(AVector, canBeInitializedFromInitializerList)
    {
        Vector<int> v = {1, 2, 3};
        ASSERT_EQ(3u, v.size());
        EXPECT_EQ(1, v[0]);
        EXPECT_EQ(2, v[1]);
        EXPECT_EQ(3, v[2]);
    }

    TEST(AVector, canBeCopyConstructed)
    {
        Vector<int> v = {1, 2, 3};
        Vector<int> w(v);

        EXPECT_EQ(3u, v.size());
        ASSERT_EQ(3u, w.size());
        EXPECT_EQ(1, w[0]);
        EXPECT_EQ(2, w[1]);
        EXPECT_EQ(3, w[2]);
    }

    TEST(AVector, canBeAssignTo)
    {
        Vector<int> v = {1, 2, 3};
        Vector<int> w;
        w = v;

        EXPECT_EQ(3u, v.size());
        ASSERT_EQ(3u, w.size());
        EXPECT_EQ(1, w[0]);
        EXPECT_EQ(2, w[1]);
        EXPECT_EQ(3, w[2]);
    }

    TEST(AVector, canBeMoveConstructed)
    {
        // this test is not portable because it relies on implementation specific behavior
        // of vector move operation
        Vector<int> v = {1, 2, 3};
        Vector<int> w(std::move(v));

        ASSERT_EQ(3u, w.size());
        EXPECT_EQ(1, w[0]);
        EXPECT_EQ(2, w[1]);
        EXPECT_EQ(3, w[2]);
    }

    TEST(AVector, canBeMoveAssigned)
    {
        // this test is not portable because it relies on implementation specific behavior
        // of vector move operation
        Vector<int> v = {1, 2, 3};
        Vector<int> w;
        w = std::move(v);

        ASSERT_EQ(3u, w.size());
        EXPECT_EQ(1, w[0]);
        EXPECT_EQ(2, w[1]);
        EXPECT_EQ(3, w[2]);
    }
}
