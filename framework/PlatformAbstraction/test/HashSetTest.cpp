//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Collections/HashSet.h"
#include "ComplexTestType.h"
#include <gtest/gtest.h>
#include <vector>

namespace ramses_internal
{
using TestType = ComplexTestType<>;

TEST(HashSet, Constructor_Default)
{
    HashSet<int32_t> set;
    (void)set;
}

TEST(HashSet, copyConstructor)
{
    HashSet<int32_t> set;
    set.put(1);
    set.put(2);
    set.put(3);

    HashSet<int32_t> set2 = set; // copy

    set.clear();

    EXPECT_TRUE(set2.contains(1));
    EXPECT_TRUE(set2.contains(2));
    EXPECT_TRUE(set2.contains(3));
}

TEST(HashSet, put)
{
    int32_t value2 = 10;
    int32_t value = 5;

    HashSet<int32_t>h1;

    auto it1 = h1.put(value);
    auto it2 = h1.put(value2);
    auto it3 = h1.put(value2);

    EXPECT_NE(h1.end(), it1);
    EXPECT_NE(h1.end(), it2);
    EXPECT_NE(h1.end(), it3);

    EXPECT_EQ(h1.find(value), it1);
    EXPECT_EQ(h1.find(value2), it2);
    EXPECT_EQ(it2, it3);
}

TEST(HashSet, count)
{
    int32_t value2 = 10;
    int32_t value = 5;
    HashSet<int32_t>h1;

    EXPECT_EQ(0u, h1.size());
    h1.put(value);
    h1.put(value2);

    EXPECT_EQ(2u, h1.size());
    EXPECT_TRUE(h1.remove(value2));
    EXPECT_EQ(1u, h1.size());
}

TEST(HashSet, clear)
{
    int32_t value = 5;
    int32_t value2 = 6;

    HashSet<int32_t>h1;
    h1.put(value);
    h1.put(value2);

    EXPECT_EQ(2u, h1.size());
    h1.clear();
    EXPECT_EQ(0u, h1.size());
}

TEST(HashSet, remove)
{
    int32_t value = 5;
    int32_t value2 = 6;

    HashSet<int32_t> h1;

    h1.put(value);
    EXPECT_FALSE(h1.remove(value2));

    h1.put(value2);
    EXPECT_EQ(2u, h1.size());

    EXPECT_TRUE(h1.remove(value2));
    EXPECT_EQ(1u, h1.size());
}

TEST(HashSet, hasElement)
{
    int32_t value = 5;
    int32_t value2 = 6;

    HashSet<int32_t> h1;

    h1.put(value);
    EXPECT_FALSE(h1.contains(value2));

    h1.put(value2);
    EXPECT_TRUE(h1.contains(value2));
}

TEST(HashSet, findsContainedElement)
{
    HashSet<int> container;
    container.put(5);
    container.put(6);
    container.put(7);

    ASSERT_NE(container.end(), container.find(5));
    ASSERT_NE(container.end(), container.find(6));
    ASSERT_NE(container.end(), container.find(7));
    EXPECT_EQ(5, *container.find(5));
    EXPECT_EQ(6, *container.find(6));
    EXPECT_EQ(7, *container.find(7));

    // const access
    const HashSet<int>& ccontainer = container;
    ASSERT_NE(ccontainer.end(), ccontainer.find(5));
    ASSERT_NE(ccontainer.end(), ccontainer.find(6));
    ASSERT_NE(ccontainer.end(), ccontainer.find(7));
    EXPECT_EQ(5, *ccontainer.find(5));
    EXPECT_EQ(6, *ccontainer.find(6));
    EXPECT_EQ(7, *ccontainer.find(7));
}

TEST(HashSet, doesNotFindElementNotContained)
{
    HashSet<int> container;
    container.put(5);
    container.put(6);
    container.put(7);

    EXPECT_EQ(container.end(), container.find(55));
    EXPECT_EQ(container.end(), container.find(99));

    // const access
    const HashSet<int>& ccontainer = container;
    EXPECT_EQ(ccontainer.end(), ccontainer.find(55));
    EXPECT_EQ(ccontainer.end(), ccontainer.find(99));
}

TEST(HashSetIterator, hasNext)
{
    int32_t value = 10;
    int32_t value2 = 12;

    HashSet<int32_t> h1;
    HashSet<int32_t>::Iterator it = h1.begin();

    EXPECT_EQ(it, h1.end());

    h1.put(value);
    h1.put(value2);

    it = h1.begin();
    EXPECT_NE(it, h1.end());
}

TEST(HashSetIterator, next)
{
    int32_t value = 10;
    int32_t value2 = 12;

    HashSet<int32_t> h1;

    int32_t check_value = 0;
    int32_t check_value2 = 0;

    HashSet<int32_t>::Iterator it = h1.begin();

    EXPECT_TRUE(it == h1.end());
    h1.put(value);
    h1.put(value2);

    it = h1.begin();
    check_value = *it;
    EXPECT_TRUE(check_value == value || check_value == value2);

    it++;
    check_value = *it;
    EXPECT_TRUE(check_value == value || check_value == value2);

    EXPECT_NE(check_value, check_value2);
}

TEST(HashSetIterator, ForEach)
{
    HashSet<int32_t> hashSet;

    hashSet.put(32);
    hashSet.put(43);
    hashSet.put(44);

    HashSet<int32_t> testHashSet;
    for (auto el : hashSet)
    {
        testHashSet.put(el);
    }

    EXPECT_TRUE(testHashSet.contains(32));
    EXPECT_TRUE(testHashSet.contains(43));
    EXPECT_TRUE(testHashSet.contains(44));

}

TEST(HashSet, returnedIteratorPointsToNextElementAfterDeletion)
{
    HashSet<uint32_t> set;

    set.put(1);
    set.put(2);
    set.put(3);

    // point to middle element
    HashSet<uint32_t>::Iterator i1 = set.begin();
    HashSet<uint32_t>::Iterator i2 = set.begin();
    ++i2;
    HashSet<uint32_t>::Iterator i3 = set.begin();
    ++i3;
    ++i3;

    // all iterators point to different elements
    // no assumption which value each iterator points to, because no order is defined
    ASSERT_NE(*i1, *i2);
    ASSERT_NE(*i2, *i3);
    ASSERT_NE(*i1, *i3);

    i2 = set.remove(i2);

    // i2 now points at next element -> i3
    EXPECT_EQ(i2, i3);

    i1 = set.remove(i1);
    EXPECT_EQ(i1, i3);
}

TEST(HashSet, canRemoveElementsDuringCycle)
{
    HashSet<uint32_t> set;

    set.put(1);
    set.put(2);
    set.put(3);

    HashSet<uint32_t>::Iterator iter = set.begin();
    while (iter != set.end())
    {
        iter = set.remove(iter);
    }

    EXPECT_EQ(0u, set.size());
}

TEST(HashSet, HashSetWithComplexType)
{
    TestType::Reset();
    {
        HashSet<TestType> s;
        std::vector<TestType> v;
        for (size_t i = 0; i < 20; ++i)
        {
            TestType ctt(i*10);
            v.push_back(ctt);
            s.put(ctt);
        }

        EXPECT_EQ(v.size(), s.size());
        for (const auto& ctt : v)
        {
            EXPECT_TRUE(s.contains(ctt));
        }
    }
    EXPECT_EQ(TestType::dtor_count, TestType::ctor_count + TestType::copyctor_count);
}

TEST(HashSet, swapMemberFunction)
{
    HashSet<uint32_t> first;
    HashSet<uint32_t> second;

    first.put(1);
    first.put(2);
    second.put(3);

    first.swap(second);
    EXPECT_EQ(2u, second.size());
    EXPECT_EQ(1u, first.size());

    EXPECT_TRUE(second.contains(1));
    EXPECT_TRUE(second.contains(2));
    EXPECT_TRUE(first.contains(3));
}

TEST(HashSet, swapGlobal)
{
    HashSet<uint32_t> first;
    HashSet<uint32_t> second;

    first.put(1);
    first.put(2);
    second.put(3);

    using std::swap;
    swap(first, second);
    EXPECT_EQ(2u, second.size());
    EXPECT_EQ(1u, first.size());
}

TEST(HashSet, TestMoveConstructor)
{
    HashSet<uint32_t> set1;
    set1.put(1);
    set1.put(2);
    set1.put(3);

    HashSet<uint32_t> set2(std::move(set1));

    EXPECT_EQ(3u, set2.size());
    EXPECT_TRUE(set2.contains(1));
    EXPECT_TRUE(set2.contains(2));
    EXPECT_TRUE(set2.contains(3));
}

TEST(HashSet, TestAssign)
{
    HashSet<uint32_t> set1;
    set1.put(1);
    set1.put(2);
    set1.put(3);

    HashSet<uint32_t> set2;
    set2 = set1;

    EXPECT_EQ(3u, set1.size());
    EXPECT_TRUE(set1.contains(1));
    EXPECT_TRUE(set1.contains(2));
    EXPECT_TRUE(set1.contains(3));

    EXPECT_EQ(3u, set2.size());
    EXPECT_TRUE(set2.contains(1));
    EXPECT_TRUE(set2.contains(2));
    EXPECT_TRUE(set2.contains(3));
}


TEST(HashSet, TestMoveAssign)
{
    HashSet<uint32_t> set1;
    set1.put(1);
    set1.put(2);
    set1.put(3);

    HashSet<uint32_t> set2;
    set2 = std::move(set1);

    EXPECT_EQ(3u, set2.size());
    EXPECT_TRUE(set2.contains(1));
    EXPECT_TRUE(set2.contains(2));
    EXPECT_TRUE(set2.contains(3));
}

TEST(HashSet, TestReserve)
{
    HashSet<uint32_t> set(40);
    EXPECT_GE(set.capacity(), 40u);
    EXPECT_LE(set.capacity(), 2 * 40u);

    set.reserve(100);
    EXPECT_GE(set.capacity(), 100u);
    EXPECT_LE(set.capacity(), 2 * 100u);

    const size_t old = set.capacity();
    set.reserve(90);
    EXPECT_EQ(old, set.capacity());
}

}
