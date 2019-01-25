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

#include "ramses-capu/util/AlgorithmRaw.h"
#include "ramses-capu/Config.h"
#include "BidirectionalTestContainer.h"
#include "gmock/gmock.h"
#include "ComplexTestType.h"

namespace ramses_capu
{
    class AlgorithmRawTest : public ComplexTestTypeTestBase, public ::testing::Test
    {
    };

    TEST_F(AlgorithmRawTest, CopyToRawPointers)
    {
        uint32_t src[3] = { 2, 3, 4 };
        uint32_t dst[4] = { 0 };

        uint32_t *result = copy_to_raw(src, src + 3, dst);
        EXPECT_EQ(dst + 3, result);
        EXPECT_EQ(2u, dst[0]);
        EXPECT_EQ(3u, dst[1]);
        EXPECT_EQ(4u, dst[2]);
        EXPECT_EQ(0u, dst[3]);
    }

    TEST_F(AlgorithmRawTest, CopyToRawComplexType)
    {
        ComplexTestType src[3] = {1, 2, 3};
        alignas(ComplexTestType) uint8_t dst_mem[sizeof(ComplexTestType) * 4] = { 0 };
        ComplexTestType *dst = reinterpret_cast<ComplexTestType*>(dst_mem);

        ComplexTestType::Reset();
        ComplexTestType *result = copy_to_raw(src, src + 3, dst);
        EXPECT_EQ(dst + 3, result);
        EXPECT_EQ(0u, ComplexTestType::ctor_count);
        EXPECT_EQ(3u, ComplexTestType::copyctor_count);
        EXPECT_EQ(0u, ComplexTestType::dtor_count);

        EXPECT_EQ(1u, dst[0].value);
        EXPECT_EQ(2u, dst[1].value);
        EXPECT_EQ(3u, dst[2].value);
    }

    TEST_F(AlgorithmRawTest, CopyToRawZeroElements)
    {
        uint32_t src[2] = { 2, 3 };
        uint32_t dst[1] = { 0 };

        uint32_t *result = copy_to_raw(src, src + 0, dst);
        EXPECT_EQ(dst, result);
        EXPECT_EQ(0u, dst[0]);
    }

    TEST_F(AlgorithmRawTest, MoveToRawPointers)
    {
        uint32_t src[3] = { 2, 3, 4 };
        uint32_t dst[4] = { 0 };

        uint32_t *result = move_to_raw(src, src + 3, dst);
        EXPECT_EQ(dst + 3, result);
        EXPECT_EQ(2u, dst[0]);
        EXPECT_EQ(3u, dst[1]);
        EXPECT_EQ(4u, dst[2]);
        EXPECT_EQ(0u, dst[3]);
    }

    TEST_F(AlgorithmRawTest, MoveToRawZeroElements)
    {
        uint32_t src[2] = { 2, 3 };
        uint32_t dst[1] = { 0 };

        uint32_t *result = move_to_raw(src, src + 0, dst);
        EXPECT_EQ(dst, result);
        EXPECT_EQ(0u, dst[0]);
    }

    TEST_F(AlgorithmRawTest, MoveToRawComplexType)
    {
        ComplexTestType src[3] = {1, 2, 3};
        alignas(ComplexTestType) uint8_t dst_mem[sizeof(ComplexTestType) * 4] = { 0 };
        ComplexTestType *dst = reinterpret_cast<ComplexTestType*>(dst_mem);

        ComplexTestType::Reset();
        ComplexTestType *result = move_to_raw(src, src + 3, dst);
        EXPECT_EQ(dst + 3, result);
        EXPECT_EQ(0u, ComplexTestType::ctor_count);
        EXPECT_EQ(3u, ComplexTestType::copyctor_count);
        EXPECT_EQ(0u, ComplexTestType::dtor_count);

        EXPECT_EQ(1u, dst[0].value);
        EXPECT_EQ(2u, dst[1].value);
        EXPECT_EQ(3u, dst[2].value);
    }

    TEST_F(AlgorithmRawTest, MoveToRawMovableComplexType)
    {
        MoveableComplexTestType src[3] = {1, 2, 3};
        alignas(MoveableComplexTestType) uint8_t dst_mem[sizeof(MoveableComplexTestType) * 4] = { 0 };
        MoveableComplexTestType *dst = reinterpret_cast<MoveableComplexTestType*>(dst_mem);

        MoveableComplexTestType::Reset();
        MoveableComplexTestType *result = move_to_raw(src, src + 3, dst);
        EXPECT_EQ(dst + 3, result);
        EXPECT_EQ(0u, MoveableComplexTestType::ctor_count);
        EXPECT_EQ(0u, MoveableComplexTestType::copyctor_count);
        EXPECT_EQ(3u, MoveableComplexTestType::movector_count);
        EXPECT_EQ(0u, MoveableComplexTestType::dtor_count);

        EXPECT_EQ(1u, dst[0].value);
        EXPECT_EQ(2u, dst[1].value);
        EXPECT_EQ(3u, dst[2].value);
    }

    TEST_F(AlgorithmRawTest, CopyToRawBidirrectionalContainerIntegralType)
    {
        BidirectionalTestContainer<uint32_t> src(3);
        src[0] = 1;
        src[1] = 2;
        src[2] = 3;
        uint32_t dst[4] = { 0 };

        uint32_t *result = copy_to_raw(src.begin(), src.end(), dst);
        EXPECT_EQ(dst + 3, result);
        EXPECT_EQ(1u, dst[0]);
        EXPECT_EQ(2u, dst[1]);
        EXPECT_EQ(3u, dst[2]);
        EXPECT_EQ(0u, dst[3]);
    }

    TEST_F(AlgorithmRawTest, CopyToRawBidirrectionalContainerComplexType)
    {
        BidirectionalTestContainer<ComplexTestType> src(3);
        src[0].value = 1;
        src[1].value = 2;
        src[2].value = 3;
        alignas(ComplexTestType) uint8_t dst_mem[sizeof(ComplexTestType) * 4] = { 0 };
        ComplexTestType *dst = reinterpret_cast<ComplexTestType*>(dst_mem);

        ComplexTestType *result = copy_to_raw(src.begin(), src.end(), dst);
        EXPECT_EQ(dst + 3, result);
        EXPECT_EQ(1u, dst[0].value);
        EXPECT_EQ(2u, dst[1].value);
        EXPECT_EQ(3u, dst[2].value);
        EXPECT_EQ(0u, dst[3].value);

        destruct_raw(dst, dst + 3);
    }

    TEST_F(AlgorithmRawTest, FillRawIntegral)
    {
        alignas(uint_t)  uint8_t mem[sizeof(uint_t) * 4] = { 0xFF };
        uint_t *p = reinterpret_cast<uint_t*>(mem);
        fill_n_raw(p, 3, 1u);
        EXPECT_EQ(1u, p[0]);
        EXPECT_EQ(1u, p[1]);
        EXPECT_EQ(1u, p[2]);
        EXPECT_NE(1u, p[3]);
    }

    TEST_F(AlgorithmRawTest, FillRawIntegralDefaultValue)
    {
        // memory with one's
        alignas(uint_t) uint8_t mem[sizeof(uint_t) * 4]  = { 0xFF };
        Memory::Set(mem, 1, sizeof(uint_t) * 4);

        uint_t *p = reinterpret_cast<uint_t*>(mem);
        fill_n_raw(p, 3);
        EXPECT_EQ(0u, p[0]);
        EXPECT_EQ(0u, p[1]);
        EXPECT_EQ(0u, p[2]);
        EXPECT_NE(0u, p[3]);
    }

    TEST_F(AlgorithmRawTest, FillRawComplexType)
    {
        alignas(ComplexTestType) uint8_t mem[sizeof(ComplexTestType) * 4] = { 0 };
        ComplexTestType *p = reinterpret_cast<ComplexTestType*>(mem);

        ComplexTestType initElement(5u);
        fill_n_raw(p, 3, initElement);
        EXPECT_EQ(1u, ComplexTestType::ctor_count);
        EXPECT_EQ(3u, ComplexTestType::copyctor_count);
        EXPECT_EQ(0u, ComplexTestType::dtor_count);

        EXPECT_EQ(5u, p[0].value);
        EXPECT_EQ(5u, p[1].value);
        EXPECT_EQ(5u, p[2].value);
        EXPECT_NE(5u, p[3].value);

        destruct_raw(p, p + 3);
    }

    TEST_F(AlgorithmRawTest, FillRawBidirectionalContainerIntegralTypeDefaultValue)
    {
        BidirectionalTestContainer<uint_t> c(4);
        c[0] = 1;
        c[1] = 1;
        c[2] = 1;
        c[3] = 1;

        fill_n_raw(c.begin(), 3);
        EXPECT_EQ(0u, c[0]);
        EXPECT_EQ(0u, c[1]);
        EXPECT_EQ(0u, c[2]);
        EXPECT_NE(0u, c[3]);
    }

    TEST_F(AlgorithmRawTest, FillRawBidirectionalContainerComplexTypeDefaultValue)
    {
        {
            BidirectionalTestContainer<ComplexTestType> c(4);
            c[0].value = 1;
            c[1].value = 1;
            c[2].value = 1;
            c[3].value = 1;

            fill_n_raw(c.begin(), 3);
            EXPECT_EQ(0u, c[0].value);
            EXPECT_EQ(0u, c[1].value);
            EXPECT_EQ(0u, c[2].value);
            EXPECT_NE(0u, c[3].value);
        }

        ComplexTestType::Reset(); // TODO(tobias) ignore correct counts, this test is broken regarding object lifecycles
    }

    TEST_F(AlgorithmRawTest, FillRawBidirectionalContainerIntegralType)
    {
        BidirectionalTestContainer<uint_t> c(4);
        c[0] = 1;
        c[1] = 1;
        c[2] = 1;
        c[3] = 1;

        const uint_t fillValue = 99u;
        fill_n_raw(c.begin(), 3, fillValue);
        EXPECT_EQ(fillValue, c[0]);
        EXPECT_EQ(fillValue, c[1]);
        EXPECT_EQ(fillValue, c[2]);
        EXPECT_NE(fillValue, c[3]);
    }

    TEST_F(AlgorithmRawTest, FillRawBidirectionalContainerComplexType)
    {
        BidirectionalTestContainer<ComplexTestType> c(4);
        c[0].value = 1;
        c[1].value = 1;
        c[2].value = 1;
        c[3].value = 1;

        const uint_t fillValue = 99u;
        fill_n_raw(c.begin(), 3, fillValue);
        EXPECT_EQ(fillValue, c[0].value);
        EXPECT_EQ(fillValue, c[1].value);
        EXPECT_EQ(fillValue, c[2].value);
        EXPECT_NE(fillValue, c[3].value);
    }

    TEST_F(AlgorithmRawTest, DestructRawComplexType)
    {
        alignas(ComplexTestType) uint8_t mem[sizeof(ComplexTestType) * 3] = { 0 };
        ComplexTestType *p = reinterpret_cast<ComplexTestType*>(mem);
        new(p + 0) ComplexTestType();
        new(p + 1) ComplexTestType();
        new(p + 2) ComplexTestType();

        destruct_raw(p, p + 3);
        EXPECT_EQ(3u, ComplexTestType::ctor_count);
        EXPECT_EQ(0u, ComplexTestType::copyctor_count);
        EXPECT_EQ(3u, ComplexTestType::dtor_count);
    }

    TEST_F(AlgorithmRawTest, DestructRawIntegrals)
    {
        alignas(uint_t) uint8_t mem[sizeof(uint_t) * 3] = { 0 };
        uint_t *p = reinterpret_cast<uint_t*>(mem);
        new(p + 0) uint_t(2);
        new(p + 1) uint_t(3);
        new(p + 2) uint_t(4);

        destruct_raw(p, p + 3);
    }
}
