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
#include "ramses-capu/util/Traits.h"
#include "ramses-capu/container/Hash.h"
#include "ramses-capu/Config.h"

class SomeClass
{};

enum SomeEnum
{
    SOME_ENUM_MEMBER
};

class Tester
{
public:

    template <typename T>
    static uint32_t type(T val)
    {
        val = *&val;
        enum
        {
           type = static_cast<uint32_t>(ramses_capu::is_CAPU_PRIMITIVE<T>::Value) | static_cast<uint32_t>(ramses_capu::is_CAPU_REFERENCE<T>::Value) | static_cast<uint32_t>(ramses_capu::is_CAPU_POINTER<T>::Value)
        };
        return type;
    }
};

class SomeClassWithEnumSize
{
    uint32_t myValue;

    SomeClassWithEnumSize(){
        UNUSED(myValue);
    }
};

union SomeUnion
{
    uint32_t intVal1;
    uint32_t intVal2;
    struct shortStruct
    {
        uint16_t shortVal1;
        uint16_t shortVal2;
        uint16_t shortVal3;
        uint16_t shortVal4;
    };
};

union SomeUnionWithEnumSize
{
    char data[sizeof(SomeEnum)];
    struct data2Struct
    {
        char data2[sizeof(SomeEnum)];
    };
};

TEST(Traits, TestTypeIdentifier)
{
    EXPECT_EQ(CAPU_TYPE_PRIMITIVE, ramses_capu::Type<uint64_t>::Identifier);
    EXPECT_EQ(CAPU_TYPE_PRIMITIVE, ramses_capu::Type<uint32_t>::Identifier);
    EXPECT_EQ(CAPU_TYPE_PRIMITIVE, ramses_capu::Type<int32_t>::Identifier);
    EXPECT_EQ(CAPU_TYPE_PRIMITIVE, ramses_capu::Type<uint16_t>::Identifier);
    EXPECT_EQ(CAPU_TYPE_PRIMITIVE, ramses_capu::Type<int16_t>::Identifier);
    EXPECT_EQ(CAPU_TYPE_PRIMITIVE, ramses_capu::Type<uint8_t>::Identifier);
    EXPECT_EQ(CAPU_TYPE_PRIMITIVE, ramses_capu::Type<int8_t>::Identifier);
    EXPECT_EQ(CAPU_TYPE_PRIMITIVE, ramses_capu::Type<char>::Identifier);

    EXPECT_EQ(CAPU_TYPE_POINTER, ramses_capu::Type<void*>::Identifier);
    EXPECT_EQ(CAPU_TYPE_POINTER, ramses_capu::Type<SomeClass*>::Identifier);
    EXPECT_EQ(CAPU_TYPE_POINTER, ramses_capu::Type<SomeClass**>::Identifier);
    EXPECT_EQ(CAPU_TYPE_POINTER, ramses_capu::Type<SomeClass***>::Identifier);

    EXPECT_EQ(CAPU_TYPE_REFERENCE, ramses_capu::Type<SomeClass&>::Identifier);
    EXPECT_EQ(CAPU_TYPE_REFERENCE, ramses_capu::Type<uint32_t&>::Identifier);
    EXPECT_EQ(CAPU_TYPE_REFERENCE, ramses_capu::Type<SomeEnum&>::Identifier);

    EXPECT_EQ(CAPU_TYPE_ENUM, ramses_capu::Type<SomeEnum>::Identifier);

    EXPECT_EQ(CAPU_TYPE_POINTER, ramses_capu::Type<SomeEnum*>::Identifier);
    EXPECT_EQ(CAPU_TYPE_POINTER, ramses_capu::Type<SomeEnum**>::Identifier);
    EXPECT_EQ(CAPU_TYPE_POINTER, ramses_capu::Type<SomeEnum***>::Identifier);

    EXPECT_EQ(CAPU_TYPE_VOID, ramses_capu::Type<void>::Identifier);

    EXPECT_EQ(CAPU_TYPE_CLASS, ramses_capu::Type<SomeUnion>::Identifier);
    EXPECT_EQ(CAPU_TYPE_CLASS, ramses_capu::Type<SomeClass>::Identifier);
    EXPECT_EQ(CAPU_TYPE_CLASS, ramses_capu::Type<SomeUnionWithEnumSize>::Identifier);
    EXPECT_EQ(CAPU_TYPE_CLASS, ramses_capu::Type<SomeClassWithEnumSize>::Identifier);
}

TEST(Traits, TestPrimitiveTypes)
{
    uint64_t sometype1 = 42;
    uint32_t sometype2 = 42;
    int32_t  sometype3 = 42;
    uint16_t sometype4 = 42;
    int16_t  sometype5 = 42;
    uint8_t  sometype6 = 42;
    int8_t   sometype7 = 42;
    char   sometype8 = 42;
    float  sometype10 = 42.f;
    double sometype11 = 42.0;
    bool   sometype12 = true;

    EXPECT_EQ(static_cast<uint32_t>(CAPU_TYPE_PRIMITIVE), Tester::type(sometype1));
    EXPECT_EQ(static_cast<uint32_t>(CAPU_TYPE_PRIMITIVE), Tester::type(sometype2));
    EXPECT_EQ(static_cast<uint32_t>(CAPU_TYPE_PRIMITIVE), Tester::type(sometype3));
    EXPECT_EQ(static_cast<uint32_t>(CAPU_TYPE_PRIMITIVE), Tester::type(sometype4));
    EXPECT_EQ(static_cast<uint32_t>(CAPU_TYPE_PRIMITIVE), Tester::type(sometype5));
    EXPECT_EQ(static_cast<uint32_t>(CAPU_TYPE_PRIMITIVE), Tester::type(sometype6));
    EXPECT_EQ(static_cast<uint32_t>(CAPU_TYPE_PRIMITIVE), Tester::type(sometype7));
    EXPECT_EQ(static_cast<uint32_t>(CAPU_TYPE_PRIMITIVE), Tester::type(sometype8));
    EXPECT_EQ(static_cast<uint32_t>(CAPU_TYPE_PRIMITIVE), Tester::type(sometype10));
    EXPECT_EQ(static_cast<uint32_t>(CAPU_TYPE_PRIMITIVE), Tester::type(sometype11));
    EXPECT_EQ(static_cast<uint32_t>(CAPU_TYPE_PRIMITIVE), Tester::type(sometype12));
}

TEST(Traits, TestPointerAndReferences)
{
    SomeClass clazz;
    EXPECT_EQ(static_cast<uint32_t>(CAPU_TYPE_POINTER), Tester::type(&clazz));
    EXPECT_EQ(static_cast<uint32_t>(CAPU_TYPE_REFERENCE), Tester::type<SomeClass&>(clazz));
}
