//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Utils/ConsoleLogAppender.h"
#include "Utils/LogMessage.h"
#include "Utils/LogContext.h"
#include "PlatformAbstraction/PlatformTime.h"
#include "PlatformAbstraction/PlatformEnvironmentVariables.h"
#include "PlatformAbstraction/PlatformConsole.h"
#include "fmt/format.h"
#include "fmt/chrono.h"

namespace ramses_internal
{
    ConsoleLogAppender::ConsoleLogAppender()
        : m_callback([]() {})
        , m_colorsEnabled(!PlatformEnvironmentVariables::HasEnvVar("DISABLE_CONSOLE_COLORS"))
    {
    }

    void ConsoleLogAppender::log(const LogMessage& logMessage)
    {
        if(static_cast<int>(logMessage.getLogLevel()) > static_cast<int>(m_logLevel.load()))
            return;

        // TODO(tobias) make static initializer
        Console::EnsureConsoleInitialized();

        const uint64_t now = PlatformTime::GetMillisecondsAbsolute();
        const char* logLevelColor = nullptr;
        const char* logLevelStr = nullptr;

        switch(logMessage.getLogLevel())
        {
        case ELogLevel::Trace:
            logLevelColor = Console::White();
            logLevelStr = "Trace";
            break;
        case ELogLevel::Debug:
            logLevelColor = Console::White();
            logLevelStr = "Debug";
            break;
        case ELogLevel::Info:
            logLevelColor = Console::Green();
            logLevelStr = "Info ";
            break;
        case ELogLevel::Warn:
            logLevelColor = Console::Yellow();
            logLevelStr = "Warn ";
            break;
        case ELogLevel::Error:
            logLevelColor = Console::Red();
            logLevelStr = "Error";
            break;
        case ELogLevel::Fatal:
            logLevelColor = Console::Red();
            logLevelStr = "Fatal";
            break;
        default:
            logLevelColor = Console::Red();
            logLevelStr = "?????";
            break;
        }

        struct tm posix_tm;
        time_t posix_time = static_cast<time_t>(now / 1000);
        // use thread-safe localtime instead of std::localtime that returns internal shared buffer
#ifdef _MSC_VER
        localtime_s(&posix_tm, &posix_time);
#else
        localtime_r(&posix_time, &posix_tm);
#endif

        if (m_colorsEnabled)
            fmt::print("{}{:%Y%m%d-%H:%M:%S}.{:03}{} | {}{}{} | {}{}{} | {}\n",
                       Console::White(), posix_tm, now % 1000, Console::Default(),
                       logLevelColor, logLevelStr, Console::Default(),
                       Console::Cyan(), logMessage.getContext().getContextId(), Console::Default(),
                       logMessage.getStream().data());
        else
            fmt::print("{:%Y%m%d-%H:%M:%S}.{:03} | {} | {} | {}\n",
                       posix_tm, now % 1000, logLevelStr,logMessage.getContext().getContextId(), logMessage.getStream().data());
        std::fflush(stdout);

        m_callback();
    }

    void ConsoleLogAppender::setAfterLogCallback(const std::function<void()>& callback)
    {
        m_callback = callback;
    }

    void ConsoleLogAppender::removeAfterLogCallback()
    {
        m_callback = []() {};
    }
}
