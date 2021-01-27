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
    template <typename T>
    class AHeapArray : public testing::Test
    {
    };

    using TypesToTest = ::testing::Types<uint8_t, uint16_t, uint32_t, uint64_t, int8_t, int16_t, int32_t, int64_t>;
    TYPED_TEST_SUITE(AHeapArray, TypesToTest);

    TYPED_TEST(AHeapArray, DefaultCtorCreatesEmpty)
    {
        HeapArray<TypeParam> a;
        EXPECT_TRUE(a.data() == nullptr);
        EXPECT_EQ(0u, a.size());
    }

    TYPED_TEST(AHeapArray, CanCreateWithSize)
    {
        HeapArray<TypeParam> a(4);
        EXPECT_TRUE(a.data() != nullptr);
        EXPECT_EQ(4u, a.size());
    }

    TYPED_TEST(AHeapArray, CanCreateWithSizeAndData)
    {
        TypeParam data[4] = {1, 2, 3, 4};
        HeapArray<TypeParam> a(4, data);
        EXPECT_TRUE(a.data() != nullptr);
        ASSERT_EQ(4u, a.size());
        EXPECT_EQ(data, a.span());
    }

    TYPED_TEST(AHeapArray, CanGetConstData)
    {
        HeapArray<TypeParam> a(4);
        const auto& ca = a;
        EXPECT_EQ(a.data(), ca.data());
    }

    TYPED_TEST(AHeapArray, IsZeroAfterSetZero)
    {
        TypeParam data[4] = {1, 2, 3, 4};
        TypeParam zero[4] = {0, 0, 0, 0};

        HeapArray<TypeParam> a(4, data);
        a.setZero();

        ASSERT_EQ(4u, a.size());
        EXPECT_EQ(zero, a.span());
    }

    TYPED_TEST(AHeapArray, CanMoveConstruct)
    {
        TypeParam data[4] = {1, 2, 3, 4};
        HeapArray<TypeParam> a(4, data);
        HeapArray<TypeParam> b(std::move(a));

        EXPECT_TRUE(b.data() != nullptr);
        ASSERT_EQ(4u, b.size());
        EXPECT_EQ(data, b.span());
    }

    TYPED_TEST(AHeapArray, CanMoveAssign)
    {
        TypeParam data[4] = {1, 2, 3, 4};
        HeapArray<TypeParam> a(4, data);
        HeapArray<TypeParam> b;
        b = std::move(a);

        EXPECT_TRUE(b.data() != nullptr);
        ASSERT_EQ(4u, b.size());
        EXPECT_EQ(data, b.span());
    }

    TYPED_TEST(AHeapArray, canBeStronglyTyped)
    {
        using STHeapArray = HeapArray<TypeParam, struct SomeTag>;
        STHeapArray a(10);
        STHeapArray b;
        b = std::move(a);
    }

    TYPED_TEST(AHeapArray, canBeReconstructedWithOtherSize)
    {
        HeapArray<TypeParam> a(4);
        HeapArray<TypeParam> b(2, std::move(a));
        EXPECT_EQ(2u, b.size());
    }

    TYPED_TEST(AHeapArray, canGetAsSpan)
    {
        TypeParam data[4] = {1, 2, 3, 4};
        HeapArray<TypeParam> a(4, data);
        EXPECT_EQ(absl::MakeSpan(data), a.span());
    }

    TYPED_TEST(AHeapArray, canGetEmptyAsSpan)
    {
        HeapArray<TypeParam> a;
        EXPECT_EQ(absl::Span<const TypeParam>(), a.span());
    }
}
