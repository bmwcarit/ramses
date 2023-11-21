//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Core/Utils/LogMessage.h"
#include "internal/Core/Utils/LogContext.h"
#include "internal/Core/Utils/RamsesLogger.h"
#include "internal/PlatformAbstraction/Collections/StringOutputStream.h"
#include "internal/PlatformAbstraction/FmtBase.h"

namespace ramses::internal
{
    extern LogContext& CONTEXT_FRAMEWORK;
    extern LogContext& CONTEXT_CLIENT;
    extern LogContext& CONTEXT_RENDERER;
    extern LogContext& CONTEXT_PERIODIC;
    extern LogContext& CONTEXT_TEXT;

    extern LogContext& CONTEXT_COMMUNICATION;
    extern LogContext& CONTEXT_PROFILING;

    extern LogContext& CONTEXT_HLAPI_CLIENT;
    extern LogContext& CONTEXT_HLAPI_RENDERER;

    extern LogContext& CONTEXT_RAMSH;
    extern LogContext& CONTEXT_SMOKETEST;
}

namespace ramses
{
    using ramses::internal::CONTEXT_FRAMEWORK;
    using ramses::internal::CONTEXT_CLIENT;
    using ramses::internal::CONTEXT_RENDERER;
    using ramses::internal::CONTEXT_PERIODIC;
    using ramses::internal::CONTEXT_TEXT;
    using ramses::internal::CONTEXT_COMMUNICATION;
    using ramses::internal::CONTEXT_PROFILING;
    using ramses::internal::CONTEXT_HLAPI_CLIENT;
    using ramses::internal::CONTEXT_HLAPI_RENDERER;
    using ramses::internal::CONTEXT_RAMSH;
    using ramses::internal::CONTEXT_SMOKETEST;
}

// LOG_* macros for log message via printf syntax
#define LOG_COMMON(context, logLevel,  ...)            \
    do {                                                                   \
        if((logLevel) <= (context).getLogLevel())                          \
            ramses::internal::GetRamsesLogger().log(ramses::internal::LogMessage{ (context), (logLevel), ::fmt::format(__VA_ARGS__) }); \
    } while (0)

#define LOG_TRACE(context, ...)                                \
    LOG_COMMON((context), ramses::ELogLevel::Trace, __VA_ARGS__)

#define LOG_INFO(context, ...)                                 \
    LOG_COMMON((context), ramses::ELogLevel::Info, __VA_ARGS__)

#define LOG_DEBUG(context, ...)                                \
    LOG_COMMON((context), ramses::ELogLevel::Debug, __VA_ARGS__)

#define LOG_WARN(context, ...)                                 \
    LOG_COMMON((context), ramses::ELogLevel::Warn, __VA_ARGS__)

#define LOG_ERROR(context, ...)                                \
    LOG_COMMON((context), ramses::ELogLevel::Error, __VA_ARGS__)

#define LOG_FATAL(context, ...)                                \
    LOG_COMMON((context), ramses::ELogLevel::Fatal, __VA_ARGS__)

// LOG_* macros for log message via callable
#define LOG_COMMON_F(context, logLevel, callable)                          \
    do {                                                                   \
        if((logLevel) <= (context).getLogLevel())                          \
        {                                                                  \
            ramses::internal::StringOutputStream ramses_log_stream(160);  \
            callable(ramses_log_stream);                                   \
            ramses::internal::GetRamsesLogger().log(ramses::internal::LogMessage{ (context), (logLevel), ramses_log_stream.release() }); \
        }                                                                  \
    } while (0)

#define LOG_TRACE_F(context, callable) \
    LOG_COMMON_F((context), ramses::ELogLevel::Trace, callable)

#define LOG_INFO_F(context, callable) \
    LOG_COMMON_F((context), ramses::ELogLevel::Info, callable)

#define LOG_DEBUG_F(context, callable) \
    LOG_COMMON_F((context), ramses::ELogLevel::Debug, callable)

#define LOG_WARN_F(context, callable) \
    LOG_COMMON_F((context), ramses::ELogLevel::Warn, callable)

#define LOG_ERROR_F(context, callable) \
    LOG_COMMON_F((context), ramses::ELogLevel::Error, callable)

#define LOG_FATAL_F(context, callable) \
    LOG_COMMON_F((context), ramses::ELogLevel::Fatal, callable)

// LOG_* macros for log message via printf syntax with callable
#define LOG_COMMON_PF(context, logLevel, callable)                                                                                                                             \
    do                                                                                                                                                                         \
    {                                                                                                                                                                          \
        if ((logLevel) <= (context).getLogLevel())                                                                                                                             \
        {                                                                                                                                                                      \
            fmt::memory_buffer ramses_fmtlib_buffer; \
            callable(ramses_fmtlib_buffer); \
            ramses::internal::GetRamsesLogger().log(ramses::internal::LogMessage{ (context), (logLevel), fmt::to_string(ramses_fmtlib_buffer) });                              \
        }                                                                                                                                                                      \
    } while (0)

#define LOG_TRACE_PF(context, callable)                                 \
    LOG_COMMON_PF((context), ramses::ELogLevel::Trace, callable)

#define LOG_INFO_PF(context, callable)                                  \
    LOG_COMMON_PF((context), ramses::ELogLevel::Info, callable)

#define LOG_DEBUG_PF(context, callable)                                 \
    LOG_COMMON_PF((context), ramses::ELogLevel::Debug, callable)

#define LOG_WARN_PF(context, callable)                                  \
    LOG_COMMON_PF((context), ramses::ELogLevel::Warn, callable)

#define LOG_ERROR_PF(context, callable)                                 \
    LOG_COMMON_PF((context), ramses::ELogLevel::Error, callable)

#define LOG_FATAL_PF(context, callable)                                 \
    LOG_COMMON_PF((context), ramses::ELogLevel::Fatal, callable)

