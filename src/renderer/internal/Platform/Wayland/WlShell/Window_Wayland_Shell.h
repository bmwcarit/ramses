//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Platform/Wayland/Window_Wayland.h"

namespace ramses::internal
{
    class Window_Wayland_Shell : public Window_Wayland
    {
    public:
        Window_Wayland_Shell(const DisplayConfigData& displayConfig, IWindowEventHandler& windowEventHandler, uint32_t id, std::chrono::microseconds frameCallbackMaxPollTime);
        ~Window_Wayland_Shell() override;
        void setTitle(std::string_view title) override;

    private:
        void registryGlobalCreated(wl_registry* wl_registry, uint32_t name, const char* interface, uint32_t version) override;
        bool createSurface() override;
        void registerSurfaceListener();

        static void configureCallback(void* userData, wl_shell_surface* surface, uint32_t edges, int32_t width, int32_t height);
        static void pingCallback(void* userData, wl_shell_surface* surface, uint32_t serial);
        static void popupDoneCallback(void* userData, wl_shell_surface* surface);

        wl_shell*      m_shellRegistry = nullptr;
        wl_shell_surface* m_shellSurface  = nullptr;

        const struct Shell_Surface_Listener : public wl_shell_surface_listener
        {
            Shell_Surface_Listener() : wl_shell_surface_listener()
            {
                ping = pingCallback;
                configure = configureCallback;
                popup_done = popupDoneCallback;
            }
        } m_shellSurfaceListener;
    };
}
