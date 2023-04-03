//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses-logic/ELogMessageType.h"
#include "ramses-logic/Logger.h"

#include "fmt/format.h"

#ifdef __ANDROID__
#include <android/log.h>
#else
#include <iostream>
#endif

#define LOG_FATAL(...) \
internal::LoggerImpl::GetInstance().log(rlogic::ELogMessageType::Fatal, __VA_ARGS__)

#define LOG_ERROR(...) \
internal::LoggerImpl::GetInstance().log(rlogic::ELogMessageType::Error, __VA_ARGS__)

#define LOG_WARN(...) \
internal::LoggerImpl::GetInstance().log(rlogic::ELogMessageType::Warn, __VA_ARGS__)

#define LOG_INFO(...) \
internal::LoggerImpl::GetInstance().log(rlogic::ELogMessageType::Info, __VA_ARGS__)

#define LOG_DEBUG(...) \
internal::LoggerImpl::GetInstance().log(rlogic::ELogMessageType::Debug, __VA_ARGS__)

#define LOG_TRACE(...) \
internal::LoggerImpl::GetInstance().log(rlogic::ELogMessageType::Trace, __VA_ARGS__)

namespace rlogic::internal
{
    static inline const char* GetLogMessageTypeString(ELogMessageType type)
    {
        switch (type)
        {
        case ELogMessageType::Off:
            assert(false && "Should never call this!");
            return "";
        case ELogMessageType::Fatal:
            return "FATAL ";
        case ELogMessageType::Error:
            return "ERROR";
        case ELogMessageType::Warn:
            return "WARN ";
        case ELogMessageType::Info:
            return "INFO ";
        case ELogMessageType::Debug:
            return "DEBUG";
        case ELogMessageType::Trace:
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
        void log(ELogMessageType messageType, const char(&fmtString)[N], const ARGS&... args);
        template <typename... ARGS>
        void log(ELogMessageType messageType, std::string_view fmtString, const ARGS&... args);

        void setLogVerbosityLimit(ELogMessageType verbosityLimit);
        [[nodiscard]] ELogMessageType getLogVerbosityLimit() const;
        void setLogHandler(Logger::LogHandlerFunc logHandlerFunc);
        void setDefaultLogging(bool loggingEnabled);

        static LoggerImpl& GetInstance();

    private:
        LoggerImpl() noexcept;

        [[nodiscard]] bool logMessageExceedsVerbosityLimit(ELogMessageType messageType) const
        {
            return (messageType > m_logVerbosityLimit);
        }

        static void PrintLogMessage(ELogMessageType messageType, const std::string& message);

        Logger::LogHandlerFunc m_logHandler;
        bool                   m_defaultLogging = true;

        ELogMessageType m_logVerbosityLimit = ELogMessageType::Info;

    };

    // Note: we are forcing here format string to be literal to avoid issues.
    // Otherwise a generated string could potentially contain content from user application
    // (e.g. name, script code, malicious or invalid file) which if passed
    // to Fmt directly as format string could cause undesired behavior (e.g. if contains curly brackets).
    template <size_t N, typename... ARGS>
    // NOLINTNEXTLINE(modernize-avoid-c-arrays) need type representing string literals
    inline void LoggerImpl::log(ELogMessageType messageType, const char(&fmtString)[N], const ARGS&... args)
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
    void LoggerImpl::log(ELogMessageType /*messageType*/, std::string_view /*fmtString*/, const ARGS&... /*args*/)
    {
        // See comment above for reasons
        static_assert(TemplatedFalse<ARGS...>::value, "Always use literal as format string when logging, e.g. 'LOG_ERROR(\"{}\", errorMsg)'");
    }

    inline void LoggerImpl::PrintLogMessage(ELogMessageType messageType, const std::string& message)
    {
#ifdef __ANDROID__

        android_LogPriority logLevel;

        switch (messageType)
        {
        case ELogMessageType::Trace:
            logLevel = ANDROID_LOG_VERBOSE;
            break;
        case ELogMessageType::Debug:
            logLevel = ANDROID_LOG_DEBUG;
            break;
        case ELogMessageType::Info:
            logLevel = ANDROID_LOG_INFO;
            break;
        case ELogMessageType::Warn:
            logLevel = ANDROID_LOG_WARN;
            break;
        case ELogMessageType::Error:
            logLevel = ANDROID_LOG_ERROR;
            break;
        case ELogMessageType::Fatal:
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
