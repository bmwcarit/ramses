//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_WAYLANDOUTPUTTESTPARAMS_H
#define RAMSES_WAYLANDOUTPUTTESTPARAMS_H

#include <cstdint>

namespace ramses_internal
{
    struct WaylandOutputTestParams
    {
        enum
        {
            WaylandOutput_GeometryReceived = 1,
            WaylandOutput_ModeReceived = 2,
            WaylandOutput_ScaleReceived = 4,
            WaylandOutput_DoneReceived = 8
        };
        uint8_t m_waylandOutputReceivedFlags = 0u;

        //geometry
        int32_t x = 0;
        int32_t y = 0;
        int32_t physicalWidth = 0;
        int32_t physicalHeight = 0;
        int32_t subpixel = 0;
        int32_t transform = 0;

        //mode
        uint32_t modeFlags = 0u;
        int32_t width = 0;
        int32_t height = 0;
        int32_t refreshRate = 0;

        //scale
        int32_t factor = 0;
    };
}

#endif
