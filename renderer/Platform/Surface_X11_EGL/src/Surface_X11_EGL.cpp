//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Surface_X11_EGL/Surface_X11_EGL.h"
#include "Window_X11/Window_X11.h"
#include "Context_EGL/Context_EGL.h"

#include "Utils/LogMacros.h"

namespace ramses_internal
{
    Surface_X11_EGL::Surface_X11_EGL(Window_X11& window, Context_EGL& context)
        : Surface_Base(window, context)
        , m_context(context)
    {
    }

    Surface_X11_EGL::~Surface_X11_EGL()
    {
        disable();
    }

    Bool Surface_X11_EGL::swapBuffers()
    {
        return m_context.swapBuffers();
    }

    Bool Surface_X11_EGL::enable()
    {
        return m_context.enable();
    }

    Bool Surface_X11_EGL::disable()
    {
        return m_context.disable();
    }
}
