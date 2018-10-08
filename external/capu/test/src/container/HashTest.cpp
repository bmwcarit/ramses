/*
 * Copyright (C) 2012 BMW Car IT GmbH
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

#include "ramses-capu/container/Hash.h"
#include "util/ComplexTestType.h"
#include "gtest/gtest.h"

namespace ramses_capu
{
    TEST(HashTest, HashInt32)
    {
        int32_t intVal = 42;
        uint_t expectedHash = 42u;
        EXPECT_EQ(expectedHash, Hash<int32_t>()(intVal));
    }

    TEST(HashTest, HashInt64)
    {
        int64_t intVal = 858918934591ll;
#ifdef CAPU_ARCH_32
        uint32_t expected32bitHash = 1582000875u;
        EXPECT_EQ(expected32bitHash, Hash<int64_t>()(intVal));
        EXPECT_EQ(expected32bitHash, internal::HashCalculator<uint32_t>::Hash(intVal));
#else
        uint64_t expected64bitHash = 855712829724u;
        EXPECT_EQ(expected64bitHash, Hash<int64_t>()(intVal));
        EXPECT_EQ(expected64bitHash, internal::HashCalculator<uint64_t>::Hash(intVal));
#endif
    }

    TEST(HashTest, HashPointer)
    {
        const uint8_t* bytePtr = reinterpret_cast<uint8_t*>(0xabcdef);
        const uint_t ptrAsValue = 0xabcdef;
        EXPECT_EQ(HashValue(ptrAsValue), HashValue(bytePtr));
    }

    enum Someenum
    {
        MEMBER0 = 1234,
        MEMBER1
    };

    TEST(HashTest, HashEnum)
    {
        Someenum val = MEMBER0;

        // we assume that we hash the enum identically to a primitive (int) with the same value
        EXPECT_EQ(HashValue(static_cast<uint_t>(MEMBER0)), HashValue(val));
    }

    TEST(HashResizerTest, resizeToOneBit)
    {
        EXPECT_EQ(1u, Resizer::Resize(0x12345, 1));
        EXPECT_EQ(0u, Resizer::Resize(0x432, 1));
        EXPECT_EQ(1u, Resizer::Resize(0x1, 1));
        EXPECT_EQ(0u, Resizer::Resize(0x0, 1));
    }

    TEST(HashResizerTest, resizeTo4Bits)
    {
        EXPECT_EQ(5u, Resizer::Resize(0x12345, 4));
        EXPECT_EQ(2u, Resizer::Resize(0x432, 4));
        EXPECT_EQ(1u, Resizer::Resize(0x1, 4));
        EXPECT_EQ(0u, Resizer::Resize(0x0, 4));
    }

    TEST(HashResizerTest, resizeToMoreBitsThanPossible)
    {
#ifdef CAPU_ARCH_32
        EXPECT_EQ(0x12345678U, Resizer::Resize(0x12345678U, 34));
        EXPECT_EQ(0x432U, Resizer::Resize(0x432, 34));
#else
        EXPECT_EQ(0x1234567890LLU, Resizer::Resize(0x1234567890LLU, 65));
        EXPECT_EQ(0x432U, Resizer::Resize(0x432, 65));
#endif
    }

    TEST(HashResizerTest, resizeToExactMatchingBits)
    {
#ifdef CAPU_ARCH_32
        EXPECT_EQ(0x12345678U, Resizer::Resize(0x12345678U, 32));
        EXPECT_EQ(0x432U, Resizer::Resize(0x432, 32));
#else
        EXPECT_EQ(0x123456789LLU, Resizer::Resize(0x123456789LLU, 64));
        EXPECT_EQ(0x432U, Resizer::Resize(0x432, 64));
#endif
    }

    TEST(HashCombine, usesSeedAndValue)
    {
        uint_t seed1 = 0;
        HashCombine(seed1, 123);

        uint_t seed2 = 123;
        HashCombine(seed2, 123);

        uint_t seed3 = 123;
        HashCombine(seed3, 0);

        EXPECT_NE(seed1, seed2);
        EXPECT_NE(seed1, seed3);
    }

    TEST(HashCombine, usesExtraArguments)
    {
        uint_t seed1 = 0;
        HashCombine(seed1, 123);

        uint_t seed2 = 0;
        HashCombine(seed2, 123, 456);

        EXPECT_NE(seed1, seed2);
    }

    TEST(HashValue, givesSameResultAsHashCombine)
    {
        uint_t seed1 = 0;
        HashCombine(seed1, 123);
        uint_t seed2 = 0;
        HashCombine(seed2, 123, 456, 789);

        EXPECT_EQ(seed1, HashValue(123));
        EXPECT_EQ(seed2, HashValue(123, 456, 789));
    }

    TEST(HashTest, HashArrayUsesAllElementsForPrimitiveType)
    {
        const uint32_t ar1[] = {1, 2, 3};
        const uint32_t ar2[] = {0, 2, 3};
        const uint32_t ar3[] = {1, 2, 2};
        const uint32_t ar4[] = {1, 2, 3, 4};
        const uint32_t ar5[] = {1, 2, 3, 4};

        EXPECT_NE(HashValue(ar1), HashValue(ar2));
        EXPECT_NE(HashValue(ar1), HashValue(ar3));
        EXPECT_NE(HashValue(ar1), HashValue(ar4));
        EXPECT_EQ(HashValue(ar4), HashValue(ar5));
    }

    TEST(HashTest, HashArrayUsesAllElementsForComplexType)
    {
        const ComplexTestType g1(11);
        const ComplexTestType g2(12);
        const ComplexTestType g3(20);

        const ComplexTestType ar1[] = {g1, g2};
        const ComplexTestType ar2[] = {g2, g1};
        const ComplexTestType ar3[] = {g1, g2, g3};
        const ComplexTestType ar4[] = {g1, g2, g3};

        EXPECT_NE(HashValue(ar1), HashValue(ar2));
        EXPECT_NE(HashValue(ar1), HashValue(ar3));
        EXPECT_EQ(HashValue(ar3), HashValue(ar4));
    }

    TEST(HashTest, HashingArrayOfPrimitivesIsSameAsCombiningArrayValues)
    {
        const uint32_t ar[] = {1, 2, 3};
        uint_t manualHash = 0;
        HashCombine(manualHash, ar[0]);
        HashCombine(manualHash, ar[1]);
        HashCombine(manualHash, ar[2]);
        EXPECT_EQ(manualHash, Hash<uint32_t[3]>()(ar));
    }

    TEST(HashTest, HashingArrayOfComplexTypesIsSameAsCombiningArrayValues)
    {
        const ComplexTestType ar[] = {ComplexTestType(true), ComplexTestType(true), ComplexTestType(true)};
        uint_t manualHash = 0;
        HashCombine(manualHash, ar[0]);
        HashCombine(manualHash, ar[1]);
        HashCombine(manualHash, ar[2]);
        EXPECT_EQ(manualHash, Hash<ComplexTestType[3]>()(ar));
    }

    TEST(HashTest, HashMemoryRangeRespectLength)
    {
        unsigned char ar[] = {1, 2, 3, 4};
        EXPECT_EQ(HashMemoryRange(ar, 1), HashMemoryRange(ar, 1));
        EXPECT_EQ(HashMemoryRange(ar, 4), HashMemoryRange(ar, 4));
        EXPECT_NE(HashMemoryRange(ar, 1), HashMemoryRange(ar, 2));
    }

    TEST(HashTest, HashMemoryRangeWorksForEmptyRange)
    {
        unsigned char ar[] = {1};
        EXPECT_EQ(HashMemoryRange(ar, 0),      HashMemoryRange(ar, 0));
        EXPECT_EQ(HashMemoryRange(nullptr, 0), HashMemoryRange(nullptr, 0));
        EXPECT_EQ(HashMemoryRange(ar, 0),      HashMemoryRange(nullptr, 0));
    }
}
