//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "EmbeddedCompositor_Wayland/WaylandCompositorConnection.h"
#include "EmbeddedCompositor_Wayland/EmbeddedCompositor_Wayland.h"
#include "EmbeddedCompositor_Wayland/WaylandSurface.h"
#include "EmbeddedCompositor_Wayland/WaylandRegion.h"
#include "EmbeddedCompositor_Wayland/WaylandClient.h"
#include "EmbeddedCompositor_Wayland/IWaylandResource.h"
#include "Utils/LogMacros.h"

namespace ramses_internal
{
    WaylandCompositorConnection::WaylandCompositorConnection(IWaylandClient& client, uint32_t version, uint32_t id, IEmbeddedCompositor_Wayland& embeddedCompositor)
        : m_version(version)
        , m_embeddedCompositor(embeddedCompositor)
    {
        LOG_DEBUG(CONTEXT_RENDERER, "WaylandCompositorConnection::WaylandCompositorConnection Connection created");

        uid_t userId;
        gid_t groupId;
        client.getCredentials(m_processId, userId, groupId);

        m_resource = client.resourceCreate(&wl_compositor_interface, version, id);
        if (nullptr != m_resource)
        {
            m_resource->setImplementation(&m_compositorInterface, this, ResourceDestroyedCallback);
            LOG_INFO(CONTEXT_RENDERER, "WaylandCompositorConnection::WaylandCompositorConnection(): Compositor interface is now provided to process with PID :" << m_processId);
        }
        else
        {
            //TODO Mohamed: enhance log message
            LOG_ERROR(CONTEXT_RENDERER, "WaylandCompositorConnection::WaylandCompositorConnection(): could not create wayland resource");
            client.postNoMemory();
        }
    }

    bool WaylandCompositorConnection::wasSuccessfullyInitialized() const
    {
        return nullptr != m_resource;
    }

    WaylandCompositorConnection::~WaylandCompositorConnection()
    {
        LOG_INFO(CONTEXT_RENDERER, "WaylandCompositorConnection::~WaylandCompositorConnection Connection destroyed created by process with PID: " << m_processId);

        m_embeddedCompositor.removeWaylandCompositorConnection(*this);
        if (nullptr != m_resource)
        {
            // Remove ResourceDestroyedCallback
            m_resource->setImplementation(&m_compositorInterface, this, nullptr);
            delete m_resource;
        }
    }

    void WaylandCompositorConnection::resourceDestroyed()
    {
        LOG_TRACE(CONTEXT_RENDERER, "WaylandCompositorConnection::resourceDestroyed");
        assert(nullptr != m_resource);

        // wl_resource is destroyed outside by the Wayland library, so m_resource loses the ownership of the
        // Wayland resource, so that we don't call wl_resource_destroy.
        m_resource->disownWaylandResource();
    }

    void WaylandCompositorConnection::compositorCreateSurface(IWaylandClient& client, uint32_t id)
    {
        LOG_TRACE(CONTEXT_RENDERER, "WaylandCompositorConnection::compositorCreateSurface");

        new WaylandSurface(m_embeddedCompositor, client, m_version, id);
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
        WaylandCompositorConnection* compositorConnection =
            static_cast<WaylandCompositorConnection*>(wl_resource_get_user_data(clientResource));

        compositorConnection->resourceDestroyed();
        delete compositorConnection;
    }

    void WaylandCompositorConnection::CompositorCreateSurfaceCallback(wl_client* client, wl_resource* clientResource, uint32_t id)
    {
        WaylandClient waylandClient(client);
        WaylandCompositorConnection* waylandCompositorConnection = static_cast<WaylandCompositorConnection*>(wl_resource_get_user_data(clientResource));
        waylandCompositorConnection->compositorCreateSurface(waylandClient, id);
    }

    void WaylandCompositorConnection::CompositorCreateRegionCallback(wl_client* client, wl_resource* clientResource, uint32_t id)
    {
        WaylandClient waylandClient(client);
        WaylandCompositorConnection* waylandCompositorConnection = static_cast<WaylandCompositorConnection*>(wl_resource_get_user_data(clientResource));
        waylandCompositorConnection->compositorCreateRegion(waylandClient, id);
    }
}
