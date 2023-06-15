//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "framework_common_gmock_header.h"
#include "gtest/gtest.h"
#include "Utils/MemoryPool.h"

using namespace testing;

namespace ramses_internal
{
    template <typename T>
    class AMemoryPool : public testing::Test
    {
    public:
        AMemoryPool()
            : memoryPool(InitialSize)
        {
            allocatedObject = memoryPool.allocate();
            undefinedObject = std::numeric_limits<typename T::handle_type>::max() - 1;
        }

    protected:
        T    memoryPool;
        typename T::handle_type allocatedObject;
        typename T::handle_type undefinedObject;

        static const uint32_t InitialSize = 100u;
    };

    using MemoryPoolTypes = ::testing::Types<
        MemoryPool<int, uint32_t>,
        MemoryPool<int, uint16_t>
    >;

    TYPED_TEST_SUITE(AMemoryPool, MemoryPoolTypes);

    TYPED_TEST(AMemoryPool, HasInitialSizes)
    {
        EXPECT_EQ(uint32_t(this->InitialSize), this->memoryPool.getTotalCount());
        EXPECT_EQ(1u, this->memoryPool.getActualCount());
    }

    TYPED_TEST(AMemoryPool, UnexistingObjectIsNotAllocated)
    {
        EXPECT_FALSE(this->memoryPool.isAllocated(this->undefinedObject));
    }

    TYPED_TEST(AMemoryPool, AllocatedObjectIsReportedToBeAllocated)
    {
        EXPECT_TRUE(this->memoryPool.isAllocated(this->allocatedObject));
    }

    TYPED_TEST(AMemoryPool, AllocatingSpecificHandlesResultsInSameValidHandle)
    {
        const typename TypeParam::handle_type wantedHandle = this->allocatedObject + 1u;
        ASSERT_FALSE(this->memoryPool.isAllocated(wantedHandle));
        const typename TypeParam::handle_type handle = this->memoryPool.allocate(wantedHandle);
        ASSERT_EQ(wantedHandle, handle);
        ASSERT_TRUE(this->memoryPool.isAllocated(wantedHandle));
    }

    TYPED_TEST(AMemoryPool, AllocationWithSkippingHandlesResultsInUnallocatedHandlesInBetween)
    {
        const typename TypeParam::handle_type wantedHandle = this->allocatedObject + 3u;
        const typename TypeParam::handle_type handle = this->memoryPool.allocate(wantedHandle);
        ASSERT_EQ(wantedHandle, handle);
        ASSERT_FALSE(this->memoryPool.isAllocated(this->allocatedObject + 1u));
        ASSERT_FALSE(this->memoryPool.isAllocated(this->allocatedObject + 2u));
        ASSERT_TRUE(this->memoryPool.isAllocated(wantedHandle));
    }

    TYPED_TEST(AMemoryPool, ReducesActualObjectCountByOneAfterDeletion)
    {
        uint32_t totalObjectsBeforeDeletion = this->memoryPool.getTotalCount();
        uint32_t actualObjectsBeforeDeletion = this->memoryPool.getActualCount();

        ASSERT_GT(actualObjectsBeforeDeletion, 0u);

        this->memoryPool.release(this->allocatedObject);

        EXPECT_EQ(totalObjectsBeforeDeletion, this->memoryPool.getTotalCount());
        EXPECT_EQ(actualObjectsBeforeDeletion - 1, this->memoryPool.getActualCount());
    }

    TYPED_TEST(AMemoryPool, DoesNotAffectOtherObjectsWhenDeletingObject)
    {
        typename TypeParam::handle_type someObject = this->memoryPool.allocate();
        const typename TypeParam::object_type* someObjectMemory = this->memoryPool.getMemory(someObject);

        this->memoryPool.release(this->allocatedObject);

        EXPECT_EQ(someObjectMemory, this->memoryPool.getMemory(someObject));
    }

    TYPED_TEST(AMemoryPool, AllocateUsingSpecificHandle)
    {
        uint32_t totalObjects = this->memoryPool.getTotalCount();
        uint32_t actualObjects = this->memoryPool.getActualCount();

        typename TypeParam::handle_type desiredHandle = static_cast<typename TypeParam::handle_type>(this->InitialSize - 10u);
        typename TypeParam::handle_type someObject = this->memoryPool.allocate(desiredHandle);
        EXPECT_EQ(someObject, desiredHandle);

        EXPECT_EQ(totalObjects, this->memoryPool.getTotalCount());
        EXPECT_EQ(actualObjects + 1u, this->memoryPool.getActualCount());
    }

    TYPED_TEST(AMemoryPool, AllocateUsingSpecificHandleBeyondCapacity)
    {
        const uint32_t totalObjects = this->memoryPool.getTotalCount();

        typename TypeParam::handle_type desiredHandle = static_cast<typename TypeParam::handle_type>(totalObjects + 10u);
        typename TypeParam::handle_type someObject = this->memoryPool.allocate(desiredHandle);
        EXPECT_EQ(someObject, desiredHandle);

        EXPECT_EQ(totalObjects + 10u + 1u, this->memoryPool.getTotalCount());
    }

    TYPED_TEST(AMemoryPool, AllocatingFullInitialCapacityDoesNotChangeCapacity)
    {
        const uint32_t totalCapacity = this->memoryPool.getTotalCount();
        const uint32_t usedCapacity = this->memoryPool.getActualCount();
        const uint32_t freeCapacity = totalCapacity - usedCapacity;

        for (uint32_t i = 0u; i < freeCapacity; ++i)
        {
            this->memoryPool.allocate();
        }

        EXPECT_EQ(totalCapacity, this->memoryPool.getTotalCount());
        EXPECT_EQ(totalCapacity, this->memoryPool.getActualCount());
    }

    TYPED_TEST(AMemoryPool, preallocatesMemory)
    {
        TypeParam pool;
        EXPECT_EQ(0u, pool.getTotalCount());
        EXPECT_EQ(0u, pool.getActualCount());

        pool.preallocateSize(3);
        EXPECT_EQ(3u, pool.getTotalCount());
        EXPECT_EQ(0u, pool.getActualCount());
    }

    TYPED_TEST(AMemoryPool, neverShrinksOnPreallocation)
    {
        TypeParam pool;
        pool.preallocateSize(9);
        pool.preallocateSize(3);
        EXPECT_EQ(9u, pool.getTotalCount());
        EXPECT_EQ(0u, pool.getActualCount());
    }

    TYPED_TEST(AMemoryPool, neverChangesAllocatedCountOnPreallocation)
    {
        TypeParam pool;
        pool.preallocateSize(3);
        pool.allocate(1);
        pool.allocate(2);
        EXPECT_EQ(2u, pool.getActualCount());

        pool.preallocateSize(6);
        EXPECT_EQ(6u, pool.getTotalCount());
        EXPECT_EQ(2u, pool.getActualCount());
    }
}
