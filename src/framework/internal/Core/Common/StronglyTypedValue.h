//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/StronglyTypedValue.h"
#include "internal/Core/Utils/Warnings.h"
#include "internal/PlatformAbstraction/Hash.h"
#include "internal/PlatformAbstraction/Collections/StringOutputStream.h"
#include "internal/PlatformAbstraction/Macros.h"
#include <type_traits>
#include <functional>

#define MAKE_STRONGLYTYPEDVALUE_PRINTABLE(stronglyType) \
    template <> \
    struct fmt::formatter<::stronglyType>: formatter<string_view>  {    \
        template<typename ParseContext> \
        constexpr auto parse(ParseContext& ctx)  { \
            return ctx.begin(); \
        } \
        template<typename FormatContext> \
        constexpr auto format(const ::stronglyType& str, FormatContext& ctx) {    \
            return fmt::format_to(ctx.out(), "{}", str.getValue()); \
        } \
    };

