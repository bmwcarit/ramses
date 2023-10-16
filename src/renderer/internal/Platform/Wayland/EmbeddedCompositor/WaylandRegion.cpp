//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Platform/Wayland/EmbeddedCompositor/WaylandRegion.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/WaylandClient.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/INativeWaylandResource.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/IEmbeddedCompositor_Wayland.h"
#include "internal/Core/Utils/ThreadLocalLogForced.h"

namespace ramses::internal
{
    WaylandRegion::WaylandRegion(IEmbeddedCompositor_Wayland& compositor,
                                 IWaylandClient&              client,
                                 uint32_t                     version,
                                 uint32_t                     id)
        : m_compositor(compositor)
    {
        LOG_TRACE(CONTEXT_RENDERER, "WaylandRegion::WaylandRegion");

        m_resource = client.resourceCreate(&wl_region_interface, static_cast<int>(version), id);
        if (nullptr != m_resource)
        {
            m_resource->setImplementation(&m_regionInterface, this, ResourceDestroyedCallback);
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandRegion::WaylandRegion Could not create wayland region!");
            client.postNoMemory();
        }

        m_compositor.addWaylandRegion(*this);
    }

    WaylandRegion::~WaylandRegion()
    {
        LOG_TRACE(CONTEXT_RENDERER, "WaylandRegion::~WaylandRegion");

        m_compositor.removeWaylandRegion(*this);
        if (nullptr != m_resource)
        {
            // Remove ResourceDestroyedCallback
            m_resource->setImplementation(&m_regionInterface, this, nullptr);
            m_resource->destroy();
            delete m_resource;
        }
    }

    void WaylandRegion::resourceDestroyed()
    {
        LOG_TRACE(CONTEXT_RENDERER, "WaylandRegion::resourceDestroyed");

        // wl_resource is destroyed outside by the Wayland library
        // destroy m_resoruce to indicate that wl_resource_destroy does not neeed to be called in destructor
        delete m_resource;
        m_resource = nullptr;
    }

    void WaylandRegion::regionAdd([[maybe_unused]] IWaylandClient& client, int32_t x, int32_t y, int32_t width, int32_t height)
    {
        LOG_TRACE(CONTEXT_RENDERER, "WaylandRegion::regionAdd x: " << x << " y: " << y << " width: " << width << " height: " << height);
    }

    void WaylandRegion::regionSubtract([[maybe_unused]] IWaylandClient& client, int32_t x, int32_t y, int32_t width, int32_t height)
    {
        LOG_TRACE(CONTEXT_RENDERER,
                  "WaylandRegion::regionSubtract x: " << x << " y: " << y << " width: " << width << " height: " << height);
    }

    void WaylandRegion::RegionDestroyCallback([[maybe_unused]] wl_client* client, wl_resource* regionResource)
    {
        auto* region = static_cast<WaylandRegion*>(wl_resource_get_user_data(regionResource));
        delete region;
    }

    void WaylandRegion::RegionAddCallback(
        wl_client* client, wl_resource* regionResource, int32_t x, int32_t y, int32_t width, int32_t height)
    {
        auto* region = static_cast<WaylandRegion*>(wl_resource_get_user_data(regionResource));
        WaylandClient  waylandClient(client);
        region->regionAdd(waylandClient, x, y, width, height);
    }

    void WaylandRegion::RegionSubtractCallback(
        wl_client* client, wl_resource* regionResource, int32_t x, int32_t y, int32_t width, int32_t height)
    {
        auto* region = static_cast<WaylandRegion*>(wl_resource_get_user_data(regionResource));
        WaylandClient  waylandClient(client);
        region->regionSubtract(waylandClient, x, y, width, height);
    }

    void WaylandRegion::ResourceDestroyedCallback(wl_resource* regionResource)
    {
        auto* region = static_cast<WaylandRegion*>(wl_resource_get_user_data(regionResource));
        region->resourceDestroyed();
        delete region;
    }
}
