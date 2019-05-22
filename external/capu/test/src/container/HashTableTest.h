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

#ifndef RAMSES_CAPU_HASHTABLETEST_H
#define RAMSES_CAPU_HASHTABLETEST_H

#include <gtest/gtest.h>

#include "ramses-capu/container/HashTable.h"

class HashTableTest: public ::testing::Test
{
public:
    struct MyStruct
    {
        uint32_t a;
        int16_t  b;

        uint32_t getA()
        {
            return a;
        }

        bool operator== (const MyStruct& other) const
        {
            return (a == other.a);
        }
    };
    typedef ramses_capu::HashTable<int32_t, int32_t> Int32HashMap;

    static const uint32_t PERFORMANCE_MAP_SIZE = 1000000;

    enum SomeEnumeration
    {
        TEST_VALUE1,
        TEST_VALUE2,
    };

    void SetUp();
    void TearDown();

    void resetRefCnts();
    void expectRefCnt(int32_t refCnt);
protected:
    Int32HashMap testHashMap;
};

namespace ramses_capu
{
    template<>
    struct Hash<HashTableTest::MyStruct>
    {
        uint_t operator()(const HashTableTest::MyStruct& c)
        {
            return HashValue(c.a, c.b);
        }
    };
}

#endif // RAMSES_CAPU_HASHTABLETEST_H
