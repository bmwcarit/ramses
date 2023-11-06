//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Platform/Wayland/EmbeddedCompositor/WaylandShellSurface.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/WaylandClient.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/NativeWaylandResource.h"
#include "internal/Core/Utils/LogMacros.h"
#include <cassert>

namespace ramses::internal
{
    WaylandShellSurface::WaylandShellSurface(IWaylandClient& client, INativeWaylandResource& shellConnectionResource, uint32_t id, IWaylandSurface& surface)
        : m_clientCredentials(client.getCredentials())
    {
        LOG_INFO(CONTEXT_RENDERER, "WaylandShellSurface::WaylandShellSurface  " << m_clientCredentials);

        m_resource = client.resourceCreate(&wl_shell_surface_interface, shellConnectionResource.getVersion(), id);
        if (nullptr != m_resource)
        {
            m_resource->setImplementation(&m_shellSurfaceInterface, this, ResourceDestroyedCallback);
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandShellSurface::WaylandShellSurface(): Could not create wayland resource  " << m_clientCredentials);
            client.postNoMemory();
        }

        if (surface.hasShellSurface())
        {
            LOG_ERROR(CONTEXT_RENDERER,
                      "WaylandShellSurface::WaylandShellSurface The surface already has a shell-surface attached  " << m_clientCredentials);
            shellConnectionResource.postError(WL_SHELL_ERROR_ROLE, "surface already has a shell-surface");
        }
        else
        {
            m_surface = &surface;
            surface.setShellSurface(this);
        }
    }

    bool WaylandShellSurface::wasSuccessfullyInitialized() const
    {
        return nullptr != m_resource;
    }

    WaylandShellSurface::~WaylandShellSurface()
    {
        LOG_INFO(CONTEXT_RENDERER, "WaylandShellSurface::~WaylandShellSurface  " << m_clientCredentials);

        if (nullptr != m_surface)
        {
            m_surface->setShellSurface(nullptr);
        }

        if (nullptr != m_resource)
        {
            // Remove ResourceDestroyedCallback
            m_resource->setImplementation(&m_shellSurfaceInterface, this, nullptr);
            m_resource->destroy();
            delete m_resource;
        }
    }

    void WaylandShellSurface::resourceDestroyed()
    {
        LOG_TRACE(CONTEXT_RENDERER, "WaylandShellSurface::resourceDestroyed");
        assert(nullptr != m_resource);

        // wl_resource is destroyed outside by the Wayland library
        // destroy m_resoruce to indicate that wl_resource_destroy does not neeed to be called in destructor
        delete m_resource;
        m_resource = nullptr;
    }

    void WaylandShellSurface::surfaceWasDeleted()
    {
        m_surface = nullptr;
    }

    const std::string& WaylandShellSurface::getTitle() const
    {
        return m_title;
    }

    void WaylandShellSurface::shellSurfacePong([[maybe_unused]] IWaylandClient& client, [[maybe_unused]] uint32_t serial)
    {
        LOG_INFO(CONTEXT_RENDERER, "WaylandShellSurface::shellSurfacePong");

        if (m_surface == nullptr)
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandShellSurface::shellSurfacePong Surface has already been deleted!");
        }
    }

    void WaylandShellSurface::shellSurfaceMove([[maybe_unused]] IWaylandClient& client, [[maybe_unused]] INativeWaylandResource& seatResource, [[maybe_unused]] uint32_t serial)
    {
        LOG_INFO(CONTEXT_RENDERER, "WaylandShellSurface::shellSurfaceMove");

        if (m_surface == nullptr)
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandShellSurface::shellSurfacePong Surface has already been deleted!");
        }
    }

    void WaylandShellSurface::shellSurfaceResize([[maybe_unused]] IWaylandClient&         client,
                                                 [[maybe_unused]] INativeWaylandResource& seatResource,
                                                 [[maybe_unused]] uint32_t                serial,
                                                 [[maybe_unused]] uint32_t                edges)
    {
        LOG_INFO(CONTEXT_RENDERER, "WaylandShellSurface::shellSurfaceResize");

        if (m_surface == nullptr)
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandShellSurface::shellSurfaceResize Surface has already been deleted!");
        }
    }

    void WaylandShellSurface::shellSurfaceSetToplevel([[maybe_unused]] IWaylandClient& client)
    {
        LOG_INFO(CONTEXT_RENDERER, "WaylandShellSurface::shellSurfaceSetToplevel");

        if (m_surface == nullptr)
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandShellSurface::shellSurfaceSetToplevel Surface has already been deleted!");
        }
    }

    void WaylandShellSurface::shellSurfaceSetTransient([[maybe_unused]] IWaylandClient&         client,
                                                       [[maybe_unused]] INativeWaylandResource& parentSurfaceResource,
                                                       [[maybe_unused]] int32_t                 x,
                                                       [[maybe_unused]] int32_t                 y,
                                                       [[maybe_unused]] uint32_t                flags)
    {
        LOG_INFO(CONTEXT_RENDERER, "WaylandShellSurface::shellSurfaceSetTransient");

        if (m_surface == nullptr)
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandShellSurface::shellSurfaceSetTransient Surface has already been deleted!");
        }
    }

    void WaylandShellSurface::shellSurfaceSetFullscreen([[maybe_unused]] IWaylandClient& client, [[maybe_unused]] uint32_t method, [[maybe_unused]] uint32_t framerate)
    {
        LOG_INFO(CONTEXT_RENDERER, "WaylandShellSurface::shellSurfaceSetFullscreen");

        if (m_surface == nullptr)
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandShellSurface::shellSurfaceSetFullscreen Surface has already been deleted!");
        }
    }

    void WaylandShellSurface::shellSurfaceSetPopup([[maybe_unused]] IWaylandClient&         client,
                                                   [[maybe_unused]] INativeWaylandResource& seatResource,
                                                   [[maybe_unused]] uint32_t                serial,
                                                   [[maybe_unused]] INativeWaylandResource& parentSurfaceResource,
                                                   [[maybe_unused]] int32_t                 x,
                                                   [[maybe_unused]] int32_t                 y,
                                                   [[maybe_unused]] uint32_t                flags)
    {
        LOG_INFO(CONTEXT_RENDERER, "WaylandShellSurface::shellSurfaceSetPopup");

        if (m_surface == nullptr)
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandShellSurface::shellSurfaceSetPopup Surface has already been deleted!");
        }
    }

    void WaylandShellSurface::shellSurfaceSetMaximized([[maybe_unused]] IWaylandClient& client)
    {
        LOG_INFO(CONTEXT_RENDERER, "WaylandShellSurface::shellSurfaceSetMaximized");

        if (m_surface == nullptr)
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandShellSurface::shellSurfaceSetMaximized Surface has already been deleted!");
        }
    }

    void WaylandShellSurface::shellSurfaceSetTitle([[maybe_unused]] IWaylandClient& client, const char* title)
    {
        LOG_INFO(CONTEXT_RENDERER, "WaylandShellSurface::shellSurfaceSetTitle title: " << title);

        if (m_surface == nullptr)
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandShellSurface::shellSurfacePong Surface has already been deleted!");
        }
        else
        {
            m_title = title;
        }
    }

    void WaylandShellSurface::shellSurfaceSetClass([[maybe_unused]] IWaylandClient& client, [[maybe_unused]] const char* className)
    {
        LOG_INFO(CONTEXT_RENDERER, "WaylandShellSurface::shellSurfaceSetClass");

        if (m_surface == nullptr)
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandShellSurface::shellSurfaceSetClass Surface has already been deleted!");
        }
    }


    void WaylandShellSurface::ResourceDestroyedCallback(wl_resource* surfaceResource)
    {
        auto* shellSurface = static_cast<WaylandShellSurface*>(wl_resource_get_user_data(surfaceResource));
        shellSurface->resourceDestroyed();
        delete shellSurface;
    }

    void WaylandShellSurface::ShellSurfacePongCallback(wl_client* client, wl_resource* surfaceResource, uint32_t serial)
    {
        auto* shellSurface = static_cast<WaylandShellSurface*>(wl_resource_get_user_data(surfaceResource));
        WaylandClient waylandClient(client);
        shellSurface->shellSurfacePong(waylandClient, serial);
    }

    void WaylandShellSurface::ShellSurfaceMoveCallback(wl_client* client, wl_resource* surfaceResource, wl_resource* seatResource, uint32_t serial)
    {
        auto* shellSurface = static_cast<WaylandShellSurface*>(wl_resource_get_user_data(surfaceResource));
        WaylandClient waylandClient(client);
        NativeWaylandResource waylandSeatResource(seatResource);
        shellSurface->shellSurfaceMove(waylandClient, waylandSeatResource, serial);
    }

    void WaylandShellSurface::ShellSurfaceResizeCallback(wl_client* client, wl_resource* surfaceResource, wl_resource* seatResource, uint32_t serial, uint32_t edges)
    {
        auto* shellSurface = static_cast<WaylandShellSurface*>(wl_resource_get_user_data(surfaceResource));
        WaylandClient waylandClient(client);
        NativeWaylandResource waylandSeatResource(seatResource);
        shellSurface->shellSurfaceResize(waylandClient, waylandSeatResource, serial, edges);
    }

    void WaylandShellSurface::ShellSurfaceSetToplevelCallback(wl_client* client, wl_resource* surfaceResource)
    {
        auto* shellSurface = static_cast<WaylandShellSurface*>(wl_resource_get_user_data(surfaceResource));
        WaylandClient waylandClient(client);
        shellSurface->shellSurfaceSetToplevel(waylandClient);
    }

    void WaylandShellSurface::ShellSurfaceSetTransientCallback(wl_client* client, wl_resource* surfaceResource, wl_resource* parentSurfaceResource, int32_t x,
                                                               int32_t y, uint32_t flags)
    {
        auto* shellSurface = static_cast<WaylandShellSurface*>(wl_resource_get_user_data(surfaceResource));
        WaylandClient waylandClient(client);
        NativeWaylandResource waylandParentSurfaceResource(parentSurfaceResource);
        shellSurface->shellSurfaceSetTransient(waylandClient, waylandParentSurfaceResource, x, y, flags);
    }

    void WaylandShellSurface::ShellSurfaceSetFullscreenCallback(wl_client* client, wl_resource* surfaceResource, uint32_t method, uint32_t framerate,
                                                                wl_resource* /*outputResource*/)
    {
        auto* shellSurface = static_cast<WaylandShellSurface*>(wl_resource_get_user_data(surfaceResource));
        WaylandClient waylandClient(client);

        shellSurface->shellSurfaceSetFullscreen(waylandClient, method, framerate);
    }

    void WaylandShellSurface::ShellSurfaceSetPopupCallback(wl_client* client, wl_resource* surfaceResource, wl_resource* seatResource, uint32_t serial,
                                                           wl_resource* parentSurfaceResource, int32_t x, int32_t y, uint32_t flags)
    {
        auto* shellSurface = static_cast<WaylandShellSurface*>(wl_resource_get_user_data(surfaceResource));
        WaylandClient waylandClient(client);
        NativeWaylandResource waylandSeatResource(seatResource);
        NativeWaylandResource waylandParentSurfaceResource(parentSurfaceResource);
        shellSurface->shellSurfaceSetPopup(waylandClient, waylandSeatResource, serial, waylandParentSurfaceResource, x, y, flags);
    }

    void WaylandShellSurface::ShellSurfaceSetMaximizedCallback(wl_client* client, wl_resource* surfaceResource, wl_resource* /*outputResource*/)
    {
        auto* shellSurface = static_cast<WaylandShellSurface*>(wl_resource_get_user_data(surfaceResource));
        WaylandClient waylandClient(client);

        shellSurface->shellSurfaceSetMaximized(waylandClient);
    }

    void WaylandShellSurface::ShellSurfaceSetTitleCallback(wl_client* client, wl_resource* surfaceResource, const char* title)
    {
        auto* shellSurface = static_cast<WaylandShellSurface*>(wl_resource_get_user_data(surfaceResource));
        WaylandClient waylandClient(client);
        shellSurface->shellSurfaceSetTitle(waylandClient, title);
    }

    void WaylandShellSurface::ShellSurfaceSetClassCallback(wl_client* client, wl_resource* surfaceResource, const char* className)
    {
        auto* shellSurface = static_cast<WaylandShellSurface*>(wl_resource_get_user_data(surfaceResource));
        WaylandClient waylandClient(client);
        shellSurface->shellSurfaceSetClass(waylandClient, className);
    }
};
