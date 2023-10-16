//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "fmt/format.h"

namespace ramses::internal
{
    struct SimpleFormatterBase
    {
        template<typename ParseContext>
        constexpr auto parse(ParseContext& ctx)
        {
            return ctx.begin();
        }
    };
}

namespace ramses {
    template <typename T, typename = typename std::enable_if<std::is_enum<T>::value, void>::type>
    auto format_as(T f)
    {
        return fmt::underlying(f);
    }
}
