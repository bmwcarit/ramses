//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_THREADLOCALLOG_H
#define RAMSES_THREADLOCALLOG_H

#include "Utils/LogMacros.h"

namespace ramses_internal
{
    class ThreadLocalLog
    {
    public:
        static void SetPrefix(int prefixId);
        static int GetPrefix();
        static int GetPrefixUnchecked();
    };
}

#define LOG_COMMON_R(context, logLevel, message) \
    LOG_COMMON((context), logLevel, ::ramses_internal::ThreadLocalLog::GetPrefix() << ": " << message)

#define LOG_TRACE_R(context, message) \
    LOG_COMMON_R(context, ::ramses_internal::ELogLevel::Trace, message)

#define LOG_DEBUG_R(context, message) \
    LOG_COMMON_R(context, ::ramses_internal::ELogLevel::Debug, message)

#define LOG_INFO_R(context, message) \
    LOG_COMMON_R(context, ::ramses_internal::ELogLevel::Info, message)

#define LOG_WARN_R(context, message) \
    LOG_COMMON_R(context, ::ramses_internal::ELogLevel::Warn, message)

#define LOG_ERROR_R(context, message) \
    LOG_COMMON_R(context, ::ramses_internal::ELogLevel::Error, message)

#define LOG_FATAL_R(context, message) \
    LOG_COMMON_R(context, ::ramses_internal::ELogLevel::Fatal, message)


#define LOG_COMMON_RP(context, logLevel, ...) \
    LOG_COMMON_P((context), logLevel, ::fmt::format("{}: {}", ::ramses_internal::ThreadLocalLog::GetPrefix(), ::fmt::format(__VA_ARGS__)))

#define LOG_TRACE_RP(context, ...) \
    LOG_COMMON_RP(context, ::ramses_internal::ELogLevel::Trace, __VA_ARGS__)

#define LOG_DEBUG_RP(context, ...) \
    LOG_COMMON_RP(context, ::ramses_internal::ELogLevel::Debug, __VA_ARGS__)

#define LOG_INFO_RP(context, ...) \
    LOG_COMMON_RP(context, ::ramses_internal::ELogLevel::Info, __VA_ARGS__)

#define LOG_WARN_RP(context, ...) \
    LOG_COMMON_RP(context, ::ramses_internal::ELogLevel::Warn, __VA_ARGS__)

#define LOG_ERROR_RP(context, ...) \
    LOG_COMMON_RP(context, ::ramses_internal::ELogLevel::Error, __VA_ARGS__)

#define LOG_FATAL_RP(context, ...) \
    LOG_COMMON_RP(context, ::ramses_internal::ELogLevel::Fatal, __VA_ARGS__)


#define LOG_COMMON_RF(context, logLevel, callable)                                       \
    do {                                                                                 \
        if((logLevel) <= (context).getLogLevel())                                        \
        {                                                                                \
            ::ramses_internal::StringOutputStream ramses_log_stream(160);                \
            ramses_log_stream << ::ramses_internal::ThreadLocalLog::GetPrefix() << ": "; \
            callable(ramses_log_stream);                                                 \
            ::ramses_internal::GetRamsesLogger().log(::ramses_internal::LogMessage((context), (logLevel), ramses_log_stream)); \
        }                                                                                \
    } while (0)

#define LOG_TRACE_RF(context, callable) \
    LOG_COMMON_RF((context), ::ramses_internal::ELogLevel::Trace, callable)

#define LOG_INFO_RF(context, callable) \
    LOG_COMMON_RF((context), ::ramses_internal::ELogLevel::Info, callable)

#define LOG_DEBUG_RF(context, callable) \
    LOG_COMMON_RF((context), ::ramses_internal::ELogLevel::Debug, callable)

#define LOG_WARN_RF(context, callable) \
    LOG_COMMON_RF((context), ::ramses_internal::ELogLevel::Warn, callable)

#define LOG_ERROR_RF(context, callable) \
    LOG_COMMON_RF((context), ::ramses_internal::ELogLevel::Error, callable)

#define LOG_FATAL_RF(context, callable) \
    LOG_COMMON_RF((context), ::ramses_internal::ELogLevel::Fatal, callable)


#define LOG_COMMON_RPF(context, logLevel, callable)                \
    do {                                                           \
        if ((logLevel) <= (context).getLogLevel())                 \
        {                                                          \
            fmt::memory_buffer ramses_fmtlib_buffer;               \
            fmt::format_to(ramses_fmtlib_buffer, "{}: ", ::ramses_internal::ThreadLocalLog::GetPrefix()); \
            callable(ramses_fmtlib_buffer);                        \
            ::ramses_internal::GetRamsesLogger().log(::ramses_internal::LogMessage((context), (logLevel), ::ramses_internal::StringOutputStream(fmt::to_string(ramses_fmtlib_buffer)))); \
        }                                                          \
    } while (0)

#define LOG_TRACE_RPF(context, callable)                                 \
    LOG_COMMON_RPF((context), ::ramses_internal::ELogLevel::Trace, callable)

#define LOG_INFO_RPF(context, callable)                                  \
    LOG_COMMON_RPF((context), ::ramses_internal::ELogLevel::Info, callable)

#define LOG_DEBUG_RPF(context, callable)                                 \
    LOG_COMMON_RPF((context), ::ramses_internal::ELogLevel::Debug, callable)

#define LOG_WARN_RPF(context, callable)                                  \
    LOG_COMMON_RPF((context), ::ramses_internal::ELogLevel::Warn, callable)

#define LOG_ERROR_RPF(context, callable)                                 \
    LOG_COMMON_RPF((context), ::ramses_internal::ELogLevel::Error, callable)

#define LOG_FATAL_RPF(context, callable)                                 \
    LOG_COMMON_RPF((context), ::ramses_internal::ELogLevel::Fatal, callable)

#endif
