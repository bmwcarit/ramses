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

namespace ramses_internal
{
    extern LogContext& CONTEXT_FRAMEWORK;
    extern LogContext& CONTEXT_CLIENT;
    extern LogContext& CONTEXT_RENDERER;
    extern LogContext& CONTEXT_PERIODIC;
    extern LogContext& CONTEXT_DCSM;

    extern LogContext& CONTEXT_COMMUNICATION;
    extern LogContext& CONTEXT_PROFILING;

    extern LogContext& CONTEXT_HLAPI_CLIENT;
    extern LogContext& CONTEXT_HLAPI_RENDERER;

    extern LogContext& CONTEXT_RAMSH;
    extern LogContext& CONTEXT_TEST;
    extern LogContext& CONTEXT_SMOKETEST;
    extern LogContext& CONTEXT_DLT_KEEP_ALIVE;
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

// LOG_* macros for leg message via callable
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

#endif
