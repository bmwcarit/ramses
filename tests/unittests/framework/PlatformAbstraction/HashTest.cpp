//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/PlatformAbstraction/Hash.h"
#include "gtest/gtest.h"

namespace
{
    enum ESomeenum
    {
        ESomeenum_MEMBER0 = 1234,
        ESomeenum_MEMBER1
    };

    class SomeClass
    {};

    enum class SomeEnumClass
    {
        MEMBER
    };
}

template<>
struct std::hash<SomeClass>
{
    size_t operator()(const SomeClass& /*unused*/)
    {
        return 123;
    }
};

namespace ramses::internal
{
    TEST(HashCombine, usesSeedAndValue)
    {
        size_t seed1 = 0;
        HashCombine(seed1, 123);

        size_t seed2 = 123;
        HashCombine(seed2, 123);

        size_t seed3 = 123;
        HashCombine(seed3, 0);

        EXPECT_NE(seed1, seed2);
        EXPECT_NE(seed1, seed3);
    }

    TEST(HashCombine, usesExtraArguments)
    {
        size_t seed1 = 0;
        HashCombine(seed1, 123);

        size_t seed2 = 0;
        HashCombine(seed2, 123, 456);

        EXPECT_NE(seed1, seed2);
    }

    TEST(HashValue, givesSameResultAsHashCombine)
    {
        size_t seed1 = 0;
        HashCombine(seed1, 123);
        size_t seed2 = 0;
        HashCombine(seed2, 123, 456, 789);

        EXPECT_EQ(seed1, HashValue(123));
        EXPECT_EQ(seed2, HashValue(123, 456, 789));
    }

    TEST(HashTest, HashMemoryRangeRespectLength)
    {
        unsigned char ar[] = {1, 2, 3, 4};
        EXPECT_EQ(HashMemoryRange(ar, 1), HashMemoryRange(ar, 1));
        EXPECT_EQ(HashMemoryRange(ar, 4), HashMemoryRange(ar, 4));
        EXPECT_NE(HashMemoryRange(ar, 1), HashMemoryRange(ar, 2));
    }

    TEST(HashTest, HashMemoryRangeWorksForEmptyRange)
    {
        unsigned char ar[] = {1};
        EXPECT_EQ(HashMemoryRange(ar, 0),      HashMemoryRange(ar, 0));
        EXPECT_EQ(HashMemoryRange(nullptr, 0), HashMemoryRange(nullptr, 0));
        EXPECT_EQ(HashMemoryRange(ar, 0),      HashMemoryRange(nullptr, 0));
    }

    TEST(HashTest, verifyDifferentHashCallsCompile)
    {
        uint64_t       sometype1 = 42;
        uint32_t       sometype2 = 42;
        int32_t        sometype3 = 42;
        uint16_t       sometype4 = 42;
        int16_t        sometype5 = 42;
        uint8_t        sometype6 = 42;
        int8_t         sometype7 = 42;
        char         sometype8 = 42;
        bool         sometype12 = true;
        const char*  sometype14 = "HashTest2";

        SomeClass clazz;
        SomeClass* ptr = &clazz;

        // everything must compile...
        [[maybe_unused]] auto hash1  = std::hash<decltype(sometype1)>()(sometype1);
        [[maybe_unused]] auto hash2  = std::hash<decltype(sometype2)>()(sometype2);
        [[maybe_unused]] auto hash3  = std::hash<decltype(sometype3)>()(sometype3);
        [[maybe_unused]] auto hash4  = std::hash<decltype(sometype4)>()(sometype4);
        [[maybe_unused]] auto hash5  = std::hash<decltype(sometype5)>()(sometype5);
        [[maybe_unused]] auto hash6  = std::hash<decltype(sometype6)>()(sometype6);
        [[maybe_unused]] auto hash7  = std::hash<decltype(sometype7)>()(sometype7);
        [[maybe_unused]] auto hash8  = std::hash<decltype(sometype8)>()(sometype8);
        [[maybe_unused]] auto hash9  = std::hash<decltype(sometype12)>()(sometype12);
        [[maybe_unused]] auto hash10 = std::hash<decltype(sometype14)>()(sometype14);
        [[maybe_unused]] auto hash11 = std::hash<decltype(clazz)>()(clazz);
        [[maybe_unused]] auto hash12 = std::hash<decltype(ptr)>()(ptr);
        [[maybe_unused]] auto hash13 = std::hash<decltype(ESomeenum_MEMBER0)>()(ESomeenum_MEMBER0);
        [[maybe_unused]] auto hash14 = std::hash<decltype(SomeEnumClass::MEMBER)>()(SomeEnumClass::MEMBER);
    }
}
