//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_WAYLANDOUTPUTPARAMS_H
#define RAMSES_WAYLANDOUTPUTPARAMS_H

#include <cstdint>

namespace ramses_internal
{
    struct WaylandOutputParams
    {
        uint32_t width;
        uint32_t height;
    };
}

#endif
