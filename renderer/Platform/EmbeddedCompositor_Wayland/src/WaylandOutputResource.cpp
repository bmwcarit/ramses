//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "EmbeddedCompositor_Wayland/WaylandOutputResource.h"

namespace ramses_internal
{
    WaylandOutputResource::WaylandOutputResource()
    {
    }

    WaylandOutputResource::WaylandOutputResource(wl_resource* resource, bool ownership): WaylandResource(resource, ownership)
    {
    }

    void WaylandOutputResource::outputSendGeometry(int32_t     x,
                                                   int32_t     y,
                                                   int32_t     physical_width,
                                                   int32_t     physical_height,
                                                   int32_t     subpixel,
                                                   const char* make,
                                                   const char* model,
                                                   int32_t     transform)
    {
        wl_output_send_geometry(m_resource, x, y, physical_width, physical_height, subpixel, make, model, transform);
    }

    void WaylandOutputResource::outputSendMode(uint32_t flags, int32_t width, int32_t height, int32_t refresh)
    {
        wl_output_send_mode(m_resource, flags, width, height, refresh);
    }

    void WaylandOutputResource::outputSendScale(int32_t factor)
    {
        wl_output_send_scale(m_resource, factor);
    }

    void WaylandOutputResource::outputSendDone()
    {
        wl_output_send_done(m_resource);
    }
}
