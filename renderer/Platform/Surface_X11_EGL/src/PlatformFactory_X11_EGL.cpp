//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Surface_X11_EGL/PlatformFactory_X11_EGL.h"
#include "Utils/LogMacros.h"
#include "RendererLib/RendererConfig.h"

#include "Window_X11/Window_X11.h"
#include "Context_EGL/Context_EGL.h"
#include "Surface_X11_EGL/Surface_X11_EGL.h"
#include "EmbeddedCompositor_Dummy/EmbeddedCompositor_Dummy.h"

namespace ramses_internal
{
    PlatformFactory_X11_EGL::PlatformFactory_X11_EGL(const RendererConfig& rendererConfig)
        : PlatformFactory_Base(rendererConfig)
    {
    }

    ISystemCompositorController* PlatformFactory_X11_EGL::createSystemCompositorController()
    {
        return nullptr;
    }

    IWindow* PlatformFactory_X11_EGL::createWindow(const DisplayConfig& displayConfig, IWindowEventHandler& windowEventHandler)
    {
        Window_X11* platformWindow = new Window_X11(displayConfig, windowEventHandler, m_windows.size());
        return addPlatformWindow(platformWindow);
    }

    IContext* PlatformFactory_X11_EGL::createContext(IWindow& window)
    {
        Window_X11* platformWindow = getPlatformWindow<Window_X11>(window);
        assert(0 != platformWindow);

        std::vector<EGLint> contextAttributes;
        getContextAttributes(contextAttributes);
        std::vector<EGLint> surfaceAttributes;
        getSurfaceAttributes(platformWindow->getMSAASampleCount(), surfaceAttributes);

        Context_EGL* platformContext = new Context_EGL(
                    platformWindow->getNativeDisplayHandle(),
                    reinterpret_cast<Context_EGL::Generic_EGLNativeWindowType>(platformWindow->getNativeWindowHandle()),
                    &contextAttributes[0],
                    &surfaceAttributes[0],
                    nullptr,
                    1,
                    0);

        return addPlatformContext(platformContext);
    }

    ISurface* PlatformFactory_X11_EGL::createSurface(IWindow& window, IContext& context)
    {
        Window_X11* platformWindow = getPlatformWindow<Window_X11>(window);
        Context_EGL* platformContext = getPlatformContext<Context_EGL>(context);
        assert(0 != platformWindow);
        assert(0 != platformContext);
        Surface_X11_EGL* platformSurface = new Surface_X11_EGL(*platformWindow, *platformContext);
        return addPlatformSurface(platformSurface);
    }

    IEmbeddedCompositor* PlatformFactory_X11_EGL::createEmbeddedCompositor()
    {
        EmbeddedCompositor_Dummy* compositor = new EmbeddedCompositor_Dummy();
        return addEmbeddedCompositor(compositor);
    }
}
