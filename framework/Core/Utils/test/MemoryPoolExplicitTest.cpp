//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "framework_common_gmock_header.h"
#include "gtest/gtest.h"
#include "Utils/MemoryPoolExplicit.h"
#include "ErrorTestUtils.h"

using namespace testing;

namespace ramses_internal
{
    template <typename T>
    class AMemoryPoolExplicit : public testing::Test
    {
    public:
        AMemoryPoolExplicit()
            : memoryPool(InitialSize)
            , allocatedObject(memoryPool.allocate(static_cast<typename T::handle_type>(InitialSize / 2u)))
            , unallocatedObject(allocatedObject - 1u)
        {
        }

    protected:
        T    memoryPool;
        const typename T::handle_type allocatedObject;
        const typename T::handle_type unallocatedObject;

        static const UInt32 InitialSize = 100u;
    };

    typedef ::testing::Types <
        MemoryPoolExplicit<ComparableObject, UInt32>,
        MemoryPoolExplicit<ComparableObject, UInt16>
    > MemoryPoolTypes;

    TYPED_TEST_SUITE(AMemoryPoolExplicit, MemoryPoolTypes);

    TYPED_TEST(AMemoryPoolExplicit, HasInitialSize)
    {
        EXPECT_EQ(UInt32(this->InitialSize), this->memoryPool.getTotalCount());
    }

    TYPED_TEST(AMemoryPoolExplicit, ReportsUnallocatedObject)
    {
        EXPECT_FALSE(this->memoryPool.isAllocated(this->unallocatedObject));
    }

    TYPED_TEST(AMemoryPoolExplicit, AllocatedObjectIsReportedToBeAllocated)
    {
        EXPECT_TRUE(this->memoryPool.isAllocated(this->allocatedObject));
    }

    TYPED_TEST(AMemoryPoolExplicit, AllocatingSpecificHandlesResultsInSameValidHandle)
    {
        const typename TypeParam::handle_type wantedHandle = this->allocatedObject + 1u;
        ASSERT_FALSE(this->memoryPool.isAllocated(wantedHandle));
        const typename TypeParam::handle_type handle = this->memoryPool.allocate(wantedHandle);
        ASSERT_EQ(wantedHandle, handle);
        ASSERT_TRUE(this->memoryPool.isAllocated(wantedHandle));
    }

    TYPED_TEST(AMemoryPoolExplicit, AllocationWithSkippingHandlesResultsInUnallocatedHandlesInBetween)
    {
        const typename TypeParam::handle_type wantedHandle = this->allocatedObject + 3u;
        const typename TypeParam::handle_type handle = this->memoryPool.allocate(wantedHandle);
        ASSERT_EQ(wantedHandle, handle);
        ASSERT_FALSE(this->memoryPool.isAllocated(this->allocatedObject + 1u));
        ASSERT_FALSE(this->memoryPool.isAllocated(this->allocatedObject + 2u));
        ASSERT_TRUE(this->memoryPool.isAllocated(wantedHandle));
    }

    TYPED_TEST(AMemoryPoolExplicit, DoesNotAffectOtherObjectsWhenDeletingObject)
    {
        const typename TypeParam::handle_type wantedHandle = this->allocatedObject + 3u;
        const typename TypeParam::handle_type someObject = this->memoryPool.allocate(wantedHandle);
        const typename TypeParam::object_type* someObjectMemory = this->memoryPool.getMemory(someObject);

        this->memoryPool.release(this->allocatedObject);

        EXPECT_EQ(someObjectMemory, this->memoryPool.getMemory(someObject));
    }

    TYPED_TEST(AMemoryPoolExplicit, AllocateUsingSpecificHandle)
    {
        const UInt32 totalObjects = this->memoryPool.getTotalCount();

        const typename TypeParam::handle_type desiredHandle = static_cast<typename TypeParam::handle_type>(this->InitialSize - 10u);
        const typename TypeParam::handle_type someObject = this->memoryPool.allocate(desiredHandle);
        EXPECT_EQ(someObject, desiredHandle);

        EXPECT_EQ(totalObjects, this->memoryPool.getTotalCount());
    }

    TYPED_TEST(AMemoryPoolExplicit, CanAllocateFullCapacity)
    {
        const UInt32 totalCapacity = this->memoryPool.getTotalCount();

        for (UInt32 i = 0u; i < totalCapacity; ++i)
        {
            const typename TypeParam::handle_type handle = static_cast<typename TypeParam::handle_type>(i);
            if (handle != this->allocatedObject)
            {
                this->memoryPool.allocate(handle);
            }
        }

        for (UInt32 i = 0u; i < totalCapacity; ++i)
        {
            const typename TypeParam::handle_type handle = static_cast<typename TypeParam::handle_type>(i);
            EXPECT_TRUE(this->memoryPool.isAllocated(handle));
        }

        EXPECT_EQ(totalCapacity, this->memoryPool.getTotalCount());
    }

    TYPED_TEST(AMemoryPoolExplicit, preallocatesMemory)
    {
        TypeParam pool;
        EXPECT_EQ(0u, pool.getTotalCount());

        pool.preallocateSize(3);
        EXPECT_EQ(3u, pool.getTotalCount());
    }

    TYPED_TEST(AMemoryPoolExplicit, neverShrinksOnPreallocation)
    {
        TypeParam pool;
        pool.preallocateSize(9);
        pool.preallocateSize(3);
        EXPECT_EQ(9u, pool.getTotalCount());
    }
}
