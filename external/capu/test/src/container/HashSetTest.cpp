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

#include <gtest/gtest.h>
#include "ramses-capu/container/HashSet.h"
#include "ramses-capu/Error.h"
#include "ramses-capu/Config.h"
#include "util/ComplexTestType.h"
#include <vector>

TEST(HashSet, Constructor_Default)
{
    ramses_capu::HashSet<int32_t>* list = new ramses_capu::HashSet<int32_t > ();
    delete list;
}

TEST(HashSet, copyConstructor)
{
    ramses_capu::HashSet<int32_t> list;
    list.put(1);
    list.put(2);
    list.put(3);

    ramses_capu::HashSet<int32_t> list2 = list; // copy

    list.clear();

    EXPECT_TRUE(list2.hasElement(1));
    EXPECT_TRUE(list2.hasElement(2));
    EXPECT_TRUE(list2.hasElement(3));
}

TEST(HashSet, put)
{
    int32_t value2 = 10;
    int32_t value = 5;

    ramses_capu::HashSet<int32_t>* h1 = new ramses_capu::HashSet<int32_t > ();

    // add new key
    EXPECT_EQ(ramses_capu::CAPU_OK,  h1->put(value));

    // add new key
    EXPECT_EQ(ramses_capu::CAPU_OK, h1->put(value2));

    // add existing key
    EXPECT_EQ(ramses_capu::CAPU_ERROR, h1->put(value2));

    delete h1;
}

TEST(HashSet, count)
{
    int32_t value2 = 10;
    int32_t value = 5;
    ramses_capu::HashSet<int32_t>* h1 = new ramses_capu::HashSet<int32_t > ();

    //check count
    EXPECT_EQ(0u, h1->count());

    // add new value
    EXPECT_EQ(ramses_capu::CAPU_OK, h1->put(value));

    // add new value
    EXPECT_EQ(ramses_capu::CAPU_OK, h1->put(value2));

    EXPECT_EQ(2u, h1->count());

    EXPECT_EQ(ramses_capu::CAPU_OK, h1->remove(value2));

    EXPECT_EQ(1u, h1->count());

    delete h1;
}

TEST(HashSet, clear)
{
    int32_t value = 5;
    int32_t value2 = 6;

    ramses_capu::HashSet<int32_t>* h1 = new ramses_capu::HashSet<int32_t > ();
    // add new keys
    EXPECT_EQ(ramses_capu::CAPU_OK, h1->put(value));

    //add new keys
    EXPECT_EQ(ramses_capu::CAPU_OK, h1->put(value2));

    // check count
    EXPECT_EQ(2u, h1->count());

    //remove all
    EXPECT_EQ(ramses_capu::CAPU_OK, h1->clear());

    //check count
    EXPECT_EQ(0u, h1->count());

    delete h1;
}

TEST(HashSet, remove)
{
    int32_t value = 5;
    int32_t value2 = 6;

    ramses_capu::HashSet<int32_t>* h1 = new ramses_capu::HashSet<int32_t > ();

    // add new keys
    EXPECT_EQ(ramses_capu::CAPU_OK, h1->put(value));

    //delete a non existing value
    EXPECT_EQ(ramses_capu::CAPU_ERANGE, h1->remove(value2));

    //add new value
    EXPECT_EQ(ramses_capu::CAPU_OK, h1->put(value2));

    // check count
    EXPECT_EQ(2u, h1->count());

    //delete existing value
    EXPECT_EQ(ramses_capu::CAPU_OK, h1->remove(value2));

    //check count
    EXPECT_EQ(1u, h1->count());

    delete h1;
}

TEST(HashSet, hasElement)
{
    int32_t value = 5;
    int32_t value2 = 6;

    ramses_capu::HashSet<int32_t>* h1 = new ramses_capu::HashSet<int32_t > ();

    // add new keys
    EXPECT_EQ(ramses_capu::CAPU_OK, h1->put(value));

    EXPECT_FALSE(h1->hasElement(value2));

    //add new value
    EXPECT_EQ(ramses_capu::CAPU_OK, h1->put(value2));

    //delete existing value
    EXPECT_TRUE(h1->hasElement(value2));

    delete h1;
}

TEST(HashSetIterator, hasNext)
{
    int32_t value = 10;
    int32_t value2 = 12;

    ramses_capu::HashSet<int32_t>* h1 = new ramses_capu::HashSet<int32_t > ();

    //create iterator
    ramses_capu::HashSet<int32_t>::Iterator it = h1->begin();

    //check hasNext
    EXPECT_EQ(it, h1->end());

    // add new values
    EXPECT_EQ(ramses_capu::CAPU_OK, h1->put(value));

    //add new value
    EXPECT_EQ(ramses_capu::CAPU_OK, h1->put(value2));

    it = h1->begin();
    EXPECT_NE(it, h1->end());

    delete h1;
}

TEST(HashSetIterator, next)
{
    int32_t value = 10;
    int32_t value2 = 12;

    ramses_capu::HashSet<int32_t>* h1 = new ramses_capu::HashSet<int32_t > ();

    int32_t check_value = 0;
    int32_t check_value2 = 0;

    //create iterator
    ramses_capu::HashSet<int32_t>::Iterator it = h1->begin();

    //check hasNext
    EXPECT_TRUE(it == h1->end());

    // add new keys
    EXPECT_EQ(ramses_capu::CAPU_OK, h1->put(value));

    //add new value
    EXPECT_EQ(ramses_capu::CAPU_OK, h1->put(value2));
    it = h1->begin();

    check_value = *it;
    EXPECT_TRUE(check_value == value || check_value == value2);

    it++;
    check_value = *it;
    EXPECT_TRUE(check_value == value || check_value == value2);

    EXPECT_NE(check_value, check_value2);
    delete h1;
}

TEST(HashSetIterator, ForEach)
{
    ramses_capu::HashSet<int32_t> hashSet;

    hashSet.put(32);
    hashSet.put(43);
    hashSet.put(44);

    ramses_capu::HashSet<int32_t> testHashSet;
    for (auto el : hashSet)
    {
        testHashSet.put(el);
    }

    EXPECT_TRUE(testHashSet.hasElement(32));
    EXPECT_TRUE(testHashSet.hasElement(43));
    EXPECT_TRUE(testHashSet.hasElement(44));

}


class HashSetPerformance : public ::testing::Test
{
public:
    HashSetPerformance()
        : count(500000)
    {}

    static ramses_capu::HashSet<uint32_t> set;
    uint32_t count;
};

ramses_capu::HashSet<uint32_t> HashSetPerformance::set;

TEST_F(HashSetPerformance, performancePut)
{
    for (uint32_t i = 0; i < count; i++)
    {
        set.put(i);
    }
}

TEST_F(HashSetPerformance, performanceIterate)
{
    ramses_capu::HashSet<uint32_t>::Iterator iter = set.begin();
    while (iter != set.end())
    {
        iter++;
    }
}

TEST_F(HashSetPerformance, performanceConstIterate)
{
    const ramses_capu::HashSet<uint32_t>& setConstRef = set;
    ramses_capu::HashSet<uint32_t>::ConstIterator iter = setConstRef.begin();
    while (iter != setConstRef.end())
    {
        iter++;
    }
}

TEST_F(HashSetPerformance, performanceRemove)
{
    for (uint32_t i = 0; i < count; i++)
    {
        set.remove(i);
    }
}

TEST(HashSet, iteratorPointsToNextElementAfterDeletion)
{
    ramses_capu::HashSet<uint32_t> set;

    set.put(1);
    set.put(2);
    set.put(3);

    // point to middle element
    ramses_capu::HashSet<uint32_t>::Iterator i1 = set.begin();
    ramses_capu::HashSet<uint32_t>::Iterator i2 = set.begin(); ++i2;
    ramses_capu::HashSet<uint32_t>::Iterator i3 = set.begin(); ++i3; ++i3;

    // all iterators point to different elements
    // no assumption which value each iterator points to, because no order is defined
    ASSERT_NE(*i1, *i2);
    ASSERT_NE(*i2, *i3);
    ASSERT_NE(*i1, *i3);

    set.removeAt(i2);

    // i2 now points at next element -> i3
    EXPECT_EQ(i2, i3);

    set.removeAt(i1);
    EXPECT_EQ(i1, i3);
}

TEST(HashSet, canRemoveElementsDuringCycle)
{
    ramses_capu::HashSet<uint32_t> set;

    set.put(1);
    set.put(2);
    set.put(3);

    ramses_capu::HashSet<uint32_t>::Iterator iter = set.begin();
    while (iter != set.end())
    {
        set.removeAt(iter);
    }

    EXPECT_EQ(0u, set.count());
}

TEST(HashSet, HashSetWithComplexType)
{
    ComplexTestType::Reset();
    {
        ramses_capu::HashSet<ComplexTestType> s;
        std::vector<ComplexTestType> v;
        for (ramses_capu::uint_t i = 0; i < 20; ++i)
        {
            ComplexTestType ctt(i*10);
            v.push_back(ctt);
            s.put(ctt);
        }

        EXPECT_EQ(v.size(), s.count());
        for (const auto& ctt : v)
        {
            EXPECT_TRUE(s.hasElement(ctt));
        }
    }
    EXPECT_EQ(ComplexTestType::dtor_count, ComplexTestType::ctor_count + ComplexTestType::copyctor_count);
}

TEST(HashSet, swapMemberFunction)
{
    ramses_capu::HashSet<uint32_t> first;
    ramses_capu::HashSet<uint32_t> second;

    first.put(1);
    first.put(2);
    second.put(3);

    first.swap(second);
    EXPECT_EQ(2u, second.count());
    EXPECT_EQ(1u, first.count());

    EXPECT_TRUE(second.hasElement(1));
    EXPECT_TRUE(second.hasElement(2));
    EXPECT_TRUE(first.hasElement(3));
}

TEST(HashSet, swapGlobal)
{
    ramses_capu::HashSet<uint32_t> first;
    ramses_capu::HashSet<uint32_t> second;

    first.put(1);
    first.put(2);
    second.put(3);

    using std::swap;
    swap(first, second);
    EXPECT_EQ(2u, second.count());
    EXPECT_EQ(1u, first.count());
}

TEST(HashSet, TestMoveConstructor)
{
    ramses_capu::HashSet<uint32_t> set1;
    set1.put(1);
    set1.put(2);
    set1.put(3);

    ramses_capu::HashSet<uint32_t> set2(std::move(set1));

    EXPECT_EQ(0u, set1.count());
    EXPECT_FALSE(set1.hasElement(1));
    EXPECT_FALSE(set1.hasElement(2));
    EXPECT_FALSE(set1.hasElement(3));

    EXPECT_EQ(3u, set2.count());
    EXPECT_TRUE(set2.hasElement(1));
    EXPECT_TRUE(set2.hasElement(2));
    EXPECT_TRUE(set2.hasElement(3));
}

TEST(HashSet, TestAssign)
{
    ramses_capu::HashSet<uint32_t> set1;
    set1.put(1);
    set1.put(2);
    set1.put(3);

    ramses_capu::HashSet<uint32_t> set2;
    set2 = set1;

    EXPECT_EQ(3u, set1.count());
    EXPECT_TRUE(set1.hasElement(1));
    EXPECT_TRUE(set1.hasElement(2));
    EXPECT_TRUE(set1.hasElement(3));

    EXPECT_EQ(3u, set2.count());
    EXPECT_TRUE(set2.hasElement(1));
    EXPECT_TRUE(set2.hasElement(2));
    EXPECT_TRUE(set2.hasElement(3));
}


TEST(HashSet, TestMoveAssign)
{
    ramses_capu::HashSet<uint32_t> set1;
    set1.put(1);
    set1.put(2);
    set1.put(3);

    ramses_capu::HashSet<uint32_t> set2;
    set2 = std::move(set1);

    EXPECT_EQ(0u, set1.count());
    EXPECT_FALSE(set1.hasElement(1));
    EXPECT_FALSE(set1.hasElement(2));
    EXPECT_FALSE(set1.hasElement(3));

    EXPECT_EQ(3u, set2.count());
    EXPECT_TRUE(set2.hasElement(1));
    EXPECT_TRUE(set2.hasElement(2));
    EXPECT_TRUE(set2.hasElement(3));
}

TEST(HashSet, TestReserve)
{
    ramses_capu::HashSet<uint32_t> set(10);
    EXPECT_GE(set.capacity(), 10u);
    EXPECT_LE(set.capacity(), 2*10u);

    set.reserve(100);
    EXPECT_GE(set.capacity(), 100u);
    EXPECT_LE(set.capacity(), 2*100u);

    const ramses_capu::uint_t old = set.capacity();
    set.reserve(90);
    EXPECT_EQ(old, set.capacity());
}
