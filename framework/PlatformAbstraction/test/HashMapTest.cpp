//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Collections/HashMap.h"
#include "ComplexTestType.h"
#include "gtest/gtest.h"
#include <vector>

namespace {
    struct MyStruct
    {
        uint32_t a;
        int16_t  b;

        bool operator== (const MyStruct& other) const
        {
            return (a == other.a);
        }
    };

    class SomeClass
    {
    public:
        uint32_t i;
        SomeClass()
        {
            i = 0;
        }
        explicit SomeClass(int val)
        {
            i = val;
        }

        SomeClass(const SomeClass&) = default;
        SomeClass& operator=(const SomeClass&) = default;

        bool operator==(const SomeClass& other) const
        {
            // will be called to compare keys!
            return other.i == i;
        }
    };
}

template<>
struct std::hash<MyStruct>
{
    size_t operator()(const MyStruct& c)
    {
        return ramses_internal::HashValue(c.a, c.b);
    }
};

template<>
struct std::hash<SomeClass>
{
    size_t operator()(const SomeClass& c)
    {
        return ramses_internal::HashValue(c.i);
    }
};


namespace ramses_internal
{

using RCKey = ramses_internal::ComplexTestType<struct KeyTag>;
using RCValue = ramses_internal::ComplexTestType<struct ValueTag>;
using TestType = ComplexTestType<>;

class HashMapTest: public ::testing::Test
{
public:
    void SetUp() override
    {
        RCKey::Reset();
        RCValue::Reset();
    }

    void expectRefCnt(int32_t refCnt)
    {
        EXPECT_EQ(refCnt, RCKey::RefCnt());
        EXPECT_EQ(refCnt, RCValue::RefCnt());
    }
};

TEST_F(HashMapTest, TestWithEnum)
{
    enum ESomeEnumeration
    {
        ESomeEnumeration_TEST_VALUE1,
        ESomeEnumeration_TEST_VALUE2,
    };

    HashMap<ESomeEnumeration, int32_t> map;
    map.put(ESomeEnumeration_TEST_VALUE1, 3);
    EXPECT_EQ(3, map[ESomeEnumeration_TEST_VALUE1]);
}

TEST_F(HashMapTest, TestCopyConstructor)
{
    HashMap<int32_t, int32_t> map1;
    map1.put(1, 10);
    map1.put(2, 20);
    map1.put(3, 30);

    HashMap<int32_t, int32_t> newmap = map1; // copy operation

    EXPECT_EQ(map1.size(), newmap.size());
    EXPECT_TRUE(newmap.contains(1));
    EXPECT_TRUE(newmap.contains(2));
    EXPECT_TRUE(newmap.contains(3));

    // Test remove
    map1.remove(1);
    newmap.remove(2);

    EXPECT_TRUE(newmap.contains(1));
    EXPECT_FALSE(map1.contains(1));
    EXPECT_TRUE(map1.contains(2));
    EXPECT_FALSE(newmap.contains(2));

    // Test put
    map1.put(10, 10);
    newmap.put(20, 20);
    EXPECT_TRUE(map1.contains(10));
    EXPECT_FALSE(newmap.contains(10));
    EXPECT_TRUE(newmap.contains(20));
    EXPECT_FALSE(map1.contains(20));

    // Test clear
    map1.clear();
    EXPECT_EQ(static_cast<uint32_t>(0), map1.size());
    EXPECT_TRUE(newmap.contains(20));
}

TEST_F(HashMapTest, TestMoveConstructor)
{
    HashMap<int32_t, int32_t> map1;
    map1.put(1, 10);
    map1.put(2, 20);
    map1.put(3, 30);

    HashMap<int32_t, int32_t> map2(std::move(map1));

    EXPECT_EQ(3u, map2.size());
    EXPECT_TRUE(map2.contains(1));
    EXPECT_TRUE(map2.contains(2));
    EXPECT_TRUE(map2.contains(3));
}

TEST_F(HashMapTest, TestMoveAssign)
{
    HashMap<int32_t, int32_t> map1;
    map1.put(1, 10);
    map1.put(2, 20);
    map1.put(3, 30);

    HashMap<int32_t, int32_t> map2;
    map2 = std::move(map1);

    EXPECT_EQ(3u, map2.size());
    EXPECT_TRUE(map2.contains(1));
    EXPECT_TRUE(map2.contains(2));
    EXPECT_TRUE(map2.contains(3));
}

TEST_F(HashMapTest, TestReserve)
{
    HashMap<int32_t, int32_t> map(40);
    EXPECT_GE(map.capacity(), 40u);
    EXPECT_LE(map.capacity(), 2*40u);

    map.reserve(100);
    EXPECT_GE(map.capacity(), 100u);
    EXPECT_LE(map.capacity(), 2*100u);

    const size_t old = map.capacity();
    map.reserve(90);
    EXPECT_EQ(old, map.capacity());
}

TEST_F(HashMapTest, CanInsertWithoutRehashWhenConstructedWithCapacity)
{
    for (uint32_t param = 1; param < 400; ++param)
    {
        SCOPED_TRACE(param);
        HashMap<int32_t, int32_t> map(param);
        const size_t oldCapacity = map.capacity();
        EXPECT_GE(oldCapacity, param);

        for (unsigned i = 0; i < param; ++i)
        {
            map.put(i, i);
            EXPECT_EQ(i+1, map.size());
            EXPECT_EQ(oldCapacity, map.capacity());
        }
    }
}

TEST_F(HashMapTest, CanInsertWithoutRehashWhenReservedToCapacity)
{
    for (uint32_t param = 1; param < 400; ++param)
    {
        SCOPED_TRACE(param);
        HashMap<int32_t, int32_t> map;

        map.reserve(param);
        const size_t oldCapacity = map.capacity();
        EXPECT_GE(oldCapacity, param);

        for (unsigned i = 0; i < param; ++i)
        {
            map.put(i, i);
            EXPECT_EQ(i+1, map.size());
            EXPECT_EQ(oldCapacity, map.capacity());
        }
    }
}

TEST_F(HashMapTest, AssignmentOperator)
{
    HashMap<uint32_t, uint32_t> table;

    table.put(1, 2);
    table.put(3, 4);
    table.put(5, 6);

    HashMap<uint32_t, uint32_t> table2;

    table2 = table;

    ASSERT_EQ(3u, table2.size());
    EXPECT_EQ(2u, table2[1]);
    EXPECT_EQ(4u, table2[3]);
    EXPECT_EQ(6u, table2[5]);
}

TEST_F(HashMapTest, TestMapRehashWithObjects)
{
    HashMap<SomeClass, uint32_t> table(2);
    SomeClass c1(1);
    SomeClass c2(2);
    SomeClass c3(3);
    SomeClass c4(4);

    table.put(c1, 1);
    table.put(c2, 2);
    table.put(c3, 3);
    table.put(c4, 4); // rehashing

    SomeClass cTest(3); // same i value as c3 -> 3 must get returned
    EXPECT_EQ(3u, table[cTest]);
}

TEST_F(HashMapTest, TestClear)
{
    HashMap<int32_t, int32_t> newmap;

    newmap.put(1, 10);
    newmap.put(2, 20);
    newmap.put(3, 30);

    newmap.clear();
    EXPECT_EQ(static_cast<uint32_t>(0), newmap.size());

    auto it = newmap.find(1);
    EXPECT_EQ(newmap.end(), it);

    it = newmap.find(2);
    EXPECT_EQ(newmap.end(), it);

    it = newmap.find(3);
    EXPECT_EQ(newmap.end(), it);

    newmap.put(1, 10);
    it = newmap.find(1);
    EXPECT_NE(newmap.end(), it);
    EXPECT_EQ(10, it->value);
}

TEST_F(HashMapTest, TestClear2)
{
    HashMap<int32_t, int32_t> table;

    table.put(1, 1);
    table.put(2, 2);

    table.clear();

    table.put(1, 1);
    table.put(2, 2); // may not fail!
}

TEST_F(HashMapTest, TestContains)
{
    HashMap<int32_t, int32_t> newmap;
    EXPECT_FALSE(newmap.contains(1));
    EXPECT_FALSE(newmap.contains(2));
    EXPECT_FALSE(newmap.contains(3));

    newmap.put(1, 10);
    EXPECT_TRUE(newmap.contains(1));
    EXPECT_FALSE(newmap.contains(2));
    EXPECT_FALSE(newmap.contains(3));

    newmap.put(2, 20);
    EXPECT_TRUE(newmap.contains(1));
    EXPECT_TRUE(newmap.contains(2));
    EXPECT_FALSE(newmap.contains(3));

    newmap.put(3, 30);
    EXPECT_TRUE(newmap.contains(1));
    EXPECT_TRUE(newmap.contains(2));
    EXPECT_TRUE(newmap.contains(3));

    newmap.clear();
    EXPECT_FALSE(newmap.contains(1));
    EXPECT_FALSE(newmap.contains(2));
    EXPECT_FALSE(newmap.contains(3));
}

TEST_F(HashMapTest, IteratorRemove)
{
    HashMap<int32_t, int32_t> newmap;

    newmap.put(0, 12);
    newmap.put(3, 10);
    newmap.put(2, 11);

    for (HashMap<int32_t, int32_t>::Iterator iter = newmap.begin(); iter != newmap.end();)
    {
        iter = newmap.remove(iter);
    }

    EXPECT_EQ(0u, newmap.size());
    EXPECT_EQ(newmap.end(), newmap.find(0));
    EXPECT_EQ(newmap.end(), newmap.find(2));
    EXPECT_EQ(newmap.end(), newmap.find(3));

    newmap.put(0, 12);
    newmap.put(3, 10);
    newmap.put(2, 11);

    HashMap<int32_t, int32_t>::Iterator iter2 = newmap.begin();
    int32_t expectedValue = iter2->value;
    int32_t oldValue = 0;
    iter2 = newmap.remove(iter2, &oldValue);
    EXPECT_EQ(expectedValue, oldValue);

    while (iter2 != newmap.end())
    {
        iter2 = newmap.remove(iter2);
    }

    EXPECT_EQ(0u, newmap.size());
}

TEST_F(HashMapTest, find)
{
    HashMap<int32_t, int32_t> newmap;
    EXPECT_EQ(newmap.end(), newmap.find(4));
    newmap.put(1, 5);
    EXPECT_EQ(5, (*newmap.find(1)).value);
}

using SomeHashMap = HashMap<int32_t, SomeClass>;
TEST_F(HashMapTest, operator_subscript_read)
{
    HashMap<int32_t, int32_t> newmap;
    newmap.put(1, 5);
    int32_t val = newmap[1];
    EXPECT_EQ(5, val);

    // non trivial object
    SomeHashMap newmap2;
    newmap2.put(1, SomeClass(5));
    SomeClass val2 = newmap2[1];
    EXPECT_EQ(5u, val2.i);

    EXPECT_FALSE(newmap2.contains(5));

    // not existing non trivial object must return default constructed reference
    SomeClass& val3 = newmap2[5];
    EXPECT_EQ(0u, val3.i);

    // key must also be contained in the hash map now
    EXPECT_TRUE(newmap2.contains(5));

    // change val3 and test again
    val3.i = 42u;
    SomeClass val4 = newmap2[5];
    EXPECT_EQ(42u, val4.i);

    newmap2[7].i = 55u;
    EXPECT_EQ(55u, newmap2[7].i);
}

TEST_F(HashMapTest, operator_subscript_assign)
{
    /* native types */
    HashMap<int32_t, int32_t> newmap;
    int32_t val_in = 5;
    newmap[1] = val_in;
    int32_t val_out = newmap[1];
    EXPECT_EQ(val_in, val_out);

    val_in = 10;
    newmap[1] = val_in;
    val_out = newmap[1];
    EXPECT_EQ(val_in, val_out);

    /* Objects */
    MyStruct structKey;
    std::memset(&structKey, 0, sizeof(structKey)); // make valgrind happy
    structKey.a = 1;
    MyStruct structValue;
    structValue.a = 13;
    MyStruct structReturn;

    HashMap<MyStruct, MyStruct> tab;
    tab[structKey] = structValue;
    structReturn = tab[structKey];

    EXPECT_EQ(structValue, structReturn);
}

TEST_F(HashMapTest, TestAddGet2)
{
    HashMap<int32_t, int32_t> map;
    map.put(3, 3);
    EXPECT_FALSE(map.contains(19));
}
TEST_F(HashMapTest, TestAddGet)
{
    HashMap<int32_t, int32_t> newmap;
    const HashMap<int32_t, int32_t>& newConstMap(newmap);

    EXPECT_EQ(0u, newmap.size());

    newmap.put(1, 10);
    newmap.put(2, 20);
    newmap.put(3, 30);

    EXPECT_EQ(3u, newmap.size());

    auto cit = newConstMap.find(1);
    EXPECT_NE(newConstMap.end(), cit);
    EXPECT_EQ(10, cit->value);

    auto it = newmap.find(1);
    EXPECT_NE(newmap.end(), it);
    EXPECT_EQ(10, it->value);

    it = newmap.find(2);
    EXPECT_NE(newmap.end(), it);
    EXPECT_EQ(20, it->value);

    it = newmap.find(3);
    EXPECT_NE(newmap.end(), it);
    EXPECT_EQ(30, it->value);

    newmap.remove(1);

    it = newmap.find(1);
    EXPECT_EQ(newmap.end(), it);

    it = newmap.find(2);
    EXPECT_NE(newmap.end(), it);
    EXPECT_EQ(20, it->value);

    it = newmap.find(3);
    EXPECT_NE(newmap.end(), it);
    EXPECT_EQ(30, it->value);

    EXPECT_EQ(2u, newmap.size());

    newmap.remove(2);

    it = newmap.find(1);
    EXPECT_EQ(newmap.end(), it);

    it = newmap.find(2);
    EXPECT_EQ(newmap.end(), it);

    it = newmap.find(3);
    EXPECT_NE(newmap.end(), it);
    EXPECT_EQ(30, it->value);

    EXPECT_EQ(1u, newmap.size());

    newmap.remove(3);


    it = newmap.find(1);
    EXPECT_EQ(newmap.end(), it);

    it = newmap.find(2);
    EXPECT_EQ(newmap.end(), it);

    it = newmap.find(3);
    EXPECT_EQ(newmap.end(), it);

    EXPECT_EQ(0u, newmap.size());
}

TEST_F(HashMapTest, TestRemove1)
{
    HashMap<int32_t, int32_t> newmap;

    newmap.put(1, 10);
    newmap.remove(1);
    newmap.put(2, 10);

    HashMap<int32_t, int32_t>::Iterator it = newmap.begin();
    while (it != newmap.end())
    {
        it++;
    }
}

TEST_F(HashMapTest, TestRemove)
{
    HashMap<int32_t, int32_t> newmap;

    newmap.put(1, 10);
    newmap.put(2, 20);
    newmap.put(3, 30);
    EXPECT_EQ(3u, newmap.size());

    EXPECT_TRUE(newmap.remove(2));
    EXPECT_EQ(2u, newmap.size());
    EXPECT_EQ(newmap.end(), newmap.find(2));

    int32_t value;
    EXPECT_TRUE(newmap.remove(3, &value));
    EXPECT_EQ(1u, newmap.size());
    EXPECT_EQ(30, value);

    EXPECT_EQ(newmap.end(), newmap.find(2));
}

TEST_F(HashMapTest, TestIterator1)
{
    HashMap<int32_t, int32_t> newmap(5);

    newmap.put(1, 10);
    newmap.put(2, 20);
    newmap.put(3, 30);

    HashMap<int32_t, int32_t>::Iterator it = newmap.begin();

    // TODO check values
    EXPECT_NE(it, newmap.end());
    const HashMap<int32_t, int32_t>::Pair& entry1 = *(it++);
    EXPECT_EQ(entry1.key * 10, entry1.value);

    EXPECT_NE(it, newmap.end());
    const HashMap<int32_t, int32_t>::Pair& entry2 = *(it++);
    EXPECT_EQ(entry2.key * 10, entry2.value);

    EXPECT_NE(it, newmap.end());
    const HashMap<int32_t, int32_t>::Pair& entry3 = *(it++);
    EXPECT_EQ(entry3.key * 10, entry3.value);

    EXPECT_EQ(it, newmap.end());
}

TEST_F(HashMapTest, TestIterator2)
{
    HashMap<int32_t, int32_t> newmap(5);

    newmap.put(1, 10);
    newmap.put(2, 20);
    newmap.put(3, 30);

    HashMap<int32_t, int32_t>::Iterator it = newmap.begin();

    EXPECT_NE(it, newmap.end());

    EXPECT_EQ((*it).key * 10, (*it).value);
    it++;

    EXPECT_TRUE(it != newmap.end());
    const HashMap<int32_t, int32_t>::Pair& entry2 = *it;
    EXPECT_EQ(entry2.key * 10, entry2.value);
    it++;

    EXPECT_NE(it, newmap.end());
    EXPECT_EQ((*it).key * 10, (*it).value);
    it++;

    EXPECT_TRUE(it == newmap.end());
}


TEST_F(HashMapTest, TestConstIterator1)
{
    HashMap<int32_t, int32_t> newmap(5);
    const HashMap<int32_t, int32_t>& newmapConstRef = newmap;

    newmap.put(1, 10);
    newmap.put(2, 20);
    newmap.put(3, 30);

    HashMap<int32_t, int32_t>::ConstIterator it = newmapConstRef.begin();

    // TODO check values
    EXPECT_NE(it, newmapConstRef.end());
    const HashMap<int32_t, int32_t>::Pair& entry1 = *(it++);
    EXPECT_EQ(entry1.key * 10, entry1.value);

    EXPECT_NE(it, newmapConstRef.end());
    const HashMap<int32_t, int32_t>::Pair& entry2 = *(it++);
    EXPECT_EQ(entry2.key * 10, entry2.value);

    EXPECT_NE(it, newmapConstRef.end());
    const HashMap<int32_t, int32_t>::Pair& entry3 = *(it++);
    EXPECT_EQ(entry3.key * 10, entry3.value);

    EXPECT_EQ(it, newmapConstRef.end());
}

TEST_F(HashMapTest, TestConstIterator2)
{
    HashMap<int32_t, int32_t> newmap(5);
    const HashMap<int32_t, int32_t>& newmapConstRef = newmap;

    newmap.put(1, 10);
    newmap.put(2, 20);
    newmap.put(3, 30);

    HashMap<int32_t, int32_t>::ConstIterator it = newmapConstRef.begin();

    EXPECT_NE(it, newmapConstRef.end());

    EXPECT_EQ((*it).key * 10, (*it).value);
    it++;

    EXPECT_TRUE(it != newmapConstRef.end());
    const HashMap<int32_t, int32_t>::Pair& entry2 = *it;
    EXPECT_EQ(entry2.key * 10, entry2.value);
    it++;

    EXPECT_NE(it, newmapConstRef.end());
    EXPECT_EQ((*it).key * 10, (*it).value);
    it++;

    EXPECT_TRUE(it == newmapConstRef.end());
}


TEST_F(HashMapTest, IteratorPrimitiveKeyComplexValue)
{
    MyStruct testStruct;
    testStruct.a =  1;
    testStruct.b = -1;

    HashMap<uint32_t, MyStruct> hashTable;

    hashTable.put(0, testStruct);

    HashMap<uint32_t, MyStruct>::Iterator hashTableIter = hashTable.begin();

    EXPECT_EQ(0u, hashTableIter->key);
    EXPECT_EQ(1u, hashTableIter->value.a);
    EXPECT_EQ(-1, hashTableIter->value.b);

    hashTableIter->value.a = 5;
    hashTableIter->value.b = -5;

    EXPECT_EQ(0u, hashTableIter->key);
    EXPECT_EQ(5u, hashTableIter->value.a);
    EXPECT_EQ(-5, hashTableIter->value.b);

    hashTableIter = hashTable.begin();

    EXPECT_EQ(0u, hashTableIter->key);
    EXPECT_EQ(5u, hashTableIter->value.a);
    EXPECT_EQ(-5, hashTableIter->value.b);
}

TEST_F(HashMapTest, IteratorPrimitiveKeyComplexPtrValue)
{
    MyStruct testStruct;
    testStruct.a =  1;
    testStruct.b = -1;

    HashMap<uint32_t, MyStruct*> hashTable;

    hashTable.put(0, &testStruct);

    HashMap<uint32_t, MyStruct*>::Iterator hashTableIter = hashTable.begin();

    EXPECT_EQ(&testStruct, hashTableIter->value);
    EXPECT_EQ(0u, hashTableIter->key);
    EXPECT_EQ(1u, hashTableIter->value->a);
    EXPECT_EQ(-1, hashTableIter->value->b);

    hashTableIter->value->a = 5;
    hashTableIter->value->b = -5;

    EXPECT_EQ(0u, hashTableIter->key);
    EXPECT_EQ(5u, hashTableIter->value->a);
    EXPECT_EQ(-5, hashTableIter->value->b);

    hashTableIter = hashTable.begin();

    EXPECT_EQ(0u, hashTableIter->key);
    EXPECT_EQ(5u, hashTableIter->value->a);
    EXPECT_EQ(-5, hashTableIter->value->b);
}

TEST_F(HashMapTest, IteratorMethodCall)
{
    MyStruct testStruct;
    testStruct.a =  1;
    testStruct.b = -1;

    HashMap<uint32_t, MyStruct> hashTable;

    hashTable.put(0, testStruct);

    HashMap<uint32_t, MyStruct>::Iterator hashTableIter = hashTable.begin();

    EXPECT_EQ(1u, hashTableIter->value.a);
    EXPECT_EQ(1u, (*hashTableIter).value.a);
}

TEST_F(HashMapTest, TestRehasing)
{
    HashMap<int32_t, int32_t> newmap(2); // only 3 entries

    newmap.put(1, 10);
    newmap.put(2, 20);
    newmap.put(3, 30);
    newmap.put(4, 40);
    EXPECT_EQ(4u, newmap.size());

    EXPECT_EQ(10, newmap[1]);
    EXPECT_EQ(20, newmap[2]);
    EXPECT_EQ(30, newmap[3]);
    EXPECT_EQ(40, newmap[4]);
}

TEST_F(HashMapTest, TestWildRemoving)
{
    HashMap<int32_t, int32_t> newmap;

    for (int i = 0; i < 1000; i++)
    {
        newmap.put(i, i % 10);
    }

    // this will create a 'wild' chain of free elements
    // with free gaps of entries of the memory block of the map
    for (int i = 999; i >= 0; i -= 2)
    {
        newmap.remove(i);
    }

    // now fill up the gaps
    for (int i = 1; i < 1000; i += 2)
    {
        newmap.put(i, i % 10);
    }

    for (int i = 0; i < 1000; i += 2)
    {
        newmap.put(i, i % 10);
    }

    newmap.put(2000, 2000); // TODO how to check that everything worked?
    EXPECT_EQ(static_cast<uint32_t>(1001), newmap.size());
}

TEST_F(HashMapTest, ForEach)
{
    HashMap<int32_t, int32_t> hashMap;

    hashMap.put(32,33);
    hashMap.put(43,44);
    hashMap.put(44,45);

    HashMap<int32_t, int32_t> someHashMap;

    for (auto p : hashMap)
    {
        someHashMap.put(p.key, p.value);
    }

    EXPECT_EQ(33, someHashMap.find(32)->value);
    EXPECT_EQ(44, someHashMap.find(43)->value);
    EXPECT_EQ(45, someHashMap.find(44)->value);

}

TEST_F(HashMapTest, HashMapWithComplexKey)
{
    TestType::Reset();
    {
        HashMap<TestType, size_t> ht;
        std::vector<TestType> vec;
        for (size_t i = 0; i < 20; ++i)
        {
            TestType ctt(i*20);
            ht.put(ctt, i);
            vec.push_back(ctt);
        }

        for (size_t i = 0; i < vec.size(); ++i)
        {
            const TestType& ctt = vec[i];
            auto it = ht.find(ctt);
            EXPECT_NE(it, ht.end());
            EXPECT_EQ(it->key, ctt);
            EXPECT_EQ(it->value, i);
        }
    }
    EXPECT_EQ(TestType::dtor_count, TestType::ctor_count + TestType::copyctor_count);
}

TEST_F(HashMapTest, swapMemberFunction)
{
    HashMap<int32_t, int32_t> first;
    HashMap<int32_t, int32_t> second;

    first.put(1, 11);
    first.put(2, 12);
    second.put(100, 101);

    first.swap(second);
    EXPECT_EQ(2u, second.size());
    EXPECT_EQ(1u, first.size());

    auto it = second.find(1);
    EXPECT_NE(second.end(), it);
    EXPECT_EQ(11, it->value);

    it = second.find(2);
    EXPECT_NE(second.end(), it);
    EXPECT_EQ(12, it->value);

    it = first.find(100);
    EXPECT_NE(first.end(), it);
    EXPECT_EQ(101, it->value);
}

TEST_F(HashMapTest, swapGlobal)
{
    HashMap<int32_t, int32_t> first;
    HashMap<int32_t, int32_t> second;

    first.put(1, 11);
    first.put(2, 12);
    second.put(100, 101);

    using std::swap;
    swap(first, second);
    EXPECT_EQ(2u, second.size());
    EXPECT_EQ(1u, first.size());
}

TEST_F(HashMapTest, basicRefCountLifecycle)
{
    expectRefCnt(0);
    {
        HashMap<RCKey, RCValue> ht;
        expectRefCnt(0);

        // add stuff
        ht.put(RCKey(1), RCValue(2));
        expectRefCnt(1);
        ht.put(RCKey(2), RCValue(3));
        expectRefCnt(2);

        // overwrite
        ht.put(RCKey(1), RCValue(4));
        expectRefCnt(2);

        // get
        auto it = ht.find(RCKey(1));
        EXPECT_EQ(RCValue(4u), it->value);

        // remove
        {
            RCValue v;
            ht.remove(RCKey(2), &v);
            EXPECT_EQ(RCValue(3u), v);
        }
        expectRefCnt(1);
    }
    // destructor destructs all
    expectRefCnt(0);
}

TEST_F(HashMapTest, clearDestructsAllElements)
{
    HashMap<RCKey, RCValue> ht;
    ht.put(RCKey(1), RCValue(2));
    ht.put(RCKey(2), RCValue(3));
    ht.put(RCKey(3), RCValue(4));
    expectRefCnt(3);
    ht.clear();
    expectRefCnt(0);
}

TEST_F(HashMapTest, copyCtorCopyConstructsObjects)
{
    HashMap<RCKey, RCValue> ht;
    ht.put(RCKey(1), RCValue(2));
    ht.put(RCKey(2), RCValue(3));
    ht.put(RCKey(3), RCValue(4));
    expectRefCnt(3);

    HashMap<RCKey, RCValue> ht2(ht);
    expectRefCnt(6);

    ht.clear();
    expectRefCnt(3);
}

TEST_F(HashMapTest, assignmentCopiesObjects)
{
    HashMap<RCKey, RCValue> ht2;
    {
        HashMap<RCKey, RCValue> ht;
        ht.put(RCKey(1), RCValue(2));
        ht.put(RCKey(2), RCValue(3));
        ht.put(RCKey(3), RCValue(4));
        expectRefCnt(3);

        ht2 = ht;
        expectRefCnt(3 + 3);
    }
    expectRefCnt(3);
}

TEST_F(HashMapTest, arrayAccessOperatorConstructsNewElementIfNotExisting)
{
    HashMap<RCKey, RCValue> ht;
    ht.put(RCKey(1), RCValue(2));
    expectRefCnt(1);
    ht[RCKey(2)] = RCValue(3);
    expectRefCnt(2);
}

TEST_F(HashMapTest, arrayAccessOperatorReusesElementIfExisting)
{
    HashMap<RCKey, RCValue> ht;
    ht.put(RCKey(1), RCValue(2));
    expectRefCnt(1);
    ht[RCKey(1)] = RCValue(3);
    expectRefCnt(1);
}

TEST_F(HashMapTest, swapKeepsNumberOfObjectsUnchanged)
{
    HashMap<RCKey, RCValue> first;
    HashMap<RCKey, RCValue> second;
    first.put(RCKey(1), RCValue(2));
    first.put(RCKey(2), RCValue(3));
    second.put(RCKey(3), RCValue(4));
    expectRefCnt(3);
    first.swap(second);
    expectRefCnt(3);
}

TEST_F(HashMapTest, canConstructWithZeroCapacity)
{
    HashMap<RCKey, RCValue> ht(0);
    EXPECT_EQ(0u, ht.size());
    EXPECT_GE(ht.capacity(), ht.DefaultHashMapCapacity);
}

TEST_F(HashMapTest, constructWithCapacityHasAtLeastDefaultCapacity)
{
    using HM = HashMap<RCKey, RCValue>;
    HM ht(HM::DefaultHashMapCapacity - 1);
    EXPECT_EQ(0u, ht.size());
    EXPECT_GE(ht.capacity(), HM::DefaultHashMapCapacity);
}

} // namespace ramses_internal
