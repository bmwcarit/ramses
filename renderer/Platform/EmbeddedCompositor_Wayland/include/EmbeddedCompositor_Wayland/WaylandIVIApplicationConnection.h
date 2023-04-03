//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------
#ifndef RAMSES_WAYLANDIVIAPPLICATIONCONNECTION_H
#define RAMSES_WAYLANDIVIAPPLICATIONCONNECTION_H

#include "EmbeddedCompositor_Wayland/IWaylandIVIApplicationConnection.h"
#include "EmbeddedCompositor_Wayland/WaylandClient.h"
#include "ivi-application-server-protocol.h"
#include "wayland-server.h"

namespace ramses_internal
{
    class EmbeddedCompositor_Wayland;

    class WaylandIVIApplicationConnection: public IWaylandIVIApplicationConnection
    {
    public:
        WaylandIVIApplicationConnection(IWaylandClient& client, uint32_t version, uint32_t id, EmbeddedCompositor_Wayland& compositor);
        ~WaylandIVIApplicationConnection() override;
        [[nodiscard]] bool wasSuccessfullyInitialized() const;

        void resourceDestroyed() override;
        void iviApplicationIVISurfaceCreate(IWaylandClient& client, uint32_t iviId, INativeWaylandResource& surfaceResource, uint32_t id) override;

    private:
        static void ResourceDestroyedCallback(wl_resource* iviApplicationConnectionResource);
        static void IVIApplicationIVISurfaceCreateCallback(wl_client* client, wl_resource* iviApplicationConnectionResource, uint32_t iviId, wl_resource* surfaceResource, uint32_t id);

        EmbeddedCompositor_Wayland&     m_compositor;
        const WaylandClientCredentials  m_clientCredentials;
        INativeWaylandResource*               m_resource = nullptr;

        const struct IVIApplication_Interface : private ivi_application_interface
        {
            IVIApplication_Interface()
            {
                surface_create = IVIApplicationIVISurfaceCreateCallback;
            }
        } m_iviApplicationInterface;
    };
}

#endif
