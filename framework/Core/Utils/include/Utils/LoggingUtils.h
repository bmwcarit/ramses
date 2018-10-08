//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_LOGGINGUTILS_H
#define RAMSES_LOGGINGUTILS_H

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

#endif
