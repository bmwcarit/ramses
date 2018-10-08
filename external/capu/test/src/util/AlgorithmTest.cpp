/*
* Copyright (C) 2015 BMW Car IT GmbH
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "ramses-capu/util/Algorithm.h"
#include "ramses-capu/container/vector.h"
#include "ramses-capu/container/String.h"
#include "ramses-capu/Config.h"
#include "gmock/gmock.h"
#include "BidirectionalTestContainer.h"
#include "ComplexTestType.h"

namespace ramses_capu
{
    TEST(AlgorithmTest, CopyPointers)
    {
        uint32_t src[3] = {2, 3, 4};
        uint32_t dst[4] = { 0 };

        uint32_t* result = ramses_capu::copy(src, src + 3, dst);
        EXPECT_EQ(dst + 3, result);
        EXPECT_EQ(2u, dst[0]);
        EXPECT_EQ(3u, dst[1]);
        EXPECT_EQ(4u, dst[2]);
        EXPECT_EQ(0u, dst[3]);
    }

    TEST(AlgorithmTest, CopyVector)
    {
        vector<uint_t> src(3, 0);
        src[0] = 2;
        src[1] = 3;
        src[2] = 4;
        vector<uint_t> dst(4, 0);

        const vector<uint_t>::Iterator result = ramses_capu::copy(src.begin(), src.end(), dst.begin());
        EXPECT_EQ(dst.begin() + 3u, result);
        EXPECT_EQ(2u, dst[0]);
        EXPECT_EQ(3u, dst[1]);
        EXPECT_EQ(4u, dst[2]);
        EXPECT_EQ(0u, dst[3]);
    }

    TEST(AlgorithmTest, CopyVectorWithSmallElementSizeAndOverlap)
    {
        // Try to trigger problems with optimized overlapped copying with random access iterators
        vector<uint8_t> vec(80);
        for (uint_t i = 0; i < vec.size(); ++i)
        {
            vec[i] = static_cast<uint8_t>(i + 1);
        }

        vector<uint8_t>::Iterator result = ramses_capu::copy(vec.begin() + 1, vec.end(), vec.begin());
        EXPECT_EQ(vec.end() - 1, result);
        for (uint_t i = 0; i < vec.size() - 1; ++i)
        {
            SCOPED_TRACE(i);
            EXPECT_EQ(static_cast<uint8_t>(i + 2), vec[i]);
        }
        EXPECT_EQ(80u, vec.back());
    }

    TEST(AlgorithmTest, CopyVectorIntegralType)
    {
        BidirectionalTestContainer<uint_t> src(3);
        src[0] = 2;
        src[1] = 3;
        src[2] = 4;
        vector<uint_t> dst(4, 0);

        const vector<uint_t>::Iterator result = ramses_capu::copy(src.begin(), src.end(), dst.begin());
        EXPECT_EQ(dst.begin() + 3u, result);
        EXPECT_EQ(2u, dst[0]);
        EXPECT_EQ(3u, dst[1]);
        EXPECT_EQ(4u, dst[2]);
        EXPECT_EQ(0u, dst[3]);
    }

    TEST(AlgorithmTest, CopyVectorComplexType)
    {
        BidirectionalTestContainer<ComplexTestType> src(3);
        src[0] = 2;
        src[1] = 3;
        src[2] = 4;
        vector<ComplexTestType> dst(4, 0);

        const vector<ComplexTestType>::Iterator result = ramses_capu::copy(src.begin(), src.end(), dst.begin());
        EXPECT_EQ(dst.begin() + 3u, result);
        EXPECT_EQ(ComplexTestType(2u), dst[0]);
        EXPECT_EQ(ComplexTestType(3u), dst[1]);
        EXPECT_EQ(ComplexTestType(4u), dst[2]);
        EXPECT_EQ(ComplexTestType(0u), dst[3]);
    }

    TEST(AlgorithmTest, CopyZeroElements)
    {
        uint32_t src[2] = { 2, 3 };
        uint32_t dst[1] = { 0 };

        uint32_t* result = ramses_capu::copy(src, src + 0, dst);
        EXPECT_EQ(dst, result);
        EXPECT_EQ(0u, dst[0]);
    }

    TEST(AlgorithmTest, CopyEmptyVector)
    {
        vector<uint_t> src;
        vector<uint_t> dst(2, 0);

        vector<uint_t>::Iterator result = ramses_capu::copy(src.begin(), src.end(), dst.begin());
        EXPECT_EQ(dst.begin(), result);
        EXPECT_EQ(0u, dst[0]);
    }

    TEST(AlgorithmTest, CopyBackwardsPointers)
    {
        uint32_t arr[4] = {2, 3, 4, 99};

        uint32_t* result = copy_backward(arr, arr + 3, arr + 4);
        EXPECT_EQ(arr + 1, result);
        EXPECT_EQ(2u, arr[0]);
        EXPECT_EQ(2u, arr[1]);
        EXPECT_EQ(3u, arr[2]);
        EXPECT_EQ(4u, arr[3]);
    }

    TEST(AlgorithmTest, CopyBackwardIntegralType)
    {
        BidirectionalTestContainer<uint32_t> src(3);
        src[0] = 2;
        src[1] = 3;
        src[2] = 4;
        vector<uint_t> dst(4, 0);

        vector<uint_t>::Iterator result = ramses_capu::copy_backward(src.begin(), src.end(), dst.end()-1u);
        EXPECT_EQ(dst.begin(), result);
        EXPECT_EQ(2u, dst[0]);
        EXPECT_EQ(3u, dst[1]);
        EXPECT_EQ(4u, dst[2]);
        EXPECT_EQ(0u, dst[3]);
    }

    TEST(AlgorithmTest, CopyBackwardComplexType)
    {
        BidirectionalTestContainer<ComplexTestType> src(3);
        src[0] = 2;
        src[1] = 3;
        src[2] = 4;
        vector<ComplexTestType> dst(4, 0);

        vector<ComplexTestType>::Iterator result = ramses_capu::copy_backward(src.begin(), src.end(), dst.end() - 1u);
        EXPECT_EQ(dst.begin(), result);
        EXPECT_EQ(ComplexTestType(2u), dst[0]);
        EXPECT_EQ(ComplexTestType(3u), dst[1]);
        EXPECT_EQ(ComplexTestType(4u), dst[2]);
        EXPECT_EQ(ComplexTestType(0u), dst[3]);
    }

    TEST(AlgorithmTest, FillArray)
    {
        uint_t a[3] = { 0 };
        uint_t* result = fill_n(a, 2, 1u);
        EXPECT_EQ(a + 2, result);
        EXPECT_EQ(1u, a[0]);
        EXPECT_EQ(1u, a[1]);
        EXPECT_EQ(0u, a[2]);
    }

    TEST(AlgorithmTest, FillArrayWithZeroElements)
    {
        uint_t a[3] = { 99, 99, 99 };
        uint_t* result = fill_n(a, 0, 1u);
        EXPECT_EQ(a, result);
        EXPECT_EQ(99u, a[0]);
        EXPECT_EQ(99u, a[1]);
        EXPECT_EQ(99u, a[2]);
    }

    TEST(AlgorithmTest, FillVector)
    {
        vector<uint_t> v(3, 0);
        vector<uint_t>::Iterator result = ramses_capu::fill_n(v.begin(), 2, 1u);
        EXPECT_EQ(v.begin() + 2u, result);
        EXPECT_EQ(1u, v[0]);
        EXPECT_EQ(1u, v[1]);
        EXPECT_EQ(0u, v[2]);
    }

    TEST(AlgorithmTest, FindWithPointers)
    {
        uint_t a[4] = { 2, 3, 0, 1 };
        EXPECT_EQ(a + 0, ramses_capu::find(a, a + 4, 2u));
        EXPECT_EQ(a + 2, ramses_capu::find(a, a + 4, 0u));
        EXPECT_EQ(a + 4, ramses_capu::find(a, a + 4, 10u));
    }

    TEST(AlgorithmTest, FindWithMultipleOccurences)
    {
        uint_t a[4] = { 3, 2, 1, 2 };
        EXPECT_EQ(a + 1, ramses_capu::find(a, a + 4, 2u));
        EXPECT_EQ(a + 3, ramses_capu::find(a + 2, a + 4, 2u));
    }

    TEST(AlgorithmTest, FindInVector)
    {
        vector<uint_t> v(5);
        v[0] = 6;
        v[1] = 5;
        v[2] = 1;
        v[3] = 7;
        v[4] = 2;
        EXPECT_EQ(v.begin() + 4, ramses_capu::find(v.begin(), v.end(), 2u));
        EXPECT_EQ(v.begin() + 0, ramses_capu::find(v.begin(), v.end(), 6u));
        EXPECT_EQ(v.end(), ramses_capu::find(v.begin(), v.end(), 10u));
    }

    TEST(AlgorithmTest, FindInBidirectionalContainer)
    {
        BidirectionalTestContainer<uint_t> v(4);
        v[0] = 1;
        v[1] = 4;
        v[2] = 7;
        v[3] = 2;
        EXPECT_EQ(v.iteratorAt(1), ramses_capu::find(v.begin(), v.end(), 4u));
        EXPECT_EQ(v.end(), ramses_capu::find(v.begin(), v.end(), 10u));
    }

    TEST(AlgorithmTest, FillVectorWithZeroElements)
    {
        vector<uint_t> v(3, 99);
        vector<uint_t>::Iterator result = ramses_capu::fill_n(v.begin(), 0, 1u);
        EXPECT_EQ(v.begin(), result);
        EXPECT_EQ(99u, v[0]);
        EXPECT_EQ(99u, v[1]);
        EXPECT_EQ(99u, v[2]);
    }

    TEST(AlgorithmTest, EqualSame)
    {
        uint_t a[3] = { 1, 2, 3 };
        uint_t b[3] = { 1, 2, 3 };
        EXPECT_TRUE(ramses_capu::equal(a, a+3, b));
    }

    TEST(AlgorithmTest, EqualSameComplexType)
    {
        String a[3] = { "1", "2", "3" };
        String b[3] = { "1", "2", "3" };
        EXPECT_TRUE(ramses_capu::equal(a, a + 3, b));
    }

    TEST(AlgorithmTest, EqualNotSame)
    {
        uint_t a[3] = { 1, 2, 3 };
        uint_t b[3] = { 1, 5, 3 };
        EXPECT_FALSE(ramses_capu::equal(a, a + 3, b));
    }

    TEST(AlgorithmTest, EqualNotSameComplexType)
    {
        String a[3] = { "1", "2", "3" };
        String b[3] = { "1", "5", "3" };
        EXPECT_FALSE(ramses_capu::equal(a, a + 3, b));
    }

    TEST(AlgorithmTest, EqualOnEmpty)
    {
        uint_t a[1] = { 1 };
        uint_t b[1] = { 2 };
        EXPECT_TRUE(ramses_capu::equal(a, a + 0, b));
    }
}
