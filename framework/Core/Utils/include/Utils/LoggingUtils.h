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
#include <assert.h>
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

#define MAKE_ENUM_CLASS_PRINTABLE(type, nameArray, oneAfterLastElement)                                                                            \
    /* ensure that nameArray have the same number of elements as the enum types */                                                                 \
    static_assert(static_cast<std::size_t>(oneAfterLastElement) == sizeof(nameArray) / sizeof(nameArray[0]), "number of elements does not match"); \
    static_assert(std::is_enum<type>::value && !std::is_convertible<type, int>::value, "Must use with enum class");                                \
    template <> struct fmt::formatter<type> : public ramses_internal::SimpleFormatterBase                                                          \
    {                                                                                                                                              \
        template <typename FormatContext> auto format(type index, FormatContext& ctx)                                                              \
        {                                                                                                                                          \
            const auto value = static_cast<std::underlying_type_t<type>>(index);                                                                   \
            if (value < 0 || value >= static_cast<std::underlying_type_t<type>>(oneAfterLastElement))                                                           \
                return fmt::format_to(ctx.out(), "<INVALID " #type " {}>", value);                                                                        \
            return fmt::format_to(ctx.out(), "{}", nameArray[value]);                                                                              \
        }                                                                                                                                          \
    };

#define MAKE_ENUM_CLASS_PRINTABLE_NO_EXTRA_LAST(type, nameArray, lastEnumElement)       \
    /* ensure that nameArray have the same number of elements as the enum types */                                  \
    static_assert(static_cast<std::size_t>(lastEnumElement)+1 == sizeof(nameArray) / sizeof(nameArray[0]), "number of elements does not match"); \
    static_assert(std::is_enum<type>::value && !std::is_convertible<type, int>::value, "Must use with enum class"); \
    template <> struct fmt::formatter<type> : public ramses_internal::SimpleFormatterBase                           \
    {                                                                                                               \
        static constexpr const auto lastValue = static_cast<std::underlying_type_t<type>>(lastEnumElement); \
        template <typename FormatContext> auto format(type index, FormatContext& ctx)                               \
        {                                                                                                           \
            const auto value = static_cast<std::underlying_type_t<type>>(index);                                    \
            if (value < 0 || value > lastValue) \
                return fmt::format_to(ctx.out(), "<INVALID " #type " {}>", value);                                  \
            return fmt::format_to(ctx.out(), "{}", nameArray[value]);                                               \
        }                                                                                                           \
    };


#endif
