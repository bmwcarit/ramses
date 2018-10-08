//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_WAYLANDUTILITIES_H
#define RAMSES_WAYLANDUTILITIES_H

#include "RendererAPI/Types.h"

struct wl_display;

namespace ramses_internal
{
    class WaylandUtilities
    {
    public:
        static Bool IsValidSocket(int fileDescriptor);
        static Bool IsEnvironmentInProperState();
        static Bool DoesWaylandSupportDisplayAddSockedFD();
        static int  DisplayAddSocketFD(wl_display* display, int sock_fd);
    };
} // namespace ramses_internal

#endif
