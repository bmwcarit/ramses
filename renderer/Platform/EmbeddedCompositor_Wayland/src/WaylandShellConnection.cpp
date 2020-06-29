//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "EmbeddedCompositor_Wayland/WaylandShellConnection.h"
#include "EmbeddedCompositor_Wayland/WaylandShellSurface.h"
#include "EmbeddedCompositor_Wayland/IWaylandShellConnection.h"
#include "EmbeddedCompositor_Wayland/WaylandResource.h"
#include "Utils/LogMacros.h"
#include <cassert>

namespace ramses_internal
{
    WaylandShellConnection::WaylandShellConnection(IWaylandClient& client, uint32_t version, uint32_t id)
    {
        LOG_TRACE(CONTEXT_RENDERER, "WaylandShellConnection::WaylandShellConnection Connection created");

        m_resource = client.resourceCreate(&wl_shell_interface, version, id);
        if (m_resource)
        {
            m_resource->setImplementation(&m_shellInterface, this, ResourceDestroyedCallback);
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandShellConnection::WaylandShellConnection(): Could not create wayland resource");
            client.postNoMemory();
        }
    }

    WaylandShellConnection::~WaylandShellConnection()
    {
        LOG_TRACE(CONTEXT_RENDERER, "WaylandShellConnection::~WaylandShellConnection Connection destroyed");

        if (nullptr != m_resource)
        {
            // Remove ResourceDestroyedCallback
            m_resource->setImplementation(&m_shellInterface, this, nullptr);
            delete m_resource;
        }
    }

    bool WaylandShellConnection::wasSuccessfullyInitialized() const
    {
        return nullptr != m_resource;
    }

    void WaylandShellConnection::resourceDestroyed()
    {
        LOG_TRACE(CONTEXT_RENDERER, "WaylandShellConnection::resourceDestroyed");
        assert(nullptr != m_resource);

        // wl_resource is destroyed outside by the Wayland library, so m_resource looses the ownership of the
        // Wayland resource, so that we don't call wl_resource_destroy.
        m_resource->disownWaylandResource();
    }

    void WaylandShellConnection::shellGetShellSurface(IWaylandClient& client, uint32_t id, IWaylandResource& surfaceResource)
    {
        IWaylandSurface* clientSurface = reinterpret_cast<IWaylandSurface*>(surfaceResource.getUserData());
        assert(nullptr != clientSurface);
        assert(nullptr != m_resource);
        WaylandShellSurface* waylandShellSurface = new WaylandShellSurface(client, *m_resource, id, *clientSurface);
        // Registers callback for destruction, when the corresponding wl_shell_surface is destroyed

        if (!waylandShellSurface->wasSuccessfullyInitialized())
        {
            delete waylandShellSurface;
        }
    }

    void WaylandShellConnection::ResourceDestroyedCallback(wl_resource* shellConnectionResource)
    {
        WaylandShellConnection* shellConnection =
            static_cast<WaylandShellConnection*>(wl_resource_get_user_data(shellConnectionResource));

        shellConnection->resourceDestroyed();
        delete shellConnection;
    }

    void WaylandShellConnection::ShellGetShellSurfaceCallback(wl_client *client, wl_resource* shellConnectionResource, uint32_t id, wl_resource *surfaceResource)
    {
        WaylandShellConnection* shellConnection =
            static_cast<WaylandShellConnection*>(wl_resource_get_user_data(shellConnectionResource));

        WaylandClient waylandClient(client);
        WaylandResource waylandSurfaceResource(surfaceResource, false);
        shellConnection->shellGetShellSurface(waylandClient, id, waylandSurfaceResource);
    }
}
