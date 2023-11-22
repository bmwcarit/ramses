//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Platform/Wayland/EmbeddedCompositor/WaylandCompositorConnection.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/EmbeddedCompositor_Wayland.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/WaylandSurface.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/WaylandRegion.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/WaylandClient.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/INativeWaylandResource.h"
#include "internal/Core/Utils/LogMacros.h"
#include <cassert>

namespace ramses::internal
{
    WaylandCompositorConnection::WaylandCompositorConnection(IWaylandClient& client, uint32_t version, uint32_t id, IEmbeddedCompositor_Wayland& embeddedCompositor)
        : m_clientCredentials(client.getCredentials())
        , m_version(version)
        , m_embeddedCompositor(embeddedCompositor)
    {
        LOG_DEBUG(CONTEXT_RENDERER, "WaylandCompositorConnection::WaylandCompositorConnection Connection created");

        m_resource = client.resourceCreate(&wl_compositor_interface, static_cast<int>(version), id);
        if (nullptr != m_resource)
        {
            m_resource->setImplementation(&m_compositorInterface, this, ResourceDestroyedCallback);
            LOG_INFO(CONTEXT_RENDERER, "WaylandCompositorConnection::WaylandCompositorConnection(): Compositor interface is now provided {}", m_clientCredentials);
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandCompositorConnection::WaylandCompositorConnection(): could not create wayland resource {}", m_clientCredentials);
            client.postNoMemory();
        }
    }

    bool WaylandCompositorConnection::wasSuccessfullyInitialized() const
    {
        return nullptr != m_resource;
    }

    WaylandCompositorConnection::~WaylandCompositorConnection()
    {
        LOG_INFO(CONTEXT_RENDERER, "WaylandCompositorConnection::~WaylandCompositorConnection Connection destroyed {}", m_clientCredentials);

        m_embeddedCompositor.removeWaylandCompositorConnection(*this);
        if (nullptr != m_resource)
        {
            // Remove ResourceDestroyedCallback
            m_resource->setImplementation(&m_compositorInterface, this, nullptr);
            m_resource->destroy();
            delete m_resource;
        }
    }

    void WaylandCompositorConnection::resourceDestroyed()
    {
        LOG_TRACE(CONTEXT_RENDERER, "WaylandCompositorConnection::resourceDestroyed");
        assert(nullptr != m_resource);

        // wl_resource is destroyed outside by the Wayland library
        // destroy m_resoruce to indicate that wl_resource_destroy does not neeed to be called in destructor
        delete m_resource;
        m_resource = nullptr;
    }

    void WaylandCompositorConnection::compositorCreateSurface(IWaylandClient& client, uint32_t id)
    {
        LOG_TRACE(CONTEXT_RENDERER, "WaylandCompositorConnection::compositorCreateSurface");

        new WaylandSurface(m_embeddedCompositor, client, static_cast<int>(m_version), id);
        // Registers callback for destruction, when the corresponding wl_surface is destroyed
    }

    void WaylandCompositorConnection::compositorCreateRegion(IWaylandClient& client, uint32_t id)
    {
        LOG_TRACE(CONTEXT_RENDERER, "WaylandCompositorConnection::compositorCreateRegion");

        new WaylandRegion(m_embeddedCompositor, client, m_version, id);
        // Registers callback for destruction, when the corresponding wl_region is destroyed
    }

    void WaylandCompositorConnection::ResourceDestroyedCallback(wl_resource* clientResource)
    {
        auto* compositorConnection =
            static_cast<WaylandCompositorConnection*>(wl_resource_get_user_data(clientResource));

        compositorConnection->resourceDestroyed();
        delete compositorConnection;
    }

    void WaylandCompositorConnection::CompositorCreateSurfaceCallback(wl_client* client, wl_resource* clientResource, uint32_t id)
    {
        WaylandClient waylandClient(client);
        auto* waylandCompositorConnection = static_cast<WaylandCompositorConnection*>(wl_resource_get_user_data(clientResource));
        waylandCompositorConnection->compositorCreateSurface(waylandClient, id);
    }

    void WaylandCompositorConnection::CompositorCreateRegionCallback(wl_client* client, wl_resource* clientResource, uint32_t id)
    {
        WaylandClient waylandClient(client);
        auto* waylandCompositorConnection = static_cast<WaylandCompositorConnection*>(wl_resource_get_user_data(clientResource));
        waylandCompositorConnection->compositorCreateRegion(waylandClient, id);
    }
}
