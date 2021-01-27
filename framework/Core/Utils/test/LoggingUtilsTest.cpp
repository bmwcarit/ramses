//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Utils/LoggingUtils.h"
#include "gtest/gtest.h"

namespace ramses_internal
{
    namespace
    {
        enum EFoo
        {
            EFoo_A,
            EFoo_B,
            EFoo_Last
        };

        const char* FooNames[] = {"EFoo_A", "EFoo_B"};

        ENUM_TO_STRING(EFoo, FooNames, EFoo_Last)

        enum class LogTestBar : uint16_t
        {
            X,
            Y,
            Last
        };

        const char* LogTestBarNames[] = {"X", "Y"};

        enum class LogTestBarNoLast
        {
            X,
            Y,
        };
    }
}

MAKE_ENUM_CLASS_PRINTABLE(ramses_internal::LogTestBar,
                          "LogTestBar",
                          ramses_internal::LogTestBarNames,
                          ramses_internal::LogTestBar::Last);
MAKE_ENUM_CLASS_PRINTABLE_NO_EXTRA_LAST(ramses_internal::LogTestBarNoLast,
                                          "LogTestBarNoLast",
                                          ramses_internal::LogTestBarNames,
                                          ramses_internal::LogTestBarNoLast::Y);

namespace ramses_internal
{
    TEST(ALoggingUtils, CanUseEnumToString)
    {
        EXPECT_STREQ("EFoo_A", EnumToString(EFoo_A));
        EXPECT_STREQ("EFoo_B", EnumToString(EFoo_B));
    }

    TEST(ALoggingUtils, CanUseEnumClassFormatter)
    {
        EXPECT_EQ("LogTestBar::X", fmt::format("{}", LogTestBar::X));
        EXPECT_EQ("LogTestBar::Y", fmt::format("{}", LogTestBar::Y));
        EXPECT_EQ("X", fmt::format("{:s}", LogTestBar::X));
        EXPECT_EQ("Y", fmt::format("{:s}", LogTestBar::Y));
        EXPECT_EQ("<INVALID ramses_internal::LogTestBar 2>", fmt::to_string(LogTestBar::Last));
    }

    TEST(ALoggingUtils, CanUsEnumClassFormatterShortSyntax)
    {
        EXPECT_EQ("LogTestBarNoLast::X", fmt::format("{}", LogTestBarNoLast::X));
        EXPECT_EQ("LogTestBarNoLast::Y", fmt::format("{}", LogTestBarNoLast::Y));
        EXPECT_EQ("X", fmt::format("{:s}", LogTestBarNoLast::X));
        EXPECT_EQ("Y", fmt::format("{:s}", LogTestBarNoLast::Y));
        const LogTestBarNoLast invalidAbove = static_cast<LogTestBarNoLast>(2);
        EXPECT_EQ("<INVALID ramses_internal::LogTestBarNoLast 2>", fmt::to_string(invalidAbove));
    }
}
