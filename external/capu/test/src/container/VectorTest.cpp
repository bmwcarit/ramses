/*
 * Copyright (C) 2013 BMW Car IT GmbH
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

// Workaround for integrityOS compiler
// It supports typed tests, but gtest does not know this yet
#if defined(__GHS_VERSION_NUMBER)
# define GTEST_HAS_TYPED_TEST 1
# define GTEST_HAS_TYPED_TEST_P 1
#endif

#include "gmock/gmock.h"
#include "ramses-capu/container/vector.h"
#include "ramses-capu/container/String.h"
#include "util/BidirectionalTestContainer.h"
#include "util/ComplexTestType.h"
#include "util/IteratorTestHelper.h"

    template<typename T>
    struct NumberOfExistingObjectsHelper
    {
        static void assertNumberOfExistingObjectsEquals(ramses_capu::uint_t expectedNumber)
        {
            UNUSED(expectedNumber);
        }
    };

    template<>
    struct NumberOfExistingObjectsHelper<ComplexTestType>
    {
        static void assertNumberOfExistingObjectsEquals(ramses_capu::uint_t expectedNumber)
        {
            ASSERT_EQ(expectedNumber, ComplexTestType::ctor_count + ComplexTestType::copyctor_count - ComplexTestType::dtor_count);
        }
    };

    template <typename ElementType>
    class TypedVectorTest : public testing::Test
    {
    };

    template <typename ElementType>
    class TypedVectorEnsureSTLCompatibility : public testing::Test
    {
    };

    template <typename ElementType>
    class TypedVectorPerformanceTest : public testing::Test
    {
    };

    typedef ::testing::Types
        <
        ramses_capu::uint_t,
        ComplexTestType
        > ElementTypes;

    TYPED_TEST_CASE(TypedVectorTest, ElementTypes);
    TYPED_TEST_CASE(TypedVectorEnsureSTLCompatibility, ElementTypes);
    TYPED_TEST_CASE(TypedVectorPerformanceTest, ElementTypes);

    TYPED_TEST(TypedVectorPerformanceTest, DISABLED_STLinsertLots)
    {
        std::vector<TypeParam> stlvector(0);

        for (ramses_capu::uint_t i = 0; i < 100000; ++i)
        {
            stlvector.push_back(TypeParam(i));
        }
    }

    TYPED_TEST(TypedVectorPerformanceTest, DISABLED_CAPUinsertLots)
    {
        ramses_capu::vector<TypeParam> capuVector(0);

        for (ramses_capu::uint_t i = 0; i < 100000; ++i)
        {
            capuVector.push_back(TypeParam(i));
        }
    }

    TYPED_TEST(TypedVectorPerformanceTest, DISABLED_STLReserveLots)
    {
        std::vector<TypeParam>* stlvector = new std::vector<TypeParam>(1000000);
        delete stlvector;
    }

    TYPED_TEST(TypedVectorPerformanceTest, DISABLED_CAPUReserveLots)
    {
        ramses_capu::vector<TypeParam>* capuVector = new ramses_capu::vector<TypeParam>(1000000);
        delete capuVector;
    }

    TYPED_TEST(TypedVectorEnsureSTLCompatibility, initialDefault)
    {
        std::vector<TypeParam> stlvector;
        ramses_capu::vector<TypeParam> capuVector;

        EXPECT_EQ(stlvector.size(), capuVector.size());
    }

    TYPED_TEST(TypedVectorEnsureSTLCompatibility, sizeBehaviour)
    {
        std::vector<TypeParam> stlvector(0);
        ramses_capu::vector<TypeParam> capuVector(0);

        EXPECT_EQ(0u, stlvector.capacity());
        EXPECT_EQ(0u, capuVector.capacity());

        EXPECT_EQ(0u, stlvector.size());
        EXPECT_EQ(0u, capuVector.size());

        stlvector.push_back(0);
        capuVector.push_back(0);

        EXPECT_EQ(1u, stlvector.size());
        EXPECT_EQ(1u, capuVector.size());

        stlvector.pop_back();
        capuVector.pop_back();

        EXPECT_EQ(0u, stlvector.size());
        EXPECT_EQ(0u, capuVector.size());
    }

    TYPED_TEST(TypedVectorEnsureSTLCompatibility, sizeBehaviour2)
    {
        std::vector<TypeParam> stlvector(5);
        ramses_capu::vector<TypeParam> capuVector(5);

        EXPECT_EQ(5u, stlvector.capacity());
        EXPECT_EQ(5u, capuVector.capacity());

        EXPECT_EQ(5u, stlvector.size());
        EXPECT_EQ(5u, capuVector.size());

        stlvector.push_back(0);
        capuVector.push_back(0);

        EXPECT_EQ(6u, stlvector.size());
        EXPECT_EQ(6u, capuVector.size());

        stlvector.pop_back();
        capuVector.pop_back();

        EXPECT_EQ(5u, stlvector.size());
        EXPECT_EQ(5u, capuVector.size());
    }

    TYPED_TEST(TypedVectorEnsureSTLCompatibility, reserveBahaviour)
    {
        std::vector<TypeParam> stlvector(5);
        ramses_capu::vector<TypeParam> capuVector(5);

        EXPECT_EQ(5u, stlvector.capacity());
        EXPECT_EQ(5u, capuVector.capacity());

        EXPECT_EQ(5u, stlvector.size());
        EXPECT_EQ(5u, capuVector.size());

        stlvector.reserve(15);
        capuVector.reserve(15);

        EXPECT_EQ(15u, stlvector.capacity());
        EXPECT_EQ(15u, capuVector.capacity());

        EXPECT_EQ(5u, stlvector.size());
        EXPECT_EQ(5u, capuVector.size());

    }

    TYPED_TEST(TypedVectorEnsureSTLCompatibility, reserveConstructsClass)
    {
        {
            std::vector<TypeParam> stlvector(5);
            NumberOfExistingObjectsHelper<TypeParam>::assertNumberOfExistingObjectsEquals(5u);
        }
        {
            ramses_capu::vector<TypeParam> capuvector(5);
            NumberOfExistingObjectsHelper<TypeParam>::assertNumberOfExistingObjectsEquals(5u);
        }
    }

    TYPED_TEST(TypedVectorEnsureSTLCompatibility, deletingVectorAlsoCallsDestructorsOfElements)
    {
        {
            {
                std::vector<TypeParam> stlvector(5);
            }
            NumberOfExistingObjectsHelper<TypeParam>::assertNumberOfExistingObjectsEquals(0u);
        }
        {
            {
                ramses_capu::vector<TypeParam> capuvector(5);
            }
            NumberOfExistingObjectsHelper<TypeParam>::assertNumberOfExistingObjectsEquals(0u);
        }
    }

    TYPED_TEST(TypedVectorEnsureSTLCompatibility, resizeSmallerDeconstructsElements)
    {
        ramses_capu::vector<TypeParam> capuVector(5);
        NumberOfExistingObjectsHelper<TypeParam>::assertNumberOfExistingObjectsEquals(5u);

        capuVector.resize(0);
        NumberOfExistingObjectsHelper<TypeParam>::assertNumberOfExistingObjectsEquals(0u);
    }

    TYPED_TEST(TypedVectorEnsureSTLCompatibility, resizeBehaviour)
    {
        std::vector<TypeParam> stlvector(5);
        ramses_capu::vector<TypeParam> capuVector(5);

        EXPECT_EQ(5u, stlvector.capacity());
        EXPECT_EQ(5u, capuVector.capacity());

        EXPECT_EQ(5u, stlvector.size());
        EXPECT_EQ(5u, capuVector.size());

        stlvector.resize(15);
        capuVector.resize(15);

        EXPECT_EQ(15u, stlvector.capacity());
        EXPECT_EQ(15u, capuVector.capacity());

        EXPECT_EQ(15u, stlvector.size());
        EXPECT_EQ(15u, capuVector.size());

    }

    TYPED_TEST(TypedVectorEnsureSTLCompatibility, initialRequestedSize)
    {
        std::vector<TypeParam> stlvector(876);
        ramses_capu::vector<TypeParam> capuVector(876);

        EXPECT_EQ(876u, stlvector.capacity());
        EXPECT_EQ(876u, capuVector.capacity());
        EXPECT_EQ(stlvector.size(), capuVector.size());
    }

    TYPED_TEST(TypedVectorEnsureSTLCompatibility, resize)
    {
        std::vector<TypeParam> stlvector;
        ramses_capu::vector<TypeParam> capuVector;

        stlvector.resize(128);
        capuVector.resize(128);

        EXPECT_EQ(stlvector.size(), capuVector.size());
        EXPECT_EQ(stlvector.capacity(), capuVector.capacity());
    }

    TYPED_TEST(TypedVectorEnsureSTLCompatibility, reserve)
    {
        std::vector<TypeParam> stlvector;
        ramses_capu::vector<TypeParam> capuVector;

        stlvector.reserve(128);
        capuVector.reserve(128);

        EXPECT_EQ(stlvector.size(), capuVector.size());
        EXPECT_EQ(stlvector.capacity(), capuVector.capacity());
    }

    TYPED_TEST(TypedVectorTest, insertSingleValueWhileNeedingToGrow)
    {
        ramses_capu::vector<TypeParam> capuVector(0);

        for (ramses_capu::uint_t i = 0; i < 11; ++i)
        {
            capuVector.insert(capuVector.begin(), TypeParam(i));
        }

        EXPECT_EQ(TypeParam(10u), capuVector[0]);
        EXPECT_EQ(TypeParam(9u), capuVector[1]);
        EXPECT_EQ(TypeParam(8u), capuVector[2]);
        EXPECT_EQ(TypeParam(7u), capuVector[3]);
        EXPECT_EQ(TypeParam(6u), capuVector[4]);
        EXPECT_EQ(TypeParam(5u), capuVector[5]);
        EXPECT_EQ(TypeParam(4u), capuVector[6]);
        EXPECT_EQ(TypeParam(3u), capuVector[7]);
        EXPECT_EQ(TypeParam(2u), capuVector[8]);
        EXPECT_EQ(TypeParam(1u), capuVector[9]);
        EXPECT_EQ(TypeParam(0u), capuVector[10]);
    }

    TYPED_TEST(TypedVectorTest, insertSingleValueIntoReservedSpace)
    {
        ramses_capu::vector<TypeParam> capuVector;
        capuVector.reserve(64);

        for (ramses_capu::uint_t i = 0; i < 11; ++i)
        {
            capuVector.insert(capuVector.begin(), TypeParam(i));
        }

        EXPECT_EQ(TypeParam(10u), capuVector[0]);
        EXPECT_EQ(TypeParam(9u), capuVector[1]);
        EXPECT_EQ(TypeParam(8u), capuVector[2]);
        EXPECT_EQ(TypeParam(7u), capuVector[3]);
        EXPECT_EQ(TypeParam(6u), capuVector[4]);
        EXPECT_EQ(TypeParam(5u), capuVector[5]);
        EXPECT_EQ(TypeParam(4u), capuVector[6]);
        EXPECT_EQ(TypeParam(3u), capuVector[7]);
        EXPECT_EQ(TypeParam(2u), capuVector[8]);
        EXPECT_EQ(TypeParam(1u), capuVector[9]);
        EXPECT_EQ(TypeParam(0u), capuVector[10]);
    }

    TYPED_TEST(TypedVectorTest, insertSingleValueAtEndPosition)
    {
        ramses_capu::vector<TypeParam> capuVector(2, TypeParam(0));
        capuVector.insert(capuVector.end(), TypeParam(123));

        EXPECT_EQ(TypeParam(0u), capuVector[0]);
        EXPECT_EQ(TypeParam(0u), capuVector[1]);
        EXPECT_EQ(TypeParam(123u), capuVector[2]);
    }

    TYPED_TEST(TypedVectorTest, insertSingleValueIntoMiddle)
    {
        ramses_capu::vector<TypeParam> capuVector(2, TypeParam(0));
        capuVector.insert(capuVector.begin()+1u, TypeParam(123));

        EXPECT_EQ(TypeParam(0u), capuVector[0]);
        EXPECT_EQ(TypeParam(123u), capuVector[1]);
        EXPECT_EQ(TypeParam(0u), capuVector[2]);
    }

    TYPED_TEST(TypedVectorTest, insertRangeWhileNeedingToGrow)
    {
        ramses_capu::vector<TypeParam> source(3);
        source[0] = 1;
        source[1] = 2;
        source[2] = 3;
        ramses_capu::vector<TypeParam> capuVector(0);

        capuVector.insert(capuVector.begin(), source.begin(), source.end());

        EXPECT_EQ(TypeParam(1u), capuVector[0]);
        EXPECT_EQ(TypeParam(2u), capuVector[1]);
        EXPECT_EQ(TypeParam(3u), capuVector[2]);
    }

    TYPED_TEST(TypedVectorTest, insertRangeIntoReservedSpace)
    {
        ramses_capu::vector<TypeParam> source(3);
        source[0] = 2;
        source[1] = 3;
        source[2] = 4;
        ramses_capu::vector<TypeParam> capuVector(2);
        capuVector[0] = 1;
        capuVector[1] = 5;

        capuVector.insert(capuVector.begin() + 1u, source.begin(), source.end());

        EXPECT_EQ(TypeParam(1u), capuVector[0]);
        EXPECT_EQ(TypeParam(2u), capuVector[1]);
        EXPECT_EQ(TypeParam(3u), capuVector[2]);
        EXPECT_EQ(TypeParam(4u), capuVector[3]);
        EXPECT_EQ(TypeParam(5u), capuVector[4]);
    }

    TYPED_TEST(TypedVectorTest, insertRangeIntoReservedSpace_NumberToMoveIsLargerThanNumberToCopyIntoRaw)
    {
        ramses_capu::vector<TypeParam> source(1);
        source[0] = 2;
        ramses_capu::vector<TypeParam> capuVector(4);
        capuVector[0] = 1;
        capuVector[1] = 3;
        capuVector[2] = 4;
        capuVector[3] = 5;

        capuVector.insert(capuVector.begin() + 1u, source.begin(), source.end());

        EXPECT_EQ(TypeParam(1u), capuVector[0]);
        EXPECT_EQ(TypeParam(2u), capuVector[1]);
        EXPECT_EQ(TypeParam(3u), capuVector[2]);
        EXPECT_EQ(TypeParam(4u), capuVector[3]);
        EXPECT_EQ(TypeParam(5u), capuVector[4]);
    }

    TYPED_TEST(TypedVectorTest, insertRangeAtEndPosition)
    {
        ramses_capu::vector<TypeParam> source(3);
        source[0] = 3;
        source[1] = 4;
        source[2] = 5;
        ramses_capu::vector<TypeParam> capuVector(2);
        capuVector[0] = 1;
        capuVector[1] = 2;

        capuVector.insert(capuVector.end(), source.begin(), source.end());

        EXPECT_EQ(TypeParam(1u), capuVector[0]);
        EXPECT_EQ(TypeParam(2u), capuVector[1]);
        EXPECT_EQ(TypeParam(3u), capuVector[2]);
        EXPECT_EQ(TypeParam(4u), capuVector[3]);
        EXPECT_EQ(TypeParam(5u), capuVector[4]);
    }

    TYPED_TEST(TypedVectorTest, insertRangeFromBidirectionalIterator)
    {
        ramses_capu::BidirectionalTestContainer<TypeParam> source(3);
        source[0] = 2;
        source[1] = 3;
        source[2] = 4;
        ramses_capu::vector<TypeParam> capuVector(2);
        capuVector[0] = 1;
        capuVector[1] = 5;

        capuVector.insert(capuVector.begin() + 1u, source.begin(), source.end());

        EXPECT_EQ(TypeParam(1u), capuVector[0]);
        EXPECT_EQ(TypeParam(2u), capuVector[1]);
        EXPECT_EQ(TypeParam(3u), capuVector[2]);
        EXPECT_EQ(TypeParam(4u), capuVector[3]);
        EXPECT_EQ(TypeParam(5u), capuVector[4]);
    }

    TYPED_TEST(TypedVectorTest, Constructor)
    {
        ramses_capu::vector<TypeParam> vector;
        EXPECT_EQ(0U, vector.size());
    }

    TYPED_TEST(TypedVectorTest, ConstructorWithSize)
    {
        ramses_capu::vector<TypeParam> vector(3);
        EXPECT_EQ(3u, vector.size());
    }

    TYPED_TEST(TypedVectorTest, ConstructorWithCapacityAndValue)
    {
        ramses_capu::vector<TypeParam> vector(3, 5);

        EXPECT_EQ(TypeParam(5u), vector[0]);
        EXPECT_EQ(TypeParam(5u), vector[1]);
        EXPECT_EQ(TypeParam(5u), vector[2]);
    }

    TYPED_TEST(TypedVectorTest, CopyConstructor)
    {
        ramses_capu::vector<TypeParam>* vector = new ramses_capu::vector<TypeParam>(0);

        for(uint32_t i = 0; i < 32; ++i)
        {
            vector->push_back(TypeParam(i));
        }

        const ramses_capu::vector<TypeParam> vectorCopy(*vector);
        delete vector;

        for(uint32_t i = 0; i < 32; ++i)
        {
            EXPECT_EQ(TypeParam(i), vectorCopy[i]);
        }

        EXPECT_EQ(32u, vectorCopy.size());
    }

    TYPED_TEST(TypedVectorTest, AssignmentOperator)
    {
        ramses_capu::vector<TypeParam>* vector = new ramses_capu::vector<TypeParam>(0);

        for (uint32_t i = 0; i < 32; ++i)
        {
            vector->push_back(TypeParam(i));
        }

        ramses_capu::vector<TypeParam> vectorCopy;
        vectorCopy = *vector;
        delete vector;

        ASSERT_EQ(32u, vectorCopy.size());
        for (uint32_t i = 0; i < 32; ++i)
        {
            EXPECT_EQ(TypeParam(i), vectorCopy[i]);
        }
    }

    TYPED_TEST(TypedVectorTest, SelfAssignment)
    {
        ramses_capu::vector<TypeParam> vector;

        for (uint32_t i = 0; i < 32; ++i)
        {
            vector.push_back(TypeParam(i));
        }

        vector = *&vector;

        ASSERT_EQ(32u, vector.size());
        for (uint32_t i = 0; i < 32; ++i)
        {
            EXPECT_EQ(TypeParam(i), vector[i]);
        }
    }

    TYPED_TEST(TypedVectorTest, reserveThenResizeIntoIt)
    {
        ramses_capu::vector<TypeParam> vector;
        vector.reserve(64);
        vector.resize(32);
        EXPECT_EQ(32U, vector.size());
    }

    TYPED_TEST(TypedVectorTest, PushBack)
    {
        ramses_capu::vector<TypeParam> vector;

        vector.push_back(TypeParam(42u));
        vector.push_back(TypeParam(47u));

        EXPECT_EQ(TypeParam(42u), vector[0]);
        EXPECT_EQ(TypeParam(47u), vector[1]);
    }

    TYPED_TEST(TypedVectorTest, PushBack2)
    {
        ramses_capu::vector<TypeParam> vector(2);

        vector[0] = TypeParam(42u);
        vector[1] = TypeParam(47u);

        EXPECT_EQ(TypeParam(42u), vector[0]);
        EXPECT_EQ(TypeParam(47u), vector[1]);
    }

    TYPED_TEST(TypedVectorTest, PushBack3)
    {
        ramses_capu::vector<TypeParam> vector(0);

        vector.push_back(TypeParam(42u));

        EXPECT_EQ(TypeParam(42u), vector[0]);
    }

    TYPED_TEST(TypedVectorTest, PopBack)
    {
        ramses_capu::vector<TypeParam> vector(0);

        vector.push_back(TypeParam(42u));
        vector.pop_back();
        EXPECT_EQ(TypeParam(0u), vector.size());
    }

    TYPED_TEST(TypedVectorTest, PopBack2)
    {
        ramses_capu::vector<TypeParam> vector(0);

        vector.push_back(TypeParam(42u));
        vector.push_back(TypeParam(43u));
        vector.pop_back();
        vector.pop_back();
        vector.push_back(TypeParam(44u));

        EXPECT_EQ(1u, vector.size());
        EXPECT_EQ(TypeParam(44u), vector[0]);
    }

    TYPED_TEST(TypedVectorTest, empty)
    {
        ramses_capu::vector<TypeParam> vector(0);

        EXPECT_TRUE(vector.empty());

        vector.push_back(TypeParam(123u));
        EXPECT_FALSE(vector.empty());
    }

    TYPED_TEST(TypedVectorTest, IteratorInc)
    {
        ramses_capu::vector<TypeParam> vector;

        vector.push_back(TypeParam(1u));
        vector.push_back(TypeParam(2u));
        vector.push_back(TypeParam(3u));
        vector.push_back(TypeParam(4u));

        typename ramses_capu::vector<TypeParam>::Iterator current = vector.begin();

        EXPECT_EQ(TypeParam(1u), *current);
        ++current;
        EXPECT_EQ(TypeParam(2u), *current);
        ++current;
        EXPECT_EQ(TypeParam(3u), *current);
        ++current;
        EXPECT_EQ(TypeParam(4u), *current);
    }

    TYPED_TEST(TypedVectorTest, IteratorDec)
    {
        ramses_capu::vector<TypeParam> vector;

        vector.push_back(TypeParam(1u));
        vector.push_back(TypeParam(2u));
        vector.push_back(TypeParam(3u));
        vector.push_back(TypeParam(4u));

        typename ramses_capu::vector<TypeParam>::Iterator end = vector.end();
        --end;
        EXPECT_EQ(TypeParam(4u), *end);
        --end;
        EXPECT_EQ(TypeParam(3u), *end);
        --end;
        EXPECT_EQ(TypeParam(2u), *end);
        --end;
        EXPECT_EQ(TypeParam(1u), *end);
    }

    TYPED_TEST(TypedVectorTest, IteratorNotEqual)
    {
        ramses_capu::vector<TypeParam> vector;

        vector.push_back(TypeParam(1u));
        vector.push_back(TypeParam(2u));
        vector.push_back(TypeParam(3u));
        vector.push_back(TypeParam(4u));

        typename ramses_capu::vector<TypeParam>::Iterator start = vector.begin();
        typename ramses_capu::vector<TypeParam>::Iterator end = vector.end();

        EXPECT_TRUE(start != end);
        end = start;
        EXPECT_FALSE(start != end);

    }

    TYPED_TEST(TypedVectorTest, IteratorSmaller)
    {
        ramses_capu::vector<TypeParam> vector;

        vector.push_back(TypeParam(1u));
        vector.push_back(TypeParam(2u));
        vector.push_back(TypeParam(3u));
        vector.push_back(TypeParam(4u));

        typename ramses_capu::vector<TypeParam>::Iterator start = vector.begin();
        typename ramses_capu::vector<TypeParam>::Iterator end = vector.end();

        EXPECT_TRUE(start < end);
        EXPECT_FALSE(end < start);
    }

    TYPED_TEST(TypedVectorTest, IteratorBigger)
    {
        ramses_capu::vector<TypeParam> vector;

        vector.push_back(TypeParam(1u));
        vector.push_back(TypeParam(2u));
        vector.push_back(TypeParam(3u));
        vector.push_back(TypeParam(4u));

        typename ramses_capu::vector<TypeParam>::Iterator start = vector.begin();
        typename ramses_capu::vector<TypeParam>::Iterator end = vector.end();

        EXPECT_TRUE(end > start);
        EXPECT_FALSE(start > end);
    }

    TYPED_TEST(TypedVectorTest, IteratorAddValue)
    {
        ramses_capu::vector<TypeParam> vector;

        vector.push_back(TypeParam(1u));
        vector.push_back(TypeParam(2u));
        vector.push_back(TypeParam(3u));
        vector.push_back(TypeParam(4u));

        typename ramses_capu::vector<TypeParam>::Iterator start = vector.begin();

        EXPECT_EQ(TypeParam(1u), *(start + 0u));
        EXPECT_EQ(TypeParam(2u), *(start + 1u));
        EXPECT_EQ(TypeParam(3u), *(start + 2u));
        EXPECT_EQ(TypeParam(4u), *(start + 3u));
    }

    TYPED_TEST(TypedVectorTest, IteratorSubValue)
    {
        ramses_capu::vector<TypeParam> vector;

        vector.push_back(TypeParam(1u));
        vector.push_back(TypeParam(2u));
        vector.push_back(TypeParam(3u));
        vector.push_back(TypeParam(4u));

        typename ramses_capu::vector<TypeParam>::Iterator end = vector.end();

        EXPECT_EQ(TypeParam(4u), *(end - 1u));
        EXPECT_EQ(TypeParam(3u), *(end - 2u));
        EXPECT_EQ(TypeParam(2u), *(end - 3u));
        EXPECT_EQ(TypeParam(1u), *(end - 4u));
    }

    TYPED_TEST(TypedVectorTest, Iterator)
    {
        ramses_capu::vector<TypeParam> vector;

        vector.push_back(TypeParam(42u));
        vector.push_back(TypeParam(47u));

        ramses_capu::vector<TypeParam> vector2;

        for (typename ramses_capu::vector<TypeParam>::Iterator iter = vector.begin(); iter != vector.end(); ++iter)
        {
            vector2.push_back(*iter);
        }

        EXPECT_EQ(TypeParam(42u), vector2[0]);
        EXPECT_EQ(TypeParam(47u), vector2[1]);
    }

    TYPED_TEST(TypedVectorTest, IteratorOnConstVector)
    {
        ramses_capu::vector<TypeParam> vector;

        const ramses_capu::vector<TypeParam>& constVector = vector;

        vector.push_back(TypeParam(42u));
        vector.push_back(TypeParam(47u));

        ramses_capu::vector<TypeParam> vector2;

        for (typename ramses_capu::vector<TypeParam>::ConstIterator iter = constVector.begin(); iter != constVector.end(); ++iter)
        {
            vector2.push_back(*iter);
        }

        EXPECT_EQ(TypeParam(2u), vector2.size());
        EXPECT_EQ(TypeParam(42u), vector2[0]);
        EXPECT_EQ(TypeParam(47u), vector2[1]);
    }

    TYPED_TEST(TypedVectorTest, IteratorOnConstVectorWithInitialCapacity)
    {
        ramses_capu::vector<TypeParam> vector(2);

        const ramses_capu::vector<TypeParam>& constVector = vector;

        vector[0] = TypeParam(42u);
        vector[1] = TypeParam(47u);

        ramses_capu::vector<TypeParam> vector2;

        for (typename ramses_capu::vector<TypeParam>::ConstIterator iter = constVector.begin(); iter != constVector.end(); ++iter)
        {
            vector2.push_back(*iter);
        }

        EXPECT_EQ(2u, vector2.size());
        EXPECT_EQ(TypeParam(42u), vector2[0]);
        EXPECT_EQ(TypeParam(47u), vector2[1]);
    }

    TYPED_TEST(TypedVectorTest, IteratorOnConstVectorWithInitialCapacityAndValues)
    {
        ramses_capu::vector<TypeParam> vector(12, TypeParam(55u));

        const ramses_capu::vector<TypeParam>& constVector = vector;

        ramses_capu::vector<TypeParam> vector2(0);

        for (typename ramses_capu::vector<TypeParam>::ConstIterator iter = constVector.begin(); iter != constVector.end(); ++iter)
        {
            vector2.push_back(*iter);
        }

        EXPECT_EQ(12u, vector2.size());
        EXPECT_EQ(TypeParam(55u), vector2[0]);
        EXPECT_EQ(TypeParam(55u), vector2[1]);
        EXPECT_EQ(TypeParam(55u), vector2[2]);
        EXPECT_EQ(TypeParam(55u), vector2[3]);
        EXPECT_EQ(TypeParam(55u), vector2[4]);
        EXPECT_EQ(TypeParam(55u), vector2[5]);
        EXPECT_EQ(TypeParam(55u), vector2[6]);
        EXPECT_EQ(TypeParam(55u), vector2[7]);
        EXPECT_EQ(TypeParam(55u), vector2[8]);
        EXPECT_EQ(TypeParam(55u), vector2[9]);
        EXPECT_EQ(TypeParam(55u), vector2[10]);
        EXPECT_EQ(TypeParam(55u), vector2[11]);
    }

    TYPED_TEST(TypedVectorTest, AccessOperator)
    {
        ramses_capu::vector<TypeParam> vector;

        vector.push_back(TypeParam(42u));
        vector.push_back(TypeParam(47u));

        vector[0] = TypeParam(47u);
        vector[1] = TypeParam(42u);

        EXPECT_EQ(TypeParam(47u), vector[0]);
        EXPECT_EQ(TypeParam(42u), vector[1]);
    }

    TYPED_TEST(TypedVectorTest, Resize)
    {
        ramses_capu::vector<TypeParam> vector(0);

        vector.push_back(TypeParam(1));
        vector.push_back(TypeParam(2));

        vector.resize(19);

        EXPECT_EQ(TypeParam(1u), vector[0]);
        EXPECT_EQ(TypeParam(2u), vector[1]);

        vector.resize(1);

        EXPECT_EQ(TypeParam(1u), vector[0]);
    }

    TYPED_TEST(TypedVectorTest, Clear)
    {
        TypeParam struct1 = TypeParam(47);

        TypeParam struct2 = TypeParam(8);

        ramses_capu::vector<TypeParam> vector;

        vector.push_back(struct1);
        vector.push_back(struct2);

        vector.clear();

        vector.push_back(struct2);
        vector.push_back(struct1);

        EXPECT_EQ(struct2, vector[0]);
        EXPECT_EQ(struct1, vector[1]);
    }

    TYPED_TEST(TypedVectorTest, EraseIteratorFromMiddle)
    {
        ramses_capu::vector<TypeParam> vector(0);

        vector.push_back(TypeParam(1));
        vector.push_back(TypeParam(2));
        vector.push_back(TypeParam(3));
        vector.push_back(TypeParam(4));
        vector.push_back(TypeParam(5));

        vector.erase(vector.begin() + 2u);

        EXPECT_EQ(4u, vector.size());
        NumberOfExistingObjectsHelper<TypeParam>::assertNumberOfExistingObjectsEquals(4);
        EXPECT_EQ(TypeParam(1), vector[0]);
        EXPECT_EQ(TypeParam(2), vector[1]);
        EXPECT_EQ(TypeParam(4), vector[2]);
        EXPECT_EQ(TypeParam(5), vector[3]);
    }

    TYPED_TEST(TypedVectorTest, EraseIterator)
    {
        ramses_capu::vector<TypeParam> vector(0);

        vector.push_back(TypeParam(1));
        vector.push_back(TypeParam(2));
        vector.push_back(TypeParam(3));
        vector.push_back(TypeParam(4));
        vector.push_back(TypeParam(5));

        typename ramses_capu::vector<TypeParam>::Iterator iter = vector.begin();

        EXPECT_EQ(ramses_capu::CAPU_EINVAL, vector.erase(iter + 8u));

        vector.erase(iter + 2u);

        EXPECT_EQ(4u, vector.size());
        NumberOfExistingObjectsHelper<TypeParam>::assertNumberOfExistingObjectsEquals(4);
        EXPECT_EQ(TypeParam(1), vector[0]);
        EXPECT_EQ(TypeParam(2), vector[1]);
        EXPECT_EQ(TypeParam(4), vector[2]);
        EXPECT_EQ(TypeParam(5), vector[3]);

        vector.erase(vector.begin());

        EXPECT_EQ(3u, vector.size());
        NumberOfExistingObjectsHelper<TypeParam>::assertNumberOfExistingObjectsEquals(3);
        EXPECT_EQ(TypeParam(2), vector[0]);
        EXPECT_EQ(TypeParam(4), vector[1]);
        EXPECT_EQ(TypeParam(5), vector[2]);

        vector.erase(vector.end() - 1u);

        EXPECT_EQ(2u, vector.size());
        NumberOfExistingObjectsHelper<TypeParam>::assertNumberOfExistingObjectsEquals(2);
        EXPECT_EQ(TypeParam(2), vector[0]);
        EXPECT_EQ(TypeParam(4), vector[1]);

        vector.erase(vector.begin());
        vector.erase(vector.begin());

        EXPECT_EQ(TypeParam(0u), vector.size());
        NumberOfExistingObjectsHelper<TypeParam>::assertNumberOfExistingObjectsEquals(0);
    }

    TYPED_TEST(TypedVectorTest, EraseIndex)
    {
        ramses_capu::vector<TypeParam> vector;

        vector.push_back(TypeParam(1u));
        vector.push_back(TypeParam(2u));
        vector.push_back(TypeParam(3u));
        vector.push_back(TypeParam(4u));
        vector.push_back(TypeParam(5u));

        typename ramses_capu::vector<TypeParam>::Iterator iter = vector.begin();

        ++iter;
        ++iter;

        ramses_capu::status_t result = vector.erase(iter);

        EXPECT_TRUE(ramses_capu::CAPU_OK == result);
        EXPECT_EQ(4u, vector.size());
        EXPECT_EQ(TypeParam(1u), vector[0]);
        EXPECT_EQ(TypeParam(2u), vector[1]);
        EXPECT_EQ(TypeParam(4u), vector[2]);
        EXPECT_EQ(TypeParam(5u), vector[3]);

        result = vector.erase(vector.begin());

        EXPECT_TRUE(ramses_capu::CAPU_OK == result);
        EXPECT_EQ(3u, vector.size());
        EXPECT_EQ(TypeParam(2u), vector[0]);
        EXPECT_EQ(TypeParam(4u), vector[1]);
        EXPECT_EQ(TypeParam(5u), vector[2]);

        iter = vector.end();

        result = vector.erase(iter);

        EXPECT_FALSE(ramses_capu::CAPU_OK == result);
        EXPECT_EQ(3u, vector.size());
        EXPECT_EQ(TypeParam(2u), vector[0]);
        EXPECT_EQ(TypeParam(4u), vector[1]);
        EXPECT_EQ(TypeParam(5u), vector[2]);

        --iter;

        result = vector.erase(iter);

        EXPECT_TRUE(ramses_capu::CAPU_OK == result);
        EXPECT_EQ(2u, vector.size());
        EXPECT_EQ(TypeParam(2u), vector[0]);
        EXPECT_EQ(TypeParam(4u), vector[1]);

        result = vector.erase(vector.begin());

        EXPECT_TRUE(ramses_capu::CAPU_OK == result);

        result = vector.erase(vector.begin());

        EXPECT_TRUE(ramses_capu::CAPU_OK == result);
        EXPECT_EQ(0u, vector.size());
    }

    TYPED_TEST(TypedVectorTest, EraseWithElementOld)
    {
        ramses_capu::vector<TypeParam> vector;

        vector.push_back(TypeParam(1u));
        vector.push_back(TypeParam(2u));
        vector.push_back(TypeParam(3u));
        vector.push_back(TypeParam(4u));
        vector.push_back(TypeParam(5u));

        TypeParam old;

        vector.erase(vector.begin(), &old);
        EXPECT_EQ(old, TypeParam(1u));

        vector.erase(vector.begin()+2u, &old);
        EXPECT_EQ(old, TypeParam(4u));
    }

    TYPED_TEST(TypedVectorTest, EraseAdd)
    {
        ramses_capu::vector<TypeParam> vector;

        vector.push_back(TypeParam(1u));
        vector.push_back(TypeParam(2u));
        vector.push_back(TypeParam(3u));
        vector.push_back(TypeParam(4u));
        vector.push_back(TypeParam(5u));

        vector.erase(3);
        vector.erase(1);
        vector.push_back(TypeParam(2u));
        vector.push_back(TypeParam(4u));

        EXPECT_EQ(5u, vector.size());
        EXPECT_EQ(TypeParam(1u), vector[0]);
        EXPECT_EQ(TypeParam(3u), vector[1]);
        EXPECT_EQ(TypeParam(5u), vector[2]);
        EXPECT_EQ(TypeParam(2u), vector[3]);
        EXPECT_EQ(TypeParam(4u), vector[4]);
    }

    TYPED_TEST(TypedVectorTest, ForEach)
    {
        ramses_capu::vector<TypeParam> vector;

        vector.push_back(TypeParam(32));
        vector.push_back(TypeParam(43));
        vector.push_back(TypeParam(44));

        ramses_capu::vector<TypeParam> testVector;
        for (const auto& el : vector)
        {
            testVector.push_back(el);
        }

        EXPECT_EQ(TypeParam(32), testVector[0]);
        EXPECT_EQ(TypeParam(43), testVector[1]);
        EXPECT_EQ(TypeParam(44), testVector[2]);

    }

    TYPED_TEST(TypedVectorTest, Compare)
    {
        ramses_capu::vector<TypeParam> vector1;
        ramses_capu::vector<TypeParam> vector2;
        ramses_capu::vector<TypeParam> vector3;

        vector1.push_back(TypeParam(1));
        vector1.push_back(TypeParam(2));
        vector1.push_back(TypeParam(3));

        vector2.push_back(TypeParam(1));
        vector2.push_back(TypeParam(2));
        vector2.push_back(TypeParam(2));

        EXPECT_FALSE(vector1 == vector2);

        vector2[2] = TypeParam(3);

        EXPECT_TRUE(vector1 == vector2);

        EXPECT_FALSE(vector3 == vector2);

    }

    TYPED_TEST(TypedVectorTest, TestFrontBack)
    {
        ramses_capu::vector<TypeParam> vec;
        vec.push_back(TypeParam(1));
        vec.push_back(TypeParam(2));
        vec.push_back(TypeParam(3));

        const ramses_capu::vector<TypeParam>& constVec = vec;
        EXPECT_EQ(TypeParam(1), vec.front());
        EXPECT_EQ(TypeParam(1), constVec.front());

        EXPECT_EQ(TypeParam(3), vec.back());
        EXPECT_EQ(TypeParam(3), constVec.back());
    }

    TEST(VectorTest, CompareComplexType)
    {
        ramses_capu::vector<ramses_capu::String> vector1;
        ramses_capu::vector<ramses_capu::String> vector2;
        ramses_capu::vector<ramses_capu::String> vector3;

        vector1.push_back("test");
        vector2.push_back("something");

        EXPECT_FALSE(vector1 == vector2);

        vector2.clear();
        vector2.push_back("test");

        EXPECT_TRUE(vector1 ==  vector2);

        EXPECT_FALSE(vector3 == vector2);
    }

    TEST(VectorTest, iteratorDifference)
    {
        ramses_capu::vector<ramses_capu::uint_t> vec(3, 0);

        EXPECT_EQ(3, vec.end() - vec.begin());
        EXPECT_EQ(-3, vec.begin() - vec.end());
        EXPECT_EQ(0, vec.begin() - vec.begin());
    }

    TEST(VectorTest, IteratorAccessOperators)
    {
        ramses_capu::vector<ComplexTestType> vec(3, 0);

        ramses_capu::vector<ComplexTestType>::Iterator it = vec.begin();
        EXPECT_EQ(0u, (*it).value);
        EXPECT_EQ(0u, it->value);
    }

    TEST(VectorTest, ConstIteratorAccessOperators)
    {
        ramses_capu::vector<ComplexTestType> vec(3, 0);
        const ramses_capu::vector<ComplexTestType>& constVec = vec;

        ramses_capu::vector<ComplexTestType>::ConstIterator it = constVec.begin();
        EXPECT_EQ(0u, (*it).value);
        EXPECT_EQ(0u, it->value);
    }

    TEST(VectorTest, relationalOpsOnIterators)
    {
        ramses_capu::vector<ramses_capu::uint_t> vec(2, 0);

        EXPECT_TRUE(vec.begin() == vec.begin());
        EXPECT_TRUE(vec.begin() != vec.end());
        EXPECT_TRUE(vec.begin() < vec.begin() + 1u);
        EXPECT_TRUE(vec.begin() + 1u > vec.begin());
    }

    TEST(VectorTest, relationalOpsOnIteratorsForConstValues)
    {
        ramses_capu::vector<const ramses_capu::uint_t*> vec(2, 0);

        EXPECT_TRUE(vec.begin() == vec.begin());
        EXPECT_TRUE(vec.begin() != vec.end());
        EXPECT_TRUE(vec.begin() < vec.begin() + 1u);
        EXPECT_TRUE(vec.begin() + 1u > vec.begin());
    }

    TEST(VectorTest, relationalOpsOnConstIterators)
    {
        ramses_capu::vector<ramses_capu::uint_t> vec(2, 0);
        const ramses_capu::vector<ramses_capu::uint_t>& vecConst = vec;

        EXPECT_TRUE(vecConst.begin() == vecConst.begin());
        EXPECT_TRUE(vecConst.begin() != vecConst.end());
        EXPECT_TRUE(vecConst.begin() < vecConst.begin() + 1u);
        EXPECT_TRUE(vecConst.begin() + 1u > vecConst.begin());
    }

    TEST(VectorTest, relationalOpsOnMixedteratorAndConstIterators)
    {
        ramses_capu::vector<ramses_capu::uint_t> vec(2, 0);
        const ramses_capu::vector<ramses_capu::uint_t>& vecConst = vec;

        EXPECT_TRUE(vecConst.begin() == vec.begin());
        EXPECT_TRUE(vecConst.begin() != vec.end());

        EXPECT_TRUE(vec.begin() == vecConst.begin());
        EXPECT_TRUE(vec.begin() != vecConst.end());
    }

    TEST(VectorTest, SwapVectorViaMemberFunction)
    {
        ramses_capu::vector<ramses_capu::uint_t> vec(3);
        vec[0] = 1;
        vec[1] = 2;
        vec[2] = 3;
        vec.reserve(20);
        ramses_capu::vector<ramses_capu::uint_t> other(1);
        other[0] = 11;

        vec.swap(other);
        EXPECT_EQ(1u, vec.capacity());
        EXPECT_EQ(20u, other.capacity());
        ASSERT_EQ(1u, vec.size());
        EXPECT_EQ(11u, vec[0]);
        ASSERT_EQ(3u, other.size());
        EXPECT_EQ(1u, other[0]);
        EXPECT_EQ(2u, other[1]);
        EXPECT_EQ(3u, other[2]);
    }

    TEST(VectorTest, SwapVectorViaSwapFreeFunction)
    {
        ramses_capu::vector<ramses_capu::uint_t> vec(3);
        vec[0] = 1;
        vec[1] = 2;
        vec[2] = 3;
        vec.reserve(20);
        ramses_capu::vector<ramses_capu::uint_t> other(1);
        other[0] = 11;

        using std::swap;
        swap(vec, other);
        EXPECT_EQ(1u, vec.capacity());
        EXPECT_EQ(20u, other.capacity());
        ASSERT_EQ(1u, vec.size());
        EXPECT_EQ(11u, vec[0]);
        ASSERT_EQ(3u, other.size());
        EXPECT_EQ(1u, other[0]);
        EXPECT_EQ(2u, other[1]);
        EXPECT_EQ(3u, other[2]);
    }

    TEST(VectorTest, Shrink)
    {
        ramses_capu::vector<ramses_capu::uint_t> vec(3, 0);
        vec[0] = 1;
        vec[1] = 2;
        vec[2] = 3;
        vec.reserve(20);
        EXPECT_EQ(20u, vec.capacity());

        vec.shrink_to_fit();
        EXPECT_EQ(3u, vec.capacity());
        EXPECT_EQ(3u, vec.size());

        vec.shrink_to_fit();
        EXPECT_EQ(3u, vec.capacity());
        EXPECT_EQ(3u, vec.size());

        EXPECT_EQ(1u, vec[0]);
        EXPECT_EQ(2u, vec[1]);
        EXPECT_EQ(3u, vec[2]);
    }

    TEST(VectorTest, ExponentialGrowOnPushBack)
    {
        ramses_capu::vector<ramses_capu::uint_t> vec;
        EXPECT_EQ(0u, vec.capacity());
        vec.push_back(0);
        EXPECT_EQ(1u, vec.capacity());
        vec.push_back(0);
        EXPECT_EQ(2u, vec.capacity());
        vec.push_back(0);
        EXPECT_EQ(4u, vec.capacity());
        vec.push_back(0);
        vec.push_back(0);
        EXPECT_EQ(8u, vec.capacity());
    }

    TEST(VectorTest, ExponentialGrowOnSmallResize)
    {
        ramses_capu::vector<ramses_capu::uint_t> vec;
        EXPECT_EQ(0u, vec.capacity());
        vec.resize(vec.size() + 1);
        EXPECT_EQ(1u, vec.capacity());
        vec.resize(vec.size() + 1);
        EXPECT_EQ(2u, vec.capacity());
        vec.resize(vec.size() + 1);
        EXPECT_EQ(4u, vec.capacity());
        vec.resize(vec.size() + 2);
        EXPECT_EQ(8u, vec.capacity());
    }

    TEST(VectorTest, ExponentialGrowOnInsertSingle)
    {
        ramses_capu::vector<ramses_capu::uint_t> vec;
        EXPECT_EQ(0u, vec.capacity());
        vec.insert(vec.end(), 0);
        EXPECT_EQ(1u, vec.capacity());
        vec.insert(vec.end(), 0);
        EXPECT_EQ(2u, vec.capacity());
        vec.insert(vec.end(), 0);
        EXPECT_EQ(4u, vec.capacity());
        vec.insert(vec.end(), 0);
        vec.insert(vec.end(), 0);
        EXPECT_EQ(8u, vec.capacity());
    }

    TEST(VectorTest, ExponentialGrowOnInsertSmallRange)
    {
        ramses_capu::vector<ramses_capu::uint_t> vec;
        ramses_capu::vector<ramses_capu::uint_t> rangeVec(1);

        EXPECT_EQ(0u, vec.capacity());
        vec.insert(vec.end(), rangeVec.begin(), rangeVec.end());
        EXPECT_EQ(1u, vec.capacity());
        vec.insert(vec.end(), rangeVec.begin(), rangeVec.end());
        EXPECT_EQ(2u, vec.capacity());
        vec.insert(vec.end(), rangeVec.begin(), rangeVec.end());
        EXPECT_EQ(4u, vec.capacity());
        vec.insert(vec.end(), rangeVec.begin(), rangeVec.end());
        vec.insert(vec.end(), rangeVec.begin(), rangeVec.end());
        EXPECT_EQ(8u, vec.capacity());
    }

    TEST(VectorTest, MixedGrowthBehaviorOnResize)
    {
        ramses_capu::vector<ramses_capu::uint_t> vec;
        EXPECT_EQ(0u, vec.capacity());
        vec.resize(1);
        EXPECT_EQ(1u, vec.capacity());
        vec.resize(3);
        EXPECT_EQ(3u, vec.capacity());
        vec.resize(4);
        EXPECT_EQ(6u, vec.capacity());
        vec.resize(15);
        EXPECT_EQ(15u, vec.capacity());
    }

    TEST(VectorTest, MixedGrowthBehaviorOnInsertRange)
    {
        ramses_capu::vector<ramses_capu::uint_t> vec;
        ramses_capu::vector<ramses_capu::uint_t> rangeVec(11);

        EXPECT_EQ(0u, vec.capacity());
        vec.insert(vec.end(), rangeVec.begin(), rangeVec.begin() + 1);
        EXPECT_EQ(1u, vec.capacity());
        vec.insert(vec.end(), rangeVec.begin(), rangeVec.begin() + 2);
        EXPECT_EQ(3u, vec.capacity());
        vec.insert(vec.end(), rangeVec.begin(), rangeVec.begin() + 1);
        EXPECT_EQ(6u, vec.capacity());
        vec.insert(vec.end(), rangeVec.begin(), rangeVec.end());
        EXPECT_EQ(15u, vec.capacity());
    }

    TEST(VectorTest, cbeginReturnsConstIteratorToBegin)
    {
        ramses_capu::vector<ramses_capu::uint_t> vec;
        vec.push_back(1);
        ramses_capu::vector<ramses_capu::uint_t>::ConstIterator it = vec.cbegin();
        EXPECT_TRUE(it == vec.begin());
    }

    TEST(VectorTest, cendReturnsConstIteratorToEnd)
    {
        ramses_capu::vector<ramses_capu::uint_t> vec;
        vec.push_back(1);
        ramses_capu::vector<ramses_capu::uint_t>::ConstIterator it = vec.cend();
        EXPECT_TRUE(it == vec.end());
    }

    TEST(VectorTest, dataReturnPointerToDataStore)
    {
        ramses_capu::vector<ramses_capu::uint_t> vec;
        vec.push_back(1);
        const ramses_capu::vector<ramses_capu::uint_t>& vecConst = *const_cast<const ramses_capu::vector<ramses_capu::uint_t>*>(&vec);
        EXPECT_TRUE(vec.data() == &vec.front());
        EXPECT_TRUE(vecConst.data() == &vec.front());
    }

    TEST(VectorTest, dataOnEmptyVectorReturnsNull)
    {
        ramses_capu::vector<ramses_capu::uint_t> vec;
        const ramses_capu::vector<ramses_capu::uint_t>& vecConst = *const_cast<const ramses_capu::vector<ramses_capu::uint_t>*>(&vec);
        EXPECT_TRUE(vec.data() == NULL);
        EXPECT_TRUE(vecConst.data() == NULL);
    }

    TEST(VectorTest, IteratorFullfillsStandard)
    {
        typedef ramses_capu::vector<ramses_capu::uint_t> UintVector;
        ramses_capu::IteratorTestHelper::IteratorHasTag<UintVector, std::random_access_iterator_tag>();

        UintVector v;
        v.push_back(1);
        v.push_back(2);
        v.push_back(3);

        ramses_capu::IteratorTestHelper::IteratorImplementsAllNecessaryMethods(v);
    }

    TEST(VectorTest, ConstructFromEmptyInitializerList)
    {
        ramses_capu::vector<int> v({});
        EXPECT_EQ(0u, v.size());
    }

    TEST(VectorTest, ConstructFromInitializerList)
    {
        ramses_capu::vector<int> v({1, 2, 3});
        ASSERT_EQ(3u, v.size());
        EXPECT_EQ(1, v[0]);
        EXPECT_EQ(2, v[1]);
        EXPECT_EQ(3, v[2]);
    }

    TEST(VectorTest, MoveConstructFromOther)
    {
        ramses_capu::vector<int> other({1, 2});
        ramses_capu::vector<int> v(std::move(other));

        ASSERT_EQ(0u, other.size());  // not allowed on std::vector, relies on implementation
        ASSERT_EQ(2u, v.size());
        EXPECT_EQ(1, v[0]);
        EXPECT_EQ(2, v[1]);
    }

    TEST(VectorTest, MoveAssignFromOther)
    {
        ramses_capu::vector<int> v = {1};
        ramses_capu::vector<int> other = {1, 2};

        v = std::move(other);

        ASSERT_EQ(0u, other.size());  // not allowed on std::vector, relies on implementation
        ASSERT_EQ(2u, v.size());
        EXPECT_EQ(1, v[0]);
        EXPECT_EQ(2, v[1]);
    }

    TEST(VectorTest, ReserveCanMoveInternally)
    {
        ramses_capu::vector<MoveableComplexTestType> v(3);

        MoveableComplexTestType::Reset();
        v.reserve(v.capacity() + 1);
        EXPECT_EQ(0u, MoveableComplexTestType::copyctor_count);
        EXPECT_EQ(3u, MoveableComplexTestType::movector_count);
    }

    TEST(VectorTest, PushBackHasMoveOverload)
    {
        ramses_capu::vector<MoveableComplexTestType> v;
        v.reserve(2);

        MoveableComplexTestType::Reset();
        v.push_back(MoveableComplexTestType(1));
        v.push_back(MoveableComplexTestType(1));
        EXPECT_EQ(0u, MoveableComplexTestType::copyctor_count);
        EXPECT_EQ(2u, MoveableComplexTestType::movector_count);
    }

    TEST(VectorTest, HasEmplaceBack)
    {
        ramses_capu::vector<MoveableComplexTestType> v;
        v.reserve(2);

        MoveableComplexTestType::Reset();
        v.emplace_back(1, 2, 3);
        v.emplace_back(1, 2, 3);
        EXPECT_EQ(2u, MoveableComplexTestType::ctor_count);
        EXPECT_EQ(0u, MoveableComplexTestType::copyctor_count);
        EXPECT_EQ(0u, MoveableComplexTestType::movector_count);
    }

    TEST(VectorTest, CanWorkWithMoveOnlyType)
    {
        ramses_capu::vector<MoveOnlyComplexTestType> v;
        v.reserve(2);

        v.push_back(MoveOnlyComplexTestType(1));
        v.emplace_back(2);
        v.push_back(MoveOnlyComplexTestType(3));
        v.push_back(MoveOnlyComplexTestType(4));

        ASSERT_EQ(4u, v.size());
        EXPECT_EQ(1, v[0].value);
        EXPECT_EQ(2, v[1].value);
        EXPECT_EQ(3, v[2].value);
        EXPECT_EQ(4, v[3].value);

        v.pop_back();
        EXPECT_EQ(3u, v.size());

        v.resize(2);

        ASSERT_EQ(2u, v.size());
        EXPECT_EQ(1, v[0].value);
        EXPECT_EQ(2, v[1].value);
    }
