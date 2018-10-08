//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_WAYLANDOUTPUT_H
#define RAMSES_WAYLANDOUTPUT_H

#include "PlatformAbstraction/PlatformTypes.h"
#include "wayland-client-protocol.h"

namespace ramses_internal
{
    class WaylandOutput
    {
    public:
        WaylandOutput(wl_registry* registry, uint32_t name);
        ~WaylandOutput();

    private:
        void outputHandleMode(uint32_t flags, int32_t width, int32_t height, int32_t refresh);

        static void OutputHandleGeometryCallback(void*       data,
                                                 wl_output*  wl_output,
                                                 int32_t     x,
                                                 int32_t     y,
                                                 int32_t     physical_width,
                                                 int32_t     physical_height,
                                                 int32_t     subpixel,
                                                 const char* make,
                                                 const char* model,
                                                 int32_t     transform);
        static void OutputHandleModeCallback(
            void* data, wl_output* wl_output, uint32_t flags, int32_t width, int32_t height, int32_t refresh);

        wl_output* m_output = nullptr;
        uint32_t m_screenId = 0;
        const struct Output_Listener : public wl_output_listener
        {
            Output_Listener()
            {
                geometry = OutputHandleGeometryCallback;
                mode     = OutputHandleModeCallback;
            }
        } m_outputListener;
    };
}

#endif
