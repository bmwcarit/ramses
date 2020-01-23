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

#include "ramses-capu/container/HashTable.h"
#include "util/ComplexTestType.h"
#include "gtest/gtest.h"
#include <vector>

namespace {
    template <typename Tag>
    class RefCntType
    {
    public:
        RefCntType()
            : value(-1)
        {
            ++refCnt;
        }

        RefCntType(int32_t value_)
            : value(value_)
        {
            ++refCnt;
        }

        RefCntType(const RefCntType& o)
            : value(o.value)
        {
            ++refCnt;
        }

        ~RefCntType()
        {
            --refCnt;
        }

        RefCntType& operator=(const RefCntType&) = default;  // does not change alive objects

        friend bool operator==(const RefCntType& a, const RefCntType& b)
        {
            return a.value == b.value;
        }

        int32_t value;
        static int32_t refCnt;
    };

    template <typename Tag>
    int32_t RefCntType<Tag>::refCnt = 0;

    struct KeyTag {};
    typedef RefCntType<KeyTag> RCKey;
    struct ValueTag {};
    typedef RefCntType<ValueTag> RCValue;

    typedef ramses_capu::HashTable<RCKey, RCValue> RCHashTable;

    struct MyStruct
    {
        uint32_t a;
        int16_t  b;

        bool operator== (const MyStruct& other) const
        {
            return (a == other.a);
        }
    };

    typedef ramses_capu::HashTable<int32_t, int32_t> Int32HashMap;
}

class HashTableTest: public ::testing::Test
{
public:
    void SetUp() override
    {
        RCKey::refCnt = 0;
        RCValue::refCnt = 0;
    }

    void expectRefCnt(int32_t refCnt)
    {
        EXPECT_EQ(refCnt, RCKey::refCnt);
        EXPECT_EQ(refCnt, RCValue::refCnt);
    }
};

namespace ramses_capu
{
    template<>
    struct Hash<MyStruct>
    {
        uint_t operator()(const MyStruct& c)
        {
            return HashValue(c.a, c.b);
        }
    };

    template<typename T>
    struct Hash<RefCntType<T>>
    {
        uint_t operator()(const RefCntType<T>& c)
        {
            return HashValue(c.value);
        }
    };
}

TEST_F(HashTableTest, TestWithEnum)
{
    enum SomeEnumeration
    {
        TEST_VALUE1,
        TEST_VALUE2,
    };

    ramses_capu::HashTable<SomeEnumeration, int32_t> map;
    map.put(TEST_VALUE1, 3);
    EXPECT_EQ(3, map[TEST_VALUE1]);
}

TEST_F(HashTableTest, TestCopyConstructor)
{
    Int32HashMap map1;
    map1.put(1, 10);
    map1.put(2, 20);
    map1.put(3, 30);

    Int32HashMap newmap = map1; // copy operation

    EXPECT_EQ(map1.count(), newmap.count());
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
    EXPECT_EQ(static_cast<uint32_t>(0), map1.count());
    EXPECT_TRUE(newmap.contains(20));
}

TEST_F(HashTableTest, TestMoveConstructor)
{
    Int32HashMap map1;
    map1.put(1, 10);
    map1.put(2, 20);
    map1.put(3, 30);

    Int32HashMap map2(std::move(map1));

    EXPECT_EQ(3u, map2.count());
    EXPECT_TRUE(map2.contains(1));
    EXPECT_TRUE(map2.contains(2));
    EXPECT_TRUE(map2.contains(3));
}

TEST_F(HashTableTest, TestMoveAssign)
{
    Int32HashMap map1;
    map1.put(1, 10);
    map1.put(2, 20);
    map1.put(3, 30);

    Int32HashMap map2;
    map2 = std::move(map1);

    EXPECT_EQ(3u, map2.count());
    EXPECT_TRUE(map2.contains(1));
    EXPECT_TRUE(map2.contains(2));
    EXPECT_TRUE(map2.contains(3));
}

TEST_F(HashTableTest, TestReserve)
{
    Int32HashMap map(10);
    EXPECT_GE(map.capacity(), 10u);
    EXPECT_LE(map.capacity(), 2*10u);

    map.reserve(100);
    EXPECT_GE(map.capacity(), 100u);
    EXPECT_LE(map.capacity(), 2*100u);

    const ramses_capu::uint_t old = map.capacity();
    map.reserve(90);
    EXPECT_EQ(old, map.capacity());
}

TEST_F(HashTableTest, CanInsertWithoutRehashWhenConstructedWithCapacity)
{
    for (ramses_capu::uint_t param = 1; param < 400; ++param)
    {
        SCOPED_TRACE(param);
        ramses_capu::HashTable<int32_t, int32_t> map(param);
        const ramses_capu::uint_t oldCapacity = map.capacity();
        EXPECT_GE(oldCapacity, param);

        for (unsigned i = 0; i < param; ++i)
        {
            map.put(i, i);
            EXPECT_EQ(i+1, map.count());
            EXPECT_EQ(oldCapacity, map.capacity());
        }
    }
}

TEST_F(HashTableTest, CanInsertWithoutRehashWhenReservedToCapacity)
{
    for (ramses_capu::uint_t param = 1; param < 400; ++param)
    {
        SCOPED_TRACE(param);
        ramses_capu::HashTable<int32_t, int32_t> map;

        map.reserve(param);
        const ramses_capu::uint_t oldCapacity = map.capacity();
        EXPECT_GE(oldCapacity, param);

        for (unsigned i = 0; i < param; ++i)
        {
            map.put(i, i);
            EXPECT_EQ(i+1, map.count());
            EXPECT_EQ(oldCapacity, map.capacity());
        }
    }
}

namespace
{
    class SomeClass
    {
    public:
        uint32_t i;
        SomeClass()
        {
            i = 0;
        }
        SomeClass(int val)
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

namespace ramses_capu
{
    template<>
    struct Hash<SomeClass>
    {
        uint_t operator()(const SomeClass& c)
        {
            return HashValue(c.i);
        }
    };
}

TEST_F(HashTableTest, AssignmentOperator)
{
    ramses_capu::HashTable<uint32_t, uint32_t> table;

    table.put(1, 2);
    table.put(3, 4);
    table.put(5, 6);

    ramses_capu::HashTable<uint32_t, uint32_t> table2;

    table2 = table;

    ASSERT_EQ(3u, table2.count());
    EXPECT_EQ(2u, table2[1]);
    EXPECT_EQ(4u, table2[3]);
    EXPECT_EQ(6u, table2[5]);
}

TEST_F(HashTableTest, TestMapRehashWithObjects)
{
    ramses_capu::HashTable<SomeClass, uint32_t> table(2);
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

TEST_F(HashTableTest, TestClear)
{
    Int32HashMap newmap;

    newmap.put(1, 10);
    newmap.put(2, 20);
    newmap.put(3, 30);

    newmap.clear();
    EXPECT_EQ(static_cast<uint32_t>(0), newmap.count());

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

TEST_F(HashTableTest, TestClear2)
{
    ramses_capu::HashTable<int32_t, int32_t> table;

    table.put(1, 1);
    table.put(2, 2);

    table.clear();

    table.put(1, 1);
    table.put(2, 2); // may not fail!
}

TEST_F(HashTableTest, TestContains)
{
    Int32HashMap newmap;
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

TEST_F(HashTableTest, IteratorRemove)
{
    Int32HashMap newmap;

    newmap.put(0, 12);
    newmap.put(3, 10);
    newmap.put(2, 11);

    for (Int32HashMap::Iterator iter = newmap.begin(); iter != newmap.end();)
    {
        iter = newmap.remove(iter);
    }

    EXPECT_EQ(0u, newmap.count());
    EXPECT_EQ(newmap.end(), newmap.find(0));
    EXPECT_EQ(newmap.end(), newmap.find(2));
    EXPECT_EQ(newmap.end(), newmap.find(3));

    newmap.put(0, 12);
    newmap.put(3, 10);
    newmap.put(2, 11);

    Int32HashMap::Iterator iter2 = newmap.begin();
    int32_t expectedValue = iter2->value;
    int32_t oldValue = 0;
    iter2 = newmap.remove(iter2, &oldValue);
    EXPECT_EQ(expectedValue, oldValue);

    while (iter2 != newmap.end())
    {
        iter2 = newmap.remove(iter2);
    }

    EXPECT_EQ(0u, newmap.count());
}

TEST_F(HashTableTest, find)
{
    Int32HashMap newmap;
    EXPECT_EQ(newmap.end(), newmap.find(4));
    newmap.put(1, 5);
    EXPECT_EQ(5, (*newmap.find(1)).value);
}

typedef ramses_capu::HashTable<int32_t, SomeClass> SomeHashMap;
TEST_F(HashTableTest, operator_subscript_read)
{
    Int32HashMap newmap;
    newmap.put(1, 5);
    int32_t val = newmap[1];
    EXPECT_EQ(5, val);

    // non trivial object
    SomeHashMap newmap2;
    newmap2.put(1, 5);
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

TEST_F(HashTableTest, operator_subscript_assign)
{
    /* native types */
    Int32HashMap newmap;
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
    ramses_capu::Memory::Set(&structKey, 0, sizeof(structKey)); // make valgrind happy
    structKey.a = 1;
    MyStruct structValue;
    structValue.a = 13;
    MyStruct structReturn;

    ramses_capu::HashTable<MyStruct, MyStruct> tab;
    tab[structKey] = structValue;
    structReturn = tab[structKey];

    EXPECT_EQ(structValue, structReturn);
}

TEST_F(HashTableTest, TestAddGet2)
{
    ramses_capu::HashTable<int32_t, int32_t> map;
    map.put(3, 3);
    EXPECT_FALSE(map.contains(19));
}
TEST_F(HashTableTest, TestAddGet)
{
    Int32HashMap newmap;
    const Int32HashMap& newConstMap(newmap);

    EXPECT_EQ(0u, newmap.count());

    newmap.put(1, 10);
    newmap.put(2, 20);
    newmap.put(3, 30);

    EXPECT_EQ(3u, newmap.count());

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

    EXPECT_EQ(2u, newmap.count());

    newmap.remove(2);

    it = newmap.find(1);
    EXPECT_EQ(newmap.end(), it);

    it = newmap.find(2);
    EXPECT_EQ(newmap.end(), it);

    it = newmap.find(3);
    EXPECT_NE(newmap.end(), it);
    EXPECT_EQ(30, it->value);

    EXPECT_EQ(1u, newmap.count());

    newmap.remove(3);


    it = newmap.find(1);
    EXPECT_EQ(newmap.end(), it);

    it = newmap.find(2);
    EXPECT_EQ(newmap.end(), it);

    it = newmap.find(3);
    EXPECT_EQ(newmap.end(), it);

    EXPECT_EQ(0u, newmap.count());
}

TEST_F(HashTableTest, TestRemove1)
{
    Int32HashMap newmap;

    newmap.put(1, 10);
    newmap.remove(1);
    newmap.put(2, 10);

    Int32HashMap::Iterator it = newmap.begin();
    while (it != newmap.end())
    {
        it++;
    }
}

TEST_F(HashTableTest, TestRemove)
{
    Int32HashMap newmap;

    newmap.put(1, 10);
    newmap.put(2, 20);
    newmap.put(3, 30);
    EXPECT_EQ(3u, newmap.count());

    EXPECT_TRUE(newmap.remove(2));
    EXPECT_EQ(2u, newmap.count());
    EXPECT_EQ(newmap.end(), newmap.find(2));

    int32_t value;
    EXPECT_TRUE(newmap.remove(3, &value));
    EXPECT_EQ(1u, newmap.count());
    EXPECT_EQ(30, value);

    EXPECT_EQ(newmap.end(), newmap.find(2));
}

TEST_F(HashTableTest, TestIterator1)
{
    Int32HashMap newmap(5);

    newmap.put(1, 10);
    newmap.put(2, 20);
    newmap.put(3, 30);

    Int32HashMap::Iterator it = newmap.begin();

    // TODO check values
    EXPECT_NE(it, newmap.end());
    const Int32HashMap::Pair& entry1 = *(it++);
    EXPECT_EQ(entry1.key * 10, entry1.value);

    EXPECT_NE(it, newmap.end());
    const Int32HashMap::Pair& entry2 = *(it++);
    EXPECT_EQ(entry2.key * 10, entry2.value);

    EXPECT_NE(it, newmap.end());
    const Int32HashMap::Pair& entry3 = *(it++);
    EXPECT_EQ(entry3.key * 10, entry3.value);

    EXPECT_EQ(it, newmap.end());
}

TEST_F(HashTableTest, TestIterator2)
{
    Int32HashMap newmap(5);

    newmap.put(1, 10);
    newmap.put(2, 20);
    newmap.put(3, 30);

    Int32HashMap::Iterator it = newmap.begin();

    EXPECT_NE(it, newmap.end());

    EXPECT_EQ((*it).key * 10, (*it).value);
    it++;

    EXPECT_TRUE(it != newmap.end());
    const Int32HashMap::Pair& entry2 = *it;
    EXPECT_EQ(entry2.key * 10, entry2.value);
    it++;

    EXPECT_NE(it, newmap.end());
    EXPECT_EQ((*it).key * 10, (*it).value);
    it++;

    EXPECT_TRUE(it == newmap.end());
}


TEST_F(HashTableTest, TestConstIterator1)
{
    Int32HashMap newmap(5);
    const Int32HashMap& newmapConstRef = newmap;

    newmap.put(1, 10);
    newmap.put(2, 20);
    newmap.put(3, 30);

    Int32HashMap::ConstIterator it = newmapConstRef.begin();

    // TODO check values
    EXPECT_NE(it, newmapConstRef.end());
    const Int32HashMap::Pair& entry1 = *(it++);
    EXPECT_EQ(entry1.key * 10, entry1.value);

    EXPECT_NE(it, newmapConstRef.end());
    const Int32HashMap::Pair& entry2 = *(it++);
    EXPECT_EQ(entry2.key * 10, entry2.value);

    EXPECT_NE(it, newmapConstRef.end());
    const Int32HashMap::Pair& entry3 = *(it++);
    EXPECT_EQ(entry3.key * 10, entry3.value);

    EXPECT_EQ(it, newmapConstRef.end());
}

TEST_F(HashTableTest, TestConstIterator2)
{
    Int32HashMap newmap(5);
    const Int32HashMap& newmapConstRef = newmap;

    newmap.put(1, 10);
    newmap.put(2, 20);
    newmap.put(3, 30);

    Int32HashMap::ConstIterator it = newmapConstRef.begin();

    EXPECT_NE(it, newmapConstRef.end());

    EXPECT_EQ((*it).key * 10, (*it).value);
    it++;

    EXPECT_TRUE(it != newmapConstRef.end());
    const Int32HashMap::Pair& entry2 = *it;
    EXPECT_EQ(entry2.key * 10, entry2.value);
    it++;

    EXPECT_NE(it, newmapConstRef.end());
    EXPECT_EQ((*it).key * 10, (*it).value);
    it++;

    EXPECT_TRUE(it == newmapConstRef.end());
}


TEST_F(HashTableTest, IteratorPrimitiveKeyComplexValue)
{
    MyStruct testStruct;
    testStruct.a =  1;
    testStruct.b = -1;

    ramses_capu::HashTable<uint32_t, MyStruct> hashTable;

    hashTable.put(0, testStruct);

    ramses_capu::HashTable<uint32_t, MyStruct>::Iterator hashTableIter = hashTable.begin();

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

TEST_F(HashTableTest, IteratorPrimitiveKeyComplexPtrValue)
{
    MyStruct testStruct;
    testStruct.a =  1;
    testStruct.b = -1;

    ramses_capu::HashTable<uint32_t, MyStruct*> hashTable;

    hashTable.put(0, &testStruct);

    ramses_capu::HashTable<uint32_t, MyStruct*>::Iterator hashTableIter = hashTable.begin();

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

TEST_F(HashTableTest, IteratorMethodCall)
{
    MyStruct testStruct;
    testStruct.a =  1;
    testStruct.b = -1;

    ramses_capu::HashTable<uint32_t, MyStruct> hashTable;

    hashTable.put(0, testStruct);

    ramses_capu::HashTable<uint32_t, MyStruct>::Iterator hashTableIter = hashTable.begin();

    EXPECT_EQ(1u, hashTableIter->value.a);
    EXPECT_EQ(1u, (*hashTableIter).value.a);
}

TEST_F(HashTableTest, TestRehasing)
{
    Int32HashMap newmap(2); // only 3 entries

    newmap.put(1, 10);
    newmap.put(2, 20);
    newmap.put(3, 30);
    newmap.put(4, 40);
    EXPECT_EQ(4u, newmap.count());

    EXPECT_EQ(10, newmap[1]);
    EXPECT_EQ(20, newmap[2]);
    EXPECT_EQ(30, newmap[3]);
    EXPECT_EQ(40, newmap[4]);
}

TEST_F(HashTableTest, TestWildRemoving)
{
    Int32HashMap newmap;

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
    EXPECT_EQ(static_cast<uint32_t>(1001), newmap.count());
}

TEST_F(HashTableTest, ForEach)
{
    Int32HashMap hashMap;

    hashMap.put(32,33);
    hashMap.put(43,44);
    hashMap.put(44,45);

    Int32HashMap someHashMap;

    for (auto p : hashMap)
    {
        someHashMap.put(p.key, p.value);
    }

    EXPECT_EQ(33, someHashMap.find(32)->value);
    EXPECT_EQ(44, someHashMap.find(43)->value);
    EXPECT_EQ(45, someHashMap.find(44)->value);

}

TEST_F(HashTableTest, HashTableWithComplexKey)
{
    ComplexTestType::Reset();
    {
        ramses_capu::HashTable<ComplexTestType, ramses_capu::uint_t> ht;
        std::vector<ComplexTestType> vec;
        for (ramses_capu::uint_t i = 0; i < 20; ++i)
        {
            ComplexTestType ctt(i*20);
            ht.put(ctt, i);
            vec.push_back(ctt);
        }

        for (ramses_capu::uint_t i = 0; i < vec.size(); ++i)
        {
            const ComplexTestType& ctt = vec[i];
            auto it = ht.find(ctt);
            EXPECT_NE(it, ht.end());
            EXPECT_EQ(it->key, ctt);
            EXPECT_EQ(it->value, i);
        }
    }
    EXPECT_EQ(ComplexTestType::dtor_count, ComplexTestType::ctor_count + ComplexTestType::copyctor_count);
}

TEST_F(HashTableTest, swapMemberFunction)
{
    Int32HashMap first;
    Int32HashMap second;

    first.put(1, 11);
    first.put(2, 12);
    second.put(100, 101);

    first.swap(second);
    EXPECT_EQ(2u, second.count());
    EXPECT_EQ(1u, first.count());

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

TEST_F(HashTableTest, swapGlobal)
{
    Int32HashMap first;
    Int32HashMap second;

    first.put(1, 11);
    first.put(2, 12);
    second.put(100, 101);

    using std::swap;
    swap(first, second);
    EXPECT_EQ(2u, second.count());
    EXPECT_EQ(1u, first.count());
}

TEST_F(HashTableTest, basicRefCountLifecycle)
{
    expectRefCnt(0);
    {
        RCHashTable ht;
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

TEST_F(HashTableTest, clearDestructsAllElements)
{
    RCHashTable ht;
    ht.put(RCKey(1), RCValue(2));
    ht.put(RCKey(2), RCValue(3));
    ht.put(RCKey(3), RCValue(4));
    expectRefCnt(3);
    ht.clear();
    expectRefCnt(0);
}

TEST_F(HashTableTest, copyCtorCopyConstructsObjects)
{
    RCHashTable ht;
    ht.put(RCKey(1), RCValue(2));
    ht.put(RCKey(2), RCValue(3));
    ht.put(RCKey(3), RCValue(4));
    expectRefCnt(3);

    RCHashTable ht2(ht);
    expectRefCnt(6);

    ht.clear();
    expectRefCnt(3);
}

TEST_F(HashTableTest, assignmentCopiesObjects)
{
    RCHashTable ht2;
    {
        RCHashTable ht;
        ht.put(RCKey(1), RCValue(2));
        ht.put(RCKey(2), RCValue(3));
        ht.put(RCKey(3), RCValue(4));
        expectRefCnt(3);

        ht2 = ht;
        expectRefCnt(3 + 3);
    }
    expectRefCnt(3);
}

TEST_F(HashTableTest, arrayAccessOperatorConstructsNewElementIfNotExisting)
{
    RCHashTable ht;
    ht.put(RCKey(1), RCValue(2));
    expectRefCnt(1);
    ht[RCKey(2)] = RCValue(3);
    expectRefCnt(2);
}

TEST_F(HashTableTest, arrayAccessOperatorReusesElementIfExisting)
{
    RCHashTable ht;
    ht.put(RCKey(1), RCValue(2));
    expectRefCnt(1);
    ht[RCKey(1)] = RCValue(3);
    expectRefCnt(1);
}

TEST_F(HashTableTest, swapKeepsNumberOfObjectsUnchanged)
{
    RCHashTable first, second;
    first.put(RCKey(1), RCValue(2));
    first.put(RCKey(2), RCValue(3));
    second.put(RCKey(3), RCValue(4));
    expectRefCnt(3);
    first.swap(second);
    expectRefCnt(3);
}
