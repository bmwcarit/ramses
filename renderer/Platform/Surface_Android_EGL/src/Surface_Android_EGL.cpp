//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Surface_Android_EGL/Surface_Android_EGL.h"
#include "Window_Android/Window_Android.h"
#include "Context_EGL/Context_EGL.h"

#include "Utils/LogMacros.h"

namespace ramses_internal
{
    Surface_Android_EGL::Surface_Android_EGL(Window_Android& window, Context_EGL& context)
        : Surface_Base(window, context)
        , m_context(context)
    {
    }

    Surface_Android_EGL::~Surface_Android_EGL()
    {
        disable();
    }

    Bool Surface_Android_EGL::swapBuffers()
    {
        return m_context.swapBuffers();
    }

    Bool Surface_Android_EGL::enable()
    {
        return m_context.enable();
    }

    Bool Surface_Android_EGL::disable()
    {
        return m_context.disable();
    }
}
