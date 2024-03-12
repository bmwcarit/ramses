//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "EmbeddedCompositor_Wayland/WaylandBufferResource.h"
#include "EmbeddedCompositor_Wayland/LinuxDmabufBuffer.h"
#include "EmbeddedCompositor_Wayland/LinuxDmabuf.h"

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

    int32_t WaylandBufferResource::getWidth() const
    {
        wl_shm_buffer* sharedMemoryBuffer = wl_shm_buffer_get(m_resource);

        if (sharedMemoryBuffer)
        {
            return wl_shm_buffer_get_width(sharedMemoryBuffer);
        }
        else if ((m_resource != nullptr) && wl_resource_instance_of(m_resource, &wl_buffer_interface, &LinuxDmabufBuffer::m_bufferInterface))
        {
            auto dmabuf = static_cast<LinuxDmabufBufferData*>(wl_resource_get_user_data(m_resource));
            return (dmabuf != nullptr) ? dmabuf->getWidth() : 0;
        }
        else
        {
            return 0;
        }
    }

    int32_t WaylandBufferResource::getHeight() const
    {
        wl_shm_buffer* sharedMemoryBuffer = wl_shm_buffer_get(m_resource);

        if (sharedMemoryBuffer)
        {
            return wl_shm_buffer_get_height(sharedMemoryBuffer);
        }
        else if ((m_resource != nullptr) && wl_resource_instance_of(m_resource, &wl_buffer_interface, &LinuxDmabufBuffer::m_bufferInterface))
        {
            auto dmabuf = static_cast<LinuxDmabufBufferData*>(wl_resource_get_user_data(m_resource));
            return (dmabuf != nullptr) ? dmabuf->getHeight() : 0;
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
