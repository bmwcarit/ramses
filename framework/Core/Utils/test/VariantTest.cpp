//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "framework_common_gmock_header.h"
#include "VariantTest.h"

using namespace testing;

namespace ramses_internal
{
    template <typename TYPE>
    TYPE GetSomeValue()
    {
        return TYPE(5);
    }

    template <>
    Matrix22f GetSomeValue<Matrix22f>()
    {
        return Matrix22f(1,2,3,4);
    }

    template <>
    Matrix33f GetSomeValue<Matrix33f>()
    {
        return Matrix33f(1, 2, 3, 4, 5, 6, 7, 8, 9);
    }

    template <>
    Matrix44f GetSomeValue<Matrix44f>()
    {
        return Matrix44f(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
    }

    template <typename T>
    VariantTest<T>::VariantTest()
        : m_value(GetSomeValue<T>())
    {
    }

    typedef ::testing::Types <
        Int32,
        Float,
        Vector2,
        Vector3,
        Vector4,
        Vector2i,
        Vector3i,
        Vector4i,
        Matrix22f,
        Matrix33f,
        Matrix44f
    > ItemTypes;

    TYPED_TEST_SUITE(VariantTest, ItemTypes);

    TYPED_TEST(VariantTest, SetGetValue)
    {
        Variant variant;
        variant.setValue(this->m_value);

        const TypeParam val = variant.getValue<TypeParam>();
        EXPECT_EQ(this->m_value, val);
    }
}
