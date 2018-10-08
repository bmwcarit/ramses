//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "PlatformFactory_Wayland_IVI_EGL/PlatformFactory_Wayland_IVI_EGL.h"

#include "Window_Wayland_IVI/Window_Wayland_IVI.h"
#include "EmbeddedCompositor_Dummy/EmbeddedCompositor_Dummy.h"
#include "EmbeddedCompositor_Wayland/EmbeddedCompositor_Wayland.h"
#include "TextureUploadingAdapter_Wayland/TextureUploadingAdapter_Wayland.h"
#include "SystemCompositorController_Wayland_IVI/SystemCompositorController_Wayland_IVI.h"
#include "WindowEventsPollingManager_Wayland/WindowEventsPollingManager_Wayland.h"
#include "RendererLib/RendererConfig.h"
#include "RendererLib/DisplayConfig.h"
#include "Utils/LogMacros.h"

namespace ramses_internal
{
    PlatformFactory_Wayland_IVI_EGL::PlatformFactory_Wayland_IVI_EGL(const RendererConfig& rendererConfig)
        : PlatformFactory_Wayland_EGL(rendererConfig)
    {
    }

    ISystemCompositorController* PlatformFactory_Wayland_IVI_EGL::createSystemCompositorController()
    {
        SystemCompositorController_Wayland_IVI* waylandSystemCompositorController = new SystemCompositorController_Wayland_IVI();
        return setPlatformSystemCompositorController(waylandSystemCompositorController);
    }


    IWindow* PlatformFactory_Wayland_IVI_EGL::createWindow(const DisplayConfig& displayConfig, IWindowEventHandler& windowEventHandler)
    {
        if(InvalidWaylandIviLayerId == displayConfig.getWaylandIviLayerID())
        {
            LOG_ERROR(CONTEXT_RENDERER, "Can not create Wayland IVI window because IVI layer ID was not set in display config!");
            return NULL;
        }

        if(InvalidWaylandIviSurfaceId == displayConfig.getWaylandIviSurfaceID())
        {
            LOG_ERROR(CONTEXT_RENDERER, "Can not create Wayland IVI window because IVI surface ID was not set in display config!");
            return NULL;
        }

        Window_Wayland_IVI* platformWindow = new Window_Wayland_IVI(displayConfig, windowEventHandler, m_windows.size());
        if(nullptr != addPlatformWindow(platformWindow))
            m_windowEventsPollingManager.addWindow(platformWindow);

        return platformWindow;
    }

    Bool PlatformFactory_Wayland_IVI_EGL::destroyWindow(IWindow &window)
    {
        m_windowEventsPollingManager.removeWindow(&static_cast<Window_Wayland&>(window));

        return PlatformFactory_Wayland_EGL::destroyWindow(window);
    }

    Bool PlatformFactory_Wayland_IVI_EGL::isCreatingWaylandEmbeddedCompositorRequired() const
    {
        //EC should be created if (any of) display config params are set
        const Bool areConfigParametersForEmbeddedCompositorSet = !m_rendererConfig.getWaylandSocketEmbedded().empty()
                || !m_rendererConfig.getWaylandSocketEmbeddedGroup().empty()
                || 0 <= m_rendererConfig.getWaylandSocketEmbeddedFD();

        return areConfigParametersForEmbeddedCompositorSet;
    }

    IEmbeddedCompositor* PlatformFactory_Wayland_IVI_EGL::createEmbeddedCompositor()
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

    ITextureUploadingAdapter* PlatformFactory_Wayland_IVI_EGL::createTextureUploadingAdapter(IDevice& device, IEmbeddedCompositor& embeddedCompositor, IWindow& window)
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
