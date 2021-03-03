//  -------------------------------------------------------------------------
//  Copyright (C) 2019-2019, Garmin International, Inc. and its affiliates.
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_LINUXDMABUFCONNECTION_H
#define RAMSES_LINUXDMABUFCONNECTION_H

#include "EmbeddedCompositor_Wayland/IWaylandClient.h"
#include "linux-dmabuf-unstable-v1-server-protocol.h"
#include "wayland-server.h"
#include <cstdint>

namespace ramses_internal
{
    class LinuxDmabufConnection
    {
    public:
        LinuxDmabufConnection(IWaylandClient& client, uint32_t version, uint32_t id);
        ~LinuxDmabufConnection();

        bool wasSuccessfullyInitialized() const;

        void sendFormats();

    private:
        static void ResourceDestroyedCallback(wl_resource* dmabufConnectionResource);

        static void DmabufDestroyCallback(wl_client* client, wl_resource* dmabufConnectionResource);
        static void DmabufCreateParamsCallback(wl_client* client, wl_resource* dmabufConnectionResource, uint32_t paramsId);

        void resourceDestroyed();
        void createParams(uint32_t paramsId);

        static struct zwp_linux_dmabuf_v1_interface const m_dmabufInterface;

        const WaylandClientCredentials  m_clientCredentials;
        INativeWaylandResource* m_resource = nullptr;
    };
}
#endif
