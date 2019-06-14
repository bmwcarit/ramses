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
#include "ramses-capu/os/Console.h"
#include <time.h>

namespace ramses_internal
{
    ConsoleLogAppender::ConsoleLogAppender()
        : m_callback([]() {})
        , m_colorsEnabled(!PlatformEnvironmentVariables::HasEnvVar("DISABLE_CONSOLE_COLORS"))
    {
    }

    void ConsoleLogAppender::logMessage(const LogMessage& logMessage)
    {
        const uint64_t now = PlatformTime::GetMillisecondsAbsolute();
        ramses_capu::Console::ConsoleColor logLevelColor;
        const char* logLevelStr = nullptr;

        switch(logMessage.getLogLevel())
        {
        case ELogLevel::Trace:
            logLevelColor = ramses_capu::Console::WHITE;
            logLevelStr = "Trace";
            break;
        case ELogLevel::Debug:
            logLevelColor = ramses_capu::Console::WHITE;
            logLevelStr = "Debug";
            break;
        case ELogLevel::Info:
            logLevelColor = ramses_capu::Console::GREEN;
            logLevelStr = "Info ";
            break;
        case ELogLevel::Warn:
            logLevelColor = ramses_capu::Console::YELLOW;
            logLevelStr = "Warn ";
            break;
        case ELogLevel::Error:
            logLevelColor = ramses_capu::Console::RED;
            logLevelStr = "Error";
            break;
        case ELogLevel::Fatal:
            logLevelColor = ramses_capu::Console::RED;
            logLevelStr = "Fatal";
            break;
        default:
            logLevelColor = ramses_capu::Console::RED;
            logLevelStr = "?????";
            break;
        }

        struct tm posix_tm;
        time_t posix_time = static_cast<time_t>(now / 1000);
#ifdef _MSC_VER
        localtime_s(&posix_tm, &posix_time);
#else
        localtime_r(&posix_time, &posix_tm);
#endif
        char time_buffer[25] = {0};
        strftime(time_buffer, sizeof(time_buffer), "%Y%m%d-%H:%M:%S", &posix_tm);

        if (m_colorsEnabled)
        {
            ramses_capu::Console::Print(ramses_capu::Console::WHITE, "%s.%03d ", time_buffer, now % 1000);
            ramses_capu::Console::Print("| ");
            ramses_capu::Console::Print(logLevelColor, logLevelStr);
            ramses_capu::Console::Print(" | ");
            ramses_capu::Console::Print(ramses_capu::Console::AQUA, logMessage.getContext().getContextId());
            ramses_capu::Console::Print(" | %s\n", logMessage.getStream().c_str());
        }
        else
        {
            ramses_capu::Console::Print("%s.%03d | %s | %s | %s\n", time_buffer, now % 1000, logLevelStr,logMessage.getContext().getContextId(), logMessage.getStream().c_str());
        }
        ramses_capu::Console::Flush();

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
