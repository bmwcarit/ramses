//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_WAYLANDCLIENT_H
#define RAMSES_WAYLANDCLIENT_H

#include "EmbeddedCompositor_Wayland/IWaylandClient.h"
#include "wayland-server.h"
#include <memory>

namespace ramses_internal
{
    class WaylandClient: public IWaylandClient
    {
    public:
        explicit WaylandClient(wl_client* client);
        virtual WaylandClientCredentials getCredentials() const override;
        virtual void postNoMemory() override;
        virtual INativeWaylandResource* resourceCreate(const wl_interface *interface, int version, uint32_t id) override;
        virtual WaylandCallbackResource* callbackResourceCreate(const wl_interface* interface, int version, uint32_t id) override;

    private:
        wl_client* m_client;

        // Due to current design drawback WaylandClient objects get created too often, therefore credentials
        // are lazy-initialized when needed
        mutable WaylandClientCredentials m_credentials;
    };
}

#endif
