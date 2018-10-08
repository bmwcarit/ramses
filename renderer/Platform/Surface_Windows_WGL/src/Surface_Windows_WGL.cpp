//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Surface_Windows_WGL/Surface_Windows_WGL.h"

#include "Window_Windows/Window_Windows.h"
#include "Context_WGL/Context_WGL.h"
#include "Utils/LogMacros.h"


namespace ramses_internal
{
    Surface_Windows_WGL::Surface_Windows_WGL(Window_Windows& window, Context_WGL& context)
        : Surface_Base(window, context)
        , m_window(window)
        , m_context(context)
    {
        LOG_DEBUG(CONTEXT_RENDERER, "Surface_Windows_WGL::Surface_Windows_WGL:");
    }

    Surface_Windows_WGL::~Surface_Windows_WGL()
    {
        LOG_DEBUG(CONTEXT_RENDERER, "Surface_Windows_WGL::~Surface_Windows_WGL:");
    }

    Bool Surface_Windows_WGL::swapBuffers()
    {
        LOG_TRACE(CONTEXT_RENDERER, "Surface_Windows_WGL::swapBuffers:  swapping buffers");

        return SwapBuffers(m_window.getNativeDisplayHandle()) == TRUE;
    }

    Bool Surface_Windows_WGL::enable()
    {
        LOG_TRACE(CONTEXT_RENDERER, "Surface_Windows_WGL::enable:  context_WGL enable");
        if (!wglMakeCurrent(m_window.getNativeDisplayHandle(), m_context.getNativeContextHandle()))
        {
            return false;
        }
        return true;
    }

    Bool Surface_Windows_WGL::disable()
    {
        LOG_TRACE(CONTEXT_RENDERER, "Surface_Windows_WGL::disable:  context_WGL disable");
        if (!wglMakeCurrent(NULL, NULL))
        {
            return false;
        }

        return true;
    }
}
