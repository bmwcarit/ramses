//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Platform_Integrity_RGL_EGL_ES_3_0/Platform_Integrity_RGL_EGL_ES_3_0.h"
#include "Window_Integrity_RGL/Window_Integrity_RGL.h"
#include "Surface_Integrity_RGL_EGL/Surface_Integrity_RGL_EGL.h"
#include "Context_EGL/Context_EGL.h"
#include "Device_GL/Device_GL.h"
#include "EmbeddedCompositor_Dummy/EmbeddedCompositor_Dummy.h"
#include "RendererLib/DisplayConfig.h"

namespace ramses_internal
{
    IPlatformFactory* PlatformFactory_Base::CreatePlatformFactory(const RendererConfig& rendererConfig)
    {
        return new Platform_Integrity_RGL_EGL_ES_3_0(rendererConfig);
    }

    Platform_Integrity_RGL_EGL_ES_3_0::Platform_Integrity_RGL_EGL_ES_3_0(const RendererConfig& rendererConfig)
        : PlatformFactory_Base(rendererConfig)
    {
    }

    const EGLint* Platform_Integrity_RGL_EGL_ES_3_0::getContextAttributes() const
    {
        static const EGLint contextAttributes[] =
        {
            EGL_CONTEXT_CLIENT_VERSION, 3,
            EGL_NONE
        };

        return contextAttributes;
    }

    ISystemCompositorController* Platform_Integrity_RGL_EGL_ES_3_0::createSystemCompositorController()
    {
        return nullptr;
    }

    IWindow* Platform_Integrity_RGL_EGL_ES_3_0::createWindow(const DisplayConfig& displayConfig, IWindowEventHandler& windowEventHandler)
    {
        Window_Integrity_RGL* platformWindow = new Window_Integrity_RGL(displayConfig, windowEventHandler);
        return addPlatformWindow(platformWindow);
    }

    IContext* Platform_Integrity_RGL_EGL_ES_3_0::createContext(IWindow& window)
    {
        Window_Integrity_RGL* platformWindow = getPlatformWindow<Window_Integrity_RGL>(window);
        assert(0 != platformWindow);

        Context_EGL* platformContext = new Context_EGL(platformWindow->getNativeDisplayHandle(), platformWindow->getNativeWindowHandle(), getContextAttributes(), platformWindow->getSurfaceAttributes(), nullptr, 1, 0);
        return addPlatformContext(platformContext);
    }

    IDevice* Platform_Integrity_RGL_EGL_ES_3_0::createDevice(IContext& context)
    {
        Context_EGL* platformContext = getPlatformContext<Context_EGL>(context);
        assert(0 != platformContext);
        Device_GL* device = new Device_GL(*platformContext, 3, 0, true);
        return addPlatformDevice(device);
    }

    ISurface* Platform_Integrity_RGL_EGL_ES_3_0::createSurface(IWindow& window, IContext& context)
    {
        Window_Integrity_RGL* platformWindow = getPlatformWindow<Window_Integrity_RGL>(window);
        Context_EGL* platformContext = getPlatformContext<Context_EGL>(context);
        assert(0 != platformWindow);
        assert(0 != platformContext);
        Surface_Integrity_RGL_EGL* platformSurface = new Surface_Integrity_RGL_EGL(*platformWindow, *platformContext);
        return addPlatformSurface(platformSurface);
    }

    IEmbeddedCompositor* Platform_Integrity_RGL_EGL_ES_3_0::createEmbeddedCompositor()
    {
        EmbeddedCompositor_Dummy* compositor = new EmbeddedCompositor_Dummy();
        return addEmbeddedCompositor(compositor);
    }
}
