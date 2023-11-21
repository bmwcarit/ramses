//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

// API
#include "ramses/framework/RamsesFrameworkTypes.h"

//utils
#include "internal/Core/Utils/LogMacros.h"

#define LOG_API_LOGLEVEL LOG_TRACE

#ifdef _WIN32
#define CLASS_FUNCTION_NAME __FUNCTION__
#else
#define CLASS_FUNCTION_NAME __PRETTY_FUNCTION__
#endif

#define LOG_API_VOID "void"

#define LOG_API_GENERIC_PTR_STRING(PTR) \
        static_cast<const void*>(PTR)
#define LOG_API_GENERIC_OBJECT_STRING(OBJ) \
        LOG_API_GENERIC_PTR_STRING(&(OBJ))

// For now log RamsesObjects just as pointer address
#define LOG_API_RAMSESOBJECT_PTR_STRING(PTR) \
        LOG_API_GENERIC_PTR_STRING(static_cast<const ramses::RamsesObject*>(PTR))
#define LOG_API_RAMSESOBJECT_STRING(OBJ) \
        LOG_API_RAMSESOBJECT_PTR_STRING(&(OBJ))

#define LOG_API_RESOURCE_PTR_STRING(PTR) \
    ::fmt::format("{} with resID {}", LOG_API_RAMSESOBJECT_PTR_STRING(PTR), (PTR) ? fmt::to_string((PTR)->getResourceId()) : "<invalid>")

#define LOG_API_SEPERATOR(arg1, arg2) \
        ::fmt::format("{} ; {}", arg1, arg2)

#define LOG_API_BASE(retval) \
        ::fmt::format("{} at {} ret {}", CLASS_FUNCTION_NAME, LOG_API_GENERIC_PTR_STRING(this), retval)
#define LOG_STATIC_API_BASE(retval) \
        ::fmt::format("{} ret {}", CLASS_FUNCTION_NAME, retval)
#define LOG_API_ARGUMENTS(arguments) \
        ::fmt::format(" ( args: {} )", arguments)

// Client side macros
#define LOG_HL_CLIENT_API_STR(...) \
        LOG_API_LOGLEVEL(ramses::internal::CONTEXT_HLAPI_CLIENT, __VA_ARGS__)

#define LOG_HL_CLIENT_API_NOARG(retval) \
        LOG_HL_CLIENT_API_STR(LOG_API_BASE(retval))
#define LOG_HL_CLIENT_API1(retval, arg1) \
        LOG_HL_CLIENT_API_STR(::fmt::format("{}{}", LOG_API_BASE(retval), LOG_API_ARGUMENTS(arg1)))
#define LOG_HL_CLIENT_API2(retval, arg1, arg2) \
        LOG_HL_CLIENT_API1(retval, LOG_API_SEPERATOR(arg1, arg2))
#define LOG_HL_CLIENT_API3(retval, arg1, arg2, arg3) \
        LOG_HL_CLIENT_API2(retval, arg1, LOG_API_SEPERATOR(arg2, arg3))
#define LOG_HL_CLIENT_API4(retval, arg1, arg2, arg3, arg4) \
        LOG_HL_CLIENT_API3(retval, arg1, arg2, LOG_API_SEPERATOR(arg3, arg4))
#define LOG_HL_CLIENT_API5(retval, arg1, arg2, arg3, arg4, arg5) \
        LOG_HL_CLIENT_API4(retval, arg1, arg2, arg3, LOG_API_SEPERATOR(arg4, arg5))
#define LOG_HL_CLIENT_API6(retval, arg1, arg2, arg3, arg4, arg5, arg6) \
        LOG_HL_CLIENT_API5(retval, arg1, arg2, arg3, arg4, LOG_API_SEPERATOR(arg5, arg6))
#define LOG_HL_CLIENT_API7(retval, arg1, arg2, arg3, arg4, arg5, arg6, arg7) \
        LOG_HL_CLIENT_API6(retval, arg1, arg2, arg3, arg4, arg5, LOG_API_SEPERATOR(arg6, arg7))
#define LOG_HL_CLIENT_API8(retval, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8) \
        LOG_HL_CLIENT_API7(retval, arg1, arg2, arg3, arg4, arg5, arg6, LOG_API_SEPERATOR(arg7, arg8))
#define LOG_HL_CLIENT_API9(retval, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9) \
        LOG_HL_CLIENT_API8(retval, arg1, arg2, arg3, arg4, arg5, arg6, arg7, LOG_API_SEPERATOR(arg8, arg9))
#define LOG_HL_CLIENT_API10(retval, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10) \
        LOG_HL_CLIENT_API9 (retval, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, LOG_API_SEPERATOR(arg9, arg10))
#define LOG_HL_CLIENT_API11(retval, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11) \
        LOG_HL_CLIENT_API10(retval, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, LOG_API_SEPERATOR(arg10, arg11))

#define LOG_HL_CLIENT_STATIC_API1(retval, arg1) \
        LOG_HL_CLIENT_API_STR(::fmt::format("{}{}", LOG_STATIC_API_BASE(retval), LOG_API_ARGUMENTS(arg1)))
#define LOG_HL_CLIENT_STATIC_API2(retval, arg1, arg2) \
        LOG_HL_CLIENT_STATIC_API1(retval, LOG_API_SEPERATOR(arg1, arg2))
#define LOG_HL_CLIENT_STATIC_API3(retval, arg1, arg2, arg3) \
        LOG_HL_CLIENT_STATIC_API2(retval, arg1, LOG_API_SEPERATOR(arg2, arg3))
#define LOG_HL_CLIENT_STATIC_API4(retval, arg1, arg2, arg3, arg4) \
        LOG_HL_CLIENT_STATIC_API3(retval, arg1, arg2, LOG_API_SEPERATOR(arg3, arg4))
#define LOG_HL_CLIENT_STATIC_API5(retval, arg1, arg2, arg3, arg4, arg5) \
        LOG_HL_CLIENT_STATIC_API4(retval, arg1, arg2, arg3, LOG_API_SEPERATOR(arg4, arg5))

// Renderer side macros
#define LOG_HL_RENDERER_API_STR(str) \
        LOG_API_LOGLEVEL(ramses::internal::CONTEXT_HLAPI_RENDERER, str)

#define LOG_HL_RENDERER_API_NOARG(retval) \
        LOG_HL_RENDERER_API_STR(LOG_API_BASE(retval))
#define LOG_HL_RENDERER_API1(retval, arg1) \
        LOG_HL_RENDERER_API_STR(::fmt::format("{}{}", LOG_API_BASE(retval), LOG_API_ARGUMENTS(arg1)))
#define LOG_HL_RENDERER_API2(retval, arg1, arg2) \
        LOG_HL_RENDERER_API1(retval, LOG_API_SEPERATOR(arg1, arg2))
#define LOG_HL_RENDERER_API3(retval, arg1, arg2, arg3) \
        LOG_HL_RENDERER_API2(retval, arg1, LOG_API_SEPERATOR(arg2, arg3))
#define LOG_HL_RENDERER_API4(retval, arg1, arg2, arg3, arg4) \
        LOG_HL_RENDERER_API3(retval, arg1, arg2, LOG_API_SEPERATOR(arg3, arg4))
#define LOG_HL_RENDERER_API5(retval, arg1, arg2, arg3, arg4, arg5) \
        LOG_HL_RENDERER_API4(retval, arg1, arg2, arg3, LOG_API_SEPERATOR(arg4, arg5))
#define LOG_HL_RENDERER_API6(retval, arg1, arg2, arg3, arg4, arg5, arg6) \
        LOG_HL_RENDERER_API5(retval, arg1, arg2, arg3, arg4, LOG_API_SEPERATOR(arg5, arg6))
#define LOG_HL_RENDERER_API7(retval, arg1, arg2, arg3, arg4, arg5, arg6, arg7) \
        LOG_HL_RENDERER_API6(retval, arg1, arg2, arg3, arg4, arg5, LOG_API_SEPERATOR(arg6, arg7))
#define LOG_HL_RENDERER_API8(retval, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8) \
        LOG_HL_RENDERER_API7(retval, arg1, arg2, arg3, arg4, arg5, arg6, LOG_API_SEPERATOR(arg7, arg8))
#define LOG_HL_RENDERER_API9(retval, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9) \
        LOG_HL_RENDERER_API8(retval, arg1, arg2, arg3, arg4, arg5, arg6, arg7, LOG_API_SEPERATOR(arg8, arg9))

#define LOG_HL_RENDERER_STATIC_API1(retval, arg1) \
        LOG_HL_RENDERER_API_STR(::fmt::format("{}{}", LOG_STATIC_API_BASE(retval), LOG_API_ARGUMENTS(arg1)))
#define LOG_HL_RENDERER_STATIC_API2(retval, arg1, arg2) \
        LOG_HL_RENDERER_STATIC_API1(retval, LOG_API_SEPERATOR(arg1, arg2))
#define LOG_HL_RENDERER_STATIC_API3(retval, arg1, arg2, arg3) \
        LOG_HL_RENDERER_STATIC_API2(retval, arg1, LOG_API_SEPERATOR(arg2, arg3))
#define LOG_HL_RENDERER_STATIC_API4(retval, arg1, arg2, arg3, arg4) \
        LOG_HL_RENDERER_STATIC_API3(retval, arg1, arg2, LOG_API_SEPERATOR(arg3, arg4))
#define LOG_HL_RENDERER_STATIC_API5(retval, arg1, arg2, arg3, arg4, arg5) \
        LOG_HL_RENDERER_STATIC_API4(retval, arg1, arg2, arg3, LOG_API_SEPERATOR(arg4, arg5))
