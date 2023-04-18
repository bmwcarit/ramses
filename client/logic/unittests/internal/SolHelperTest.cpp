//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"

#include "internals/SolHelper.h"

namespace sol_helper
{
    TEST(ASolHelper, ConvertsSolTypeToString)
    {
        EXPECT_EQ("none", sol_helper::GetSolTypeName(sol::type::none));
        EXPECT_EQ("nil", sol_helper::GetSolTypeName(sol::type::nil));
        EXPECT_EQ("string", sol_helper::GetSolTypeName(sol::type::string));
        EXPECT_EQ("number", sol_helper::GetSolTypeName(sol::type::number));
        EXPECT_EQ("thread", sol_helper::GetSolTypeName(sol::type::thread));
        EXPECT_EQ("bool", sol_helper::GetSolTypeName(sol::type::boolean));
        EXPECT_EQ("function", sol_helper::GetSolTypeName(sol::type::function));
        EXPECT_EQ("userdata", sol_helper::GetSolTypeName(sol::type::userdata));
        EXPECT_EQ("lightuserdata", sol_helper::GetSolTypeName(sol::type::lightuserdata));
        EXPECT_EQ("table", sol_helper::GetSolTypeName(sol::type::table));
        EXPECT_EQ("poly", sol_helper::GetSolTypeName(sol::type::poly));
    }
}
