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

    bool Platform_Wayland_EGL::createEmbeddedCompositor(const DisplayConfig& displayConfig)
    {
        //TODO Mohamed: remove use of EC dummy as soon as it is possible to create multiple displays on wayland
        if (!isCreatingWaylandEmbeddedCompositorRequired())
        {
            LOG_INFO(CONTEXT_RENDERER, "Embedded compositor not created because RendererConfig parameters were not set");
            return Platform_EGL<Window_Wayland>::createEmbeddedCompositor(displayConfig);
        }
        else
        {
            auto compositor = std::make_unique<EmbeddedCompositor_Wayland>(m_rendererConfig, displayConfig, *m_context);
            if (compositor->init())
                m_embeddedCompositor = std::move(compositor);

            return m_embeddedCompositor != nullptr;
        }
    }

    void Platform_Wayland_EGL::createTextureUploadingAdapter()
    {
        assert(m_device);
        //TODO Mohamed: remove use of EC dummy as soon as it is possible to create multiple displays on wayland
        if (!isCreatingWaylandEmbeddedCompositorRequired())
        {
            Platform_EGL<Window_Wayland>::createTextureUploadingAdapter();
        }
        else
        {
            const Window_Wayland* platformWindow = static_cast<const Window_Wayland*>(m_window.get());
            const EmbeddedCompositor_Wayland* platformEmbeddedCompositor = static_cast<EmbeddedCompositor_Wayland*>(m_embeddedCompositor.get());

            wl_display* windowWaylandDisplay = platformWindow->getNativeDisplayHandle();
            wl_display* embeddedCompositingDisplay = platformEmbeddedCompositor->getEmbeddedCompositingDisplay();

            m_textureUploadingAdapter = std::make_unique<TextureUploadingAdapter_Wayland>(*m_device, windowWaylandDisplay, embeddedCompositingDisplay);
        }
    }

    uint32_t Platform_Wayland_EGL::getSwapInterval() const
    {
        return 0u;
    }
}
