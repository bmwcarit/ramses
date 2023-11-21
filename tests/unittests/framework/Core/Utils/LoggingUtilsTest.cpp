//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Core/Utils/LoggingUtils.h"
#include "gtest/gtest.h"

namespace ramses::internal
{
    namespace
    {
        enum EFoo
        {
            EFoo_A,
            EFoo_B,
        };

        const std::array FooNames = {"EFoo_A", "EFoo_B"};

        ENUM_TO_STRING(EFoo, FooNames, EFoo_B)

        const std::array LogTestBarNames = {"X", "Y"};

        enum class LogTestBarNoLast
        {
            X,
            Y,
        };
    }
}

MAKE_ENUM_CLASS_PRINTABLE(ramses::internal::LogTestBarNoLast,
                                          "LogTestBarNoLast",
                                          ramses::internal::LogTestBarNames,
                                          ramses::internal::LogTestBarNoLast::Y);

namespace ramses::internal
{
    TEST(ALoggingUtils, CanUseEnumToString)
    {
        EXPECT_STREQ("EFoo_A", EnumToString(EFoo_A));
        EXPECT_STREQ("EFoo_B", EnumToString(EFoo_B));
    }

    TEST(ALoggingUtils, CanUsEnumClassFormatterShortSyntax)
    {
        EXPECT_EQ("LogTestBarNoLast::X", fmt::format("{}", LogTestBarNoLast::X));
        EXPECT_EQ("LogTestBarNoLast::Y", fmt::format("{}", LogTestBarNoLast::Y));
        EXPECT_EQ("X", fmt::format("{:s}", LogTestBarNoLast::X));
        EXPECT_EQ("Y", fmt::format("{:s}", LogTestBarNoLast::Y));
        const auto invalidAbove = static_cast<LogTestBarNoLast>(2);
        EXPECT_EQ("<INVALID ramses::internal::LogTestBarNoLast 2>", fmt::to_string(invalidAbove));
    }
}
