//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Surface_Wayland_EGL/Surface_Wayland_EGL.h"
#include "Window_Wayland/Window_Wayland.h"
#include "Context_EGL/Context_EGL.h"
#include "Utils/LogMacros.h"

namespace ramses_internal
{
    Surface_Wayland_EGL::Surface_Wayland_EGL(Window_Wayland& window, Context_EGL& context)
        : Surface_Base(window, context)
        , m_context(context)
    {
        LOG_DEBUG(CONTEXT_RENDERER, "Surface_Wayland_EGL::Surface_Wayland_EGL");
    }

    Surface_Wayland_EGL::~Surface_Wayland_EGL()
    {
        LOG_DEBUG(CONTEXT_RENDERER, "Surface_Wayland_EGL::~Surface_Wayland_EGL");
        disable();
    }

    Bool Surface_Wayland_EGL::swapBuffers()
    {
        return m_context.swapBuffers();
    }

    Bool Surface_Wayland_EGL::enable()
    {
        return m_context.enable();
    }

    Bool Surface_Wayland_EGL::disable()
    {
        return m_context.disable();
    }
}
