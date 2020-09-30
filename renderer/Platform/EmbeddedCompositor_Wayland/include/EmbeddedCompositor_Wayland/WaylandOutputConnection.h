//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------
#ifndef RAMSES_WAYLANDOUTPUTCONNECTION_H
#define RAMSES_WAYLANDOUTPUTCONNECTION_H

#include "EmbeddedCompositor_Wayland/IWaylandClient.h"
#include "EmbeddedCompositor_Wayland/WaylandOutputParams.h"
#include "wayland-server.h"

namespace ramses_internal
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
            {
                release = OutputReleaseCallback;
            }
        } m_outputInterface;
    };
}

#endif
