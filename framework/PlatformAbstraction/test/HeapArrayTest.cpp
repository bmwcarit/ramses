//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Collections/HeapArray.h"
#include "gtest/gtest.h"

namespace ramses_internal
{
    TEST(AHeapArray, DefaultCtorCreatesEmpty)
    {
        HeapArray<Byte> a;
        EXPECT_TRUE(a.data() == nullptr);
        EXPECT_EQ(0u, a.size());
    }

    TEST(AHeapArray, CanCreateWithSize)
    {
        HeapArray<Byte> a(4);
        EXPECT_TRUE(a.data() != nullptr);
        EXPECT_EQ(4u, a.size());
    }

    TEST(AHeapArray, CanCreateWithSizeAndData)
    {
        Byte data[4] = {1, 2, 3, 4};
        HeapArray<Byte> a(4, data);
        EXPECT_TRUE(a.data() != nullptr);
        ASSERT_EQ(4u, a.size());
        EXPECT_EQ(0, PlatformMemory::Compare(a.data(), data, 4));
    }

    TEST(AHeapArray, CanGetConstData)
    {
        HeapArray<Byte> a(4);
        const auto& ca = a;
        EXPECT_EQ(a.data(), ca.data());
    }

    TEST(AHeapArray, IsZeroAfterSetZero)
    {
        Byte data[4] = {1, 2, 3, 4};
        Byte zero[4] = {0, 0, 0, 0};

        HeapArray<Byte> a(4, data);
        a.setZero();

        ASSERT_EQ(4u, a.size());
        EXPECT_EQ(0, PlatformMemory::Compare(a.data(), zero, 4));
    }

    TEST(AHeapArray, CanMoveConstruct)
    {
        Byte data[4] = {1, 2, 3, 4};
        HeapArray<Byte> a(4, data);
        HeapArray<Byte> b(std::move(a));

        EXPECT_TRUE(a.data() == nullptr);
        EXPECT_EQ(0u, a.size());

        EXPECT_TRUE(b.data() != nullptr);
        ASSERT_EQ(4u, b.size());
        EXPECT_EQ(0, PlatformMemory::Compare(b.data(), data, 4));
    }

    TEST(AHeapArray, CanMoveAssign)
    {
        Byte data[4] = {1, 2, 3, 4};
        HeapArray<Byte> a(4, data);
        HeapArray<Byte> b;
        b = std::move(a);

        EXPECT_TRUE(a.data() == nullptr);
        EXPECT_EQ(0u, a.size());

        EXPECT_TRUE(b.data() != nullptr);
        ASSERT_EQ(4u, b.size());
        EXPECT_EQ(0, PlatformMemory::Compare(b.data(), data, 4));
    }
}
