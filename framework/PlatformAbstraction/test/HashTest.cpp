//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "PlatformAbstraction/Hash.h"
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
    size_t operator()(const SomeClass&)
    {
        return 123;
    }
};

namespace ramses_internal
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
        UNUSED(std::hash<decltype(sometype1)>()(sometype1));
        UNUSED(std::hash<decltype(sometype2)>()(sometype2));
        UNUSED(std::hash<decltype(sometype3)>()(sometype3));
        UNUSED(std::hash<decltype(sometype4)>()(sometype4));
        UNUSED(std::hash<decltype(sometype5)>()(sometype5));
        UNUSED(std::hash<decltype(sometype6)>()(sometype6));
        UNUSED(std::hash<decltype(sometype7)>()(sometype7));
        UNUSED(std::hash<decltype(sometype8)>()(sometype8));
        UNUSED(std::hash<decltype(sometype12)>()(sometype12));
        UNUSED(std::hash<decltype(sometype14)>()(sometype14));
        UNUSED(std::hash<decltype(clazz)>()(clazz));
        UNUSED(std::hash<decltype(ptr)>()(ptr));
        UNUSED(std::hash<decltype(ESomeenum_MEMBER0)>()(ESomeenum_MEMBER0));
        UNUSED(std::hash<decltype(SomeEnumClass::MEMBER)>()(SomeEnumClass::MEMBER));
    }
}
