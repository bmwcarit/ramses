//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Platform/Wayland/EmbeddedCompositor/IWaylandSurface.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/IWaylandShellSurface.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/IWaylandClient.h"
#include "wayland-server.h"

#include <string>

namespace ramses::internal
{
    class WaylandShellSurface: public IWaylandShellSurface
    {
    public:
        WaylandShellSurface(IWaylandClient& client, INativeWaylandResource& shellConnectionResource, uint32_t id, IWaylandSurface& surface);
        ~WaylandShellSurface() override;
        [[nodiscard]] bool wasSuccessfullyInitialized() const;
        void resourceDestroyed() override;
        void surfaceWasDeleted() override;
        [[nodiscard]] const std::string& getTitle() const override;
        void shellSurfacePong(IWaylandClient& client, uint32_t serial) override;
        void shellSurfaceMove(IWaylandClient& client, INativeWaylandResource& seatResource, uint32_t serial) override;
        void shellSurfaceResize(IWaylandClient& client, INativeWaylandResource& seatResource, uint32_t serial, uint32_t edges) override;
        void shellSurfaceSetToplevel(IWaylandClient& client) override;
        void shellSurfaceSetTransient(IWaylandClient& client, INativeWaylandResource& parentSurfaceResource, int32_t x, int32_t y, uint32_t flags) override;
        void shellSurfaceSetFullscreen(IWaylandClient& client, uint32_t method, uint32_t framerate) override;
        void shellSurfaceSetPopup(IWaylandClient& client, INativeWaylandResource& seatResource, uint32_t serial, INativeWaylandResource& parentSurfaceResource, int32_t x, int32_t y, uint32_t flags) override;
        void shellSurfaceSetMaximized(IWaylandClient& client) override;
        void shellSurfaceSetTitle(IWaylandClient& client, const char* title) override;
        void shellSurfaceSetClass(IWaylandClient& client, const char* className) override;

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
                : wl_shell_surface_interface()
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

        const WaylandClientCredentials m_clientCredentials;
        INativeWaylandResource* m_resource = nullptr;
        IWaylandSurface* m_surface = nullptr;
        std::string m_title;
    };
}
