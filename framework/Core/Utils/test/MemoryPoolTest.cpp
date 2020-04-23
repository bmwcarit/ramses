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
#include "ErrorTestUtils.h"

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

        static const UInt32 InitialSize = 100u;
    };

    typedef ::testing::Types<
        MemoryPool<ComparableObject, UInt32>,
        MemoryPool<ComparableObject, UInt16>
    > MemoryPoolTypes;

    TYPED_TEST_SUITE(AMemoryPool, MemoryPoolTypes);

    TYPED_TEST(AMemoryPool, HasInitialSizes)
    {
        EXPECT_EQ(UInt32(this->InitialSize), this->memoryPool.getTotalCount());
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
        UInt32 totalObjectsBeforeDeletion = this->memoryPool.getTotalCount();
        UInt32 actualObjectsBeforeDeletion = this->memoryPool.getActualCount();

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
        UInt32 totalObjects = this->memoryPool.getTotalCount();
        UInt32 actualObjects = this->memoryPool.getActualCount();

        typename TypeParam::handle_type desiredHandle = static_cast<typename TypeParam::handle_type>(this->InitialSize - 10u);
        typename TypeParam::handle_type someObject = this->memoryPool.allocate(desiredHandle);
        EXPECT_EQ(someObject, desiredHandle);

        EXPECT_EQ(totalObjects, this->memoryPool.getTotalCount());
        EXPECT_EQ(actualObjects + 1u, this->memoryPool.getActualCount());
    }

    TYPED_TEST(AMemoryPool, AllocateUsingSpecificHandleBeyondCapacity)
    {
        const UInt32 totalObjects = this->memoryPool.getTotalCount();

        typename TypeParam::handle_type desiredHandle = static_cast<typename TypeParam::handle_type>(totalObjects + 10u);
        typename TypeParam::handle_type someObject = this->memoryPool.allocate(desiredHandle);
        EXPECT_EQ(someObject, desiredHandle);

        EXPECT_EQ(totalObjects + 10u + 1u, this->memoryPool.getTotalCount());
    }

    TYPED_TEST(AMemoryPool, AllocatingFullInitialCapacityDoesNotChangeCapacity)
    {
        const UInt32 totalCapacity = this->memoryPool.getTotalCount();
        const UInt32 usedCapacity = this->memoryPool.getActualCount();
        const UInt32 freeCapacity = totalCapacity - usedCapacity;

        for (UInt32 i = 0u; i < freeCapacity; ++i)
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
