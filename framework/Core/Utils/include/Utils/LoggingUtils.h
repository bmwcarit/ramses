//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_LOGGINGUTILS_H
#define RAMSES_LOGGINGUTILS_H

#include "PlatformAbstraction/FmtBase.h"
#include <type_traits>
#include <cassert>
#include <cstddef>

// declares mapping function from enum to strings
// Caution: enum has range from zero to lastElement
#define ENUM_TO_STRING(type, elements, lastElement) \
    /* ensure that elements have the same number of elements as the enum types */ \
    static_assert(static_cast<std::size_t>(lastElement) == sizeof(elements)/sizeof(elements[0]), "number of elements does not match"); \
    static inline const char* EnumToString(type index) \
    { \
        const std::size_t value = static_cast<std::size_t>(index); \
        if (value >= static_cast<std::size_t>(lastElement))  \
        { \
            assert(false && "EnumToString called with invalid value"); \
            return "<INVALID>"; \
        } \
        return elements[value]; \
    };

#define MAKE_ENUM_CLASS_PRINTABLE(type, enumName, elementNameArray, oneAfterLastElement)                                                                         \
    /* ensure that elementNameArray have the same number of elements as the enum types */                                                                        \
    static_assert(static_cast<uint64_t>(oneAfterLastElement) == sizeof(elementNameArray) / sizeof(elementNameArray[0]), "number of elements does not match"); \
    static_assert(std::is_enum<type>::value && !std::is_convertible<type, int>::value, "Must use with enum class");                                              \
    static_assert(sizeof(type) <= sizeof(uint64_t), #type " type may not be larger than uint64_t");                                                                  \
    template <> struct fmt::formatter<type>                                                                                                                      \
    {                                                                                                                                                            \
        bool shortLog = false;                                                                                                                                   \
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
                assert(false && "invalid format for enum class");                                                                                                \
            return it;                                                                                                                                           \
        }                                                                                                                                                        \
        template <typename FormatContext> constexpr auto format(type index, FormatContext& ctx)                                                                            \
        {                                                                                                                                                        \
            const auto value = static_cast<uint64_t>(index);                                                                                                  \
            if (value >= static_cast<uint64_t>(oneAfterLastElement))                                                                                          \
                return fmt::format_to(ctx.out(), "<INVALID " #type " {}>", value);                                                                               \
            return shortLog ? fmt::format_to(ctx.out(), "{}", elementNameArray[value]) : fmt::format_to(ctx.out(), "{}::{}", enumName, elementNameArray[value]); \
        }                                                                                                                                                        \
    };

#define MAKE_ENUM_CLASS_PRINTABLE_NO_EXTRA_LAST(type, enumName, elementNameArray, lastEnumElement)                                                               \
    /* ensure that elementNameArray have the same number of elements as the enum types */                                                                        \
    static_assert(static_cast<uint64_t>(lastEnumElement) + 1 == sizeof(elementNameArray) / sizeof(elementNameArray[0]), "number of elements does not match");    \
    static_assert(std::is_enum<type>::value && !std::is_convertible<type, int>::value, "Must use with enum class");                                              \
    static_assert(sizeof(type) <= sizeof(uint64_t), #type " type may not be larger uint64_t"); \
    template <> struct fmt::formatter<type>                                                                                                                      \
    {                                                                                                                                                            \
        static constexpr const auto                     lastValue = static_cast<uint64_t>(lastEnumElement);                                                      \
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
                assert(false && "invalid format for enum class");                                                                                                \
            return it;                                                                                                                                           \
        }                                                                                                                                                        \
        template <typename FormatContext> constexpr auto format(type index, FormatContext& ctx)                                                                            \
        {                                                                                                                                                        \
            const auto value = static_cast<uint64_t>(index);                                                                                                     \
            if (value > lastValue)                                                                                                                               \
                return fmt::format_to(ctx.out(), "<INVALID " #type " {}>", value);                                                                               \
            return shortLog ? fmt::format_to(ctx.out(), "{}", elementNameArray[value]) : fmt::format_to(ctx.out(), "{}::{}", enumName, elementNameArray[value]); \
        }                                                                                                                                                        \
    };

#endif
