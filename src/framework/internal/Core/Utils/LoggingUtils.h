//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/PlatformAbstraction/FmtBase.h"
#include "EnumTraits.h"
#include <type_traits>
#include <cassert>
#include <cstddef>
#include <array>

// declares mapping function from enum to strings
#define ENUM_TO_STRING(type, elements, lastElement) \
    /* ensure that elements have the same number of elements as the enum types */ \
    static_assert(ramses::internal::EnumTraits::VerifyElementCountIfSupported<type>(elements.size())); /* NOLINT(bugprone-macro-parentheses) */ \
    static_assert(static_cast<std::size_t>(lastElement) + 1 == elements.size(), "number of elements does not match"); /* NOLINT(bugprone-macro-parentheses) */ \
    static inline const char* EnumToString(type index) \
    { \
        const std::size_t value = static_cast<std::size_t>(index); \
        if (value > static_cast<std::size_t>(lastElement))  \
        { \
            assert(false && "EnumToString called with invalid value"); /* NOLINT(misc-static-assert)*/ \
            return "<INVALID>"; \
        } \
        return elements[value]; /* NOLINT(bugprone-macro-parentheses) */ \
    };

#define MAKE_ENUM_CLASS_PRINTABLE(type, enumName, elementNameArray, lastEnumElement)                                                               \
    /* ensure that elementNameArray have the same number of elements as the enum types */                                                                        \
    static_assert(ramses::internal::EnumTraits::VerifyElementCountIfSupported<type>(elementNameArray.size())); /* NOLINT(bugprone-macro-parentheses) */ \
    static_assert(static_cast<uint32_t>(lastEnumElement) < std::numeric_limits<uint32_t>::max());    \
    static_assert(static_cast<uint32_t>(lastEnumElement) + 1 == static_cast<uint32_t>(elementNameArray.size()), "number of elements does not match"); /* NOLINT(bugprone-macro-parentheses) */    \
    static_assert(std::is_enum<type>::value && !std::is_convertible<type, int>::value, "Must use with enum class");                                              \
    template <> struct fmt::formatter<type>: formatter<string_view>                                                                                              \
    {                                                                                                                                                            \
        static constexpr const auto                     lastValue = static_cast<uint32_t>(lastEnumElement);                                                      \
        bool                                            shortLog  = false;                                                                                       \
        template <typename ParseContext> constexpr auto parse(ParseContext& ctx)                                                                                 \
        {                                                                                                                                                        \
            auto       it  = ctx.begin();                                                                                                                        \
            const auto end = ctx.end();                                                                                                                          \
            if (it != end && *it == 's')                                                                                                                         \
            {                                                                                                                                                    \
                ++it;                                                                                                                                            \
                shortLog = true;                                                                                                                                 \
            }                                                                                                                                                    \
            if (it != end && *it != '}')                                                                                                                         \
                assert(false && "invalid format for enum class"); /* NOLINT(misc-static-assert) */                                                               \
            return it;                                                                                                                                           \
        }                                                                                                                                                        \
        template <typename FormatContext> constexpr auto format(type index, FormatContext& ctx)                                                                  \
        {                                                                                                                                                        \
            const auto value = static_cast<uint32_t>(index);                                                                                                     \
            if (value > lastValue)                                                                                                                               \
                return fmt::format_to(ctx.out(), "<INVALID " #type " {}>", value);                                                                               \
            return shortLog ? fmt::format_to(ctx.out(), "{}", elementNameArray[value]) /* NOLINT(bugprone-macro-parentheses) */                                  \
                            : fmt::format_to(ctx.out(), "{}::{}", enumName, elementNameArray[value]); /* NOLINT(bugprone-macro-parentheses) */ \
        }                                                                                                                                                        \
    };
