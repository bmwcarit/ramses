//  -------------------------------------------------------------------------
//  Copyright (C) 2019-2019, Garmin International, Inc. and its affiliates.
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_LINUXDMABUFBUFFER_H
#define RAMSES_LINUXDMABUFBUFFER_H

#include "wayland-server.h"

namespace ramses_internal
{
    class LinuxDmabufBufferData;
    class WaylandBufferResource;

    class LinuxDmabufBuffer
    {
    public:
        static void DmabufBufferDestroyCallback(wl_client* client, wl_resource* bufferResource);

        static LinuxDmabufBufferData* fromWaylandBufferResource(WaylandBufferResource& resource);

        static struct wl_buffer_interface const m_bufferInterface;
    };
}

#endif
