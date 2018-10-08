//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_FONTDATA_H
#define RAMSES_FONTDATA_H

#include <stdint.h>
#include <vector>

namespace ramses
{
    struct FontData
    {
        uint32_t type;
        std::vector<uint8_t> data;
    };
}

#endif
