//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------
#pragma once

#include "internal/Platform/Wayland/EmbeddedCompositor/IWaylandCompositorConnection.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/IWaylandClient.h"
#include "wayland-server.h"

namespace ramses::internal
{
    class IEmbeddedCompositor_Wayland;

    class WaylandCompositorConnection: public IWaylandCompositorConnection
    {
    public:
        WaylandCompositorConnection(IWaylandClient& client, uint32_t version, uint32_t id, IEmbeddedCompositor_Wayland& embeddedCompositor);
        ~WaylandCompositorConnection() override;
        [[nodiscard]] bool wasSuccessfullyInitialized() const;

        void resourceDestroyed() override;
        void compositorCreateSurface(IWaylandClient& client, uint32_t id) override;
        void compositorCreateRegion(IWaylandClient& client, uint32_t id) override;

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
                : wl_compositor_interface()
            {
                create_surface = CompositorCreateSurfaceCallback;
                create_region  = CompositorCreateRegionCallback;
            }
        } m_compositorInterface;
    };
}
