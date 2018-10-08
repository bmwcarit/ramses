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

#include "gtest/gtest.h"
#include "ramses-capu/container/Hash.h"

namespace ramses_capu
{
    namespace
    {
        class SomeClass
        {};
    }

    template<>
    struct Hash<SomeClass>
    {
        uint_t operator()(const SomeClass&)
        {
            return 123;
        }
    };

    enum SomeEnum
    {
        SOME_ENUM_MEMBER
    };

    enum class SomeEnumClass
    {
        MEMBER
    };

    TEST(CapuDefaultHashFunctionTest, useTraitsToFindCorrectHashFunction)
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
        char         sometype13[] = "HashTest";
        const char*  sometype14 = "HashTest2";

        SomeClass clazz;
        SomeClass* ptr = &clazz;

        // everything must compile...
        ramses_capu::Hash<decltype(sometype1)>()(sometype1);
        ramses_capu::Hash<decltype(sometype2)>()(sometype2);
        ramses_capu::Hash<decltype(sometype3)>()(sometype3);
        ramses_capu::Hash<decltype(sometype4)>()(sometype4);
        ramses_capu::Hash<decltype(sometype5)>()(sometype5);
        ramses_capu::Hash<decltype(sometype6)>()(sometype6);
        ramses_capu::Hash<decltype(sometype7)>()(sometype7);
        ramses_capu::Hash<decltype(sometype8)>()(sometype8);
        ramses_capu::Hash<decltype(sometype12)>()(sometype12);
        ramses_capu::Hash<decltype(sometype13)>()(sometype13);
        ramses_capu::Hash<decltype(sometype14)>()(sometype14);
        ramses_capu::Hash<decltype(clazz)>()(clazz);
        ramses_capu::Hash<decltype(ptr)>()(ptr);
        ramses_capu::Hash<decltype(SOME_ENUM_MEMBER)>()(SOME_ENUM_MEMBER);
        ramses_capu::Hash<decltype(SomeEnumClass::MEMBER)>()(SomeEnumClass::MEMBER);

    }
}
