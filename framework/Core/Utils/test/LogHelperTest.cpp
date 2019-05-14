//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Utils/LogHelper.h"
#include "gtest/gtest.h"

namespace ramses_internal
{
    TEST(ALogHelper_StringToLogLevel, GivesCorrectLevels)
    {
        {
            ELogLevel ll;
            EXPECT_TRUE(LogHelper::StringToLogLevel("trace", ll));
            EXPECT_EQ(ELogLevel::Trace, ll);
        }
        {
            ELogLevel ll;
            EXPECT_TRUE(LogHelper::StringToLogLevel("info", ll));
            EXPECT_EQ(ELogLevel::Info, ll);
        }
        {
            ELogLevel ll;
            EXPECT_TRUE(LogHelper::StringToLogLevel("0", ll));
            EXPECT_EQ(ELogLevel::Off, ll);
        }
    }

    TEST(ALogHelper_StringToLogLevel, IgnoresCase)
    {
        {
            ELogLevel ll;
            EXPECT_TRUE(LogHelper::StringToLogLevel("TRACE", ll));
            EXPECT_EQ(ELogLevel::Trace, ll);
        }
        {
            ELogLevel ll;
            EXPECT_TRUE(LogHelper::StringToLogLevel("inFo", ll));
            EXPECT_EQ(ELogLevel::Info, ll);
        }
    }

    TEST(ALogHelper_StringToLogLevel, ReturnFalseOnInvalidLogLevels)
    {
        ELogLevel ll = ELogLevel::Info;
        EXPECT_FALSE(LogHelper::StringToLogLevel("", ll));
        EXPECT_FALSE(LogHelper::StringToLogLevel("123", ll));
        EXPECT_FALSE(LogHelper::StringToLogLevel("-1", ll));
        EXPECT_FALSE(LogHelper::StringToLogLevel("XX", ll));
        EXPECT_FALSE(LogHelper::StringToLogLevel("Info0", ll));
        EXPECT_EQ(ELogLevel::Info, ll);
    }

    TEST(ALogHelper_ParseContextFilters, canExtractSingleCommandCorrectly)
    {
        std::vector<LogHelper::ContextFilter> filterCommands = LogHelper::ParseContextFilters("4:logContextString");

        ASSERT_EQ(1u, filterCommands.size());
        EXPECT_EQ(ELogLevel::Info, filterCommands[0].first);
        EXPECT_STREQ("logContextString", filterCommands[0].second.c_str());
    }

    TEST(ALogHelper_ParseContextFilters, canExtractMultipleCommandsCorrectly)
    {
        std::vector<LogHelper::ContextFilter> filterCommands = LogHelper::ParseContextFilters("6:six,3:three,4:four");

        ASSERT_EQ(3u, filterCommands.size());
        EXPECT_EQ(ELogLevel::Trace, filterCommands[0].first);
        EXPECT_STREQ("six", filterCommands[0].second.c_str());

        EXPECT_EQ(ELogLevel::Warn, filterCommands[1].first);
        EXPECT_STREQ("three", filterCommands[1].second.c_str());

        EXPECT_EQ(ELogLevel::Info, filterCommands[2].first);
        EXPECT_STREQ("four", filterCommands[2].second.c_str());
    }

    TEST(ALogHelper_ParseContextFilters, canHandleDescriptiveLoglevelNames)
    {
        std::vector<LogHelper::ContextFilter> filterCommands = LogHelper::ParseContextFilters("trace:six,warn:three,info:four");

        ASSERT_EQ(3u, filterCommands.size());
        EXPECT_EQ(ELogLevel::Trace, filterCommands[0].first);
        EXPECT_STREQ("six", filterCommands[0].second.c_str());

        EXPECT_EQ(ELogLevel::Warn, filterCommands[1].first);
        EXPECT_STREQ("three", filterCommands[1].second.c_str());

        EXPECT_EQ(ELogLevel::Info, filterCommands[2].first);
        EXPECT_STREQ("four", filterCommands[2].second.c_str());
    }

    TEST(ALogHelper_ParseContextFilters, skipsUnknownLogLevelStrings)
    {
        std::vector<LogHelper::ContextFilter> filterCommands = LogHelper::ParseContextFilters("foo:six,warn:three,bar:four");

        ASSERT_EQ(1u, filterCommands.size());
        EXPECT_EQ(ELogLevel::Warn, filterCommands[0].first);
        EXPECT_STREQ("three", filterCommands[0].second.c_str());
    }

    TEST(ALogHelper_ParseContextFilters, canExtractCommandCorrectlyWhenPassedZeroString)
    {
        std::vector<LogHelper::ContextFilter> filterCommands = LogHelper::ParseContextFilters("");
        EXPECT_EQ(0u, filterCommands.size());
    }

    TEST(ALogHelper_ParseContextFilters, canExtractCommandCorrectlyWhenPassedCurruptString)
    {
        std::vector<LogHelper::ContextFilter> filterCommands = LogHelper::ParseContextFilters("8:");
        EXPECT_EQ(0u, filterCommands.size());

        filterCommands = LogHelper::ParseContextFilters(":");
        EXPECT_EQ(0u, filterCommands.size());

        filterCommands = LogHelper::ParseContextFilters(":context");
        EXPECT_EQ(0u, filterCommands.size());

        filterCommands = LogHelper::ParseContextFilters(",");
        EXPECT_EQ(0u, filterCommands.size());
    }

    TEST(ALogHelper_GetLoglevelFromInt, WorksForAllDirectlyMappableValues)
    {
        EXPECT_EQ(ELogLevel::Off, LogHelper::GetLoglevelFromInt(0));
        EXPECT_EQ(ELogLevel::Fatal, LogHelper::GetLoglevelFromInt(1));
        EXPECT_EQ(ELogLevel::Error, LogHelper::GetLoglevelFromInt(2));
        EXPECT_EQ(ELogLevel::Warn, LogHelper::GetLoglevelFromInt(3));
        EXPECT_EQ(ELogLevel::Info, LogHelper::GetLoglevelFromInt(4));
        EXPECT_EQ(ELogLevel::Debug, LogHelper::GetLoglevelFromInt(5));
        EXPECT_EQ(ELogLevel::Trace, LogHelper::GetLoglevelFromInt(6));
    }

    TEST(ALogHelper_GetLoglevelFromInt, ReturnsTraceForAllUnknownValues)
    {
        EXPECT_EQ(ELogLevel::Trace, LogHelper::GetLoglevelFromInt(-1));
        EXPECT_EQ(ELogLevel::Trace, LogHelper::GetLoglevelFromInt(7));
        EXPECT_EQ(ELogLevel::Trace, LogHelper::GetLoglevelFromInt(999));
    }
}
