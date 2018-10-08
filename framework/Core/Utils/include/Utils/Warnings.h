//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_WARNINGS_H
#define RAMSES_WARNINGS_H


#if !defined(STRINGIFY)
#define STRINGIFY(x) #x
#endif


#if !defined(WARNINGS_PUSH)
#   if defined(__GNUC__)
#       define WARNINGS_PUSH _Pragma("GCC diagnostic push")
#   elif defined(_MSC_VER)
#       define WARNINGS_PUSH __pragma(warning(push))
#   else
#       define WARNINGS_PUSH
#   endif
#endif


#if !defined(WARNINGS_POP)
#   if defined(__GNUC__)
#       define WARNINGS_POP _Pragma("GCC diagnostic pop")
#   elif defined(_MSC_VER)
#       define WARNINGS_POP __pragma(warning(pop))
#   else
#       define WARNINGS_POP
#   endif
#endif


#if !defined(WARNING_DISABLE_VC)
#   if defined(_MSC_VER)
#       define WARNING_DISABLE_VC(warning_) \
            __pragma(warning(disable: warning_))
#   else
#       define WARNING_DISABLE_VC(warning)
#   endif
#endif


#if !defined(WARNING_DISABLE_LINUX) //GCC and clang
#   if defined(__GNUC__)
#       define WARNING_DISABLE_LINUX(warning) \
            _Pragma(STRINGIFY(GCC diagnostic ignored #warning))
#   else
#       define WARNING_DISABLE_LINUX(warning)
#   endif
#endif


#if !defined(WARNING_DISABLE_CLANG)
#   if defined(__clang__)
#       define WARNING_DISABLE_CLANG(warning) \
            _Pragma(STRINGIFY(GCC diagnostic ignored #warning))
#   else
#       define WARNING_DISABLE_CLANG(warning)
#   endif
#endif


#if !defined(PUSH_DISABLE_C_STYLE_CAST_WARNING)
#   if defined(__GNUC__)
#       define PUSH_DISABLE_C_STYLE_CAST_WARNING \
            WARNINGS_PUSH \
            WARNING_DISABLE_LINUX(-Wold-style-cast)
#   elif defined(_MSC_VER)
#       define PUSH_DISABLE_C_STYLE_CAST_WARNING
#   else
#       define PUSH_DISABLE_C_STYLE_CAST_WARNING
#   endif
#endif


#if !defined(POP_DISABLE_C_STYLE_CAST_WARNING)
#   if defined(__GNUC__)
#       define POP_DISABLE_C_STYLE_CAST_WARNING \
            WARNINGS_POP
#   elif defined(_MSC_VER)
#       define POP_DISABLE_C_STYLE_CAST_WARNING
#   else
#       define POP_DISABLE_C_STYLE_CAST_WARNING
#   endif
#endif


#ifdef __ghs__
#define IGNORE_UNNAMED_MEMBER_WARNING_START _Pragma("ghs nowarning 620")
#define IGNORE_UNNAMED_MEMBER_WARNING_END  _Pragma("ghs endnowarning 620")
#define IGNORE_BINARY_OPERATOR_OVERLOADING_WARNING_START _Pragma("ghs nowarning 1974")
#define IGNORE_BINARY_OPERATOR_OVERLOADING_WARNING_END _Pragma("ghs endnowarning 1974")
#define IGNORE_MISSING_MACRO_ARGUMENT_WARNING_START _Pragma("ghs nowarning 76")
#define IGNORE_MISSING_MACRO_ARGUMENT_WARNING_END  _Pragma("ghs endnowarning 76")
#define IGNORE_IMPLICIT_RETURN_FROM_NON_VOID_FUNCTION_START  _Pragma("ghs nowarning 940")
#define IGNORE_IMPLICIT_RETURN_FROM_NON_VOID_FUNCTION_END  _Pragma("ghs endnowarning 940")
#define IGNORE_STORAGE_CLASS_NOT_FIRST_START  _Pragma("ghs nowarning 82")
#define IGNORE_STORAGE_CLASS_NOT_FIRST_END  _Pragma("ghs endnowarning 82")
#define IGNORE_UNREACHABLE_STATEMENT_START  _Pragma("ghs nowarning 111")
#define IGNORE_UNREACHABLE_STATEMENT_END  _Pragma("ghs endnowarning 111")
#else
#define IGNORE_UNNAMED_MEMBER_WARNING_START
#define IGNORE_UNNAMED_MEMBER_WARNING_END
#define IGNORE_BINARY_OPERATOR_OVERLOADING_WARNING_START
#define IGNORE_BINARY_OPERATOR_OVERLOADING_WARNING_END
#define IGNORE_MISSING_MACRO_ARGUMENT_WARNING_START
#define IGNORE_MISSING_MACRO_ARGUMENT_WARNING_END
#define IGNORE_IMPLICIT_RETURN_FROM_NON_VOID_FUNCTION_START
#define IGNORE_IMPLICIT_RETURN_FROM_NON_VOID_FUNCTION_END
#define IGNORE_STORAGE_CLASS_NOT_FIRST_START
#define IGNORE_STORAGE_CLASS_NOT_FIRST_END
#define IGNORE_UNREACHABLE_STATEMENT_START
#define IGNORE_UNREACHABLE_STATEMENT_END
#endif


#endif
