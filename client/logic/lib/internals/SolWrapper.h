//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once


#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wdeprecated"
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#pragma GCC diagnostic ignored "-Wcast-align"
#pragma GCC diagnostic ignored "-Wempty-body"
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wsuggest-override"
#endif

#if defined(_MSC_VER)
__pragma(warning(push))
__pragma(warning(disable: 4100))
#endif

// TODO (Violin) Workaround for a compiler issue fixed in newer sol. Remove once upgraded
#include <limits>
#include "sol/sol.hpp"

#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif

#if defined(_MSC_VER)
__pragma(warning(pop))
#endif

// Make sure sol defines are configured correctly and work uniformly on all compile units which use Sol
// Uses Sol-internal macros to ensure the exact same compiler output, independent of user-configured settings
// Write a comment in the error message why we need (or don't need) the corresponding safety setting

#if SOL_IS_OFF(SOL_EXCEPTIONS_I_)
#error "We use the default setting (ON)"
#endif

#if SOL_IS_OFF(SOL_RTTI_I_)
#error "We use the default setting (ON)"
#endif

#if SOL_IS_OFF(SOL_USE_THREAD_LOCAL_I_)
#error "We use the default setting (ON) - none of the platforms supported by Logic engine should be affected by this"
#endif

#if SOL_IS_ON(SOL_SAFE_USERTYPE_I_)
#error "We use the default setting (OFF)"
#endif

#if SOL_IS_ON(SOL_SAFE_REFERENCES_I_)
#error "We use the default setting (OFF)"
#endif

#if SOL_IS_ON(SOL_SAFE_FUNCTION_CALLS_I_)
#error "We use the default setting (OFF)"
#endif

#if SOL_IS_ON(SOL_SAFE_FUNCTION_OBJECTS_I_)
#error "We use the default setting (OFF)"
#endif

#if SOL_IS_OFF(SOL_SAFE_NUMERICS_I_)
#error "Safe numerics are required so that Lua/Sol numbers behave correctly in the logic engine!"
#endif

#if SOL_IS_ON(SOL_SAFE_GETTER_I_)
#error "We use the default setting (OFF)"
#endif

// Configured with SOL_SAFE_STACK_CHECK=1
// this is not documented in Sol, but it looks like it should be
#if SOL_IS_OFF(SOL_SAFE_STACK_CHECK_I_)
#error "Safe stack checks are required so that Lua stack grows correctly when used!"
#endif

#if SOL_IS_ON(SOL_USE_CXX_LUAJIT_I_)
#error "LuaJIT is not tested yet, thus we expect that SOL_USE_CXX_LUAJIT is off currently"
#endif

#if SOL_IS_ON(SOL_USE_LUA_HPP_I_)
#error "We use the default setting (OFF)"
#endif

// Configured with SOL_EXCEPTIONS_ALWAYS_UNSAFE=1
#if SOL_IS_ON(SOL_PROPAGATE_EXCEPTIONS_I_)
#error "We want to catch and redirect exceptions and pass to to user handler func instead of prapagating them directly through Lua"
#endif

#if SOL_IS_OFF(SOL_ALIGN_MEMORY_I_)
#error "We use the default setting (ON)"
#endif

#if SOL_IS_OFF(SOL_USE_COMPATIBILITY_LAYER_I_)
#error "We run in compatibility mode (because of Lua v5.1)"
#endif

#if SOL_IS_ON(SOL_GET_FUNCTION_POINTER_UNSAFE_I_)
#error "We use the default setting (OFF)"
#endif

#if SOL_IS_OFF(SOL_USE_NOEXCEPT_FUNCTION_TYPE_I_)
#error "We use the default setting (ON)"
#endif

#if SOL_IS_OFF(SOL_STD_VARIANT_I_)
#error "We use the default setting (ON)"
#endif

#if SOL_IS_OFF(SOL_DEFAULT_AUTOMAGICAL_USERTYPES_I_)
#error "We use the default setting (ON)"
#endif

#if SOL_IS_OFF(SOL_USERTYPE_TYPE_BINDING_INFO_I_)
#error "We use the default setting (ON)"
#endif

#if SOL_IS_ON(SOL_STRINGS_ARE_NUMBERS_I_)
#error "We use the default setting (OFF)"
#endif

#if SOL_IS_OFF(SOL_NUMBER_PRECISION_CHECKS_I_)
#error "We use the default setting (turned on automatically when SOL_SAFE_NUMERICS is ON)"
#endif

// this is not documented in Sol, but it looks like it should be
#if SOL_IS_ON(SOL_SAFE_PROXIES_I_)
#error "We use the default setting (OFF)"
#endif



