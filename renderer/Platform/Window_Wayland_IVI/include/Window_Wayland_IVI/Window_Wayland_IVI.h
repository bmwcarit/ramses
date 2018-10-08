//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_WINDOW_WAYLAND_IVI_H
#define RAMSES_WINDOW_WAYLAND_IVI_H

#include "Window_Wayland/Window_Wayland.h"

struct ivi_application;
struct ivi_surface;

namespace ramses_internal
{
    class Window_Wayland_IVI : public Window_Wayland
    {
    public:
        Window_Wayland_IVI(const DisplayConfig& displayConfig, IWindowEventHandler& windowEventHandler, UInt32 id);
        ~Window_Wayland_IVI() override;

    private:

        virtual void registryGlobalCreated(wl_registry* wl_registry, uint32_t name, const char* interface, uint32_t version) override;
        virtual bool createSurface() override;

        ivi_application* m_iviApplicationRegistry = nullptr;
        ivi_surface*     m_iviApplicationSurface  = nullptr;
    };
}

#endif
