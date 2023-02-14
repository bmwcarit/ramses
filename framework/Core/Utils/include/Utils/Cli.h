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

namespace CLI
{
    template <typename BaseT, BaseT _invalid, typename UniqueIdT>
    inline std::istringstream& operator>>(std::istringstream& is, ramses_internal::StronglyTypedValue<BaseT, _invalid, UniqueIdT>& val)
    {
        BaseT v;
        is >> v;
        val = ramses_internal::StronglyTypedValue<BaseT, _invalid, UniqueIdT>(v);
        return is;
    }

    template <typename BaseT, BaseT _invalid, typename UniqueIdT>
    inline std::stringstream& operator<<(std::stringstream& os, const ramses_internal::StronglyTypedValue<BaseT, _invalid, UniqueIdT>& val)
    {
        os << val.getValue();
        return os;
    }

    inline std::istringstream& operator>>(std::istringstream& is, ramses_internal::Vector4& val)
    {
        char separator = 0;
        is >> val.r;
        is >> separator;
        is >> val.g;
        is >> separator;
        is >> val.b;
        is >> separator;
        is >> val.a;
        return is;
    }
}

// stream operators need to be declared before including CLI/CLI.hpp
#include "CLI/CLI.hpp"
