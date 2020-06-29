//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_LOGMACROS_H
#define RAMSES_LOGMACROS_H

#include "Utils/LogMessage.h"
#include "Utils/LogContext.h"
#include "Utils/RamsesLogger.h"
#include "fmt/format.h"

namespace ramses_internal
{
    extern LogContext& CONTEXT_FRAMEWORK;
    extern LogContext& CONTEXT_CLIENT;
    extern LogContext& CONTEXT_RENDERER;
    extern LogContext& CONTEXT_PERIODIC;
    extern LogContext& CONTEXT_DCSM;
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
    using ::ramses_internal::CONTEXT_FRAMEWORK;
    using ::ramses_internal::CONTEXT_CLIENT;
    using ::ramses_internal::CONTEXT_RENDERER;
    using ::ramses_internal::CONTEXT_PERIODIC;
    using ::ramses_internal::CONTEXT_DCSM;
    using ::ramses_internal::CONTEXT_TEXT;
    using ::ramses_internal::CONTEXT_COMMUNICATION;
    using ::ramses_internal::CONTEXT_PROFILING;
    using ::ramses_internal::CONTEXT_HLAPI_CLIENT;
    using ::ramses_internal::CONTEXT_HLAPI_RENDERER;
    using ::ramses_internal::CONTEXT_RAMSH;
    using ::ramses_internal::CONTEXT_SMOKETEST;
}

// LOG_* macros for log message in macro
#define LOG_COMMON(context, logLevel, message)                             \
    do {                                                                   \
        if((logLevel) <= (context).getLogLevel())                          \
        {                                                                  \
            ::ramses_internal::StringOutputStream ramses_log_stream(80);   \
            ramses_log_stream << message;                                  \
            ::ramses_internal::GetRamsesLogger().log(::ramses_internal::LogMessage((context), (logLevel), ramses_log_stream)); \
        }                                                                  \
    } while (0)

#define LOG_TRACE(context, message) \
    LOG_COMMON((context), ::ramses_internal::ELogLevel::Trace, message)

#define LOG_INFO(context, message) \
    LOG_COMMON((context), ::ramses_internal::ELogLevel::Info, message)

#define LOG_DEBUG(context, message) \
    LOG_COMMON((context), ::ramses_internal::ELogLevel::Debug, message)

#define LOG_WARN(context, message) \
    LOG_COMMON((context), ::ramses_internal::ELogLevel::Warn, message)

#define LOG_ERROR(context, message) \
    LOG_COMMON((context), ::ramses_internal::ELogLevel::Error, message)

#define LOG_FATAL(context, message) \
    LOG_COMMON((context), ::ramses_internal::ELogLevel::Fatal, message)

// LOG_* macros for log message via printf syntax
#define LOG_COMMON_P(context, logLevel,  ...)            \
    do {                                                                   \
        if((logLevel) <= (context).getLogLevel())                          \
            ::ramses_internal::GetRamsesLogger().log(::ramses_internal::LogMessage((context), (logLevel), ::ramses_internal::StringOutputStream(::fmt::format(__VA_ARGS__)))); \
    } while (0)

#define LOG_TRACE_P(context, ...)                                \
    LOG_COMMON_P((context), ::ramses_internal::ELogLevel::Trace, __VA_ARGS__)

#define LOG_INFO_P(context, ...)                                 \
    LOG_COMMON_P((context), ::ramses_internal::ELogLevel::Info, __VA_ARGS__)

#define LOG_DEBUG_P(context, ...)                                \
    LOG_COMMON_P((context), ::ramses_internal::ELogLevel::Debug, __VA_ARGS__)

#define LOG_WARN_P(context, ...)                                 \
    LOG_COMMON_P((context), ::ramses_internal::ELogLevel::Warn, __VA_ARGS__)

#define LOG_ERROR_P(context, ...)                                \
    LOG_COMMON_P((context), ::ramses_internal::ELogLevel::Error, __VA_ARGS__)

#define LOG_FATAL_P(context, ...)                                \
    LOG_COMMON_P((context), ::ramses_internal::ELogLevel::Fatal, __VA_ARGS__)

// LOG_* macros for log message via callable
#define LOG_COMMON_F(context, logLevel, callable)                          \
    do {                                                                   \
        if((logLevel) <= (context).getLogLevel())                          \
        {                                                                  \
            ::ramses_internal::StringOutputStream ramses_log_stream(160);  \
            callable(ramses_log_stream);                                   \
            ::ramses_internal::GetRamsesLogger().log(::ramses_internal::LogMessage((context), (logLevel), ramses_log_stream)); \
        }                                                                  \
    } while (0)

#define LOG_TRACE_F(context, callable) \
    LOG_COMMON_F((context), ::ramses_internal::ELogLevel::Trace, callable)

#define LOG_INFO_F(context, callable) \
    LOG_COMMON_F((context), ::ramses_internal::ELogLevel::Info, callable)

#define LOG_DEBUG_F(context, callable) \
    LOG_COMMON_F((context), ::ramses_internal::ELogLevel::Debug, callable)

#define LOG_WARN_F(context, callable) \
    LOG_COMMON_F((context), ::ramses_internal::ELogLevel::Warn, callable)

#define LOG_ERROR_F(context, callable) \
    LOG_COMMON_F((context), ::ramses_internal::ELogLevel::Error, callable)

#define LOG_FATAL_F(context, callable) \
    LOG_COMMON_F((context), ::ramses_internal::ELogLevel::Fatal, callable)

// LOG_* macros for log message via printf syntax with callable
#define LOG_COMMON_PF(context, logLevel, callable)                                                                                                                             \
    do                                                                                                                                                                         \
    {                                                                                                                                                                          \
        if ((logLevel) <= (context).getLogLevel())                                                                                                                             \
        {                                                                                                                                                                      \
            fmt::memory_buffer ramses_fmtlib_buffer; \
            callable(ramses_fmtlib_buffer); \
            ::ramses_internal::GetRamsesLogger().log(::ramses_internal::LogMessage((context), (logLevel), ::ramses_internal::StringOutputStream(fmt::to_string(ramses_fmtlib_buffer)))); \
        }                                                                                                                                                                      \
    } while (0)

#define LOG_TRACE_PF(context, callable)                                 \
    LOG_COMMON_PF((context), ::ramses_internal::ELogLevel::Trace, callable)

#define LOG_INFO_PF(context, callable)                                  \
    LOG_COMMON_PF((context), ::ramses_internal::ELogLevel::Info, callable)

#define LOG_DEBUG_PF(context, callable)                                 \
    LOG_COMMON_PF((context), ::ramses_internal::ELogLevel::Debug, callable)

#define LOG_WARN_PF(context, callable)                                  \
    LOG_COMMON_PF((context), ::ramses_internal::ELogLevel::Warn, callable)

#define LOG_ERROR_PF(context, callable)                                 \
    LOG_COMMON_PF((context), ::ramses_internal::ELogLevel::Error, callable)

#define LOG_FATAL_PF(context, callable)                                 \
    LOG_COMMON_PF((context), ::ramses_internal::ELogLevel::Fatal, callable)


#endif
