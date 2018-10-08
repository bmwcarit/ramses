//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "EmbeddedCompositor_Wayland/WaylandOutputGlobal.h"
#include "EmbeddedCompositor_Wayland/IWaylandDisplay.h"
#include "EmbeddedCompositor_Wayland/WaylandClient.h"
#include "EmbeddedCompositor_Wayland/WaylandOutputConnection.h"
#include "PlatformAbstraction/PlatformMath.h"
#include "Utils/LogMacros.h"

namespace ramses_internal
{
    WaylandOutputGlobal::WaylandOutputGlobal()
    {
    }

    WaylandOutputGlobal::~WaylandOutputGlobal()
    {
        assert(nullptr == m_outputGlobal);
    }

    bool WaylandOutputGlobal::init(IWaylandDisplay& serverDisplay)
    {
        m_systemCompositorDisplay = wl_display_connect(nullptr);

        if (!m_systemCompositorDisplay)
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandOutputGlobal::init wl_display_connect failed!");
            return false;
        }

        m_registry = wl_display_get_registry(m_systemCompositorDisplay);

        if (!m_registry)
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandOutputGlobal::init wl_display_get_registry failed!");
            return false;
        }

        wl_registry_add_listener(m_registry, &m_registryListener, this);

        wl_display_roundtrip(m_systemCompositorDisplay);

        if (m_firstOutputFromSystemCompositor == nullptr)
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandOutputGlobal::init System compositor provides no output!");
            return false;
        }

        /** Makes a second roundtrip to receive current output mode. */
        wl_display_roundtrip(m_systemCompositorDisplay);

        // RAMSES currently supports the wayland output interface up to version 3.
        // For supporting newer versions, before increasing the number here, take care that newly introduced callbacks
        // are ALL handled in WaylandOutputConnection::Output_Interface.
        const int maximumSupportedOutputInterfaceVersion = 3;

        const int supportedOutputInterfaceVersion = min(maximumSupportedOutputInterfaceVersion, wl_output_interface.version);
        m_outputGlobal = serverDisplay.createGlobal(&wl_output_interface, supportedOutputInterfaceVersion, this, OutputBindCallback);

        if (nullptr == m_outputGlobal)
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandOutputGlobal::init(): Failed to create global!");
            return false;
        }

        return true;
    }

    void WaylandOutputGlobal::destroy()
    {
        if (m_firstOutputFromSystemCompositor != nullptr)
        {
            wl_output_destroy(m_firstOutputFromSystemCompositor);
            m_firstOutputFromSystemCompositor = nullptr;
        }

        if (m_systemCompositor != nullptr)
        {
            wl_compositor_destroy(m_systemCompositor);
        }

        if (m_registry != nullptr)
        {
            wl_registry_destroy(m_registry);
            m_registry = nullptr;
        }

        if (m_systemCompositorDisplay != nullptr)
        {
            wl_display_disconnect(m_systemCompositorDisplay);
            m_systemCompositorDisplay = nullptr;
        }

        delete m_outputGlobal;
        m_outputGlobal = nullptr;
    }

    void WaylandOutputGlobal::outputHandleGeometry(wl_output*  wl_output,
                                             int32_t     x,
                                             int32_t     y,
                                             int32_t     physical_width,
                                             int32_t     physical_height,
                                             int32_t     subpixel,
                                             const char* make,
                                             const char* model,
                                             int32_t     transform)
    {
        UNUSED(wl_output)
        UNUSED(x)
        UNUSED(y)
        UNUSED(physical_width)
        UNUSED(physical_height)
        UNUSED(subpixel)
        UNUSED(make)
        UNUSED(model)
        UNUSED(transform)
    }

    void WaylandOutputGlobal::OutputHandleGeometryCallback(void* data, wl_output* wl_output, int32_t x, int32_t y, int32_t physical_width,
                                int32_t physical_height, int32_t subpixel, const char* make, const char* model, int32_t transform)
    {
        WaylandOutputGlobal* waylandOutput = static_cast<WaylandOutputGlobal*>(data);
        waylandOutput->outputHandleGeometry(wl_output, x, y, physical_width, physical_height, subpixel, make, model, transform);
    }

    void WaylandOutputGlobal::outputHandleMode(wl_output* wl_output, uint32_t flags, int32_t width, int32_t height, int32_t refresh)
    {
        UNUSED(wl_output)
        UNUSED(flags)

        if (flags & WL_OUTPUT_MODE_CURRENT)
        {
            m_width   = width;
            m_height  = height;
            m_refresh = refresh;
        }
    }

    void WaylandOutputGlobal::OutputHandleModeCallback(void* data, wl_output* wl_output, uint32_t flags, int32_t width, int32_t height, int32_t refresh)
    {
        WaylandOutputGlobal* waylandOutput = static_cast<WaylandOutputGlobal*>(data);
        waylandOutput->outputHandleMode(wl_output, flags, width, height, refresh);
    }

    void WaylandOutputGlobal::registryHandleGlobal(wl_registry* registry, uint32_t id, const char* interface, uint32_t version)
    {
        UNUSED(version)

        if (strcmp(interface, "wl_compositor") == 0)
        {
            m_systemCompositor = static_cast<wl_compositor*>(wl_registry_bind(registry, id, &wl_compositor_interface, 1));
            assert(m_systemCompositor);
        }

        if (strcmp(interface, "wl_output") == 0 && m_firstOutputFromSystemCompositor == nullptr)
        {
            // Because of lacking architectural requirements, it is currently undefined how RAMSES should deal with wl_output.
            // Currently, it forwards the settings of the first output received from the system compositor to all
            // clients
            // TODO (Violin) Find out requirements towards wl_output

            const uint32_t outputInterfaceVersion = 1;
            m_firstOutputFromSystemCompositor = static_cast<wl_output*>(wl_registry_bind(registry, id, &wl_output_interface, outputInterfaceVersion));
            assert(m_firstOutputFromSystemCompositor);
            wl_output_add_listener(m_firstOutputFromSystemCompositor, &m_outputListener, this);
        }
    }

    void WaylandOutputGlobal::RegistryHandleGlobalCallback(void* data, wl_registry* registry, uint32_t id, const char* interface, uint32_t version)
    {
        WaylandOutputGlobal* waylandOutput = static_cast<WaylandOutputGlobal*>(data);
        waylandOutput->registryHandleGlobal(registry, id, interface, version);
    }

    void WaylandOutputGlobal::registryHandleGlobalRemove(wl_registry* registry, uint32_t name)
    {
        UNUSED(registry)
        UNUSED(name)
    }

    void WaylandOutputGlobal::RegistryHandleGlobalRemoveCallback(void* data, wl_registry* registry, uint32_t name)
    {
        WaylandOutputGlobal* waylandOutput = static_cast<WaylandOutputGlobal*>(data);
        waylandOutput->registryHandleGlobalRemove(registry, name);
    }

    void WaylandOutputGlobal::outputBind(IWaylandClient& client, uint32_t version, uint32_t id)
    {
        WaylandOutputConnection* waylandOutputConnection = new WaylandOutputConnection(client, version, id, m_width, m_height, m_refresh);
        // Registers callback for destruction, when the corresponding resource is destroyed

        if (!waylandOutputConnection->wasSuccessfullyInitialized())
        {
            delete waylandOutputConnection;
        }
    }

    void WaylandOutputGlobal::OutputBindCallback(wl_client* client, void* data, uint32_t version, uint32_t id)
    {
        WaylandOutputGlobal* waylandOutput = static_cast<WaylandOutputGlobal*>(data);
        WaylandClient waylandClient(client);
        waylandOutput->outputBind(waylandClient, version, id);
    }

    void WaylandOutputGlobal::getResolution(int32_t& width, int32_t& height) const
    {
        width = m_width;
        height = m_height;
    }

    int32_t WaylandOutputGlobal::getRefreshRate() const
    {
        return m_refresh;
    }
}
