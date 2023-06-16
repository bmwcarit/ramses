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

#undef Status
#undef None
#undef Always

#include "Platform_Base/Context_Base.h"

namespace ramses_internal
{
    struct EglSurfaceData
    {
        EglSurfaceData()
            : eglDisplay(nullptr)
            , eglConfig(nullptr)
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
        // TODO(violin/tobias) Define a special type here for use as EGLNativeWindowType and EGLNativeDisplayType instead of using it directly. This prevents
        //    that different includes of this file have different typedefs for both types, depending on other (e.g. wayland) headers included first. It
        //    should be safe to cast between them and void* because the source and final destination of this value should use the same
        //    format and we only treat it as an opque value.
        //    Try to remove this workaround in the future by e.g. refactoring Context usage or making Context templated.
        //    Additionally everone uses pointer types except rgl
        using Generic_EGLNativeDisplayType = void*;
        using Generic_EGLNativeWindowType = void*;

        Context_EGL(Generic_EGLNativeDisplayType eglDisplay, Generic_EGLNativeWindowType eglWindow, const EGLint* contextAttributes, const EGLint* surfaceAttributes, const EGLint* windowSurfaceAttributes, EGLint swapInterval, Context_EGL* sharedContext = nullptr);
        ~Context_EGL() override;

        bool init();

        bool swapBuffers() override;
        bool enable() override;
        bool disable() override;

        void* getProcAddress(const char* name) const override;

        [[nodiscard]] EGLDisplay getEglDisplay() const;

    private:
        bool getEglDisplayFromNativeHandle();
        bool initializeEgl();
        bool queryEglExtensions();
        bool bindEglAPI();
        void logAllFoundEglConfigs() const;
        void logErrorHints(const EGLint* surfaceAttributes) const;
        bool chooseEglConfig();
        void logUnmatchedEglConfigParams(const EGLint* surfaceAttributes) const;
        bool createEglSurface();
        bool createEglContext();

        [[nodiscard]] bool isInitialized() const;

        EglSurfaceData m_eglSurfaceData;
        Generic_EGLNativeDisplayType m_nativeDisplay;
        Generic_EGLNativeWindowType m_nativeWindow;
        const EGLint* m_contextAttributes;
        const EGLint* m_surfaceAttributes;
        const EGLint* m_windowSurfaceAttributes;
        const EGLint m_swapInterval;
    };

}

#endif
