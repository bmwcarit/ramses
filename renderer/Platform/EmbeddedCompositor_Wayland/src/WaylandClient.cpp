//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "EmbeddedCompositor_Wayland/WaylandClient.h"
#include "EmbeddedCompositor_Wayland/WaylandResource.h"
#include "EmbeddedCompositor_Wayland/WaylandCallbackResource.h"

namespace ramses_internal
{
    WaylandClient::WaylandClient(wl_client* client)
        : m_client(client)
    {
        assert(client != nullptr);
    }

    void WaylandClient::getCredentials(pid_t& processId, uid_t& userId, gid_t& groupId)
    {
        wl_client_get_credentials(m_client, &processId, &userId, &groupId);
    }

    void WaylandClient::postNoMemory()
    {
        wl_client_post_no_memory(m_client);
    }

    IWaylandResource* WaylandClient::resourceCreate(const wl_interface* interface, int version, uint32_t id)
    {
        wl_resource* resource = wl_resource_create(m_client, interface, version, id);
        return new WaylandResource(resource, true);
    }

    WaylandCallbackResource* WaylandClient::callbackResourceCreate(const wl_interface* interface, int version, uint32_t id)
    {
        wl_resource* resource = wl_resource_create(m_client, interface, version, id);
        return new WaylandCallbackResource(resource, true);
    }
}
