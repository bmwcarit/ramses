//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "EmbeddedCompositor_Wayland/WaylandShellSurface.h"
#include "EmbeddedCompositor_Wayland/WaylandClient.h"
#include "EmbeddedCompositor_Wayland/WaylandResource.h"
#include "Utils/LogMacros.h"

namespace ramses_internal
{
    WaylandShellSurface::WaylandShellSurface(IWaylandClient& client, IWaylandResource& shellConnectionResource, uint32_t id, IWaylandSurface& surface)
    {
        LOG_INFO(CONTEXT_RENDERER, "WaylandShellSurface::WaylandShellSurface");

        m_resource = client.resourceCreate(&wl_shell_surface_interface, shellConnectionResource.getVersion(), id);
        if (nullptr != m_resource)
        {
            m_resource->setImplementation(&m_shellSurfaceInterface, this, ResourceDestroyedCallback);
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandShellSurface::WaylandShellSurface(): Could not create wayland resource!");
            client.postNoMemory();
        }

        if (surface.hasShellSurface())
        {
            // TODO (AI): Print out PID of process which created the new shell-surface
            LOG_ERROR(CONTEXT_RENDERER,
                      "WaylandShellSurface::WaylandShellSurface The surface already has a shell-surface attached!");
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
        LOG_INFO(CONTEXT_RENDERER, "WaylandShellSurface::~WaylandShellSurface");

        if (nullptr != m_surface)
        {
            m_surface->setShellSurface(nullptr);
        }

        if (nullptr != m_resource)
        {
            // Remove ResourceDestroyedCallback
            m_resource->setImplementation(&m_shellSurfaceInterface, this, nullptr);
            delete m_resource;
        }
    }

    void WaylandShellSurface::resourceDestroyed()
    {
        LOG_TRACE(CONTEXT_RENDERER, "WaylandShellSurface::resourceDestroyed");
        assert(nullptr != m_resource);

        // wl_resource is destroyed outside by the Wayland library, so m_resource loses the ownership of the
        // Wayland resource, so that we don't call wl_resource_destroy.
        m_resource->disownWaylandResource();
    }

    void WaylandShellSurface::surfaceWasDeleted()
    {
        m_surface = nullptr;
    }

    const String& WaylandShellSurface::getTitle() const
    {
        return m_title;
    }

    void WaylandShellSurface::shellSurfacePong(IWaylandClient& client, uint32_t serial)
    {
        LOG_INFO(CONTEXT_RENDERER, "WaylandShellSurface::shellSurfacePong");
        UNUSED(client)
        UNUSED(serial)

        if (m_surface == nullptr)
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandShellSurface::shellSurfacePong Surface has already been deleted!");
        }
    }

    void WaylandShellSurface::shellSurfaceMove(IWaylandClient& client, IWaylandResource& seatResource, uint32_t serial)
    {
        LOG_INFO(CONTEXT_RENDERER, "WaylandShellSurface::shellSurfaceMove");

        UNUSED(client)
        UNUSED(serial)
        UNUSED(seatResource)

        if (m_surface == nullptr)
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandShellSurface::shellSurfacePong Surface has already been deleted!");
        }
    }

    void WaylandShellSurface::shellSurfaceResize(IWaylandClient& client, IWaylandResource& seatResource, uint32_t serial, uint32_t edges)
    {
        LOG_INFO(CONTEXT_RENDERER, "WaylandShellSurface::shellSurfaceResize");

        UNUSED(client)
        UNUSED(serial)
        UNUSED(seatResource)
        UNUSED(edges)

        if (m_surface == nullptr)
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandShellSurface::shellSurfaceResize Surface has already been deleted!");
        }
    }

    void WaylandShellSurface::shellSurfaceSetToplevel(IWaylandClient& client)
    {
        LOG_INFO(CONTEXT_RENDERER, "WaylandShellSurface::shellSurfaceSetToplevel");

        UNUSED(client)

        if (m_surface == nullptr)
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandShellSurface::shellSurfaceSetToplevel Surface has already been deleted!");
        }
    }

    void WaylandShellSurface::shellSurfaceSetTransient(IWaylandClient& client, IWaylandResource& parentSurfaceResource, int32_t x, int32_t y, uint32_t flags)
    {
        LOG_INFO(CONTEXT_RENDERER, "WaylandShellSurface::shellSurfaceSetTransient");

        UNUSED(client)
        UNUSED(parentSurfaceResource)
        UNUSED(x)
        UNUSED(y)
        UNUSED(flags)

        if (m_surface == nullptr)
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandShellSurface::shellSurfaceSetTransient Surface has already been deleted!");
        }
    }

    void WaylandShellSurface::shellSurfaceSetFullscreen(IWaylandClient& client, uint32_t method, uint32_t framerate)
    {
        LOG_INFO(CONTEXT_RENDERER, "WaylandShellSurface::shellSurfaceSetFullscreen");

        UNUSED(client)
        UNUSED(method)
        UNUSED(framerate)

        if (m_surface == nullptr)
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandShellSurface::shellSurfaceSetFullscreen Surface has already been deleted!");
        }
    }

    void WaylandShellSurface::shellSurfaceSetPopup(IWaylandClient& client, IWaylandResource& seatResource, uint32_t serial, IWaylandResource& parentSurfaceResource, int32_t x,
                                                   int32_t y, uint32_t flags)
    {
        LOG_INFO(CONTEXT_RENDERER, "WaylandShellSurface::shellSurfaceSetPopup");

        UNUSED(client)
        UNUSED(seatResource)
        UNUSED(serial)
        UNUSED(parentSurfaceResource)
        UNUSED(x)
        UNUSED(y)
        UNUSED(flags)

        if (m_surface == nullptr)
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandShellSurface::shellSurfaceSetPopup Surface has already been deleted!");
        }
    }

    void WaylandShellSurface::shellSurfaceSetMaximized(IWaylandClient& client)
    {
        LOG_INFO(CONTEXT_RENDERER, "WaylandShellSurface::shellSurfaceSetMaximized");

        UNUSED(client)

        if (m_surface == nullptr)
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandShellSurface::shellSurfaceSetMaximized Surface has already been deleted!");
        }
    }

    void WaylandShellSurface::shellSurfaceSetTitle(IWaylandClient& client, const char* title)
    {
        LOG_INFO(CONTEXT_RENDERER, "WaylandShellSurface::shellSurfaceSetTitle title: " << title);
        UNUSED(client)

        if (m_surface == nullptr)
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandShellSurface::shellSurfacePong Surface has already been deleted!");
        }
        else
        {
            m_title = title;
        }
    }

    void WaylandShellSurface::shellSurfaceSetClass(IWaylandClient& client, const char* className)
    {
        LOG_INFO(CONTEXT_RENDERER, "WaylandShellSurface::shellSurfaceSetClass");

        UNUSED(client)
        UNUSED(className)

        if (m_surface == nullptr)
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandShellSurface::shellSurfaceSetClass Surface has already been deleted!");
        }
    }


    void WaylandShellSurface::ResourceDestroyedCallback(wl_resource* surfaceResource)
    {
        WaylandShellSurface* shellSurface = reinterpret_cast<WaylandShellSurface*>(wl_resource_get_user_data(surfaceResource));
        shellSurface->resourceDestroyed();
        delete shellSurface;
    }

    void WaylandShellSurface::ShellSurfacePongCallback(wl_client* client, wl_resource* surfaceResource, uint32_t serial)
    {
        WaylandShellSurface* shellSurface = reinterpret_cast<WaylandShellSurface*>(wl_resource_get_user_data(surfaceResource));
        WaylandClient waylandClient(client);
        shellSurface->shellSurfacePong(waylandClient, serial);
    }

    void WaylandShellSurface::ShellSurfaceMoveCallback(wl_client* client, wl_resource* surfaceResource, wl_resource* seatResource, uint32_t serial)
    {
        WaylandShellSurface* shellSurface = reinterpret_cast<WaylandShellSurface*>(wl_resource_get_user_data(surfaceResource));
        WaylandClient waylandClient(client);
        WaylandResource waylandSeatResource(seatResource, false);
        shellSurface->shellSurfaceMove(waylandClient, waylandSeatResource, serial);
    }

    void WaylandShellSurface::ShellSurfaceResizeCallback(wl_client* client, wl_resource* surfaceResource, wl_resource* seatResource, uint32_t serial, uint32_t edges)
    {
        WaylandShellSurface* shellSurface = reinterpret_cast<WaylandShellSurface*>(wl_resource_get_user_data(surfaceResource));
        WaylandClient waylandClient(client);
        WaylandResource waylandSeatResource(seatResource, false);
        shellSurface->shellSurfaceResize(waylandClient, waylandSeatResource, serial, edges);
    }

    void WaylandShellSurface::ShellSurfaceSetToplevelCallback(wl_client* client, wl_resource* surfaceResource)
    {
        WaylandShellSurface* shellSurface = reinterpret_cast<WaylandShellSurface*>(wl_resource_get_user_data(surfaceResource));
        WaylandClient waylandClient(client);
        shellSurface->shellSurfaceSetToplevel(waylandClient);
    }

    void WaylandShellSurface::ShellSurfaceSetTransientCallback(wl_client* client, wl_resource* surfaceResource, wl_resource* parentSurfaceResource, int32_t x,
                                                               int32_t y, uint32_t flags)
    {
        WaylandShellSurface* shellSurface = reinterpret_cast<WaylandShellSurface*>(wl_resource_get_user_data(surfaceResource));
        WaylandClient waylandClient(client);
        WaylandResource waylandParentSurfaceResource(parentSurfaceResource, false);
        shellSurface->shellSurfaceSetTransient(waylandClient, waylandParentSurfaceResource, x, y, flags);
    }

    void WaylandShellSurface::ShellSurfaceSetFullscreenCallback(wl_client* client, wl_resource* surfaceResource, uint32_t method, uint32_t framerate,
                                                                wl_resource* /*outputResource*/)
    {
        WaylandShellSurface* shellSurface = reinterpret_cast<WaylandShellSurface*>(wl_resource_get_user_data(surfaceResource));
        WaylandClient waylandClient(client);

        shellSurface->shellSurfaceSetFullscreen(waylandClient, method, framerate);
    }

    void WaylandShellSurface::ShellSurfaceSetPopupCallback(wl_client* client, wl_resource* surfaceResource, wl_resource* seatResource, uint32_t serial,
                                                           wl_resource* parentSurfaceResource, int32_t x, int32_t y, uint32_t flags)
    {
        WaylandShellSurface* shellSurface = reinterpret_cast<WaylandShellSurface*>(wl_resource_get_user_data(surfaceResource));
        WaylandClient waylandClient(client);
        WaylandResource waylandSeatResource(seatResource, false);
        WaylandResource waylandParentSurfaceResource(parentSurfaceResource, false);
        shellSurface->shellSurfaceSetPopup(waylandClient, waylandSeatResource, serial, waylandParentSurfaceResource, x, y, flags);
    }

    void WaylandShellSurface::ShellSurfaceSetMaximizedCallback(wl_client* client, wl_resource* surfaceResource, wl_resource* /*outputResource*/)
    {
        WaylandShellSurface* shellSurface = reinterpret_cast<WaylandShellSurface*>(wl_resource_get_user_data(surfaceResource));
        WaylandClient waylandClient(client);

        shellSurface->shellSurfaceSetMaximized(waylandClient);
    }

    void WaylandShellSurface::ShellSurfaceSetTitleCallback(wl_client* client, wl_resource* surfaceResource, const char* title)
    {
        WaylandShellSurface* shellSurface = reinterpret_cast<WaylandShellSurface*>(wl_resource_get_user_data(surfaceResource));
        WaylandClient waylandClient(client);
        shellSurface->shellSurfaceSetTitle(waylandClient, title);
    }

    void WaylandShellSurface::ShellSurfaceSetClassCallback(wl_client* client, wl_resource* surfaceResource, const char* className)
    {
        WaylandShellSurface* shellSurface = reinterpret_cast<WaylandShellSurface*>(wl_resource_get_user_data(surfaceResource));
        WaylandClient waylandClient(client);
        shellSurface->shellSurfaceSetClass(waylandClient, className);
    }
};
