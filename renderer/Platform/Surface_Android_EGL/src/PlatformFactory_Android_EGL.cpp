//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Surface_Android_EGL/PlatformFactory_Android_EGL.h"
#include "Utils/LogMacros.h"
#include "RendererLib/RendererConfig.h"

#include "Window_Android/Window_Android.h"
#include "Context_EGL/Context_EGL.h"
#include "Surface_Android_EGL/Surface_Android_EGL.h"
#include "EmbeddedCompositor_Dummy/EmbeddedCompositor_Dummy.h"

namespace ramses_internal
{
    PlatformFactory_Android_EGL::PlatformFactory_Android_EGL(const RendererConfig& rendererConfig)
        : PlatformFactory_Base(rendererConfig)
    {
    }

    ISystemCompositorController* PlatformFactory_Android_EGL::createSystemCompositorController()
    {
        return nullptr;
    }

    IWindow* PlatformFactory_Android_EGL::createWindow(const DisplayConfig& displayConfig, IWindowEventHandler& windowEventHandler)
    {
        Window_Android* platformWindow = new Window_Android(displayConfig, windowEventHandler, m_windows.size());
        return addPlatformWindow(platformWindow);
    }

    IContext* PlatformFactory_Android_EGL::createContext(IWindow& window)
    {
        Window_Android* platformWindow = getPlatformWindow<Window_Android>(window);
        assert(0 != platformWindow);

        std::vector<EGLint> contextAttributes;
        getContextAttributes(contextAttributes);
        std::vector<EGLint> surfaceAttributes;
        getSurfaceAttributes(platformWindow->getMSAASampleCount(), surfaceAttributes);

        Context_EGL* platformContext = new Context_EGL(
                    platformWindow->getNativeDisplayHandle(),
                    platformWindow->getNativeWindowHandle(),
                    &contextAttributes[0],
                    &surfaceAttributes[0],
                    nullptr,
                    1,
                    0);

        return addPlatformContext(platformContext);
    }

    ISurface* PlatformFactory_Android_EGL::createSurface(IWindow& window, IContext& context)
    {
        Window_Android* platformWindow = getPlatformWindow<Window_Android>(window);
        Context_EGL* platformContext = getPlatformContext<Context_EGL>(context);
        assert(0 != platformWindow);
        assert(0 != platformContext);
        Surface_Android_EGL* platformSurface = new Surface_Android_EGL(*platformWindow, *platformContext);
        return addPlatformSurface(platformSurface);
    }

    IEmbeddedCompositor* PlatformFactory_Android_EGL::createEmbeddedCompositor()
    {
        EmbeddedCompositor_Dummy* compositor = new EmbeddedCompositor_Dummy();
        return addEmbeddedCompositor(compositor);
    }
}
