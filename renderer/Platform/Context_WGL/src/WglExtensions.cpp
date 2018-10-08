//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Context_WGL/WglExtensions.h"
#include "Window_Windows/HiddenWindow.h"
#include "Utils/LogMacros.h"
#include "Platform_Base/Context_Base.h"

namespace ramses_internal
{
    Procs::Procs()
        : wglChoosePixelFormatARB(0)
        , wglGetPixelFormatAttribivARB(0)
        , wglCreateContextAttribsARB(0)
        , wglGetExtensionsStringARB(0)
        , wglSwapIntervalEXT(0)
    {
    }

    WglExtensions::WglExtensions()
        : m_loaded(false)
    {
        HiddenWindow hiddenWindow;

        // Create a temporary hidden window to query extensions - setPixelFormat can't be called more than once on same HDC
        if (!hiddenWindow.successfullyCreated)
        {
            return;
        }

        if (!HiddenWindow::InitSimplePixelFormat(hiddenWindow.displayHandle))
        {
            return;
        }

        HGLRC tmpContext = wglCreateContext(hiddenWindow.displayHandle);

        if (!tmpContext != 0)
        {
            LOG_ERROR(CONTEXT_RENDERER, "WglExtensions::WglExtensions:  can't create graphic context");
            return;
        }

        if (!wglMakeCurrent(hiddenWindow.displayHandle, tmpContext))
        {
            LOG_ERROR(CONTEXT_RENDERER, "WglExtensions::WglExtensions:  can't activate graphic context");
            return;
        }

        //load context procedures
        procs.wglGetExtensionsStringARB = reinterpret_cast<PFNWGLGETEXTENSIONSSTRINGARBPROC>(wglGetProcAddress("wglGetExtensionsStringARB"));
        if (!procs.wglGetExtensionsStringARB)
        {
            LOG_ERROR(CONTEXT_RENDERER, "WglExtensions::WglExtensions:  could not load wglGetExtensionsStringARB(), does your graphics driver support this function?");
            return;
        }

        // Loading context extensions
        const Char* contextExtensionsNativeString = procs.wglGetExtensionsStringARB(hiddenWindow.displayHandle);
        if (0 != contextExtensionsNativeString)
        {
            LOG_INFO(CONTEXT_RENDERER, "WglExtensions::WglExtensions:  " << contextExtensionsNativeString);
            Context_Base::ParseContextExtensionsHelper(contextExtensionsNativeString, m_extensionNames);
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "WglExtensions::WglExtensions:  could not get WGL extensions string, no context extensions are loaded");
            return;
        }

        //we need WGL_ARB_create_context in order to choose different context types
        if (isExtensionAvailable("create_context"))
        {
            procs.wglCreateContextAttribsARB = reinterpret_cast<PFNWGLCREATECONTEXTATTRIBSARBPROC>(wglGetProcAddress("wglCreateContextAttribsARB"));
            if (!procs.wglCreateContextAttribsARB)
            {
                LOG_ERROR(CONTEXT_RENDERER, "WglExtensions::WglExtensions:  could not load wglCreateContextAttribsARB(), does your graphics driver support this function?");
                return;
            }
        }

        if (isExtensionAvailable("pixel_format"))
        {
            procs.wglChoosePixelFormatARB = reinterpret_cast<PFNWGLCHOOSEPIXELFORMATARBPROC>(wglGetProcAddress("wglChoosePixelFormatARB"));
            procs.wglGetPixelFormatAttribivARB = reinterpret_cast<PFNWGLGETPIXELFORMATATTRIBIVARBPROC>(wglGetProcAddress("wglGetPixelFormatAttribivARB"));

            if (!procs.wglChoosePixelFormatARB || !procs.wglGetPixelFormatAttribivARB)
            {
                LOG_ERROR(CONTEXT_RENDERER, "WglExtensions::WglExtensions:  could not load wglChoosePixelFormatARB(), does your graphics driver support this function?");
                return;
            }
        }

        if (isExtensionAvailable("swap_control"))
        {
            procs.wglSwapIntervalEXT = reinterpret_cast<PFNWGLSWAPINTERVALEXTPROC>(wglGetProcAddress("wglSwapIntervalEXT"));
        }

        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(tmpContext);

        m_loaded = true;
    }

    Bool WglExtensions::areLoaded() const
    {
        return m_loaded;
    }

    Bool WglExtensions::isExtensionAvailable(const String& extensionName)
    {
        // try out various prefixes; add more if required
        String nameEXT = "WGL_EXT_" + extensionName;
        String nameARB = "WGL_ARB_" + extensionName;

        return  m_extensionNames.hasElement(nameEXT) ||
            m_extensionNames.hasElement(nameARB);
    }

}
