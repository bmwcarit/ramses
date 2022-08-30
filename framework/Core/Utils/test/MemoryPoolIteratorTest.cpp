//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Utils/MemoryPoolIterator.h"
#include "Utils/MemoryPool.h"
#include "Utils/MemoryPoolExplicit.h"
#include "Common/StronglyTypedValue.h"
#include "framework_common_gmock_header.h"
#include "gtest/gtest.h"

using namespace testing;

namespace ramses_internal
{
    template <typename T>
    class AMemoryPoolIterator : public testing::Test
    {
    public:
        AMemoryPoolIterator()
            : memoryPool(InitialSize)
        {
            allocatedObject = memoryPool.allocate(typename T::handle_type(0u));
        }

    protected:
        T    memoryPool;
        typename T::handle_type allocatedObject;

        static const UInt32 InitialSize = 100u;
    };

    namespace
    {
        struct DummyMemoryHandleTag {};
        using DummyMemoryHandle = TypedMemoryHandle<DummyMemoryHandleTag>;

        struct DummyTestObject
        {
            uint32_t integer;
        };
    }

    using MemoryPoolTypes = ::testing::Types<
        MemoryPool<DummyTestObject, UInt32>,
        MemoryPool<DummyTestObject, UInt16>,
        MemoryPool<DummyTestObject, DummyMemoryHandle>,
        MemoryPoolExplicit<DummyTestObject, UInt32>,
        MemoryPoolExplicit<DummyTestObject, UInt16>,
        MemoryPoolExplicit<DummyTestObject, DummyMemoryHandle>
    >;

    TYPED_TEST_SUITE(AMemoryPoolIterator, MemoryPoolTypes);

    TYPED_TEST(AMemoryPoolIterator, canIterateOverEmptyMemoryPool)
    {
        TypeParam pool;
        ASSERT_EQ(0u, pool.getTotalCount());

        typename TypeParam::iterator it = pool.begin();
        EXPECT_EQ(it, pool.end());

        typename TypeParam::const_iterator constIt = pool.cbegin();
        EXPECT_EQ(constIt, pool.cend());

        for (const auto& e : pool)
        {
            UNUSED(e);
            ASSERT_TRUE(false);
        }
    }

    TYPED_TEST(AMemoryPoolIterator, canIterateOverEmptyConstMemoryPool)
    {
        const TypeParam constPool;
        ASSERT_EQ(0u, constPool.getTotalCount());

        typename TypeParam::const_iterator itConstPool = constPool.begin();
        EXPECT_EQ(itConstPool, constPool.end());
        EXPECT_EQ(itConstPool, constPool.cend());

        typename TypeParam::const_iterator itConstPool2 = constPool.cbegin();
        EXPECT_EQ(itConstPool2, constPool.cend());
        EXPECT_EQ(itConstPool2, constPool.cend());

        for (const auto& e : constPool)
        {
            UNUSED(e);
            ASSERT_TRUE(false);
        }
    }

    TYPED_TEST(AMemoryPoolIterator, canIterateOverEmptyPreallocatedMemoryPool)
    {
        TypeParam pool;
        pool.preallocateSize(10);
        ASSERT_EQ(10u, pool.getTotalCount());

        typename TypeParam::iterator it = pool.begin();
        EXPECT_EQ(it, pool.end());

        typename TypeParam::const_iterator constIt = pool.cbegin();
        EXPECT_EQ(constIt, pool.cend());

        for (const auto& e : pool)
        {
            UNUSED(e);
            ASSERT_TRUE(false);
        }
    }

    TYPED_TEST(AMemoryPoolIterator, canIterateOverEmptyPreallocatedConstMemoryPool)
    {
        TypeParam pool;
        pool.preallocateSize(10);
        const TypeParam constPool(pool);
        ASSERT_EQ(10u, constPool.getTotalCount());

        typename TypeParam::const_iterator itConstPool = constPool.begin();
        EXPECT_EQ(itConstPool, constPool.end());
        EXPECT_EQ(itConstPool, constPool.cend());

        typename TypeParam::const_iterator itConstPool2 = constPool.cbegin();
        EXPECT_EQ(itConstPool2, constPool.cend());
        EXPECT_EQ(itConstPool2, constPool.cend());

        for (const auto& e : constPool)
        {
            UNUSED(e);
            ASSERT_TRUE(false);
        }
    }

    TYPED_TEST(AMemoryPoolIterator, canIterateOverMemoryPoolWithConstIteratorPreIncrement)
    {
        TypeParam pool;
        pool.preallocateSize(10);
        pool.allocate(typename TypeParam::handle_type(1));
        pool.allocate(typename TypeParam::handle_type(5));
        ASSERT_EQ(10u, pool.getTotalCount());

        typename TypeParam::const_iterator it = pool.cbegin();
        EXPECT_EQ((*it).first, 1u);
        EXPECT_EQ(it->first, 1u);
        EXPECT_NE(it, pool.cend());

        auto it2 = ++it;
        EXPECT_EQ((*it).first, 5u);
        EXPECT_EQ(it->first, 5u);
        EXPECT_NE(it, pool.cend());

        EXPECT_EQ((*it2).first, 5u);
        EXPECT_EQ(it2->first, 5u);
        EXPECT_NE(it2, pool.cend());

        ++it;
        EXPECT_EQ(it, pool.cend());
    }

    TYPED_TEST(AMemoryPoolIterator, canIterateOverMemoryPoolWithConstIteratorPostIncrement)
    {
        TypeParam pool;
        pool.preallocateSize(10);
        pool.allocate(typename TypeParam::handle_type(1));
        pool.allocate(typename TypeParam::handle_type(5));
        ASSERT_EQ(10u, pool.getTotalCount());

        typename TypeParam::const_iterator it = pool.cbegin();
        EXPECT_EQ((*it).first, 1u);
        EXPECT_EQ(it->first, 1u);
        EXPECT_NE(it, pool.cend());

        auto it2 = it++;
        EXPECT_EQ((*it).first, 5u);
        EXPECT_EQ(it->first, 5u);
        EXPECT_NE(it, pool.cend());

        EXPECT_EQ((*it2).first, 1u);
        EXPECT_EQ(it2->first, 1u);
        EXPECT_NE(it2, pool.cend());

        it++;
        EXPECT_EQ(it, pool.cend());
    }

    TYPED_TEST(AMemoryPoolIterator, canIterateOverMemoryPoolWithConstConsIterator)
    {
        TypeParam pool;
        pool.preallocateSize(10);
        ASSERT_EQ(10u, pool.getTotalCount());

        const typename TypeParam::const_iterator itEnd = pool.cend();
        {
            const typename TypeParam::const_iterator it = pool.cbegin();
            EXPECT_EQ(it, itEnd);
        }

        pool.allocate(typename TypeParam::handle_type(1));
        {
            const typename TypeParam::const_iterator it = pool.cbegin();
            EXPECT_EQ((*it).first, 1u);
            EXPECT_EQ(it->first, 1u);
            EXPECT_NE(it, itEnd);
            EXPECT_NE(it, pool.cend());
        }
    }

    TYPED_TEST(AMemoryPoolIterator, canIterateOverMemoryPoolWithNonConstIteratorPreIncrement)
    {
        TypeParam pool;
        pool.preallocateSize(10);
        pool.allocate(typename TypeParam::handle_type(1));
        pool.allocate(typename TypeParam::handle_type(5));
        ASSERT_EQ(10u, pool.getTotalCount());

        typename TypeParam::iterator it = pool.begin();
        EXPECT_EQ((*it).first, 1u);
        EXPECT_EQ(it->first, 1u);
        EXPECT_NE(it, pool.end());

        auto it2 = ++it;
        EXPECT_EQ((*it).first, 5u);
        EXPECT_EQ(it->first, 5u);
        EXPECT_NE(it, pool.end());

        EXPECT_EQ((*it2).first, 5u);
        EXPECT_EQ(it2->first, 5u);
        EXPECT_NE(it2, pool.end());

        ++it;
        EXPECT_EQ(it, pool.end());
    }

    TYPED_TEST(AMemoryPoolIterator, canIterateOverMemoryPoolWithNonConstIteratorPostIncrement)
    {
        TypeParam pool;
        pool.preallocateSize(10);
        pool.allocate(typename TypeParam::handle_type(1));
        pool.allocate(typename TypeParam::handle_type(5));
        ASSERT_EQ(10u, pool.getTotalCount());

        typename TypeParam::iterator it = pool.begin();
        EXPECT_EQ((*it).first, 1u);
        EXPECT_EQ(it->first, 1u);
        EXPECT_NE(it, pool.end());

        auto it2 = it++;
        EXPECT_EQ((*it).first, 5u);
        EXPECT_EQ(it->first, 5u);
        EXPECT_NE(it, pool.end());

        EXPECT_EQ((*it2).first, 1u);
        EXPECT_EQ(it2->first, 1u);
        EXPECT_NE(it2, pool.end());

        it++;
        EXPECT_EQ(it, pool.end());
    }

    TYPED_TEST(AMemoryPoolIterator, canIterateOverMemoryPoolWithConstNonConstIterator)
    {
        TypeParam pool;
        pool.preallocateSize(10);
        ASSERT_EQ(10u, pool.getTotalCount());

        const typename TypeParam::iterator itEnd = pool.end();
        {
            const typename TypeParam::iterator it = pool.begin();
            EXPECT_EQ(it, itEnd);
        }

        pool.allocate(typename TypeParam::handle_type(1));
        {
            const typename TypeParam::iterator it = pool.begin();
            EXPECT_EQ((*it).first, 1u);
            EXPECT_EQ(it->first, 1u);
            EXPECT_NE(it, itEnd);
            EXPECT_NE(it, pool.end());
        }
    }

    TYPED_TEST(AMemoryPoolIterator, canIterateOverMemoryPoolUsingForEachLoop)
    {
        TypeParam pool;
        pool.preallocateSize(10);
        pool.allocate(typename TypeParam::handle_type(1));
        pool.allocate(typename TypeParam::handle_type(2));
        pool.allocate(typename TypeParam::handle_type(5));
        pool.allocate(typename TypeParam::handle_type(8));
        ASSERT_EQ(10u, pool.getTotalCount());

        std::vector<typename TypeParam::handle_type> handles;
        for (const auto& it : pool)
        {
            handles.push_back(it.first);
        }

        const std::vector<typename TypeParam::handle_type> expectedHandles{
            typename TypeParam::handle_type{1u},
            typename TypeParam::handle_type{2u},
            typename TypeParam::handle_type{5u},
            typename TypeParam::handle_type{8u} };
        EXPECT_EQ(handles, expectedHandles);
    }

    TYPED_TEST(AMemoryPoolIterator, canIterateOverConstMemoryPoolUsingConstIterator)
    {
        TypeParam pool;
        pool.preallocateSize(10);
        pool.allocate(typename TypeParam::handle_type(1));
        pool.allocate(typename TypeParam::handle_type(2));
        pool.allocate(typename TypeParam::handle_type(5));
        pool.allocate(typename TypeParam::handle_type(8));
        ASSERT_EQ(10u, pool.getTotalCount());

        const TypeParam& constPool = pool;
        typename TypeParam::const_iterator it = constPool.begin();
        EXPECT_EQ((*it).first, 1u);
        ++it;
        EXPECT_EQ((*it).first, 2u);
        ++it;
        EXPECT_EQ((*it).first, 5u);
        ++it;
        EXPECT_EQ((*it).first, 8u);
        ++it;
        EXPECT_EQ(constPool.end(), it);
    }

    TYPED_TEST(AMemoryPoolIterator, canIterateOverConstMemoryPoolUsingForEachLoop)
    {
        TypeParam pool;
        pool.preallocateSize(10);
        pool.allocate(typename TypeParam::handle_type(1));
        pool.allocate(typename TypeParam::handle_type(2));
        pool.allocate(typename TypeParam::handle_type(5));
        pool.allocate(typename TypeParam::handle_type(8));
        ASSERT_EQ(10u, pool.getTotalCount());

        const TypeParam& constPool = pool;

        std::vector<typename TypeParam::handle_type> handles;
        for (auto it : constPool)
        {
            handles.push_back(it.first);
        }

        const std::vector<typename TypeParam::handle_type> expectedHandles{
            typename TypeParam::handle_type{1u},
            typename TypeParam::handle_type{2u},
            typename TypeParam::handle_type{5u},
            typename TypeParam::handle_type{8u} };
        EXPECT_EQ(handles, expectedHandles);
    }

    TYPED_TEST(AMemoryPoolIterator, canUpdateMemoryPoolObjectsUsingNonConstIterator)
    {
        TypeParam pool;
        pool.preallocateSize(10);
        pool.allocate(typename TypeParam::handle_type(1));
        pool.allocate(typename TypeParam::handle_type(2));
        pool.allocate(typename TypeParam::handle_type(5));
        pool.allocate(typename TypeParam::handle_type(8));
        ASSERT_EQ(10u, pool.getTotalCount());

        {
            uint32_t c = 0u;
            typename TypeParam::iterator iter = pool.begin();
            iter->second->integer = c++;

            (++iter)->second->integer = c++;
            (++iter)->second->integer = c++;
            (++iter)->second->integer = c++;
        }

        {
            uint32_t c = 0u;
            typename TypeParam::const_iterator iter = pool.cbegin();
            EXPECT_EQ(c++, (iter)->second->integer);
            EXPECT_EQ(c++, (++iter)->second->integer);
            EXPECT_EQ(c++, (++iter)->second->integer);
            EXPECT_EQ(c, (++iter)->second->integer);
        }
    }

    TYPED_TEST(AMemoryPoolIterator, canUpdateMemoryPoolObjectsUsingNonConstIteratorInForEeachLoop)
    {
        TypeParam pool;
        pool.preallocateSize(10);
        pool.allocate(typename TypeParam::handle_type(1));
        pool.allocate(typename TypeParam::handle_type(2));
        pool.allocate(typename TypeParam::handle_type(5));
        pool.allocate(typename TypeParam::handle_type(8));
        ASSERT_EQ(10u, pool.getTotalCount());

        for (auto it : pool)
        {
            static UInt32 c = 0u;
            it.second->integer = c++;
        }

        for (const auto& it : pool)
        {
            static UInt32 c = 0u;
            EXPECT_EQ(it.second->integer, c++);
        }
    }

    TYPED_TEST(AMemoryPoolIterator, returnsValidBeginIterator)
    {
        TypeParam pool;
        pool.preallocateSize(10);
        ASSERT_EQ(10u, pool.getTotalCount());

        EXPECT_EQ(pool.cbegin(), pool.cend());

        pool.allocate(typename TypeParam::handle_type(1));
        typename TypeParam::const_iterator it1 = pool.cbegin();
        EXPECT_EQ(it1->first, 1u);
        ++it1;
        EXPECT_EQ(it1, pool.cend());

        pool.allocate(typename TypeParam::handle_type(0));
        typename TypeParam::const_iterator it0 = pool.cbegin();
        EXPECT_EQ(it0->first, 0u);
        ++it0;
        EXPECT_EQ(it0->first, 1u);
        ++it0;
        EXPECT_EQ(it0, pool.cend());
    }

    TYPED_TEST(AMemoryPoolIterator, canCheckNonConstIteratorForEquality)
    {
        TypeParam pool1;
        TypeParam pool2;
        pool1.preallocateSize(10);
        pool1.allocate(typename TypeParam::handle_type(1));
        pool1.allocate(typename TypeParam::handle_type(5));
        pool2.preallocateSize(10);
        pool2.allocate(typename TypeParam::handle_type(1));
        pool2.allocate(typename TypeParam::handle_type(5));
        ASSERT_EQ(10u, pool1.getTotalCount());
        ASSERT_EQ(10u, pool2.getTotalCount());

        typename TypeParam::iterator itPool1 = pool1.begin();
        typename TypeParam::iterator itPool2 = pool2.begin();

        EXPECT_TRUE(itPool1 == pool1.begin());
        EXPECT_TRUE(itPool1 != itPool2);

        ++itPool1;
        typename TypeParam::iterator otherItPool1 = pool1.begin();
        EXPECT_TRUE(itPool1 != otherItPool1);
        ++otherItPool1;
        EXPECT_TRUE(itPool1 == otherItPool1);
    }

    TYPED_TEST(AMemoryPoolIterator, canCheckConstIteratorForEquality)
    {
        TypeParam pool1;
        TypeParam pool2;
        pool1.preallocateSize(10);
        pool1.allocate(typename TypeParam::handle_type(1));
        pool1.allocate(typename TypeParam::handle_type(5));
        pool2.preallocateSize(10);
        pool2.allocate(typename TypeParam::handle_type(1));
        pool2.allocate(typename TypeParam::handle_type(5));
        ASSERT_EQ(10u, pool1.getTotalCount());
        ASSERT_EQ(10u, pool2.getTotalCount());

        typename TypeParam::const_iterator itPool1 = pool1.cbegin();
        typename TypeParam::const_iterator itPool2 = pool2.cbegin();

        EXPECT_TRUE(itPool1 == pool1.cbegin());
        EXPECT_TRUE(itPool1 != itPool2);

        ++itPool1;
        typename TypeParam::const_iterator otherItPool1 = pool1.cbegin();
        EXPECT_TRUE(itPool1 != otherItPool1);
        ++otherItPool1;
        EXPECT_TRUE(itPool1 == otherItPool1);
    }

    TYPED_TEST(AMemoryPoolIterator, canAssignNonConstIterator)
    {
        TypeParam pool1;
        TypeParam pool2;
        pool1.preallocateSize(10);
        pool1.allocate(typename TypeParam::handle_type(1));
        pool1.allocate(typename TypeParam::handle_type(5));
        ASSERT_EQ(10u, pool1.getTotalCount());

        typename TypeParam::iterator it = pool1.begin();
        typename TypeParam::iterator otherIt = pool2.begin();

        otherIt = it;
        EXPECT_EQ(it, otherIt);

        ++otherIt;
        EXPECT_EQ(it, pool1.begin());
        EXPECT_NE(it, otherIt);

        otherIt = it;
        EXPECT_EQ(it, otherIt);
    }

    TYPED_TEST(AMemoryPoolIterator, canAssignConstIterator)
    {
        TypeParam pool1;
        TypeParam pool2;
        pool1.preallocateSize(10);
        pool1.allocate(typename TypeParam::handle_type(1));
        pool1.allocate(typename TypeParam::handle_type(5));
        ASSERT_EQ(10u, pool1.getTotalCount());

        typename TypeParam::const_iterator it = pool1.cbegin();
        typename TypeParam::const_iterator otherIt = pool2.cbegin();

        otherIt = it;
        EXPECT_EQ(it, otherIt);

        ++otherIt;
        EXPECT_EQ(it, pool1.cbegin());
        EXPECT_NE(it, otherIt);

        otherIt = it;
        EXPECT_EQ(it, otherIt);
    }

    TYPED_TEST(AMemoryPoolIterator, canCopyNonConstIterator)
    {
        TypeParam pool;
        pool.preallocateSize(10);
        pool.allocate(typename TypeParam::handle_type(1));
        pool.allocate(typename TypeParam::handle_type(5));
        ASSERT_EQ(10u, pool.getTotalCount());

        typename TypeParam::iterator it = pool.begin();
        {
            typename TypeParam::iterator otherIt(it);
            EXPECT_EQ(it, otherIt);
            EXPECT_EQ(it, pool.begin());
        }

        ++it;
        {
            typename TypeParam::iterator otherIt(it);
            EXPECT_EQ(it, otherIt);
            EXPECT_NE(it, pool.begin());
        }
    }

    TYPED_TEST(AMemoryPoolIterator, canCopyConstIterator)
    {
        TypeParam pool;
        pool.preallocateSize(10);
        pool.allocate(typename TypeParam::handle_type(1));
        pool.allocate(typename TypeParam::handle_type(5));
        ASSERT_EQ(10u, pool.getTotalCount());

        typename TypeParam::const_iterator it = pool.cbegin();
        {
            typename TypeParam::const_iterator otherIt(it);
            EXPECT_EQ(it, otherIt);
            EXPECT_EQ(it, pool.cbegin());
        }

        ++it;
        {
            typename TypeParam::const_iterator otherIt(it);
            EXPECT_EQ(it, otherIt);
            EXPECT_NE(it, pool.cbegin());
        }
    }

    TYPED_TEST(AMemoryPoolIterator, canDefaultConstructNonConstIterator)
    {
        TypeParam pool;
        pool.preallocateSize(10);
        pool.allocate(typename TypeParam::handle_type(1));
        pool.allocate(typename TypeParam::handle_type(5));
        ASSERT_EQ(10u, pool.getTotalCount());

        typename TypeParam::iterator it;
        EXPECT_NE(it, pool.begin());
        EXPECT_NE(it, pool.end());

        it = pool.begin();
        EXPECT_EQ(it, pool.begin());
    }

    TYPED_TEST(AMemoryPoolIterator, canDefaultConstructConstIterator)
    {
        TypeParam pool;
        pool.preallocateSize(10);
        pool.allocate(typename TypeParam::handle_type(1));
        pool.allocate(typename TypeParam::handle_type(5));
        ASSERT_EQ(10u, pool.getTotalCount());

        typename TypeParam::const_iterator it;
        EXPECT_NE(it, pool.cbegin());
        EXPECT_NE(it, pool.cend());

        it = pool.cbegin();
        EXPECT_EQ(it, pool.cbegin());
    }
}
