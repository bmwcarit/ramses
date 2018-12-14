//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_WAYLANDSHELLSURFACE_H
#define RAMSES_WAYLANDSHELLSURFACE_H

#include "wayland-server.h"
#include "EmbeddedCompositor_Wayland/IWaylandSurface.h"
#include "EmbeddedCompositor_Wayland/IWaylandShellSurface.h"
#include "Collections/String.h"

namespace ramses_internal
{
    class IWaylandResource;
    class WaylandShellSurface: public IWaylandShellSurface
    {
    public:
        WaylandShellSurface(IWaylandClient& client, IWaylandResource& shellConnectionResource, uint32_t id, IWaylandSurface& surface);
        ~WaylandShellSurface();
        bool wasSuccessfullyInitialized() const;
        virtual void resourceDestroyed() override;
        virtual void surfaceWasDeleted() override;
        virtual const String& getTitle() const override;
        virtual void shellSurfacePong(IWaylandClient& client, uint32_t serial) override;
        virtual void shellSurfaceMove(IWaylandClient& client, IWaylandResource& seatResource, uint32_t serial) override;
        virtual void shellSurfaceResize(IWaylandClient& client, IWaylandResource& seatResource, uint32_t serial, uint32_t edges) override;
        virtual void shellSurfaceSetToplevel(IWaylandClient& client) override;
        virtual void shellSurfaceSetTransient(IWaylandClient& client, IWaylandResource& parentSurfaceResource, int32_t x, int32_t y, uint32_t flags) override;
        virtual void shellSurfaceSetFullscreen(IWaylandClient& client, uint32_t method, uint32_t framerate) override;
        virtual void shellSurfaceSetPopup(IWaylandClient& client, IWaylandResource& seatResource, uint32_t serial, IWaylandResource& parentSurfaceResource, int32_t x, int32_t y, uint32_t flags) override;
        virtual void shellSurfaceSetMaximized(IWaylandClient& client) override;
        virtual void shellSurfaceSetTitle(IWaylandClient& client, const char* title) override;
        virtual void shellSurfaceSetClass(IWaylandClient& client, const char* className) override;

    private:
        static void ShellSurfacePongCallback(wl_client* client, wl_resource* surfaceResource, uint32_t serial);
        static void ShellSurfaceMoveCallback(wl_client* client, wl_resource* surfaceResource, wl_resource* seatResource, uint32_t serial);
        static void ShellSurfaceResizeCallback(wl_client* client, wl_resource* surfaceResource, wl_resource* seatResource, uint32_t serial, uint32_t edges);
        static void ShellSurfaceSetToplevelCallback(wl_client* client, wl_resource* surfaceResource);
        static void ShellSurfaceSetTransientCallback(wl_client* client, wl_resource* surfaceResource, wl_resource* parentSurfaceResource, int32_t x, int32_t y,
                                                     uint32_t flags);
        static void ShellSurfaceSetFullscreenCallback(wl_client* client, wl_resource* surfaceResource, uint32_t method, uint32_t framerate,
                                                      wl_resource* outputResource);
        static void ShellSurfaceSetPopupCallback(wl_client* client, wl_resource* surfaceResource, wl_resource* seatResource, uint32_t serial,
                                                 wl_resource* parentSurfaceResource, int32_t x, int32_t y, uint32_t flags);
        static void ShellSurfaceSetMaximizedCallback(wl_client* client, wl_resource* surfaceResource, wl_resource* outputResource);
        static void ShellSurfaceSetTitleCallback(wl_client* client, wl_resource* surfaceResource, const char* title);
        static void ShellSurfaceSetClassCallback(wl_client* client, wl_resource* surfaceResource, const char* className);

        static void ResourceDestroyedCallback(wl_resource* surfaceResource);


        const struct ShellSurface_Interface : private wl_shell_surface_interface
        {
            ShellSurface_Interface()
            {
                pong           = ShellSurfacePongCallback;
                move           = ShellSurfaceMoveCallback;
                resize         = ShellSurfaceResizeCallback;
                set_toplevel   = ShellSurfaceSetToplevelCallback;
                set_transient  = ShellSurfaceSetTransientCallback;
                set_fullscreen = ShellSurfaceSetFullscreenCallback;
                set_popup      = ShellSurfaceSetPopupCallback;
                set_maximized  = ShellSurfaceSetMaximizedCallback;
                set_title      = ShellSurfaceSetTitleCallback;
                set_class      = ShellSurfaceSetClassCallback;
            }
        } m_shellSurfaceInterface;

        IWaylandResource* m_resource = nullptr;
        IWaylandSurface* m_surface = nullptr;
        String m_title;
    };
}

#endif
