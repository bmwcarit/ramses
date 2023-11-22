//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Platform/Windows/Context_WGL.h"
#include "internal/Platform/Windows/HiddenWindow.h"
#include "internal/Platform/Windows/Window_Windows.h"
#include "internal/Core/Utils/LogMacros.h"

namespace ramses::internal
{
    Context_WGL::Context_WGL(EDepthBufferType depthStencilBufferType, HDC displayHandle, WglExtensions wglExtensions, const Config& config, uint32_t msaaSampleCount)
        : m_displayHandle(displayHandle)
        , m_ext(wglExtensions)
        , m_contextAttributes(createContextAttributes(config))
        , m_msaaSampleCount(msaaSampleCount)
        , m_depthStencilBufferType(depthStencilBufferType)
    {
    }

    Context_WGL::Context_WGL(Context_WGL& sharedContext, HDC displayHandle, WglExtensions wglExtensions, uint32_t msaaSampleCount)
        : m_displayHandle(displayHandle)
        , m_ext(wglExtensions)
        , m_contextAttributes(sharedContext.m_contextAttributes)
        , m_msaaSampleCount(msaaSampleCount)
        , m_wglSharedContextHandle(sharedContext.getNativeContextHandle())
        , m_depthStencilBufferType(sharedContext.m_depthStencilBufferType)
    {
    }

    bool Context_WGL::init()
    {
        LOG_DEBUG(CONTEXT_RENDERER, " Context_WGL::init:  initializing Context_WGL");

        if (!m_ext.areLoaded() || m_contextAttributes.empty())
        {
            LOG_ERROR(CONTEXT_RENDERER, "Context extension procs not loaded, can not initialize WGL context!");
            return false;
        }

        if (!initCustomPixelFormat())
        {
            if (!HiddenWindow::InitSimplePixelFormat(m_displayHandle))
            {
                return false;
            }
        }

        m_wglContextHandle = m_ext.procs.wglCreateContextAttribsARB(m_displayHandle, m_wglSharedContextHandle, m_contextAttributes.data());

        if (0 == m_wglContextHandle)
        {
            uint32_t error = GetLastError();
            LOG_ERROR(CONTEXT_RENDERER, "wglCreateContextAttribsARB failed, returned context handle is 0. GetLastError returned error code {}", error);
            return false;
        }

        if (!enable())
        {
            LOG_ERROR(CONTEXT_RENDERER, "Enabling context failed");
            return false;
        }

        if (m_ext.procs.wglSwapIntervalEXT)
        {
            if (m_ext.procs.wglSwapIntervalEXT(1) == FALSE)
            {
                LOG_WARN(CONTEXT_RENDERER, "Context_WGL::Context_WGL:  could not set swap interval!");
            }
        }

        return (0 != m_wglContextHandle);
    }

    Context_WGL::~Context_WGL()
    {
        LOG_DEBUG(CONTEXT_RENDERER, "Context_WGL::~Context_WGL:");

        disable();
        wglDeleteContext(m_wglContextHandle);
    }

    bool Context_WGL::swapBuffers()
    {
        return SwapBuffers(m_displayHandle) == TRUE;
    }

    bool Context_WGL::enable()
    {
        if (!wglMakeCurrent(m_displayHandle, m_wglContextHandle))
            return false;
        return true;
    }

    bool Context_WGL::disable()
    {
        if (!wglMakeCurrent(NULL, NULL))
            return false;
        return true;
    }

    bool Context_WGL::initCustomPixelFormat()
    {
        if (!m_ext.procs.wglChoosePixelFormatARB)
        {
            LOG_WARN(CONTEXT_RENDERER, "Context_WGL::initCustomPixelFormat() failed. m_ext.procs.wglChoosePixelFormatARB not available");
            return false;
        }

        if (!m_ext.procs.wglGetPixelFormatAttribivARB)
        {
            LOG_WARN(CONTEXT_RENDERER, "Context_WGL::initCustomPixelFormat() failed. m_ext.procs.wglGetPixelFormatAttribivARB not available");
            return false;
        }

        int     pixelFormat;
        int     valid;
        UINT    numFormats;
        float   fAttributes[] = { 0, 0 };

        const int enableMultisampling = (m_msaaSampleCount > 1) ? GL_TRUE : GL_FALSE;
        const int sampleCount = (GL_TRUE == enableMultisampling) ? m_msaaSampleCount : 0;
        const int colorBits = 24;
        const int alphaBits = 8;

        int depthBits = 0;
        int stencilBits = 0;

        switch (m_depthStencilBufferType)
        {
        case EDepthBufferType::DepthStencil:
            depthBits   = 24u;
            stencilBits = 8u;
            break;
        case EDepthBufferType::Depth:
            depthBits   = 24u;
            stencilBits = 0u;
            break;
        case EDepthBufferType::None:
            break;
        }

        int iAttributes[] =
        {
            WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
            WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
            WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
            WGL_COLOR_BITS_ARB, colorBits,
            WGL_ALPHA_BITS_ARB, alphaBits,
            WGL_DEPTH_BITS_ARB, depthBits,
            WGL_STENCIL_BITS_ARB, stencilBits,
            WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
            WGL_SAMPLE_BUFFERS_ARB, enableMultisampling,
            WGL_SAMPLES_ARB, sampleCount,
            0, 0
        };

        valid = m_ext.procs.wglChoosePixelFormatARB(m_displayHandle, iAttributes, fAttributes, 1, &pixelFormat, &numFormats);

        if (!valid || numFormats == 0)
        {
            LOG_WARN(CONTEXT_RENDERER, "Context_WGL::initCustomPixelFormat() m_ext.procs.wglChoosePixelFormatARB failed. valid = {}, numFormats = {}", valid, numFormats);
            return false;
        }

        // check if we got the required pixel format, eg. some driver skipped the samples
        int iResultAttributes[] =
        {
            WGL_COLOR_BITS_ARB,
            WGL_ALPHA_BITS_ARB,
            WGL_DEPTH_BITS_ARB,
            WGL_STENCIL_BITS_ARB,
            WGL_SAMPLES_ARB
        };

        const int numAttribs = sizeof(iResultAttributes) / sizeof(iResultAttributes[0]);
        int resultAttribs[numAttribs] = { 0 };
        valid = m_ext.procs.wglGetPixelFormatAttribivARB(m_displayHandle, pixelFormat, 0, numAttribs, iResultAttributes, resultAttribs);
        if (!valid)
        {
            LOG_ERROR(CONTEXT_RENDERER, "Context_WGL::initCustomPixelFormat() m_ext.procs.wglGetPixelFormatAttribivARB failed.");
            return false;
        }

        LOG_INFO(CONTEXT_RENDERER, "Context_WGL::initCustomPixelFormat:  OpenGL pixel format: COLOR_BITS:{}, ALPHA_BITS :{}, DEPTH_BITS : {}, STENCIL_BITS :{}, SAMPLE_COUNT : ",
            resultAttribs[0], resultAttribs[1], resultAttribs[2], resultAttribs[3], resultAttribs[4]);

        if (resultAttribs[0] != colorBits)
            LOG_WARN(CONTEXT_RENDERER, "Context_WGL::initCustomPixelFormat:  could not get Requested pixel format. actual COLOR_BITS :{} vs requested :{}", resultAttribs[0], colorBits);

        if (resultAttribs[1] != alphaBits)
            LOG_WARN(CONTEXT_RENDERER, "Context_WGL::initCustomPixelFormat:  could not get Requested pixel format. actual ALPHA_BITS :{} vs requested :{}", resultAttribs[1], alphaBits);

        if (resultAttribs[2] != depthBits)
            LOG_WARN(CONTEXT_RENDERER, "Context_WGL::initCustomPixelFormat:  could not get Requested pixel format. actual DEPTH_BITS :{} vs requested :{}", resultAttribs[2], depthBits);

        if (resultAttribs[3] != stencilBits)
            LOG_WARN(CONTEXT_RENDERER, "Context_WGL::initCustomPixelFormat:  could not get Requested pixel format. actual STENCIL_BITS :{} vs requested :{}", resultAttribs[3], stencilBits);

        if (resultAttribs[4] != sampleCount)
            LOG_WARN(CONTEXT_RENDERER, "Context_WGL::initCustomPixelFormat:  could not get Requested pixel format. actual SAMPLE_COUNT :{} vs requested :{}", resultAttribs[4], sampleCount);

        // This code is very misleading  - it looks like setting up a surface pixel format
        // but it doesn't, the data is actually unused
        PIXELFORMATDESCRIPTOR pfd =
        {
            sizeof(PIXELFORMATDESCRIPTOR),
            1,
            PFD_DRAW_TO_WINDOW |
            PFD_SUPPORT_OPENGL |
            PFD_DOUBLEBUFFER,
            PFD_TYPE_RGBA,
            24,
            0, 0, 0, 0, 0, 0,
            1,
            0,
            0,
            0, 0, 0, 0,
            24,
            8,
            0,
            PFD_MAIN_PLANE,
            0,
            0, 0, 0
        };

        if (!SetPixelFormat(m_displayHandle, pixelFormat, &pfd))   // Are We Able To Set The Pixel Format?
        {
            LOG_ERROR(CONTEXT_RENDERER, "Context_WGL::initCustomPixelFormat:  can't set the pixelformat");
            return false;
        }

        return true;
    }

    std::vector<int32_t> Context_WGL::createContextAttributes(const Config& config)
    {
        if (m_ext.isExtensionAvailable("create_context_profile"))
        {
            if (config.gles)
            {
                return std::vector<int32_t>{
                    WGL_CONTEXT_MAJOR_VERSION_ARB, config.majorVersion,
                        WGL_CONTEXT_MINOR_VERSION_ARB, config.minorVersion,
                        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_ES_PROFILE_BIT_EXT, 0,
                        0
                };
            }
            return std::vector<int32_t>{
                WGL_CONTEXT_MAJOR_VERSION_ARB, config.majorVersion,
                    WGL_CONTEXT_MINOR_VERSION_ARB, config.minorVersion,
                    //TODO: fix profile. Tracked in seprate subtask
                    //WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
                    WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
                    0
            };
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "Context_WGL::createContextAttributes:  could not load WGL context attributes");
        }

        return {};
    }

    void* Context_WGL::getProcAddress(const char* name) const
    {
        return reinterpret_cast<void*>(wglGetProcAddress(name));
    }

    HGLRC Context_WGL::getNativeContextHandle() const
    {
        return m_wglContextHandle;
    }
}
