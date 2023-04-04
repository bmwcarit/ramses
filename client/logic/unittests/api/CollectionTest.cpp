//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "ramses-logic/Collection.h"

namespace rlogic
{
    class TestCollectionItem
    {
    public:
        explicit TestCollectionItem(size_t id)
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

    using TestCollectionType = Collection<TestCollectionItem>;

    class ACollection : public ::testing::Test
    {
    protected:
        ACollection()
            : m_testCollection(m_internalContainer)
        {
            auto testCollectionItem1 = std::make_unique<TestCollectionItem>(1);
            auto testCollectionItem2 = std::make_unique<TestCollectionItem>(2);
            auto testCollectionItem3 = std::make_unique<TestCollectionItem>(3);
            m_internalContainer.push_back(testCollectionItem1.get());
            m_internalContainer.push_back(testCollectionItem2.get());
            m_internalContainer.push_back(testCollectionItem3.get());
            m_objectsOwningVector.push_back(std::move(testCollectionItem1));
            m_objectsOwningVector.push_back(std::move(testCollectionItem2));
            m_objectsOwningVector.push_back(std::move(testCollectionItem3));
        }

        std::vector<std::unique_ptr<TestCollectionItem>> m_objectsOwningVector;
        std::vector<TestCollectionItem*> m_internalContainer;
        TestCollectionType m_testCollection;
    };

    TEST_F(ACollection, ReturnsSize)
    {
        EXPECT_EQ(3u, m_testCollection.size());
    }

    TEST_F(ACollection, CanBeIteratedInRangeLoops)
    {
        size_t id = 1;
        for (auto item : m_testCollection)
        {
            EXPECT_EQ(id, item->getId());
            id++;
        }
    }

    TEST_F(ACollection, CanBeIteratedInRangeLoops_AsConstRef)
    {
        const TestCollectionType& collectionConstRef = m_testCollection;

        size_t id = 1;
        auto constIter = collectionConstRef.cbegin();
        auto constIterEnd = collectionConstRef.cend();
        for (; constIter != constIterEnd; constIter++)
        {
            const TestCollectionItem* itemPtr = *constIter;
            EXPECT_EQ(id, itemPtr->getId());
            ++id;
        }
    }

    TEST_F(ACollection, CanBeCopied)
    {
        TestCollectionType collectionCopy = m_testCollection;

        size_t id = 1;
        for (auto item : collectionCopy)
        {
            EXPECT_EQ(id, item->getId());
            id++;
        }
    }

    TEST_F(ACollection, CanBeAssigned)
    {
        TestCollectionType collectionToBeAssigned(m_internalContainer);
        collectionToBeAssigned = m_testCollection;

        size_t id = 1;
        for (auto item : collectionToBeAssigned)
        {
            EXPECT_EQ(id, item->getId());
            id++;
        }
    }

    // Shallow because changing underlying data is visible in both the original and the copy
    TEST_F(ACollection, CopyIsShallow)
    {
        TestCollectionType collectionCopy = m_testCollection;

        m_testCollection.begin()->setId(15);
        EXPECT_EQ(15u, collectionCopy.begin()->getId());
    }

    TEST_F(ACollection, CanBeUsedWithStdBeginAndStdEnd)
    {
        EXPECT_EQ(std::begin(m_testCollection), std::begin(m_testCollection));
        EXPECT_EQ(std::end(m_testCollection), std::end(m_testCollection));
        EXPECT_NE(std::end(m_testCollection), std::begin(m_testCollection));
    }

    TEST_F(ACollection, CanBePassedAsArgumentToStdAlgorithms)
    {
        TestCollectionType collectionCopy(m_testCollection);
        const TestCollectionType& collectionConstRef = m_testCollection;

        size_t i = 1;
        auto checkNames = [&i](const TestCollectionItem* item)
        {
            ASSERT_EQ(i++, item->getId());
        };
        std::for_each(m_testCollection.begin(), m_testCollection.end(), checkNames);

        i = 1;
        std::for_each(m_testCollection.cbegin(), m_testCollection.cend(), checkNames);

        i = 1;
        std::for_each(collectionConstRef.cbegin(), collectionConstRef.cend(), checkNames);

        i = 1;
        std::for_each(collectionConstRef.begin(), collectionConstRef.end(), checkNames);

        EXPECT_EQ(m_internalContainer[1], *std::find_if(m_testCollection.begin(), m_testCollection.end(), [](TestCollectionItem* item) {return item->getId() == 2; }));
        EXPECT_EQ(m_internalContainer[1], *std::find_if(collectionCopy.cbegin(), collectionCopy.cend(), [](const TestCollectionItem* item) {return item->getId() == 2; }));
        EXPECT_EQ(m_testCollection.end(), std::find_if(m_testCollection.begin(), m_testCollection.end(), [](TestCollectionItem* item) {return item->getId() == 999; }));
    }

    TEST_F(ACollection, ProvidesIncrementableIterators)
    {
        TestCollectionType collectionCopy(m_testCollection);
        auto iter = collectionCopy.begin();
        auto otherIter = iter++;
        EXPECT_NE(iter, otherIter);
        ++otherIter;
        EXPECT_EQ(otherIter, iter);
    }

    TEST_F(ACollection, ProvidesComparableIterators)
    {
        EXPECT_EQ(m_testCollection.begin(), m_testCollection.begin());
        EXPECT_EQ(m_testCollection.end(), m_testCollection.end());
        auto iter0_a = m_testCollection.begin();
        auto iter0_b = m_testCollection.begin();
        auto iter1 = ++m_testCollection.begin();
        EXPECT_EQ(iter0_a, iter0_a);
        EXPECT_NE(iter0_a, iter1);
        EXPECT_NE(iter0_b, iter1);
        EXPECT_EQ(iter1, iter1);
    }
}

