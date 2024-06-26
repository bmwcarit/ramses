//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Platform/Wayland/EmbeddedCompositor/WaylandClient.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/NativeWaylandResource.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/WaylandCallbackResource.h"
#include <cassert>

namespace ramses::internal
{
    WaylandClient::WaylandClient(wl_client* client)
        : m_client(client)
    {
        assert(client != nullptr);
    }

    WaylandClientCredentials WaylandClient::getCredentials() const
    {
        if(m_credentials.getProcessId() == -1)
        {
            pid_t processId = 0;
            uid_t userId = 0;
            gid_t groupId = 0;
            wl_client_get_credentials(m_client, &processId, &userId, &groupId);
            m_credentials = WaylandClientCredentials(processId, userId, groupId);
        }

        return m_credentials;
    }

    void WaylandClient::postNoMemory()
    {
        wl_client_post_no_memory(m_client);
    }

    INativeWaylandResource* WaylandClient::resourceCreate(const wl_interface* interface, int version, uint32_t id)
    {
        wl_resource* resource = wl_resource_create(m_client, interface, version, id);
        return new NativeWaylandResource(resource);
    }

    WaylandCallbackResource* WaylandClient::callbackResourceCreate(const wl_interface* interface, int version, uint32_t id)
    {
        wl_resource* resource = wl_resource_create(m_client, interface, version, id);
        return new WaylandCallbackResource(resource);
    }
}
