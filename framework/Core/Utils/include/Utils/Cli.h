//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "Common/StronglyTypedValue.h"
#include "Math3d/Vector4.h"
#include <sstream>
#include "CLI/CLI.hpp"

namespace ramses_internal
{
    /**
     * Converts a std::string to a Vector4 for the CLI11 command line parser
     * (see example https://github.com/CLIUtils/CLI11/blob/main/examples/custom_parse.cpp)
     */
    inline bool lexical_cast(const std::string& input, Vector4& val)
    {
        std::istringstream is;
        is.str(input);
        char separator = 0;
        is >> val.r;
        is >> separator;
        is >> val.g;
        is >> separator;
        is >> val.b;
        is >> separator;
        is >> val.a;
        return !is.fail() && (is.rdbuf()->in_avail() == 0);
    }

    /**
     * Converts a std::string to a StronglyTypedValue for the CLI11 command line parser
     */
    template <typename BaseT, BaseT _invalid, typename UniqueIdT>
    inline bool lexical_cast(const std::string& input, ramses_internal::StronglyTypedValue<BaseT, _invalid, UniqueIdT>& val)
    {
        BaseT v;
        std::istringstream is;
        is.str(input);
        is >> v;
        val = StronglyTypedValue<BaseT, _invalid, UniqueIdT>(v);
        return !is.fail() && (is.rdbuf()->in_avail() == 0);
    }
}
