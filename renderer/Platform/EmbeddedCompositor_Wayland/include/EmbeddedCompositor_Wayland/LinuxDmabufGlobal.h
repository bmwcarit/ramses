//  -------------------------------------------------------------------------
//  Copyright (C) 2019-2019, Garmin International, Inc. and its affiliates.
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_LINUXDMABUFGLOBAL_H
#define RAMSES_LINUXDMABUFGLOBAL_H

#include <stdint.h>

#include "wayland-server.h"
#include "linux-dmabuf-unstable-v1-server-protocol.h"

namespace ramses_internal
{
    class IEmbeddedCompositor_Wayland;
    class IContext;
    class WaylandDisplay;
    class IWaylandGlobal;
    class IWaylandClient;
    class IWaylandResource;
    class WaylandBufferResource;
    class LinuxDmabufBufferData;

    class LinuxDmabufGlobal
    {
    public:
        explicit LinuxDmabufGlobal(IEmbeddedCompositor_Wayland& compositor);
        ~LinuxDmabufGlobal();

        bool init(WaylandDisplay& serverDisplay, IContext& context);
        void destroy();
        void globalBind(IWaylandClient& client, uint32_t version, uint32_t id);

    private:
        static void GlobalBindCallback(wl_client* client, void* data, uint32_t version, uint32_t id);
        static void ClientConnectionDestroyCallback(wl_client* client, wl_resource* globalResource);

    private:
        IEmbeddedCompositor_Wayland& m_compositor;
        IWaylandGlobal* m_waylandGlobal = nullptr;
    };

}

#endif
