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

namespace rlogic
{
    class ScopedLogContextLevel
    {
    public:
        explicit ScopedLogContextLevel(ELogMessageType verbosityLimit)
            : m_savedLogVerbosityLimit(Logger::GetLogVerbosityLimit())
        {
            Logger::SetLogVerbosityLimit(verbosityLimit);
        }

        ScopedLogContextLevel(ELogMessageType logPriority, const rlogic::Logger::LogHandlerFunc& handler)
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
                Logger::SetLogHandler([](ELogMessageType type, std::string_view message){
                    (void)type;
                    (void)message;
                });
            }
        }

        ScopedLogContextLevel(const ScopedLogContextLevel&) = default;
        ScopedLogContextLevel& operator=(const ScopedLogContextLevel&) = default;

    private:
        ELogMessageType m_savedLogVerbosityLimit;
        bool m_unsetCustomHandler = false;
    };

    struct TestLog
    {
        ELogMessageType type;
        std::string message;
    };

    class TestLogCollector
    {
    public:
        explicit TestLogCollector(ELogMessageType verbosityLimit)
            : m_logCollector(verbosityLimit, [this](ELogMessageType type, std::string_view message)
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
