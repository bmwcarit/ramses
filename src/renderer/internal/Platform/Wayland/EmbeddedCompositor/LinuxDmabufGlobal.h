//  -------------------------------------------------------------------------
//  Copyright (C) 2019-2019, Garmin International, Inc. and its affiliates.
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "linux-dmabuf-unstable-v1-server-protocol.h"
#include "wayland-server.h"
#include <cstdint>

namespace ramses::internal
{
    class IEmbeddedCompositor_Wayland;
    class Context_EGL;
    class WaylandDisplay;
    class IWaylandGlobal;
    class IWaylandClient;

    class LinuxDmabufGlobal
    {
    public:
        explicit LinuxDmabufGlobal(IEmbeddedCompositor_Wayland& compositor);
        ~LinuxDmabufGlobal();

        bool init(WaylandDisplay& serverDisplay, Context_EGL& context);
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
