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

        wl_shell*      m_shellRegistry = nullptr;
        wl_shell_surface* m_shellSurface  = nullptr;
    };
}

#endif
