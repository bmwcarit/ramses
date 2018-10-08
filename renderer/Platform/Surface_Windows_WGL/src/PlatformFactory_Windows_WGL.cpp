//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Surface_Windows_WGL/PlatformFactory_Windows_WGL.h"
#include "Surface_Windows_WGL/Surface_Windows_WGL.h"
#include "Context_WGL/Context_WGL.h"
#include "Window_Windows/Window_Windows.h"
#include "EmbeddedCompositor_Dummy/EmbeddedCompositor_Dummy.h"

namespace ramses_internal
{
    PlatformFactory_Windows_WGL::PlatformFactory_Windows_WGL(const RendererConfig& rendererConfig)
        : PlatformFactory_Base(rendererConfig)
        , m_wglExtensions()
    {
    }

    ISystemCompositorController* PlatformFactory_Windows_WGL::createSystemCompositorController()
    {
        return nullptr;
    }

    IWindow* PlatformFactory_Windows_WGL::createWindow(const DisplayConfig& displayConfig, IWindowEventHandler& windowEventHandler)
    {
        Window_Windows* platformWindow = new Window_Windows(displayConfig, windowEventHandler, static_cast<UInt32>(m_windows.size()));
        return addPlatformWindow(platformWindow);
    }

    IContext* PlatformFactory_Windows_WGL::createContext(IWindow& window)
    {
        Window_Windows* platformWindow = getPlatformWindow<Window_Windows>(window);
        assert(0 != platformWindow);
        Context_WGL* context = new Context_WGL(platformWindow->getNativeDisplayHandle(), m_wglExtensions, getContextAttributes(), platformWindow->getMSAASampleCount(), 0);
        return addPlatformContext(context);
    }

    ISurface* PlatformFactory_Windows_WGL::createSurface(IWindow& window, IContext& context)
    {
        Window_Windows* platformWindow = getPlatformWindow<Window_Windows>(window);
        Context_WGL* platformContext = getPlatformContext<Context_WGL>(context);
        assert(0 != platformWindow);
        assert(0 != platformContext);
        Surface_Windows_WGL* surface = new Surface_Windows_WGL(*platformWindow, *platformContext);
        return addPlatformSurface(surface);
    }

    IEmbeddedCompositor* PlatformFactory_Windows_WGL::createEmbeddedCompositor()
    {
        EmbeddedCompositor_Dummy* compositor = new EmbeddedCompositor_Dummy();
        return addEmbeddedCompositor(compositor);
    }
}
