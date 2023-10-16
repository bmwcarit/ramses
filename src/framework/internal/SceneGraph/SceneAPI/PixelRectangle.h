//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include <cstdint>

namespace ramses::internal
{
    struct PixelRectangle
    {
        uint32_t x = 0u;
        uint32_t y = 0u;
        int32_t width = 0;
        int32_t height = 0;
    };
}
