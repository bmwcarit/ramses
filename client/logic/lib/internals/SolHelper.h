//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "fmt/format.h"
#include "ramses-logic/EStandardModule.h"
#include "internals/SolWrapper.h"

#include <string_view>

namespace sol_helper
{
    template<typename S, typename ...ARGS>
    static void throwSolException(const S& format_str, ARGS ...args)
    {
        throw sol::error(fmt::format(format_str, args...));
    }

    static constexpr std::string_view GetSolTypeName(sol::type type)
    {
        switch (type)
        {
        case sol::type::none:
            return "none";
        case sol::type::nil:
            return "nil";
        case sol::type::string:
            return "string";
        case sol::type::number:
            return "number";
        case sol::type::thread:
            return "thread";
        case sol::type::boolean:
            return "bool";
        case sol::type::function:
            return "function";
        case sol::type::userdata:
            return "userdata";
        case sol::type::lightuserdata:
            return "lightuserdata";
        case sol::type::table:
            return "table";
        case sol::type::poly:
            return "poly";
        };
        return "none";
    }
}
