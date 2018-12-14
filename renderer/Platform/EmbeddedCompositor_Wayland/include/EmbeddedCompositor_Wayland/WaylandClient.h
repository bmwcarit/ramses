//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------
#ifndef RAMSES_WAYLANDCLIENT_H
#define RAMSES_WAYLANDCLIENT_H

#include "wayland-server.h"
#include "EmbeddedCompositor_Wayland/IWaylandClient.h"

namespace ramses_internal
{
    class WaylandClient: public IWaylandClient
    {
    public:
        WaylandClient(wl_client* client);
        virtual void getCredentials(pid_t& processId, uid_t& userId, gid_t& groupId) override;
        virtual void postNoMemory() override;
        virtual IWaylandResource* resourceCreate(const wl_interface *interface, int version, uint32_t id) override;
        virtual WaylandCallbackResource* callbackResourceCreate(const wl_interface* interface, int version, uint32_t id) override;

    private:
        wl_client* m_client;
    };
}

#endif
