//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/PlatformAbstraction/Hash.h"
#include <utility>

template<typename A, typename B>
struct std::hash<std::pair<A, B>>
{
    size_t operator()(const std::pair<A, B>& data)
    {
        return ramses::internal::HashValue(data.first, data.second);
    }
};
