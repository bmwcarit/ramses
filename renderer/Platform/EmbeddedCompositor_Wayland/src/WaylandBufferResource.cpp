//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "EmbeddedCompositor_Wayland/WaylandBufferResource.h"

namespace ramses_internal
{
    WaylandBufferResource::WaylandBufferResource()
    {
    }

    WaylandBufferResource::WaylandBufferResource(wl_resource* resource)
        : NativeWaylandResource(resource)
    {
    }

    void WaylandBufferResource::bufferSendRelease()
    {
        wl_buffer_send_release(m_resource);
    }

    int32_t WaylandBufferResource::bufferGetSharedMemoryWidth() const
    {
        wl_shm_buffer* sharedMemoryBuffer = wl_shm_buffer_get(m_resource);

        if (sharedMemoryBuffer)
        {
            return wl_shm_buffer_get_width(sharedMemoryBuffer);
        }
        else
        {
            return 0;
        }
    }

    int32_t WaylandBufferResource::bufferGetSharedMemoryHeight() const
    {
        wl_shm_buffer* sharedMemoryBuffer = wl_shm_buffer_get(m_resource);

        if (sharedMemoryBuffer)
        {
            return wl_shm_buffer_get_height(sharedMemoryBuffer);
        }
        else
        {
            return 0;
        }
    }

    const void* WaylandBufferResource::bufferGetSharedMemoryData() const
    {
        wl_shm_buffer* sharedMemoryBuffer = wl_shm_buffer_get(m_resource);
        if (sharedMemoryBuffer)
        {
            return wl_shm_buffer_get_data(sharedMemoryBuffer);
        }
        else
        {
            return nullptr;
        }
    }

    WaylandBufferResource* WaylandBufferResource::clone() const
    {
        return new WaylandBufferResource(m_resource);
    }
}
