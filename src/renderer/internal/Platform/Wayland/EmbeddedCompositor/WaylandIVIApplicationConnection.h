//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------
#pragma once

#include "internal/Platform/Wayland/EmbeddedCompositor/IWaylandIVIApplicationConnection.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/WaylandClient.h"
#include "ivi-application-server-protocol.h"
#include "wayland-server.h"

namespace ramses::internal
{
    class EmbeddedCompositor_Wayland;

    class WaylandIVIApplicationConnection: public IWaylandIVIApplicationConnection
    {
    public:
        WaylandIVIApplicationConnection(IWaylandClient& client, uint32_t version, uint32_t id, EmbeddedCompositor_Wayland& compositor);
        ~WaylandIVIApplicationConnection() override;
        [[nodiscard]] bool wasSuccessfullyInitialized() const;

        void resourceDestroyed() override;
        void iviApplicationIVISurfaceCreate(IWaylandClient& client, WaylandIviSurfaceId iviId, INativeWaylandResource& surfaceResource, uint32_t id) override;

    private:
        static void ResourceDestroyedCallback(wl_resource* iviApplicationConnectionResource);
        static void IVIApplicationIVISurfaceCreateCallback(wl_client* client, wl_resource* iviApplicationConnectionResource, uint32_t iviId, wl_resource* surfaceResource, uint32_t id);

        EmbeddedCompositor_Wayland&     m_compositor;
        const WaylandClientCredentials  m_clientCredentials;
        INativeWaylandResource*               m_resource = nullptr;

        const struct IVIApplication_Interface : private ivi_application_interface
        {
            IVIApplication_Interface()
                : ivi_application_interface()
            {
                surface_create = IVIApplicationIVISurfaceCreateCallback;
            }
        } m_iviApplicationInterface;
    };
}
