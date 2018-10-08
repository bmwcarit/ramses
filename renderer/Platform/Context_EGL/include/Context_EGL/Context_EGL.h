//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CONTEXT_EGL_H
#define RAMSES_CONTEXT_EGL_H

#include <EGL/egl.h>

#undef Bool // Xlib.h (included from EGL/egl.h) defines Bool as int - this collides with ramses_internal::Bool
#undef Status
#undef None

#include "Platform_Base/Context_Base.h"

namespace ramses_internal
{
    struct EglSurfaceData
    {
        EglSurfaceData()
            : eglDisplay(0)
            , eglConfig(0)
            , eglSurface(EGL_NO_SURFACE)
            , eglContext(EGL_NO_CONTEXT)
            , eglSharedContext(EGL_NO_CONTEXT)
        {
        }

        EGLDisplay eglDisplay;
        EGLConfig eglConfig;
        EGLSurface eglSurface;
        EGLContext eglContext;
        EGLContext eglSharedContext;
    };

    class Context_EGL : public Context_Base
    {
    public:
        Context_EGL(EGLNativeDisplayType eglDisplay, EGLNativeWindowType eglWindow, const EGLint* contextAttributes, const EGLint* surfaceAttributes, const EGLint* windowSurfaceAttributes, EGLint swapInterval, Context_EGL* sharedContext = 0);
        ~Context_EGL() override;

        Bool init();

        Bool swapBuffers();
        Bool enable();
        Bool disable();

        void* getProcAddress(const char* name) const override;

    private:
        EglSurfaceData m_eglSurfaceData;
        EGLNativeDisplayType m_nativeDisplay;
        EGLNativeWindowType m_nativeWindow;
        const EGLint* m_contextAttributes;
        const EGLint* m_surfaceAttributes;
        const EGLint* m_windowSurfaceAttributes;
        const EGLint m_swapInterval;
    };

}

#endif
