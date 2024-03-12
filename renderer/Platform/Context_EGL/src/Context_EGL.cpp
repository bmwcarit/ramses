//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Context_EGL/Context_EGL.h"
#include "Utils/ThreadLocalLogForced.h"

namespace
{
    const char* surfaceAttributeName(EGLint surfaceAttribute)
    {
        switch (surfaceAttribute)
        {
        case EGL_SURFACE_TYPE:
            return "EGL_SURFACE_TYPE";
        case EGL_RENDERABLE_TYPE:
            return "EGL_RENDERABLE_TYPE";
        case EGL_BUFFER_SIZE:
            return "EGL_BUFFER_SIZE";
        case EGL_RED_SIZE:
            return "EGL_RED_SIZE";
        case EGL_GREEN_SIZE:
            return "EGL_GREEN_SIZE";
        case EGL_BLUE_SIZE:
            return "EGL_BLUE_SIZE";
        case EGL_ALPHA_SIZE:
            return "EGL_ALPHA_SIZE";
        case EGL_DEPTH_SIZE:
            return "EGL_DEPTH_SIZE";
        case EGL_STENCIL_SIZE:
            return "EGL_STENCIL_SIZE";
        case EGL_SAMPLE_BUFFERS:
            return "EGL_SAMPLE_BUFFERS";
        case EGL_SAMPLES:
            return "EGL_SAMPLES";
        default:
            break;
        }
        return nullptr;
    }

    const char* renderableTypeName(EGLint value)
    {
        switch(value)
        {
        case  EGL_OPENGL_ES_BIT:
            return "EGL_OPENGL_ES_BIT";
        case  EGL_OPENVG_BIT:
            return "EGL_OPENVG_BIT";
        case  EGL_OPENGL_ES2_BIT:
            return "EGL_OPENGL_ES2_BIT";
        case  EGL_OPENGL_ES3_BIT:
            return "EGL_OPENGL_ES3_BIT";
        case  EGL_OPENGL_BIT:
            return "EGL_OPENGL_BIT";
        default:
            break;
        }
        return nullptr;
    }
}

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
            LOG_DEBUG(CONTEXT_RENDERER, "Context_EGL::Context_EGL Sharing new context with existing context: " << contextHandleToShare);
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

// Due to a bug in the Android emulator, calling eglChooseConfig with nullptr as surface
// attributes kills the emulator.
#ifndef __ANDROID__
        if(!m_eglSurfaceData.eglSharedContext)
            logAllFoundEglConfigs();
#endif

        if(!chooseEglConfig())
            return false;

        if(!createEglSurface())
            return false;

        if(!createEglContext())
            return false;

        if (!enable())
           return false;

        eglSwapInterval(m_eglSurfaceData.eglDisplay, m_swapInterval);

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

#ifdef __ANDROID__
            LOG_INFO(CONTEXT_RENDERER, "Context_EGL::~Context_EGL calling eglDestroySurface");
            if (!eglDestroySurface(m_eglSurfaceData.eglDisplay, m_eglSurfaceData.eglSurface))
            {
                LOG_ERROR(CONTEXT_RENDERER, "Context_EGL::destroy eglDestroySurface failed. Error code: " << eglGetError());
            }
#else
            LOG_INFO(CONTEXT_RENDERER, "Context_EGL::~Context_EGL calling eglDestroySurface if !isSharedContext:" << isSharedContext);
            if (!isSharedContext && !eglDestroySurface(m_eglSurfaceData.eglDisplay, m_eglSurfaceData.eglSurface))
            {
                LOG_ERROR(CONTEXT_RENDERER, "Context_EGL::destroy eglDestroySurface failed. Error code: " << eglGetError());
            }
#endif

            LOG_INFO(CONTEXT_RENDERER, "Context_EGL::~Context_EGL calling eglDestroyContext");
            if (!eglDestroyContext(m_eglSurfaceData.eglDisplay, m_eglSurfaceData.eglContext))
            {
                LOG_ERROR(CONTEXT_RENDERER, "Context_EGL::destroy eglDestroyContext failed. Error code: " << eglGetError());
            }

#ifdef __ANDROID__
            LOG_INFO(CONTEXT_RENDERER, "Context_EGL::~Context_EGL calling eglReleaseThread");
            if (!eglReleaseThread())
            {
                LOG_ERROR(CONTEXT_RENDERER, "Context_EGL: eglReleaseThread failed! Error code: " << eglGetError());
            }
#endif

            LOG_DEBUG(CONTEXT_RENDERER, "Context_EGL::~Context_EGL calling eglTerminate");
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
        LOG_INFO(CONTEXT_RENDERER, "Context_EGL::~Context_EGL done.");
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
            LOG_ERROR(CONTEXT_RENDERER, "Context_EGL::enable Error: eglMakeCurrent() failed. Error code: " << eglGetError());
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
                LOG_ERROR(CONTEXT_RENDERER, "Context_EGL::disable Error: eglMakeCurrent() failed. Error code: " << eglGetError());
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
            LOG_ERROR(CONTEXT_RENDERER, "Context_EGL initialization failed at eglGetDisplay with arg: " << m_nativeDisplay << " returned EGL_NO_DISPLAY");
            return false;
        }

        return true;
    }

    bool Context_EGL::initializeEgl()
    {
#ifdef __ANDROID__
        //eglInitialize() does not need to be called several times for same egl display, i.e., does not need to be called
        //for shared context since it was already called while initializaing the "main" context
        if(m_eglSurfaceData.eglSharedContext != nullptr)
            return true;
#endif

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

    void Context_EGL::logAllFoundEglConfigs() const
    {
        constexpr std::size_t maxConfigCount = 32u;
        EGLConfig configsResult[maxConfigCount];

        EGLint configCountResult = 0;

        if(EGL_TRUE != eglChooseConfig(m_eglSurfaceData.eglDisplay, nullptr, configsResult, maxConfigCount, &configCountResult))
        {
            LOG_ERROR(CONTEXT_RENDERER, "Context_EGL: eglChooseConfig() failed. Could not retrieve available EGL configurations");
            return;
        }

        LOG_INFO(CONTEXT_RENDERER, "Context_EGL: found: " << configCountResult << " EGL configurations");
        for(EGLint i = 0; i < configCountResult; ++i)
        {
            EGLint surfaceType = 0;
            EGLint renderableType = 0;
            EGLint bufferSize = 0;
            EGLint redSize = 0;
            EGLint greenSize = 0;
            EGLint blueSize = 0;
            EGLint alphaSize = 0;
            EGLint depthSize = 0;
            EGLint stencilSize = 0;
            EGLint sampleCount = 0;

            eglGetConfigAttrib(m_eglSurfaceData.eglDisplay, configsResult[i], EGL_SURFACE_TYPE, &surfaceType);
            eglGetConfigAttrib(m_eglSurfaceData.eglDisplay, configsResult[i], EGL_RENDERABLE_TYPE, &renderableType);
            eglGetConfigAttrib(m_eglSurfaceData.eglDisplay, configsResult[i], EGL_BUFFER_SIZE, &bufferSize);
            eglGetConfigAttrib(m_eglSurfaceData.eglDisplay, configsResult[i], EGL_RED_SIZE, &redSize);
            eglGetConfigAttrib(m_eglSurfaceData.eglDisplay, configsResult[i], EGL_GREEN_SIZE, &greenSize);
            eglGetConfigAttrib(m_eglSurfaceData.eglDisplay, configsResult[i], EGL_BLUE_SIZE, &blueSize);
            eglGetConfigAttrib(m_eglSurfaceData.eglDisplay, configsResult[i], EGL_ALPHA_SIZE, &alphaSize);
            eglGetConfigAttrib(m_eglSurfaceData.eglDisplay, configsResult[i], EGL_DEPTH_SIZE, &depthSize);
            eglGetConfigAttrib(m_eglSurfaceData.eglDisplay, configsResult[i], EGL_STENCIL_SIZE, &stencilSize);
            eglGetConfigAttrib(m_eglSurfaceData.eglDisplay, configsResult[i], EGL_SAMPLES, &sampleCount);

            LOG_INFO(CONTEXT_RENDERER, "Context_EGL: Config idx: " << i
                     << ", SURFACE_TYPE: " << surfaceType
                     << ", RENDERABLE_TYPE: " << renderableType
                     << ", BUFFER_SIZE: " << bufferSize
                     << ", RED_SIZE: " << redSize
                     << ", GREEN_SIZE: " << greenSize
                     << ", BLUE_SIZE: " << blueSize
                     << ", ALPHA_SIZE: " << alphaSize
                     << ", DEPTH_SIZE: " << depthSize
                     << ", STENCIL_SIZE: " << stencilSize
                     << ", EGL_SAMPLES: " << sampleCount);
        }
    }

    void Context_EGL::logErrorHints(const EGLint *surfaceAttributes) const
    {
        constexpr std::size_t maxConfigCount = 32u;
        EGLConfig configsResult[maxConfigCount];

        EGLint configCountResult = 0;

        if (EGL_TRUE != eglChooseConfig(m_eglSurfaceData.eglDisplay, nullptr, configsResult, maxConfigCount, &configCountResult))
        {
            return;
        }

        EGLint allRenderableTypes = 0;

        for (EGLint i = 0; i < configCountResult; ++i)
        {
            EGLint renderableType = 0;
            eglGetConfigAttrib(m_eglSurfaceData.eglDisplay, configsResult[i], EGL_RENDERABLE_TYPE, &renderableType);
            allRenderableTypes |= renderableType;
        }

        auto* iter = surfaceAttributes;
        while(EGL_NONE != *iter)
        {
            const auto value = *(iter + 1);
            if (*iter == EGL_RENDERABLE_TYPE)
            {
                // explicit message for common problem in VM: no support for GLES3.1
                if ((allRenderableTypes & value) == 0)
                {
                    const auto name = renderableTypeName(value);
                    if (name != nullptr)
                    {
                        LOG_ERROR_P(CONTEXT_RENDERER, "There is no EGL configuration that supports EGL_RENDERABLE_TYPE: {} (0x{:x})", name, value);
                    }
                    else
                    {
                        LOG_ERROR_P(CONTEXT_RENDERER, "There is no EGL configuration that supports EGL_RENDERABLE_TYPE: 0x{:x}", value);
                    }
                }
                break;
            }
            iter += 2;
        }
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
            if (0 == iConfigs)
            {
                LOG_ERROR(CONTEXT_RENDERER, "Context_EGL initialization failed at eglChooseConfig(). No configs available for the requested surface attributes:");

                while (EGL_NONE != *m_surfaceAttributes)
                {
                    const auto key = *(m_surfaceAttributes );
                    const auto value = *(m_surfaceAttributes + 1);
                    const auto* name = surfaceAttributeName(key);
                    if (name != nullptr)
                    {
                        LOG_ERROR_P(CONTEXT_RENDERER, "{}(0x{:x}): {}", name, key, value);
                    }
                    else
                    {
                        LOG_ERROR_P(CONTEXT_RENDERER, "0x{:x}: {}", key, value);
                    }
                    m_surfaceAttributes += 2;
                }
#ifndef __ANDROID__
                logErrorHints(surfaceAttributes);
#endif
            }
            else
            {
                LOG_ERROR(CONTEXT_RENDERER, "Context_EGL initialization failed at  eglChooseConfig() with error code: " << eglGetError());
            }

            eglTerminate(m_eglSurfaceData.eglDisplay);
            return false;
        }

        logUnmatchedEglConfigParams(surfaceAttributes);

        return true;
    }

    void Context_EGL::logUnmatchedEglConfigParams(const EGLint* surfaceAttributes) const
    {
        const EGLint* configParamToQuery = surfaceAttributes;
        while(EGL_NONE != *configParamToQuery)
        {
            const EGLint configParamRequestedValue = *(configParamToQuery + 1);

            EGLint configParamActualValue = 0;
            eglGetConfigAttrib(m_eglSurfaceData.eglDisplay, m_eglSurfaceData.eglConfig, *configParamToQuery, &configParamActualValue);

            auto logOnFailure = [&](bool condition, const char* paramName){
                if(!condition)
                {
                    LOG_WARN(CONTEXT_RENDERER, "Context_EGL eglChooseConfig(): The chosen config does not have requested value for param: " << *configParamToQuery
                             << (paramName == nullptr? "" : "[")
                             << (paramName == nullptr? "" : paramName)
                             << (paramName == nullptr? "" : "]")
                             << ", requested value: " << configParamRequestedValue
                             << ", actual value: " << configParamActualValue);
                }
            };

            switch(*configParamToQuery)
            {
            case EGL_SURFACE_TYPE:
            case EGL_RENDERABLE_TYPE:
                logOnFailure((configParamRequestedValue & configParamActualValue) != 0, surfaceAttributeName(*configParamToQuery));
                break;
            case EGL_SAMPLES:
                logOnFailure(configParamRequestedValue <= configParamActualValue, surfaceAttributeName(*configParamToQuery));
                break;
            default:
                logOnFailure(configParamRequestedValue == configParamActualValue, surfaceAttributeName(*configParamToQuery));
                break;
            }

            configParamToQuery += 2;
        }
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
