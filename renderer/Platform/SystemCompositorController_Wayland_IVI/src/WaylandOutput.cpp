//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "SystemCompositorController_Wayland_IVI/WaylandOutput.h"
#include "Utils/LogMacros.h"

namespace ramses_internal
{
    WaylandOutput::WaylandOutput(wl_registry* registry, uint32_t name)
    {
        const uint32_t outputInterfaceVersion = 1;

        m_output =
            static_cast<wl_output*>(wl_registry_bind(registry, name, &wl_output_interface, outputInterfaceVersion));

        if (nullptr == m_output)
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandOutput::WaylandOutput wl_registry_bind failed");
            assert(false);
        }

        // GENIVI uses the proxy-id of an wl_output as it's screen-id.
        m_screenId = wl_proxy_get_id(reinterpret_cast<wl_proxy*>(m_output));

        LOG_INFO(CONTEXT_RENDERER, "WaylandOutput::WaylandOutput screen-id: " << m_screenId);

        wl_output_add_listener(m_output, &m_outputListener, this);
    }

    WaylandOutput::~WaylandOutput()
    {
        LOG_INFO(CONTEXT_RENDERER, "WaylandOutput::~WaylandOutput screen-id: " << m_screenId);

        if (nullptr != m_output)
        {
            wl_output_destroy(m_output);
        }
    }

    void WaylandOutput::outputHandleMode(uint32_t flags, int32_t width, int32_t height, int32_t refresh)
    {
        if (0 != (flags & WL_OUTPUT_MODE_CURRENT))
        {
            LOG_INFO(CONTEXT_RENDERER,
                     "WaylandOutput::outputHandleMode screen-id: " << m_screenId
                                                                   << " resolution: " << width << " x " << height
                                                                   << " refresh rate: " << refresh);
        }
    }

    void WaylandOutput::OutputHandleGeometryCallback(void*       data,
                                                     wl_output*  wl_output,
                                                     int32_t     x,
                                                     int32_t     y,
                                                     int32_t     physical_width,
                                                     int32_t     physical_height,
                                                     int32_t     subpixel,
                                                     const char* make,
                                                     const char* model,
                                                     int32_t     transform)
    {
        UNUSED(data);
        UNUSED(wl_output);
        UNUSED(x);
        UNUSED(y);
        UNUSED(physical_width);
        UNUSED(physical_height);
        UNUSED(subpixel);
        UNUSED(make);
        UNUSED(model);
        UNUSED(transform);
    }

    void WaylandOutput::OutputHandleModeCallback(
        void* data, wl_output* wl_output, uint32_t flags, int32_t width, int32_t height, int32_t refresh)
    {
        UNUSED(wl_output)
        WaylandOutput* waylandOutput = static_cast<WaylandOutput*>(data);
        waylandOutput->outputHandleMode(flags, width, height, refresh);
    }
}
