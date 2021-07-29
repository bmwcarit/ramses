//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_WLCONTEXT_H
#define RAMSES_WLCONTEXT_H

#include "RendererAPI/IWindowEventHandler.h"
#include "RendererLib/EKeyModifier.h"
#include <wayland-egl.h>

struct wl_display;
struct wl_registry;
struct wl_surface;
struct wl_compositor;
struct wl_egl_window;
struct wl_seat;
struct wl_keyboard;
struct wl_pointer;
struct wl_egl_window;

namespace ramses_internal
{
    struct WlContext
    {
        WlContext()
        {
        }

        wl_display*       display = nullptr;
        int               displayFD = -1;
        wl_registry*      registry = nullptr;
        wl_surface*       surface = nullptr;
        wl_compositor*    compositor = nullptr;
        wl_seat*          seat = nullptr;
        wl_keyboard*      keyboard = nullptr;
        wl_pointer*       pointer = nullptr;
        wl_egl_window*    window = nullptr;
        wl_egl_window*    native_window = nullptr;
        wl_callback*      frameRenderingDoneWaylandCallbacObject = nullptr;

        Bool              previousFrameRenderingDone = true;
    };
}

#endif
