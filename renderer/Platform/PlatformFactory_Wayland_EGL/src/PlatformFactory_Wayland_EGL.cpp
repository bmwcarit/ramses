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
#include "EmbeddedCompositor_Dummy/EmbeddedCompositor_Dummy.h"
#include "EmbeddedCompositor_Wayland/EmbeddedCompositor_Wayland.h"
#include "TextureUploadingAdapter_Wayland/TextureUploadingAdapter_Wayland.h"
#include "Utils/LogMacros.h"

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

        std::vector<EGLint> contextAttributes;
        getContextAttributes(contextAttributes);
        std::vector<EGLint> surfaceAttributes;
        getSurfaceAttributes(platformWindow->getMSAASampleCount(), surfaceAttributes);

        // if we do offscreen rendering, single buffer should be enough
        std::vector<EGLint> windowSurfaceAttributes;
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
                    reinterpret_cast<Context_EGL::Generic_EGLNativeWindowType>(platformWindow->getNativeWindowHandle()),
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

    Bool PlatformFactory_Wayland_EGL::isCreatingWaylandEmbeddedCompositorRequired() const
    {
        //EC should be created if (any of) display config params are set
        const Bool areConfigParametersForEmbeddedCompositorSet = !m_rendererConfig.getWaylandSocketEmbedded().empty()
                || !m_rendererConfig.getWaylandSocketEmbeddedGroup().empty()
                || 0 <= m_rendererConfig.getWaylandSocketEmbeddedFD();

        return areConfigParametersForEmbeddedCompositorSet;
    }

    IEmbeddedCompositor* PlatformFactory_Wayland_EGL::createEmbeddedCompositor()
    {
        //TODO Mohamed: remove use of EC dummy as soon as it is possible to create multiple displays on wayland
        if (!isCreatingWaylandEmbeddedCompositorRequired())
        {
            LOG_INFO(CONTEXT_RENDERER, "Embedded compositor not created because RendererConfig parameters were not set");
            EmbeddedCompositor_Dummy* compositor = new EmbeddedCompositor_Dummy();
            return addEmbeddedCompositor(compositor);
        }
        else
        {
            EmbeddedCompositor_Wayland* compositor = new EmbeddedCompositor_Wayland(m_rendererConfig);
            return addEmbeddedCompositor(compositor);
        }
    }

    ITextureUploadingAdapter* PlatformFactory_Wayland_EGL::createTextureUploadingAdapter(IDevice& device, IEmbeddedCompositor& embeddedCompositor, IWindow& window)
    {
        //TODO Mohamed: remove use of EC dummy as soon as it is possible to create multiple displays on wayland
        if (!isCreatingWaylandEmbeddedCompositorRequired())
        {
            ITextureUploadingAdapter* textureUploadingAdapter = new TextureUploadingAdapter_Base(device);
            return addTextureUploadingAdapter(textureUploadingAdapter);
        }
        else
        {
            const Window_Wayland* platformWindow = getPlatformWindow<Window_Wayland>(window);
            const EmbeddedCompositor_Wayland* platformEmbeddedCompositor = getEmbeddedCompositor<EmbeddedCompositor_Wayland>(embeddedCompositor);

            wl_display* windowWaylandDisplay = platformWindow->getNativeDisplayHandle();
            wl_display* embeddedCompositingDisplay = platformEmbeddedCompositor->getEmbeddedCompositingDisplay();

            ITextureUploadingAdapter* textureUploadingAdapter = new TextureUploadingAdapter_Wayland(device, windowWaylandDisplay, embeddedCompositingDisplay);
            return addTextureUploadingAdapter(textureUploadingAdapter);
        }
    }

}
