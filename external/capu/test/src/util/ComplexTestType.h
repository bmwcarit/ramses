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

#endif
