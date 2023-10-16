//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "fmt/format.h"
#include "gtest/gtest.h"

namespace ramses::internal
{
    TEST(AFmtlib, CanFormatStrings)
    {
        EXPECT_EQ("abc 123 def", fmt::format("abc {} {}", 123, "def"));
    }

    TEST(AFmtlib, CanUseNamedArguments)
    {
        EXPECT_EQ("foo 654 xyz", fmt::format("foo {bar} {baz}", fmt::arg("bar", 654), fmt::arg("baz", "xyz")));
    }
}
