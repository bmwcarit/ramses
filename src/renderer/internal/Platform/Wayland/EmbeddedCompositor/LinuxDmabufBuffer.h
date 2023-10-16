//  -------------------------------------------------------------------------
//  Copyright (C) 2019-2019, Garmin International, Inc. and its affiliates.
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "wayland-server.h"

namespace ramses::internal
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
