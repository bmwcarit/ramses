//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Platform/Wayland/EmbeddedCompositor/WaylandIVIApplicationConnection.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/WaylandSurface.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/WaylandIVISurface.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/NativeWaylandResource.h"
#include "internal/Core/Utils/ThreadLocalLogForced.h"
#include <cassert>

namespace ramses::internal
{
    WaylandIVIApplicationConnection::WaylandIVIApplicationConnection(IWaylandClient&             client,
                                                                     uint32_t                    version,
                                                                     uint32_t                    id,
                                                                     EmbeddedCompositor_Wayland& compositor)
        : m_compositor(compositor)
        , m_clientCredentials(client.getCredentials())
    {
        LOG_DEBUG(CONTEXT_RENDERER, "WaylandIVIApplicationConnection::WaylandIVIApplicationConnection Connection created");

        m_resource = client.resourceCreate(&ivi_application_interface, static_cast<int>(version), id);
        if (m_resource)
        {
            LOG_INFO(CONTEXT_RENDERER, "WaylandIVIApplicationConnection::WaylandIVIApplicationConnection(): ivi application interface is now provided  " << m_clientCredentials);

            m_resource->setImplementation(&m_iviApplicationInterface, this, ResourceDestroyedCallback);
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandIVIApplicationConnection::WaylandIVIApplicationConnection(): Could not create wayland resource  " << m_clientCredentials);
            client.postNoMemory();
        }
    }

    WaylandIVIApplicationConnection::~WaylandIVIApplicationConnection()
    {
        LOG_INFO(CONTEXT_RENDERER, "WaylandIVIApplicationConnection::~WaylandIVIApplicationConnection Connection destroyed  " << m_clientCredentials);

        if (m_resource)
        {
            // Remove ResourceDestroyedCallback
            m_resource->setImplementation(&m_iviApplicationInterface, this, nullptr);
            m_resource->destroy();
            delete m_resource;
        }
    }

    bool WaylandIVIApplicationConnection::wasSuccessfullyInitialized() const
    {
        return nullptr != m_resource;
    }

    void WaylandIVIApplicationConnection::resourceDestroyed()
    {
        LOG_TRACE(CONTEXT_RENDERER, "WaylandIVIApplicationConnection::iviApplicationConnectionResourceDestroyed");
        assert(nullptr != m_resource);

        // wl_resource is destroyed outside by the Wayland library
        // destroy m_resoruce to indicate that wl_resource_destroy does not neeed to be called in destructor
        delete m_resource;
        m_resource = nullptr;
    }

    void WaylandIVIApplicationConnection::iviApplicationIVISurfaceCreate(IWaylandClient&   client,
                                                                         WaylandIviSurfaceId iviId,
                                                                         INativeWaylandResource& surfaceResource,
                                                                         uint32_t          id)
    {
        LOG_INFO(CONTEXT_RENDERER, "WaylandIVIApplicationConnection::iviApplicationIVISurfaceCreate " << iviId);

        auto* clientSurface = reinterpret_cast<WaylandSurface*>(surfaceResource.getUserData());

        assert(nullptr != m_resource);
        auto* waylandIVISurface = new WaylandIVISurface(client, *m_resource, WaylandIviSurfaceId(iviId), clientSurface, id, m_compositor);
        // Registers callback for destruction, when the corresponding wl_ivi_surface is destroyed

        if (!waylandIVISurface->wasSuccessfullyInitialized())
        {
            delete waylandIVISurface;
        }
    }

    void WaylandIVIApplicationConnection::ResourceDestroyedCallback(wl_resource* iviApplicationConnectionResource)
    {
        auto* iviApplicationConnection =
            static_cast<WaylandIVIApplicationConnection*>(wl_resource_get_user_data(iviApplicationConnectionResource));

        iviApplicationConnection->resourceDestroyed();
        delete iviApplicationConnection;
    }

    void WaylandIVIApplicationConnection::IVIApplicationIVISurfaceCreateCallback(
        wl_client*   client,
        wl_resource* iviApplicationConnectionResource,
        uint32_t     iviId,
        wl_resource* surfaceResource,
        uint32_t     id)
    {
        const WaylandIviSurfaceId iviSurfaceId{iviId};
        LOG_INFO(CONTEXT_RENDERER,
                 "WaylandIVIApplicationConnection::IVIApplicationIVISurfaceCreateCallback " << iviSurfaceId << " id:" << id);
        auto* iviApplicationConnection =
            static_cast<WaylandIVIApplicationConnection*>(wl_resource_get_user_data(iviApplicationConnectionResource));
        WaylandClient waylandClient(client);
        NativeWaylandResource waylandSurfaceResource(surfaceResource);
        iviApplicationConnection->iviApplicationIVISurfaceCreate(waylandClient, iviSurfaceId, waylandSurfaceResource, id);
    }
}
