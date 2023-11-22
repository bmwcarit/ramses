//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Platform/Wayland/IVI/SystemCompositorController/WaylandOutput.h"
#include "internal/Core/Utils/LogMacros.h"
#include <cassert>

namespace ramses::internal
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

        LOG_INFO(CONTEXT_RENDERER, "WaylandOutput::WaylandOutput screen-id: {}", m_screenId);

        wl_output_add_listener(m_output, &m_outputListener, this);
    }

    WaylandOutput::~WaylandOutput()
    {
        LOG_INFO(CONTEXT_RENDERER, "WaylandOutput::~WaylandOutput screen-id: {}", m_screenId);

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
                     "WaylandOutput::outputHandleMode screen-id: {} resolution: {} x {} refresh rate: {}",
                     m_screenId, width, height, refresh);
        }
    }

    void WaylandOutput::OutputHandleGeometryCallback([[maybe_unused]] void*       data,
                                                     [[maybe_unused]] wl_output*  wl_output,
                                                     [[maybe_unused]] int32_t     x,
                                                     [[maybe_unused]] int32_t     y,
                                                     [[maybe_unused]] int32_t     physical_width,
                                                     [[maybe_unused]] int32_t     physical_height,
                                                     [[maybe_unused]] int32_t     subpixel,
                                                     [[maybe_unused]] const char* make,
                                                     [[maybe_unused]] const char* model,
                                                     [[maybe_unused]] int32_t     transform)
    {
    }

    void WaylandOutput::OutputHandleModeCallback(
        void* data, [[maybe_unused]] wl_output* wl_output, uint32_t flags, int32_t width, int32_t height, int32_t refresh)
    {
        auto* waylandOutput = static_cast<WaylandOutput*>(data);
        waylandOutput->outputHandleMode(flags, width, height, refresh);
    }
}
