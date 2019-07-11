//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Surface_Integrity_RGL_EGL/Surface_Integrity_RGL_EGL.h"
#include "Window_Integrity_RGL/Window_Integrity_RGL.h"
#include "Context_EGL/Context_EGL.h"

namespace ramses_internal
{
    Surface_Integrity_RGL_EGL::Surface_Integrity_RGL_EGL(Window_Integrity_RGL& window, Context_EGL& context)
        : Surface_Base(window, context)
        , m_context(context)
    {
    }

    Surface_Integrity_RGL_EGL::~Surface_Integrity_RGL_EGL()
    {
        disable();
    }

    Bool Surface_Integrity_RGL_EGL::swapBuffers()
    {
        return m_context.swapBuffers();
    }

    Bool Surface_Integrity_RGL_EGL::enable()
    {
        return m_context.enable();
    }

    Bool Surface_Integrity_RGL_EGL::disable()
    {
        return m_context.disable();
    }
}
