//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_INTERNAL_STRONGLYTYPEDVALUE_H
#define RAMSES_INTERNAL_STRONGLYTYPEDVALUE_H

#include "Utils/Warnings.h"
#include "ramses-capu/util/Traits.h"
#include "ramses-capu/container/Hash.h"
#include "Collections/StringOutputStream.h"
#include <type_traits>
#include <functional>

namespace ramses_internal
{
    template <typename _BaseType, _BaseType _DefaultValue, typename _UniqueId>
    class StronglyTypedValue final
    {
    public:
        typedef _BaseType BaseType;

        constexpr static StronglyTypedValue DefaultValue()
        {
            return StronglyTypedValue(_DefaultValue);
        }

        constexpr explicit StronglyTypedValue(BaseType value)
            : m_value(value)
        {
        }

        constexpr StronglyTypedValue()
            : m_value(_DefaultValue)
        {
        }

        constexpr BaseType getValue() const
        {
            return m_value;
        }

        constexpr BaseType& getReference()
        {
            return m_value;
        }

        StronglyTypedValue& operator=(const StronglyTypedValue& other) = default;

        constexpr bool operator==(const StronglyTypedValue& other) const
        {
            return m_value == other.m_value;
        }

        constexpr bool operator!=(const StronglyTypedValue& other) const
        {
            return m_value != other.m_value;
        }

        static_assert(std::is_arithmetic<BaseType>::value || std::is_pointer<BaseType>::value, "expected arithmetic or pointer basetype");
    private:
        BaseType m_value;
    };

    // StringOutputStream operator
    template <typename _BaseType, _BaseType _DefaultValue, typename _UniqueId>
    inline StringOutputStream& operator<<(StringOutputStream& os, const StronglyTypedValue<_BaseType, _DefaultValue, _UniqueId>& value)
    {
        os << value.getValue();
        return os;
    }
}

namespace std
{
    template <typename _BaseType, _BaseType _DefaultValue, typename _UniqueId>
    struct hash<ramses_internal::StronglyTypedValue<_BaseType, _DefaultValue, _UniqueId>>
    {
    public:
        size_t operator()(const ramses_internal::StronglyTypedValue<_BaseType, _DefaultValue, _UniqueId>& v) const
        {
            return static_cast<size_t>(hash<_BaseType>()(v.getValue()));
        }
    };
}

// make StronglyTypedValue hash correctly
namespace ramses_capu
{
    template<typename _BaseType, _BaseType _DefaultValue, typename _UniqueId>
    struct Hash<::ramses_internal::StronglyTypedValue<_BaseType, _DefaultValue, _UniqueId>>
    {
        uint_t operator()(const ::ramses_internal::StronglyTypedValue<_BaseType, _DefaultValue, _UniqueId>& key)
        {
            return Hash<_BaseType>()(key.getValue());
        }
    };
}
#endif
