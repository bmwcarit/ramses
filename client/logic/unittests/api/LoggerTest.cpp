//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gmock/gmock.h"

#include "ramses-logic/Logger.h"
#include "ramses-logic/ELogMessageType.h"

#include "impl/LoggerImpl.h"

#include "LogTestUtils.h"

namespace rlogic
{
    // Test default state without fixture
    TEST(ALogger_ByDefault, HasInfoAsVerbosityLimit)
    {
        EXPECT_EQ(ELogMessageType::Info, Logger::GetLogVerbosityLimit());
    }

    class ALogger : public ::testing::Test
    {
    protected:
        std::vector<ELogMessageType> m_logTypes;
        std::vector<std::string> m_logMessages;
        ScopedLogContextLevel m_logCollector{ ELogMessageType::Trace, [this](ELogMessageType type, std::string_view message)
            {
                m_logTypes.emplace_back(type);
                m_logMessages.emplace_back(message);
        }
        };
    };

    TEST_F(ALogger, LogsDifferentLogLevelsSequentially)
    {
        LOG_FATAL("Fatal");
        LOG_ERROR("Error");
        LOG_WARN("Warn");
        LOG_INFO("Info");
        LOG_DEBUG("Debug");
        LOG_TRACE("Trace");

        EXPECT_THAT(m_logTypes, ::testing::ElementsAre(ELogMessageType::Fatal, ELogMessageType::Error, ELogMessageType::Warn, ELogMessageType::Info, ELogMessageType::Debug, ELogMessageType::Trace));
        EXPECT_THAT(m_logMessages, ::testing::ElementsAre("Fatal", "Error", "Warn", "Info", "Debug", "Trace"));
    }

    TEST_F(ALogger, LogsFormattedMessage)
    {
        LOG_INFO("Info Message {}", 42);

        EXPECT_EQ(m_logMessages[0], "Info Message 42");
    }

    TEST_F(ALogger, LogsFormattedMessageWithMultipleArgumentsAndTypes)
    {
        LOG_INFO("Info Message {} {} {} {}", 42, 0.5f, "bool:", true);

        EXPECT_EQ(m_logMessages[0], "Info Message 42 0.5 bool: true");
    }

    TEST_F(ALogger, SetsDefaultLoggingOffAndOnAgain)
    {
        Logger::SetDefaultLogging(false);
        LOG_INFO("Info Message {} {} {}", 42, 42.0f, "42");
        Logger::SetDefaultLogging(true);
        LOG_INFO("Info Message {} {} {}", 42, 42.0f, "43");

        // Can't expect anything because default logging goes to stdout
    }

    TEST_F(ALogger, SetsDefaultLoggingOff_DoesNotAffectCustomLogHandler)
    {
        Logger::SetDefaultLogging(false);

        LOG_INFO("info");
        EXPECT_EQ(m_logMessages[0], "info");

        // Reset to not affect other tests
        Logger::SetDefaultLogging(true);
    }

    TEST_F(ALogger, ChangesLogVerbosityAffectsWhichMessagesAreProcessed)
    {
        Logger::SetLogVerbosityLimit(ELogMessageType::Error);

        // Simulate logs of all types. Only error and fatal error should be logged
        LOG_TRACE("trace");
        LOG_DEBUG("debug");
        LOG_FATAL("fatal");
        LOG_WARN("warn");
        LOG_INFO("info");
        LOG_DEBUG("debug");
        LOG_ERROR("error");

        EXPECT_THAT(m_logTypes, ::testing::ElementsAre(ELogMessageType::Fatal, ELogMessageType::Error));
    }
}
