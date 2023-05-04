//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "ramses-logic/Logger.h"

#include "fmt/format.h"
#include <cassert>

#ifdef __ANDROID__
#include <android/log.h>
#else
#include <iostream>
#endif

#define LOG_FATAL(...) \
internal::LoggerImpl::GetInstance().log(ramses::ELogLevel::Fatal, __VA_ARGS__)

#define LOG_ERROR(...) \
internal::LoggerImpl::GetInstance().log(ramses::ELogLevel::Error, __VA_ARGS__)

#define LOG_WARN(...) \
internal::LoggerImpl::GetInstance().log(ramses::ELogLevel::Warn, __VA_ARGS__)

#define LOG_INFO(...) \
internal::LoggerImpl::GetInstance().log(ramses::ELogLevel::Info, __VA_ARGS__)

#define LOG_DEBUG(...) \
internal::LoggerImpl::GetInstance().log(ramses::ELogLevel::Debug, __VA_ARGS__)

#define LOG_TRACE(...) \
internal::LoggerImpl::GetInstance().log(ramses::ELogLevel::Trace, __VA_ARGS__)

namespace ramses::internal
{
    static inline const char* GetLogMessageTypeString(ELogLevel type)
    {
        switch (type)
        {
        case ELogLevel::Off:
            assert(false && "Should never call this!");
            return "";
        case ELogLevel::Fatal:
            return "FATAL ";
        case ELogLevel::Error:
            return "ERROR";
        case ELogLevel::Warn:
            return "WARN ";
        case ELogLevel::Info:
            return "INFO ";
        case ELogLevel::Debug:
            return "DEBUG";
        case ELogLevel::Trace:
            return "TRACE";
        }
        assert(false);
        return "INFO ";
    }

    class LoggerImpl
    {
    public:
        ~LoggerImpl() noexcept = default;
        LoggerImpl(const LoggerImpl& other) = delete;
        LoggerImpl(LoggerImpl&& other) = delete;
        LoggerImpl& operator=(const LoggerImpl& other) = delete;
        LoggerImpl& operator=(LoggerImpl&& other) = delete;

        template <size_t N, typename... ARGS>
        // NOLINTNEXTLINE(modernize-avoid-c-arrays) need type representing string literals
        void log(ELogLevel messageType, const char(&fmtString)[N], const ARGS&... args);
        template <typename... ARGS>
        void log(ELogLevel messageType, std::string_view fmtString, const ARGS&... args);

        void setLogVerbosityLimit(ELogLevel verbosityLimit);
        [[nodiscard]] ELogLevel getLogVerbosityLimit() const;
        void setLogHandler(Logger::LogHandlerFunc logHandlerFunc);
        void setDefaultLogging(bool loggingEnabled);

        static LoggerImpl& GetInstance();

    private:
        LoggerImpl() noexcept;

        [[nodiscard]] bool logMessageExceedsVerbosityLimit(ELogLevel messageType) const
        {
            return (messageType > m_logVerbosityLimit);
        }

        static void PrintLogMessage(ELogLevel messageType, const std::string& message);

        Logger::LogHandlerFunc m_logHandler;
        bool                   m_defaultLogging = true;

        ELogLevel m_logVerbosityLimit = ELogLevel::Info;

    };

    // Note: we are forcing here format string to be literal to avoid issues.
    // Otherwise a generated string could potentially contain content from user application
    // (e.g. name, script code, malicious or invalid file) which if passed
    // to Fmt directly as format string could cause undesired behavior (e.g. if contains curly brackets).
    template <size_t N, typename... ARGS>
    // NOLINTNEXTLINE(modernize-avoid-c-arrays) need type representing string literals
    inline void LoggerImpl::log(ELogLevel messageType, const char(&fmtString)[N], const ARGS&... args)
    {
        // Early exit if log level exceeded, or no logger configured
        if (logMessageExceedsVerbosityLimit(messageType) || (!m_defaultLogging && !m_logHandler))
        {
            return;
        }

        const std::string formattedMessage = fmt::format(fmtString, args...);
        if (m_defaultLogging)
        {
            PrintLogMessage(messageType, formattedMessage);
        }
        if (nullptr != m_logHandler)
        {
            m_logHandler(messageType, formattedMessage);
        }
    }

    // workaround to make static assert below dependent on template argument
    template <typename... ARGS>
    struct TemplatedFalse : std::false_type {};

    template <typename... ARGS>
    void LoggerImpl::log(ELogLevel /*messageType*/, std::string_view /*fmtString*/, const ARGS&... /*args*/)
    {
        // See comment above for reasons
        static_assert(TemplatedFalse<ARGS...>::value, "Always use literal as format string when logging, e.g. 'LOG_ERROR(\"{}\", errorMsg)'");
    }

    inline void LoggerImpl::PrintLogMessage(ELogLevel messageType, const std::string& message)
    {
#ifdef __ANDROID__

        android_LogPriority logLevel;

        switch (messageType)
        {
        case ELogLevel::Trace:
            logLevel = ANDROID_LOG_VERBOSE;
            break;
        case ELogLevel::Debug:
            logLevel = ANDROID_LOG_DEBUG;
            break;
        case ELogLevel::Info:
            logLevel = ANDROID_LOG_INFO;
            break;
        case ELogLevel::Warn:
            logLevel = ANDROID_LOG_WARN;
            break;
        case ELogLevel::Error:
            logLevel = ANDROID_LOG_ERROR;
            break;
        case ELogLevel::Fatal:
            logLevel = ANDROID_LOG_FATAL;
            break;
        default:
            logLevel = ANDROID_LOG_UNKNOWN;
            break;
        }

        __android_log_print(logLevel, "Ramses.Logic", "%s", message.c_str());
#else
        std::cout << "[ " << GetLogMessageTypeString(messageType) << " ] " << message << std::endl;
#endif
    }

    inline LoggerImpl& LoggerImpl::GetInstance()
    {
        static LoggerImpl logger;
        return logger;
    }
}
