//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------
#pragma once

#include "internal/Platform/Wayland/EmbeddedCompositor/IWaylandClient.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/WaylandOutputParams.h"
#include "wayland-server.h"

namespace ramses::internal
{
    class WaylandOutputConnection final
    {
    public:
        WaylandOutputConnection(const WaylandOutputParams& waylandOutputParams, IWaylandClient& client, uint32_t version, uint32_t id);
        ~WaylandOutputConnection();

        void resourceDestroyed();

    private:
        static void ResourceDestroyedCallback(wl_resource* clientResource);
        static void OutputReleaseCallback(wl_client* client, wl_resource* clientResource);

        void sendOutputParams(uint32_t protocolVersion);

        const WaylandOutputParams       m_waylandOutputParams;
        const WaylandClientCredentials  m_clientCredentials;
        INativeWaylandResource*               m_resource = nullptr;
        const struct Output_Interface : public wl_output_interface
        {
            Output_Interface()
                : wl_output_interface()
            {
                release = OutputReleaseCallback;
            }
        } m_outputInterface;
    };
}
