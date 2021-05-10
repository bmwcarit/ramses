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
#include "PlatformAbstraction/Hash.h"
#include "Collections/StringOutputStream.h"
#include "PlatformAbstraction/Macros.h"
#include <type_traits>
#include <functional>

namespace ramses_internal
{
    template <typename BaseT, BaseT _invalid, typename UniqueIdT>
    class StronglyTypedValue final
    {
    public:
        using BaseType = BaseT;

        static constexpr StronglyTypedValue Invalid() noexcept
        {
            return StronglyTypedValue(_invalid);
        }

        constexpr explicit StronglyTypedValue(BaseType value) noexcept
            : m_value(value)
        {
        }

        constexpr StronglyTypedValue() noexcept
            : m_value(_invalid)
        {
        }

        RNODISCARD constexpr BaseType getValue() const
        {
            return m_value;
        }

        RNODISCARD constexpr BaseType& getReference()
        {
            return m_value;
        }

        RNODISCARD constexpr bool operator==(const StronglyTypedValue& other) const
        {
            return m_value == other.m_value;
        }

        RNODISCARD constexpr bool operator!=(const StronglyTypedValue& other) const
        {
            return m_value != other.m_value;
        }

        RNODISCARD constexpr bool isValid() const
        {
            return m_value != _invalid;
        }

        static_assert(std::is_arithmetic<BaseType>::value || std::is_pointer<BaseType>::value, "expected arithmetic or pointer basetype");
    private:
        BaseType m_value;
    };

}

#define MAKE_STRONGLYTYPEDVALUE_PRINTABLE(stronglyType) \
    template <> \
    struct fmt::formatter<::stronglyType> {    \
        template<typename ParseContext> \
        constexpr auto parse(ParseContext& ctx)  { \
            return ctx.begin(); \
        } \
        template<typename FormatContext> \
        auto format(const ::stronglyType& str, FormatContext& ctx) {    \
            return fmt::format_to(ctx.out(), "{}", str.getValue()); \
        } \
    };


namespace std
{
    template <typename BaseT, BaseT DefaultValue, typename UniqueIdT>
    struct hash<ramses_internal::StronglyTypedValue<BaseT, DefaultValue, UniqueIdT>>
    {
    public:
        size_t operator()(const ramses_internal::StronglyTypedValue<BaseT, DefaultValue, UniqueIdT>& v) const
        {
            return static_cast<size_t>(hash<BaseT>()(v.getValue()));
        }
    };
}

#endif
