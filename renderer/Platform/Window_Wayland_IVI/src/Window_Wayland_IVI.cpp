//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Window_Wayland_IVI/Window_Wayland_IVI.h"
#include "Utils/LogMacros.h"
#include "ivi-application-client-protocol.h"

namespace ramses_internal
{
    Window_Wayland_IVI::Window_Wayland_IVI(const DisplayConfig& displayConfig,
                                           IWindowEventHandler& windowEventHandler,
                                           UInt32               id)
        : Window_Wayland(displayConfig, windowEventHandler, id)
    {
    }

    Window_Wayland_IVI::~Window_Wayland_IVI()
    {
        if (m_iviApplicationSurface)
        {
            ivi_surface_destroy(m_iviApplicationSurface);
        }

        if (m_iviApplicationRegistry)
        {
            ivi_application_destroy(m_iviApplicationRegistry);
        }
    }

    void Window_Wayland_IVI::registryGlobalCreated(wl_registry* wl_registry, uint32_t name, const char* interface, uint32_t version)
    {
        Window_Wayland::registryGlobalCreated(wl_registry, name, interface, version);

        if (0 == strcmp(interface, "ivi_application"))
        {
            m_iviApplicationRegistry = reinterpret_cast<ivi_application*>(wl_registry_bind(wl_registry, name, &ivi_application_interface, 1));
            LOG_DEBUG(CONTEXT_RENDERER, "Window_Wayland_IVI::registryGlobalCreated Bound ivi-application interface");
        }
    }

    bool Window_Wayland_IVI::createSurface()
    {
        LOG_INFO(CONTEXT_RENDERER, "Window_Wayland_IVI::createSurface ivi id: " << m_waylandIviSurfaceID.getValue());

        if (m_iviApplicationRegistry == nullptr)
        {
            LOG_ERROR(CONTEXT_RENDERER, "Window_Wayland_IVI::createSurface ivi-application interface not available!");
            return false;
        }

        m_iviApplicationSurface = ivi_application_surface_create(m_iviApplicationRegistry, m_waylandIviSurfaceID.getValue(), m_wlContext.surface);

        if (m_iviApplicationSurface == nullptr)
        {
            LOG_ERROR(CONTEXT_RENDERER, "Window_Wayland_IVI::createSurface Failed to create ivi-application surface!");
            return false;
        }
        registerSurfaceListener();
        return true;
    }

    void Window_Wayland_IVI::configureCallback(void* userData, ivi_surface* surface, int32_t width, int32_t height)
    {
        Window_Wayland_IVI* window = static_cast<Window_Wayland_IVI*>(userData);
        (window->m_eventHandler).onResize(width, height);
    }

    void Window_Wayland_IVI::registerSurfaceListener()
    {
        ivi_surface_add_listener(m_iviApplicationSurface, &m_IVISurfaceListener, this);
    }
}
