//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------
#ifndef RAMSES_WAYLANDOUTPUTRESOURCE_H
#define RAMSES_WAYLANDOUTPUTRESOURCE_H

#include "EmbeddedCompositor_Wayland/WaylandResource.h"

namespace ramses_internal
{
    class WaylandOutputResource : public WaylandResource
    {
    public:
        WaylandOutputResource();
        WaylandOutputResource(wl_resource* resource, bool ownership);
        virtual void outputSendGeometry(int32_t     x,
                                        int32_t     y,
                                        int32_t     physical_width,
                                        int32_t     physical_height,
                                        int32_t     subpixel,
                                        const char* make,
                                        const char* model,
                                        int32_t     transform);
        virtual void outputSendMode(uint32_t flags, int32_t width, int32_t height, int32_t refresh);
        virtual void outputSendScale(int32_t factor);
        virtual void outputSendDone();
    };
}

#endif
