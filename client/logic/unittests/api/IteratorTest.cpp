//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "ramses-logic/Iterator.h"

namespace rlogic
{
    class TestIteratorItem
    {
    public:
        explicit TestIteratorItem(size_t id)
            : m_id(id)
        {
        }

        [[nodiscard]] size_t getId() const
        {
            return m_id;
        }

        void setId(size_t newId)
        {
            m_id = newId;
        }

    private:
        size_t m_id;
    };

    using TestIternalType = std::vector<TestIteratorItem*>;
    using TestIteratorType = Iterator<TestIteratorItem, TestIternalType, false>;
    using TestConstIteratorType = Iterator<TestIteratorItem, TestIternalType, true>;

    class AIterator : public ::testing::Test
    {
    protected:
        AIterator()
        {
            auto testIteratorItem1 = std::make_unique<TestIteratorItem>(1);
            auto testIteratorItem2 = std::make_unique<TestIteratorItem>(2);
            auto testIteratorItem3 = std::make_unique<TestIteratorItem>(3);

            m_internalContainer.push_back(testIteratorItem1.get());
            m_internalContainer.push_back(testIteratorItem2.get());
            m_internalContainer.push_back(testIteratorItem3.get());
            m_objectsOwningVector.push_back(std::move(testIteratorItem1));
            m_objectsOwningVector.push_back(std::move(testIteratorItem2));
            m_objectsOwningVector.push_back(std::move(testIteratorItem3));

            m_begin = TestIteratorType(m_internalContainer.begin());
            m_end = TestIteratorType(m_internalContainer.end());
            m_cbegin = TestConstIteratorType(m_internalContainer.begin());
            m_cend = TestConstIteratorType(m_internalContainer.end());
        }

        std::vector<std::unique_ptr<TestIteratorItem>> m_objectsOwningVector;
        std::vector<TestIteratorItem*> m_internalContainer;
        TestIteratorType m_begin;
        TestIteratorType m_end;
        TestConstIteratorType m_cbegin;
        TestConstIteratorType m_cend;
    };

    TEST_F(AIterator, CanBeDereferenced)
    {
        EXPECT_EQ(*m_internalContainer.begin(), *m_begin);
        EXPECT_EQ(*m_internalContainer.begin(), *m_cbegin);
    }

    TEST_F(AIterator, CanBePostIncremented)
    {
        auto beforeIncrement = m_begin++;
        EXPECT_EQ(m_begin->getId(), 2u);
        EXPECT_EQ(beforeIncrement->getId(), 1u);
    }

    TEST_F(AIterator, CanBePreIncremented)
    {
        auto incremented = ++m_begin;
        EXPECT_EQ(m_begin->getId(), 2u);
        EXPECT_EQ(incremented->getId(), 2u);
    }

    TEST_F(AIterator, CanBeComparedToOtherIterators)
    {
        EXPECT_EQ(m_begin, m_begin);
        EXPECT_EQ(m_end, m_end);
        EXPECT_NE(m_begin, m_end);
    }

    TEST_F(AIterator, CanBeAssignedToOtherIterator)
    {
        TestIteratorType toBeAssigned;

        toBeAssigned = ++m_begin;
        EXPECT_EQ(toBeAssigned->getId(), 2u);
    }

    TEST_F(AIterator, CanBeAssignedToConstIterator)
    {
        TestIteratorType nonConstIter(m_begin);
        TestConstIteratorType constIter;

        constIter = nonConstIter;
        EXPECT_EQ(constIter->getId(), 1u);
    }

    TEST_F(AIterator, CanBeCopiedIntoConstIterator)
    {
        TestIteratorType nonConstIter(m_begin);
        TestConstIteratorType constIter = nonConstIter;
        EXPECT_EQ(constIter->getId(), 1u);
    }

    TEST_F(AIterator, CanBeCopied)
    {
        auto iteratorCopy1 = m_begin;
        EXPECT_EQ(iteratorCopy1, m_begin);
        auto iteratorCopy2(m_begin);
        EXPECT_EQ(iteratorCopy2, m_begin);

        EXPECT_EQ(iteratorCopy1->getId(), 1u);
        EXPECT_EQ(iteratorCopy2->getId(), 1u);
    }

    TEST_F(AIterator, CanBeDefaultInitialized)
    {
        TestIteratorType defaultInitializedIterator1;
        TestIteratorType defaultInitializedIterator2;
        EXPECT_EQ(defaultInitializedIterator1, defaultInitializedIterator2);
        TestConstIteratorType defaultInitializedConstIterator1;
        TestConstIteratorType defaultInitializedConstIterator2;
        EXPECT_EQ(defaultInitializedConstIterator1, defaultInitializedConstIterator2);
    }

    TEST_F(AIterator, ProvidesNonConstAccessToIterable)
    {
        m_begin->setId(15);
        EXPECT_EQ(15u, m_begin->getId());
    }

    TEST_F(AIterator, CanCompareConstAndMutableIteratorsInAnyDirection)
    {
        EXPECT_EQ(m_cbegin, m_begin);
        EXPECT_EQ(m_begin, m_cbegin);
        EXPECT_EQ(m_cend, m_end);

        EXPECT_NE(m_cbegin, m_end);
        EXPECT_NE(m_cend, m_begin);
        EXPECT_EQ(m_cend, m_end);
    }
}
