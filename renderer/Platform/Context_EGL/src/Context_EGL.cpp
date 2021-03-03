//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Context_EGL/Context_EGL.h"
#include "Utils/ThreadLocalLogForced.h"

namespace ramses_internal
{
    Context_EGL::Context_EGL(Generic_EGLNativeDisplayType eglDisplay, Generic_EGLNativeWindowType eglWindow, const EGLint* contextAttributes, const EGLint* surfaceAttributes, const EGLint* windowSurfaceAttributes, EGLint swapInterval, Context_EGL* sharedContext /*= 0*/)
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
        if(!getEglDisplayFromNativeHandle())
            return false;

        if(!initializeEgl())
            return false;

        if(!queryEglExtensions())
            return false;

        if(!bindEglAPI())
            return false;

        if(!chooseEglConfig())
            return false;

        if(!createEglSurface())
            return false;

        if(!createEglContext())
            return false;

        if (!enable())
           return false;

        LOG_DEBUG(CONTEXT_RENDERER, "Context_EGL::init(): Setting egl swap interval to :" << m_swapInterval);
        eglSwapInterval(m_eglSurfaceData.eglDisplay, m_swapInterval);

        LOG_INFO(CONTEXT_RENDERER, "Context_EGL::init(): EGL context creation succeeded");
        return true;
    }

    Context_EGL::~Context_EGL()
    {
        //For more info: https://www.khronos.org/registry/EGL/specs/eglspec.1.4.pdf
        LOG_INFO(CONTEXT_RENDERER, "Context_EGL destroy");

        if (isInitialized())
        {
            LOG_DEBUG(CONTEXT_RENDERER, "Context_EGL::destroy destroying surface and context");

            const bool isSharedContext = m_eglSurfaceData.eglSharedContext != nullptr;

            if (!isSharedContext && !eglDestroySurface(m_eglSurfaceData.eglDisplay, m_eglSurfaceData.eglSurface))
            {
                LOG_ERROR(CONTEXT_RENDERER, "Context_EGL::destroy eglDestroySurface failed. Error code: " << eglGetError());
            }
            if (!eglDestroyContext(m_eglSurfaceData.eglDisplay, m_eglSurfaceData.eglContext))
            {
                LOG_ERROR(CONTEXT_RENDERER, "Context_EGL::destroy eglDestroyContext failed. Error code: " << eglGetError());
            }

            LOG_DEBUG(CONTEXT_RENDERER, "Context_EGL::terminateEGLDisplayIfNotUsedAnymore calling eglTerminate ");

            if (!isSharedContext && !eglTerminate(m_eglSurfaceData.eglDisplay))
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
        assert(isInitialized());
        LOG_TRACE(CONTEXT_RENDERER, "Context_EGL enable");

        const auto success = eglMakeCurrent(m_eglSurfaceData.eglDisplay, m_eglSurfaceData.eglSurface, m_eglSurfaceData.eglSurface, m_eglSurfaceData.eglContext);
        if (success != EGL_TRUE)
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

            const auto success = eglMakeCurrent(m_eglSurfaceData.eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
            if (success != EGL_TRUE)
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

    EGLDisplay Context_EGL::getEglDisplay() const
    {
        return m_eglSurfaceData.eglDisplay;
    }

    bool Context_EGL::getEglDisplayFromNativeHandle()
    {
        //For more info: https://www.khronos.org/registry/EGL/specs/eglspec.1.4.pdf
        m_eglSurfaceData.eglDisplay = eglGetDisplay(reinterpret_cast<EGLNativeDisplayType>(m_nativeDisplay));

        if (EGL_NO_DISPLAY == m_eglSurfaceData.eglDisplay)
        {
            LOG_ERROR(CONTEXT_RENDERER, "Context_EGL initialization failed at eglGetDisplay with arg:" << m_nativeDisplay << " returned EGL_NO_DISPLAY");
            return false;
        }

        return true;
    }

    bool Context_EGL::initializeEgl()
    {
        EGLint iMajorVersion;
        EGLint iMinorVersion;
        if (!eglInitialize(m_eglSurfaceData.eglDisplay, &iMajorVersion, &iMinorVersion))
        {
            LOG_ERROR(CONTEXT_RENDERER, "Context_EGL initialization failed at eglInitialize() with error code: " << eglGetError());
            return false;
        }

        return true;
    }

    bool Context_EGL::queryEglExtensions()
    {
        const Char* contextExtensionsNativeString = eglQueryString(m_eglSurfaceData.eglDisplay, EGL_EXTENSIONS);

        if (nullptr != contextExtensionsNativeString)
        {
            LOG_INFO(CONTEXT_RENDERER, "Context_EGL::init(): EGL extensions: " << contextExtensionsNativeString);
            parseContextExtensions(contextExtensionsNativeString);
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "Context_EGL initialization failed: Could not get EGL extensions string. No context extensions are loaded. Error code: " << eglGetError());
        }

        return true;
    }

    bool Context_EGL::bindEglAPI()
    {
        if (!eglBindAPI(EGL_OPENGL_ES_API))
        {
            LOG_ERROR(CONTEXT_RENDERER, "Context_EGL initialization failed at eglBindAPI() with error code: " << eglGetError());
            return false;
        }

        return true;
    }

    bool Context_EGL::chooseEglConfig()
    {
        const EGLint* surfaceAttributes = m_surfaceAttributes;

#ifdef __ANDROID__
        const EGLint surfaceAttribsForPBuffer[] =
         {
                EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
                EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
                EGL_BLUE_SIZE, 8,
                EGL_GREEN_SIZE, 8,
                EGL_RED_SIZE, 8,
                EGL_ALPHA_SIZE, 8,
                EGL_BUFFER_SIZE, 32,
                EGL_DEPTH_SIZE, 24,
                EGL_NONE
        };

        //for creation of a shared context use EGL config with surface attributes that allow creation of PBuffer
        if (m_eglSurfaceData.eglSharedContext)
            surfaceAttributes = surfaceAttribsForPBuffer;
#endif
        int iConfigs;
        if (!eglChooseConfig(m_eglSurfaceData.eglDisplay, surfaceAttributes, &m_eglSurfaceData.eglConfig, 1, &iConfigs) || (iConfigs == 0))
        {
            if(0 == iConfigs)
            {
                LOG_ERROR(CONTEXT_RENDERER, "Context_EGL initialization failed at eglChooseConfig(). No configs available for the requested surface attributes:");

                UInt32 i = 0;
                while(EGL_NONE != *m_surfaceAttributes)
                {
                    LOG_ERROR(CONTEXT_RENDERER, "" << i++ << ": " << *m_surfaceAttributes);
                    ++m_surfaceAttributes;
                }
            }
            else
            {
                LOG_ERROR(CONTEXT_RENDERER, "Context_EGL initialization failed at  eglChooseConfig() with error code: " << eglGetError());
            }

            eglTerminate(m_eglSurfaceData.eglDisplay);
            return false;
        }

        return true;
    }

    bool Context_EGL::createEglSurface()
    {
        //do not create egl surface for shared context
        if(!m_eglSurfaceData.eglSharedContext)
        {
            m_eglSurfaceData.eglSurface = eglCreateWindowSurface(m_eglSurfaceData.eglDisplay,
                                                                m_eglSurfaceData.eglConfig,
                                                                reinterpret_cast<EGLNativeWindowType>(m_nativeWindow),
                                                                m_windowSurfaceAttributes);

            if (!m_eglSurfaceData.eglSurface)
            {
                LOG_ERROR(CONTEXT_RENDERER, "Context_EGL initialization failed at eglCreateWindowSurface() with error code: " << eglGetError());
                eglTerminate(m_eglSurfaceData.eglDisplay);
                return false;
            }
        }
#ifdef __ANDROID__
        else
        {
            //TODO Mohamed: evaluate the consequences on stability and performance for creation of Pixel Buffer Surface on the different supported platforms

            //Create PBuffer (Pixel Buffer) surface with dimensions 1x1. This PBuffer surface will never be rendered into, yet it has to be created
            //because some platforms do not allow a context to be enabled/bound unless to a valid surface, i.e., do not allow binding context to
            //EGL_NO_SURFACE
            const EGLint pbufferAttribs[] =
            {
                    EGL_WIDTH, 1,
                    EGL_HEIGHT, 1,
                    EGL_TEXTURE_TARGET, EGL_NO_TEXTURE,
                    EGL_TEXTURE_FORMAT, EGL_NO_TEXTURE,
                    EGL_NONE
            };
            m_eglSurfaceData.eglSurface = eglCreatePbufferSurface(m_eglSurfaceData.eglDisplay, m_eglSurfaceData.eglConfig, pbufferAttribs);

            if (!m_eglSurfaceData.eglSurface)
            {
                LOG_ERROR(CONTEXT_RENDERER, "Context_EGL initialization failed at eglCreatePbufferSurface() with error code: " << eglGetError());
                eglTerminate(m_eglSurfaceData.eglDisplay);
                return false;
            }
        }
#endif

        return true;
    }

    bool Context_EGL::createEglContext()
    {
        m_eglSurfaceData.eglContext = eglCreateContext(m_eglSurfaceData.eglDisplay,
                                                    m_eglSurfaceData.eglConfig,
                                                    m_eglSurfaceData.eglSharedContext,
                                                    m_contextAttributes);

        if (!m_eglSurfaceData.eglContext)
        {
            LOG_ERROR(CONTEXT_RENDERER, "Context_EGL initialization failed at eglCreateContext() with error code: " << eglGetError());
            eglDestroySurface(m_eglSurfaceData.eglDisplay, m_eglSurfaceData.eglSurface);
            eglTerminate(m_eglSurfaceData.eglDisplay);
            return false;
        }

        return true;
    }

    bool Context_EGL::isInitialized() const
    {
#ifdef __ANDROID__
        return m_eglSurfaceData.eglDisplay != nullptr && m_eglSurfaceData.eglContext != nullptr && m_eglSurfaceData.eglSurface != nullptr;
#else
        return m_eglSurfaceData.eglDisplay != nullptr
               && m_eglSurfaceData.eglContext != nullptr
               //either the context is a shared context, or the context has egl surface (logical XOR)
               //i.e., a shared context must not have egl surface, and a non-shared context must have egl surface
               && ((m_eglSurfaceData.eglSurface == nullptr) != (m_eglSurfaceData.eglSharedContext == nullptr));
#endif
    }
}
