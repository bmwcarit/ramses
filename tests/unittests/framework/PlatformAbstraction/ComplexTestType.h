//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include <functional>

namespace ramses::internal
{
    template<typename TAG = void>
    class ComplexTestType
    {
    public:
        explicit ComplexTestType(size_t value_ = 0u)
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

        ComplexTestType& operator=(const ComplexTestType&) = default;

        ~ComplexTestType()
        {
            ++dtor_count;
        }

        static int RefCnt()
        {
            return ctor_count + copyctor_count - dtor_count;
        }

        static void Reset()
        {
            ctor_count     = 0;
            copyctor_count = 0;
            dtor_count     = 0;
        }

        size_t value;

        static int ctor_count;
        static int copyctor_count;
        static int dtor_count;
    };


    template <typename TAG>
    int ComplexTestType<TAG>::ctor_count     = 0u;
    template <typename TAG>
    int ComplexTestType<TAG>::copyctor_count = 0u;
    template <typename TAG>
    int ComplexTestType<TAG>::dtor_count     = 0u;
}

template <typename TAG>
struct std::hash<ramses::internal::ComplexTestType<TAG>>
{
    size_t operator()(const ramses::internal::ComplexTestType<TAG>& key)
    {
        return std::hash<size_t>()(key.value);
    }
};
