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

namespace ramses
{
    class ScopedLogContextLevel
    {
    public:
        explicit ScopedLogContextLevel(ELogLevel verbosityLimit)
            : m_savedLogVerbosityLimit(Logger::GetLogVerbosityLimit())
        {
            Logger::SetLogVerbosityLimit(verbosityLimit);
        }

        ScopedLogContextLevel(ELogLevel logPriority, const ramses::Logger::LogHandlerFunc& handler)
            : ScopedLogContextLevel(logPriority)
        {
            Logger::SetLogHandler(handler);
            m_unsetCustomHandler = true;
        }

        ~ScopedLogContextLevel()
        {
            Logger::SetLogVerbosityLimit(m_savedLogVerbosityLimit);

            if (m_unsetCustomHandler)
            {
                // Set an empty lambda to avoid side effects
                Logger::SetLogHandler([](ELogLevel type, std::string_view message){
                    (void)type;
                    (void)message;
                });
            }
        }

        ScopedLogContextLevel(const ScopedLogContextLevel&) = default;
        ScopedLogContextLevel& operator=(const ScopedLogContextLevel&) = default;

    private:
        ELogLevel m_savedLogVerbosityLimit;
        bool m_unsetCustomHandler = false;
    };

    struct TestLog
    {
        ELogLevel type;
        std::string message;
    };

    class TestLogCollector
    {
    public:
        explicit TestLogCollector(ELogLevel verbosityLimit)
            : m_logCollector(verbosityLimit, [this](ELogLevel type, std::string_view message)
                {
                    logs.emplace_back(TestLog{type, std::string{message}});
                })
        {
        }

        std::vector<TestLog> logs;

    private:
        ScopedLogContextLevel m_logCollector;
    };
}
