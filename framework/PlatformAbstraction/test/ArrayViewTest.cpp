//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "Collections/ArrayView.h"
#include "Collections/String.h"
#include "Collections/Vector.h"

#include <algorithm>

namespace ramses_internal
{
    // test data and access to it
    String data("Lorem ipsum dolor sit amet, consetetur sadipscing elitr");

    template <typename T> struct Val {};

    template <> struct Val<ArrayView<UInt32>>
    {
        static UInt32* getValue()
        {
            return reinterpret_cast<UInt32*>(data.data());
        }
    };

    template <> struct Val<ArrayView<UInt16>>
    {
        static UInt16* getValue()
        {
            return reinterpret_cast<UInt16*>(data.data());
        }
    };

    template <> struct Val<ArrayView<Char>>
    {
        static Char* getValue()
        {
            return data.data();
        }
    };

    template <> struct Val<const ArrayView<Char>>
    {
        static Char* getValue()
        {
            return data.data();
        }
    };

    // typed test cases
    template <typename ObjectType> class TypedArrayViewTest : public testing::Test
    {
    public:
    };
    typedef ::testing::Types<ArrayView<Char>, ArrayView<UInt32>, ArrayView<UInt16>> TestedTypes;
    TYPED_TEST_CASE(TypedArrayViewTest, TestedTypes);

    template <typename ObjectType> class TypedArrayViewTestAndConst : public testing::Test
    {
    public:
    };
    typedef ::testing::Types<ArrayView<Char>, const ArrayView<Char>, ArrayView<UInt32>, ArrayView<UInt16>> TestedTypesAndConst;
    TYPED_TEST_CASE(TypedArrayViewTestAndConst, TestedTypesAndConst);

    // tests
    TYPED_TEST(TypedArrayViewTestAndConst, creationOfViewAndIterator)
    {
        auto testdata = Val<TypeParam>::getValue();
        TypeParam view(testdata, 10);
        auto it1 = view.begin();
        EXPECT_EQ(it1.get(), testdata);
    }

    TYPED_TEST(TypedArrayViewTestAndConst, beginAndEnd)
    {
        auto testdata = Val<TypeParam>::getValue();
        TypeParam view(testdata, 10);
        auto it1 = view.begin();
        auto it2 = view.end();
        EXPECT_STRCASEEQ(String(reinterpret_cast<const Char*>(it1.get()), 0, 9).c_str(), String(reinterpret_cast<const Char*>(testdata), 0, 9).c_str());
        EXPECT_EQ(*--it2, testdata[9]);
    }

    TYPED_TEST(TypedArrayViewTestAndConst, acceptsNullptr)
    {
        TypeParam view(nullptr, 0);
        EXPECT_TRUE(view.begin() == view.end());
    }

    TYPED_TEST(TypedArrayViewTestAndConst, acceptsZeroSize)
    {
        auto testdata = Val<TypeParam>::getValue();
        TypeParam view(testdata, 0);
        EXPECT_TRUE(view.begin() == view.end());
    }

    TYPED_TEST(TypedArrayViewTest, iteratorAccessOperatorTest)
    {
        auto testdata = Val<TypeParam>::getValue();
        using TestType = typename std::remove_reference<decltype(*testdata)>::type;
        TypeParam view(testdata, 10);
        auto it1 = view.begin();

        EXPECT_EQ(it1.get(), testdata);
        EXPECT_EQ(*it1, testdata[0u]);

        // verify non constness
        auto oldVal = *it1;
        *it1 = static_cast<TestType>(0u);
        EXPECT_EQ(*it1, static_cast<TestType>(0u));
        *(it1.get()) = oldVal;
        EXPECT_EQ(*it1, oldVal);
    }

    TYPED_TEST(TypedArrayViewTestAndConst, constIteratorAccessOperatorTest)
    {
        auto testdata = Val<TypeParam>::getValue();
        TypeParam view(testdata, 10);
        const auto it1 = view.begin();

        EXPECT_EQ(it1.get(), testdata);
        EXPECT_EQ(*it1, testdata[0u]);

        //*it1 = 'L';         //this should not compile, not testable (yet)
        //*(it1()) = 'L';     //this should not compile, not testable (yet)
    }

    TYPED_TEST(TypedArrayViewTest, ConstIteratorAccessOperatorTest)
    {
        auto testdata = Val<TypeParam>::getValue();
        const TypeParam view(testdata, 10);
        auto it1 = view.begin();

        EXPECT_EQ(it1.get(), testdata);
        EXPECT_EQ(*it1, testdata[0u]);

        //*it1 = 'L';         //this should not compile, not testable (yet)
        //*(it1()) = 'L';     //this should not compile, not testable (yet)
    }

    TYPED_TEST(TypedArrayViewTestAndConst, iteratorIncreaseDecreaseOperatorTest)
    {
        auto testdata = Val<TypeParam>::getValue();
        TypeParam view(testdata, 10);
        auto it1 = view.begin();

        UInt32 diff = 2;
        auto it2 = it1 + diff;
        EXPECT_EQ(it2.get(), testdata + diff);
        EXPECT_EQ(*it2, testdata[diff]);

        EXPECT_EQ((++it2).get(), testdata + ++diff);
        EXPECT_EQ(*it2, testdata[diff]);

        EXPECT_EQ(it2++.get(), testdata + diff++);

        EXPECT_EQ(it2.get(), testdata + diff);
        EXPECT_EQ(*it2, testdata[diff]);

        EXPECT_EQ((--it2).get(), testdata + --diff);
        EXPECT_EQ(*it2, testdata[diff]);
    }

    TYPED_TEST(TypedArrayViewTestAndConst, iteratorArithmeticOperatorTest)
    {
        auto testdata = Val<TypeParam>::getValue();
        TypeParam view(testdata, 10);
        auto it1 = view.begin();
        auto it2 = it1;

        it1 += 4;
        EXPECT_NE(it1, it2);
        EXPECT_EQ(it1.get(), it2.get() + 4);
        EXPECT_EQ(it1 - 4, it2);
        EXPECT_EQ(it1 - 2, it2 + 2);
        EXPECT_EQ(it1 - it2, 4);
    }

    TYPED_TEST(TypedArrayViewTestAndConst, iteratorComparingOperatorTest)
    {
        auto testdata = Val<TypeParam>::getValue();
        TypeParam view(testdata, 10);
        auto it1 = view.begin();
        auto it2 = it1 + 1;
        auto it3 = it2;

        EXPECT_TRUE(it3 == it2);
        EXPECT_FALSE(it3 != it2);
        EXPECT_TRUE(it1 < it2);
        EXPECT_FALSE(it2 < it3);
        EXPECT_FALSE(it2 < it1);
    }

    TYPED_TEST(TypedArrayViewTest, simpleIteration)
    {
        auto testdata = Val<TypeParam>::getValue();
        using TestType = typename std::remove_reference<decltype(*testdata)>::type;
        TypeParam view(testdata, 10);
        std::vector<TestType> result(10);

        UInt i = 0;
        for (const auto c : view)
        {
            EXPECT_EQ(testdata[i++], c);
        }
        EXPECT_EQ(i, 10u);
    }

    TEST(ArrayViewTest, worksWithRandomIteratorAlgorithms)
    {
        String testdata("Lorem ipsum dolor sit amet");
        ArrayView<Char> view(testdata.data(), 10);

        std::sort(view.begin(), view.end());
        EXPECT_STRCASEEQ(testdata.c_str(), String(" Leimoprsum dolor sit amet").c_str());
    }
}
