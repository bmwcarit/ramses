//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Surface_EGL_Offscreen/Surface_EGL_Offscreen.h"
#include "Window_Wayland/Window_Wayland.h"
#include "Context_EGL/Context_EGL.h"
#include "Utils/LogMacros.h"

namespace ramses_internal
{
    Surface_EGL_Offscreen::Surface_EGL_Offscreen(Window_Wayland& window, Context_EGL& context)
        : Surface_Base(window, context)
        , m_context(context)
    {
        LOG_DEBUG(CONTEXT_RENDERER, "Surface_EGL_Offscreen::Surface_EGL_Offscreen");
    }

    Surface_EGL_Offscreen::~Surface_EGL_Offscreen()
    {
        LOG_DEBUG(CONTEXT_RENDERER, "Surface_EGL_Offscreen::~Surface_EGL_Offscreen");
        disable();
    }

    void Surface_EGL_Offscreen::frameRendered()
    {
        // overwrite base implementation with empty method
        // to bypass wayland's frame callback we can't
        // pass on the frameRendered call to the window
        // as specific assertions on the window's side
        // are not met.
    }

    Bool Surface_EGL_Offscreen::canRenderNewFrame() const
    {
        return true;
    }

    Bool Surface_EGL_Offscreen::swapBuffers()
    {
        return true;
    }

    Bool Surface_EGL_Offscreen::enable()
    {
        return m_context.enable();
    }

    Bool Surface_EGL_Offscreen::disable()
    {
        return m_context.disable();
    }
}
