//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PAIR_H
#define RAMSES_PAIR_H

#include "ramses-capu/container/Hash.h"
#include <utility>

namespace ramses_capu
{
    /**
     * Hash code generation for a std::pair instance. Necessary e. g. for using a std::pair as a key in a hash map.
     */
    template<typename A, typename B>
    struct Hash<std::pair<A, B>>
    {
        uint_t operator()(const std::pair<A, B>& data)
        {
            return HashValue(data.first, data.second);
        }
    };
}

#endif
