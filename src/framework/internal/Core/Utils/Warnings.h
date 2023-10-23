//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#define STRINGIFY(x) #x


#if defined(__GNUC__)
#    define WARNINGS_PUSH _Pragma("GCC diagnostic push")
#elif defined(_MSC_VER)
#    define WARNINGS_PUSH __pragma(warning(push))
#else
#    define WARNINGS_PUSH
#endif


#if defined(__GNUC__)
#    define WARNINGS_POP _Pragma("GCC diagnostic pop")
#elif defined(_MSC_VER)
#    define WARNINGS_POP __pragma(warning(pop))
#else
#    define WARNINGS_POP
#endif


#if defined(_MSC_VER)
#    define WARNING_DISABLE_VC(warning_) \
        __pragma(warning(disable: warning_))
#else
#    define WARNING_DISABLE_VC(warning)
#endif

#if defined(__GNUC__)  // GCC and clang
#    define WARNING_DISABLE_LINUX(warning) \
        _Pragma(STRINGIFY(GCC diagnostic ignored #warning))
#else
#    define WARNING_DISABLE_LINUX(warning)
#endif


#if defined(__GNUC__) && !defined(__clang__)
#    define WARNING_DISABLE_GCC(warning) \
        _Pragma(STRINGIFY(GCC diagnostic ignored #warning))
#else
#    define WARNING_DISABLE_GCC(warning)
#endif

#if defined(__GNUC__) && !defined(__clang__) && __GNUC__ >= 9
#    define WARNING_DISABLE_GCC9(warning) \
        _Pragma(STRINGIFY(GCC diagnostic ignored #warning))
#else
#    define WARNING_DISABLE_GCC9(warning)
#endif

#if defined(__clang__)
#    define WARNING_DISABLE_CLANG(warning) \
        _Pragma(STRINGIFY(GCC diagnostic ignored #warning))
#else
#    define WARNING_DISABLE_CLANG(warning)
#endif


#if defined(__GNUC__)
#    define PUSH_DISABLE_C_STYLE_CAST_WARNING \
        WARNINGS_PUSH \
        WARNING_DISABLE_LINUX(-Wold-style-cast)
#elif defined(_MSC_VER)
#    define PUSH_DISABLE_C_STYLE_CAST_WARNING
#else
#    define PUSH_DISABLE_C_STYLE_CAST_WARNING
#endif


#if defined(__GNUC__)
#    define POP_DISABLE_C_STYLE_CAST_WARNING \
        WARNINGS_POP
#elif defined(_MSC_VER)
#    define POP_DISABLE_C_STYLE_CAST_WARNING
#else
#    define POP_DISABLE_C_STYLE_CAST_WARNING
#endif
