//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------
#ifndef RAMSES_WAYLANDCOMPOSITORCONNECTION_H
#define RAMSES_WAYLANDCOMPOSITORCONNECTION_H

#include "EmbeddedCompositor_Wayland/IWaylandCompositorConnection.h"
#include "EmbeddedCompositor_Wayland/IWaylandClient.h"
#include "wayland-server.h"

namespace ramses_internal
{
    class IEmbeddedCompositor_Wayland;

    class WaylandCompositorConnection: public IWaylandCompositorConnection
    {
    public:
        WaylandCompositorConnection(IWaylandClient& client, uint32_t version, uint32_t id, IEmbeddedCompositor_Wayland& embeddedCompositor);
        virtual ~WaylandCompositorConnection() override;
        [[nodiscard]] bool wasSuccessfullyInitialized() const;

        virtual void resourceDestroyed() override;
        virtual void compositorCreateSurface(IWaylandClient& client, uint32_t id) override;
        virtual void compositorCreateRegion(IWaylandClient& client, uint32_t id) override;

    private:
        static void ResourceDestroyedCallback(wl_resource* clientResource);
        static void CompositorCreateSurfaceCallback(wl_client* client, wl_resource* clientResource, uint32_t id);
        static void CompositorCreateRegionCallback(wl_client* client, wl_resource* clientResource, uint32_t id);

        const WaylandClientCredentials  m_clientCredentials;
        INativeWaylandResource*           m_resource = nullptr;
        uint32_t                    m_version;
        IEmbeddedCompositor_Wayland& m_embeddedCompositor;

        const struct Compositor_Interface : private wl_compositor_interface
        {
            Compositor_Interface()
            {
                create_surface = CompositorCreateSurfaceCallback;
                create_region  = CompositorCreateRegionCallback;
            }
        } m_compositorInterface;
    };
}

#endif
