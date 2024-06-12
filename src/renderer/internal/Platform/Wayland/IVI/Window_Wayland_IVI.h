//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Platform/Wayland/Window_Wayland.h"
#include "ivi-application-client-protocol.h"

struct ivi_application;
struct ivi_surface;

namespace ramses::internal
{
    class Window_Wayland_IVI : public Window_Wayland
    {
    public:
        Window_Wayland_IVI(const DisplayConfigData& displayConfig, IWindowEventHandler& windowEventHandler, uint32_t id, std::chrono::microseconds frameCallbackMaxPollTime);
        ~Window_Wayland_IVI() override;

    private:

        void registryGlobalCreated(wl_registry* wl_registry, uint32_t name, const char* interface, uint32_t version) override;
        bool createSurface() override;
        void registerSurfaceListener();

        static void configureCallback(void* userData, ivi_surface* surface, int32_t width, int32_t height);

        ivi_application* m_iviApplicationRegistry = nullptr;
        ivi_surface*     m_iviApplicationSurface  = nullptr;

        const struct IVI_Surface_Listener : public ivi_surface_listener
        {
            IVI_Surface_Listener() : ivi_surface_listener()
            {
                configure = configureCallback;
            }

        } m_IVISurfaceListener;
    };
}
