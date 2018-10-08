/*
* Copyright (C) 2015 BMW Car IT GmbH
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

#ifndef RAMSES_CAPU_COMPLEXTESTTYPE_H
#define RAMSES_CAPU_COMPLEXTESTTYPE_H

#include "ramses-capu/container/vector.h"
#include "ramses-capu/container/Hash.h"
#include "ramses-capu/Config.h"
#include <gtest/gtest.h>

class ComplexTestType
{
public:
    ComplexTestType(ramses_capu::uint_t value_ = 0u)
        :value(value_)
    {
        ++ctor_count;
    }

    ComplexTestType(const ComplexTestType& other)
    {
        value = other.value;
        ++copyctor_count;
    }

    bool operator==(const ComplexTestType& other) const
    {
        return value == other.value;
    }

    bool operator!=(const ComplexTestType& other) const
    {
        return value != other.value;
    }

    ~ComplexTestType()
    {
        ++dtor_count;
    }

    static void Reset()
    {
        ctor_count = 0;
        copyctor_count = 0;
        dtor_count = 0;
    }

    ramses_capu::uint_t value;

    static ramses_capu::uint_t ctor_count;
    static ramses_capu::uint_t copyctor_count;
    static ramses_capu::uint_t dtor_count;
};

namespace ramses_capu
{
    template<>
    struct Hash<::ComplexTestType>
    {
        uint_t operator()(const ComplexTestType& key)
        {
            return Hash<ramses_capu::uint_t>()(key.value);
        }
    };
}

class MoveableComplexTestType
{
public:
    MoveableComplexTestType(ramses_capu::uint_t value_ = 0u, ramses_capu::uint_t /*value2_*/ = 0u, ramses_capu::uint_t /*value3_*/ = 0u)
        :value(value_)
    {
        ++ctor_count;
    }

    MoveableComplexTestType(const MoveableComplexTestType& other)
    {
        value = other.value;
        ++copyctor_count;
    }

    MoveableComplexTestType(MoveableComplexTestType&& other)
    {
        value = other.value;
        ++movector_count;
    }

    bool operator==(const MoveableComplexTestType& other) const
    {
        return value == other.value;
    }

    bool operator!=(const MoveableComplexTestType& other) const
    {
        return value != other.value;
    }

    ~MoveableComplexTestType()
    {
        ++dtor_count;
    }

    static void Reset()
    {
        ctor_count = 0;
        copyctor_count = 0;
        movector_count = 0;
        dtor_count = 0;
    }

    ramses_capu::uint_t value;

    static ramses_capu::uint_t ctor_count;
    static ramses_capu::uint_t copyctor_count;
    static ramses_capu::uint_t movector_count;
    static ramses_capu::uint_t dtor_count;
};

class MoveOnlyComplexTestType
{
 public:
    MoveOnlyComplexTestType()
        : value(0)
    {}

    explicit MoveOnlyComplexTestType(int v)
        : value(v)
    {}

    MoveOnlyComplexTestType(MoveOnlyComplexTestType&& other)
        : value(other.value)
    {
        other.value = -1;
    }

    MoveOnlyComplexTestType& operator=(MoveOnlyComplexTestType&& other)
    {
        value = other.value;
        other.value = -1;
        return *this;
    }

    MoveOnlyComplexTestType(const MoveOnlyComplexTestType&) = delete;
    MoveOnlyComplexTestType& operator=(const MoveOnlyComplexTestType&) = delete;

    int value;
};


class ComplexTestTypeTestBase
{
 public:
    ComplexTestTypeTestBase()
    {
        ComplexTestType::Reset();
        MoveableComplexTestType::Reset();
    }

    ~ComplexTestTypeTestBase()
    {
        EXPECT_EQ(ComplexTestType::dtor_count, ComplexTestType::ctor_count + ComplexTestType::copyctor_count);
        EXPECT_EQ(MoveableComplexTestType::dtor_count,
                  MoveableComplexTestType::ctor_count + MoveableComplexTestType::copyctor_count + MoveableComplexTestType::movector_count);

        ComplexTestType::Reset();
        MoveableComplexTestType::Reset();
    }
};


#endif
