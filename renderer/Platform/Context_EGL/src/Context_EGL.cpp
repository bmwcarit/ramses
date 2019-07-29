//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Context_EGL/Context_EGL.h"
#include "Utils/LogMacros.h"

namespace ramses_internal
{
    Context_EGL::Context_EGL(EGLNativeDisplayType eglDisplay, Generic_EGLNativeWindowType eglWindow, const EGLint* contextAttributes, const EGLint* surfaceAttributes, const EGLint* windowSurfaceAttributes, EGLint swapInterval, Context_EGL* sharedContext /*= 0*/)
        : m_nativeDisplay(eglDisplay)
        , m_nativeWindow(eglWindow)
        , m_contextAttributes(contextAttributes)
        , m_surfaceAttributes(surfaceAttributes)
        , m_windowSurfaceAttributes(windowSurfaceAttributes)
        , m_swapInterval(swapInterval)
    {
        if(nullptr != sharedContext)
        {
            const EGLContext contextHandleToShare = sharedContext->m_eglSurfaceData.eglContext;
            LOG_DEBUG(CONTEXT_RENDERER, "Context_EGL::Context_EGL Sharing new context with existing context " << contextHandleToShare);
            m_eglSurfaceData.eglSharedContext = contextHandleToShare;
        }
    }

    Bool Context_EGL::init()
    {
        m_eglSurfaceData.eglDisplay = eglGetDisplay(m_nativeDisplay);

        if (EGL_NO_DISPLAY == m_eglSurfaceData.eglDisplay)
        {
            LOG_ERROR(CONTEXT_RENDERER, "Context_EGL::init() failed, eglGetDisplay with arg:" << m_nativeDisplay << " returned EGL_NO_DISPLAY");
            return false;
        }

        EGLint iMajorVersion;
        EGLint iMinorVersion;
        if (!eglInitialize(m_eglSurfaceData.eglDisplay, &iMajorVersion, &iMinorVersion))
        {
            LOG_ERROR(CONTEXT_RENDERER, "Context_EGL::init() eglInitialize() failed! Error code: " << eglGetError());
            return false;
        }

        const Char* contextExtensionsNativeString = eglQueryString(m_eglSurfaceData.eglDisplay, EGL_EXTENSIONS);

        if (nullptr != contextExtensionsNativeString)
        {
            LOG_INFO(CONTEXT_RENDERER, "Context_EGL::init(): EGL extensions: " << contextExtensionsNativeString);
            parseContextExtensions(contextExtensionsNativeString);
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "Context_EGL::init(): Could not get EGL extensions string. No context extensions are loaded. Error code: " << eglGetError());
        }

        if (!eglBindAPI(EGL_OPENGL_ES_API))
        {
            LOG_ERROR(CONTEXT_RENDERER, "Context_EGL::init() eglBindAPI() failed! Error code: " << eglGetError());
            return false;
        }

        int iConfigs;
        if (!eglChooseConfig(m_eglSurfaceData.eglDisplay, m_surfaceAttributes, &m_eglSurfaceData.eglConfig, 1, &iConfigs) || (iConfigs == 0))
        {
            if(0 == iConfigs)
            {
                LOG_ERROR(CONTEXT_RENDERER, "Context_EGL::init(): eglChooseConfig() failed. No configs available for the requested surface attributes:");

                UInt32 i = 0;
                while(EGL_NONE != *m_surfaceAttributes)
                {
                    LOG_ERROR(CONTEXT_RENDERER, "" << i++ << ": " << *m_surfaceAttributes);
                    ++m_surfaceAttributes;
                }
            }
            else
            {
                LOG_ERROR(CONTEXT_RENDERER, "Context_EGL::init(): eglChooseConfig() failed. Error code: " << eglGetError());
            }

            eglTerminate(m_eglSurfaceData.eglDisplay);
            return false;
        }

        m_eglSurfaceData.eglSurface = eglCreateWindowSurface(
            m_eglSurfaceData.eglDisplay, m_eglSurfaceData.eglConfig,
            reinterpret_cast<EGLNativeWindowType>(m_nativeWindow), m_windowSurfaceAttributes);

        if (!m_eglSurfaceData.eglSurface)
        {
            LOG_ERROR(CONTEXT_RENDERER, "Context_EGL::init(): eglCreateWindowSurface() failed. Error code: " << eglGetError());
            eglTerminate(m_eglSurfaceData.eglDisplay);
            return false;
        }

        m_eglSurfaceData.eglContext = eglCreateContext(
            m_eglSurfaceData.eglDisplay, m_eglSurfaceData.eglConfig, m_eglSurfaceData.eglSharedContext,
            m_contextAttributes);

        if (!m_eglSurfaceData.eglContext)
        {
            LOG_ERROR(CONTEXT_RENDERER, "Context_EGL::init(): eglCreateContext() failed. Error code: " << eglGetError());
            eglDestroySurface(m_eglSurfaceData.eglDisplay, m_eglSurfaceData.eglSurface);
            eglTerminate(m_eglSurfaceData.eglDisplay);
            return false;
        }

        if (!enable())
        {
           return false;
        }

        LOG_DEBUG(CONTEXT_RENDERER, "Context_EGL::init(): Setting egl swap interval to :" << m_swapInterval);
        eglSwapInterval(m_eglSurfaceData.eglDisplay, m_swapInterval);

        LOG_INFO(CONTEXT_RENDERER, "Context_EGL::init(): EGL context creation succeeded");
        return true;
    }

    Context_EGL::~Context_EGL()
    {
        LOG_DEBUG(CONTEXT_RENDERER, "Context_EGL destroy");

        if (m_eglSurfaceData.eglDisplay && m_eglSurfaceData.eglSurface && m_eglSurfaceData.eglContext)
        {
            LOG_DEBUG(CONTEXT_RENDERER, "Context_EGL::destroy destroying surface and context");

            if (!eglDestroySurface(m_eglSurfaceData.eglDisplay, m_eglSurfaceData.eglSurface))
            {
                LOG_ERROR(CONTEXT_RENDERER, "Context_EGL::destroy eglDestroySurface failed. Error code: " << eglGetError());
            }
            if (!eglDestroyContext(m_eglSurfaceData.eglDisplay, m_eglSurfaceData.eglContext))
            {
                LOG_ERROR(CONTEXT_RENDERER, "Context_EGL::destroy eglDestroyContext failed. Error code: " << eglGetError());
            }

            LOG_DEBUG(CONTEXT_RENDERER, "Context_EGL::terminateEGLDisplayIfNotUsedAnymore calling eglTerminate ");

            if (!eglTerminate(m_eglSurfaceData.eglDisplay))
            {
                LOG_ERROR(CONTEXT_RENDERER, "Context_EGL::terminateEGLDisplayIfNotUsedAnymore eglTerminate() failed! Error code: " << eglGetError());
            }
        }
        else
        {
            /// Not initialized, so nothing to destroy.
            LOG_ERROR(CONTEXT_RENDERER, "Context_EGL::~Context_EGL Context was not successfully initialized  before.");
        }
    }

    Bool Context_EGL::swapBuffers()
    {
        LOG_TRACE(CONTEXT_RENDERER, "Context_EGL swapping buffers");
        eglSwapBuffers(m_eglSurfaceData.eglDisplay, m_eglSurfaceData.eglSurface);
        return true;
    }

    Bool Context_EGL::enable()
    {
        assert (m_eglSurfaceData.eglDisplay);
        assert (m_eglSurfaceData.eglSurface);
        assert (m_eglSurfaceData.eglContext);

        LOG_TRACE(CONTEXT_RENDERER, "Context_EGL enable");
        const Bool ok = eglMakeCurrent(m_eglSurfaceData.eglDisplay,
            m_eglSurfaceData.eglSurface, m_eglSurfaceData.eglSurface,
            m_eglSurfaceData.eglContext);

        if (!ok)
        {
            LOG_ERROR(CONTEXT_RENDERER, "Context_EGL::enable Error: eglMakeCurrent() failed. Error code " << eglGetError());
            return false;
        }

        return true;
    }

    Bool Context_EGL::disable()
    {
        if (m_eglSurfaceData.eglDisplay)
        {
            LOG_TRACE(CONTEXT_RENDERER, "Context_EGL disable");
            const Bool ok = eglMakeCurrent(m_eglSurfaceData.eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
            if (!ok)
            {
                LOG_ERROR(CONTEXT_RENDERER, "Context_EGL::disable Error: eglMakeCurrent() failed. Error code " << eglGetError());
                return false;
            }
        }
        else
        {
            LOG_DEBUG(CONTEXT_RENDERER, "Context_EGL::disable Context was not successfully initialized  before.");
            return false;
        }

        return true;
    }

    void* Context_EGL::getProcAddress(const char* name) const
    {
        return reinterpret_cast<void*>(eglGetProcAddress(name));
    }
}
