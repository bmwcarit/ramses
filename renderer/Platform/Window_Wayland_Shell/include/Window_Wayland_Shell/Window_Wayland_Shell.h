//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_WINDOW_WAYLAND_SHELL_H
#define RAMSES_WINDOW_WAYLAND_SHELL_H

#include "Window_Wayland/Window_Wayland.h"

namespace ramses_internal
{
    class Window_Wayland_Shell : public Window_Wayland
    {
    public:
        Window_Wayland_Shell(const DisplayConfig& displayConfig, IWindowEventHandler& windowEventHandler, UInt32 id);
        ~Window_Wayland_Shell() override;
        void setTitle(const String& title) override;

    private:
        virtual void registryGlobalCreated(wl_registry* wl_registry,
                                           uint32_t     name,
                                           const char*  interface,
                                           uint32_t     version) override;
        virtual bool createSurface() override;
        void registerSurfaceListener();

        static void configureCallback(void* userData, wl_shell_surface* surface, uint32_t edges, int32_t width, int32_t height);
        static void pingCallback(void* userData, wl_shell_surface* surface, uint32_t serial);
        static void popupDoneCallback(void* userData, wl_shell_surface* surface);

        wl_shell*      m_shellRegistry = nullptr;
        wl_shell_surface* m_shellSurface  = nullptr;

        const struct Shell_Surface_Listener : public wl_shell_surface_listener
        {
            Shell_Surface_Listener()
            {
                ping = pingCallback;
                configure = configureCallback;
                popup_done = popupDoneCallback;
            }
        } m_shellSurfaceListener;
    };
}

#endif
