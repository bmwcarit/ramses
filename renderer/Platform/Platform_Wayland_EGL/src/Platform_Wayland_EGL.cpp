//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Platform_Wayland_EGL/Platform_Wayland_EGL.h"
#include "Platform_Wayland_EGL/Logger_Wayland.h"
#include "RendererLib/DisplayConfig.h"
#include "RendererLib/RendererConfig.h"
#include "Platform_Base/EmbeddedCompositor_Dummy.h"
#include "EmbeddedCompositor_Wayland/EmbeddedCompositor_Wayland.h"
#include "EmbeddedCompositor_Wayland/TextureUploadingAdapter_Wayland.h"
#include "Utils/ThreadLocalLogForced.h"

namespace ramses_internal
{
    const IWindowEventsPollingManager* Platform_Wayland_EGL::getWindowEventsPollingManager() const
    {
        return &m_windowEventsPollingManager;
    }

    Platform_Wayland_EGL::Platform_Wayland_EGL(const RendererConfig& rendererConfig)
        : Platform_EGL<Window_Wayland>(rendererConfig)
        , m_windowEventsPollingManager(m_rendererConfig.getFrameCallbackMaxPollTime())
    {
        Logger_Wayland::RedirectToRamsesLogger();
    }

    Platform_Wayland_EGL::~Platform_Wayland_EGL()
    {
    }

    Bool Platform_Wayland_EGL::isCreatingWaylandEmbeddedCompositorRequired() const
    {
        //EC should be created if (any of) display config params are set
        const Bool areConfigParametersForEmbeddedCompositorSet = !m_rendererConfig.getWaylandSocketEmbedded().empty()
                || !m_rendererConfig.getWaylandSocketEmbeddedGroup().empty()
                || 0 <= m_rendererConfig.getWaylandSocketEmbeddedFD();

        return areConfigParametersForEmbeddedCompositorSet;
    }

    IEmbeddedCompositor* Platform_Wayland_EGL::createEmbeddedCompositor(const DisplayConfig& displayConfig, IContext& context)
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
            EmbeddedCompositor_Wayland* compositor = new EmbeddedCompositor_Wayland(m_rendererConfig, displayConfig, context);
            return addEmbeddedCompositor(compositor);
        }
    }

    ITextureUploadingAdapter* Platform_Wayland_EGL::createTextureUploadingAdapter(IDevice& device, IEmbeddedCompositor& embeddedCompositor, IWindow& window)
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

    uint32_t Platform_Wayland_EGL::getSwapInterval() const
    {
        return 0u;
    }
}
