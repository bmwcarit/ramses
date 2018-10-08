//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "PlatformFactory_Wayland_EGL/PlatformFactory_Wayland_EGL.h"
#include "Surface_Wayland_EGL/Surface_Wayland_EGL.h"
#include "Surface_EGL_Offscreen/Surface_EGL_Offscreen.h"
#include "RendererLib/DisplayConfig.h"
#include "RendererLib/RendererConfig.h"
#include "Logger_Wayland/Logger_Wayland.h"
#include "Window_Wayland/Window_Wayland.h"

namespace ramses_internal
{
    const IWindowEventsPollingManager* PlatformFactory_Wayland_EGL::getWindowEventsPollingManager() const
    {
        return &m_windowEventsPollingManager;
    }

    PlatformFactory_Wayland_EGL::PlatformFactory_Wayland_EGL(const RendererConfig& rendererConfig)
        : PlatformFactory_Base(rendererConfig)
        , m_windowEventsPollingManager(m_rendererConfig.getFrameCallbackMaxPollTime())
    {
        Logger_Wayland::Init();
    }

    PlatformFactory_Wayland_EGL::~PlatformFactory_Wayland_EGL()
    {
        Logger_Wayland::Deinit();
    }

    IContext* PlatformFactory_Wayland_EGL::createContext(IWindow& window)
    {
        Window_Wayland* platformWindow = getPlatformWindow<Window_Wayland>(window);
        assert(0 != platformWindow);

        Vector<EGLint> contextAttributes;
        getContextAttributes(contextAttributes);
        Vector<EGLint> surfaceAttributes;
        getSurfaceAttributes(platformWindow->getMSAASampleCount(), surfaceAttributes);

        // if we do offscreen rendering, single buffer should be enough
        Vector<EGLint> windowSurfaceAttributes;
        if(window.isOffscreen())
        {
            windowSurfaceAttributes.push_back(EGL_RENDER_BUFFER);
            windowSurfaceAttributes.push_back(EGL_SINGLE_BUFFER);
        }
        windowSurfaceAttributes.push_back(EGL_NONE);

        // Use swap interval of 0 so the renderer does not block due invisible surfaces
        const EGLint swapInterval = 0;

        Context_EGL* platformContext = new Context_EGL(
                    reinterpret_cast<EGLNativeDisplayType>(platformWindow->getNativeDisplayHandle()),
                    reinterpret_cast<EGLNativeWindowType>(platformWindow->getNativeWindowHandle()),
                    &contextAttributes[0],
                    &surfaceAttributes[0],
                    &windowSurfaceAttributes[0],
                    swapInterval,
                    0);
        return addPlatformContext(platformContext);
    }

    ISurface* PlatformFactory_Wayland_EGL::createSurface(IWindow& window, IContext& context)
    {
        Window_Wayland* platformWindow = getPlatformWindow<Window_Wayland>(window);
        Context_EGL* platformContext = getPlatformContext<Context_EGL>(context);
        assert(0 != platformWindow);
        assert(0 != platformContext);

        ISurface* platformSurface = nullptr;
        if(window.isOffscreen())
        {
            platformSurface = new Surface_EGL_Offscreen(*platformWindow, *platformContext);
        }
        else
        {
            platformSurface = new Surface_Wayland_EGL(*platformWindow, *platformContext);
        }
        return addPlatformSurface(platformSurface);
    }
}
