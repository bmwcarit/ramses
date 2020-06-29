//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Context_WGL/Context_WGL.h"

#include "Utils/LogMacros.h"
#include "Window_Windows/HiddenWindow.h"
#include "Window_Windows/Window_Windows.h"

namespace ramses_internal
{
    Context_WGL::Context_WGL(HDC displayHandle, WglExtensions wglExtensions, const Int32* contextAttributes, UInt32 msaaSampleCount, Context_WGL* sharedContext /*= NULL*/)
        : m_displayHandle(displayHandle)
        , m_ext(wglExtensions)
        , m_contextAttributes(contextAttributes)
        , m_msaaSampleCount(msaaSampleCount)
        , m_wglSharedContextHandle(0)
        , m_wglContextHandle(0)
    {
        if (0 != sharedContext)
        {
            m_wglSharedContextHandle = sharedContext->getNativeContextHandle();
        }
    }

    Bool Context_WGL::init()
    {
        LOG_DEBUG(CONTEXT_RENDERER, " Surface_Windows_WGL::init:  initializing Context_WGL");

        if (!m_ext.areLoaded() || 0 == m_contextAttributes)
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

        m_wglContextHandle = m_ext.procs.wglCreateContextAttribsARB(m_displayHandle, m_wglSharedContextHandle, m_contextAttributes);

        if (0 == m_wglContextHandle)
        {
            uint32_t error = GetLastError();
            LOG_FATAL(CONTEXT_RENDERER, "wglCreateContextAttribsARB failed, returned context handle is 0. GetLastError returned error code " << error);
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
                LOG_WARN(CONTEXT_RENDERER, "Surface_Windows_WGL::Surface_Windows_WGL:  could not set swap interval!");
            }
        }

        return (0 != m_wglContextHandle);
    }

    Context_WGL::~Context_WGL()
    {
        LOG_DEBUG(CONTEXT_RENDERER, "Surface_Windows_WGL::~Surface_Windows_WGL:");

        disable();
        wglDeleteContext(m_wglContextHandle);
    }

    Bool Context_WGL::swapBuffers()
    {
        return SwapBuffers(m_displayHandle) == TRUE;
    }

    bool Context_WGL::enable()
    {
        if (!wglMakeCurrent(m_displayHandle, m_wglContextHandle))
            return false;
        return true;
    }

    Bool Context_WGL::disable()
    {
        if (!wglMakeCurrent(NULL, NULL))
            return false;
        return true;
    }

    Bool Context_WGL::initCustomPixelFormat()
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

        int enableMultisampling = (m_msaaSampleCount > 1) ? GL_TRUE : GL_FALSE;
        int sampleCount = (GL_TRUE == enableMultisampling) ? m_msaaSampleCount : 0;
        int colorBits = 24;
        int alphaBits = 8;
        int depthBits = 16;
        int stencilBits = 8;

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
            LOG_WARN(CONTEXT_RENDERER, "Context_WGL::initCustomPixelFormat() m_ext.procs.wglChoosePixelFormatARB failed. valid = " << valid << ", numFormats = " << numFormats);
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
            LOG_WARN(CONTEXT_RENDERER, "Context_WGL::initCustomPixelFormat() m_ext.procs.wglGetPixelFormatAttribivARB failed.");
            return false;
        }
        if (resultAttribs[0] < colorBits ||
            resultAttribs[1] < alphaBits ||
            resultAttribs[2] < depthBits ||
            resultAttribs[3] < stencilBits ||
            resultAttribs[4] < sampleCount )
        {
            LOG_ERROR(CONTEXT_RENDERER, "Surface_Windows_WGL::initCustomPixelFormat:  could not get Requested pixel format. C:A:D:S:S "
                << resultAttribs[0] << ":" << resultAttribs[1] << ":" << resultAttribs[2] << ":"
                << resultAttribs[3] << ":" << resultAttribs[4]);
        }
        else
        {
            LOG_INFO(CONTEXT_RENDERER, "Surface_Windows_WGL::initCustomPixelFormat:  OpenGL pixel format C:A:D:S:S "
                << colorBits << ":" << alphaBits << ":" << depthBits << ":" << stencilBits << ":" << sampleCount);
        }

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
            LOG_ERROR(CONTEXT_RENDERER, "Surface_Windows_WGL::initCustomPixelFormat:  can't set the pixelformat");
            return false;
        }

        return true;
    }

    void* Context_WGL::getProcAddress(const Char* name) const
    {
        return reinterpret_cast<void*>(wglGetProcAddress(name));
    }

    HGLRC Context_WGL::getNativeContextHandle() const
    {
        return m_wglContextHandle;
    }
}
