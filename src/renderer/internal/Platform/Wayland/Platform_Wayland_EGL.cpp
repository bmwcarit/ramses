//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Platform/Wayland/Platform_Wayland_EGL.h"
#include "internal/Platform/Wayland/Logger_Wayland.h"
#include "internal/RendererLib/DisplayConfig.h"
#include "internal/RendererLib/RendererConfig.h"
#include "internal/RendererLib/PlatformBase/EmbeddedCompositor_Dummy.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/EmbeddedCompositor_Wayland.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/TextureUploadingAdapter_Wayland.h"
#include "internal/Core/Utils/ThreadLocalLogForced.h"

namespace ramses::internal
{
    Platform_Wayland_EGL::Platform_Wayland_EGL(const RendererConfig& rendererConfig)
        : Platform_EGL<Window_Wayland>(rendererConfig)
        , m_frameCallbackMaxPollTime(m_rendererConfig.getFrameCallbackMaxPollTime())
    {
        Logger_Wayland::RedirectToRamsesLogger();
    }

    Platform_Wayland_EGL::~Platform_Wayland_EGL() = default;

    bool Platform_Wayland_EGL::IsCreatingWaylandEmbeddedCompositorRequired(const DisplayConfig& displayConfig)
    {
        //EC should be created if (any of) display config params are set
        const bool areConfigParametersForEmbeddedCompositorSet = !displayConfig.getWaylandSocketEmbedded().empty()
                || 0 <= displayConfig.getWaylandSocketEmbeddedFD();

        return areConfigParametersForEmbeddedCompositorSet;
    }

    bool Platform_Wayland_EGL::createEmbeddedCompositor(const DisplayConfig& displayConfig)
    {
        //TODO Mohamed: remove use of EC dummy as soon as it is possible to create multiple displays on wayland
        if (!IsCreatingWaylandEmbeddedCompositorRequired(displayConfig))
        {
            LOG_INFO(CONTEXT_RENDERER, "Embedded compositor not created because RendererConfig parameters were not set");
            return Platform_EGL<Window_Wayland>::createEmbeddedCompositor(displayConfig);
        }
        auto compositor = std::make_unique<EmbeddedCompositor_Wayland>(displayConfig, static_cast<Context_EGL&>(*m_context));
        if (compositor->init())
            m_embeddedCompositor = std::move(compositor);

        return m_embeddedCompositor != nullptr;
    }

    void Platform_Wayland_EGL::createTextureUploadingAdapter(const DisplayConfig& displayConfig)
    {
        assert(m_device);
        //TODO Mohamed: remove use of EC dummy as soon as it is possible to create multiple displays on wayland
        if (!IsCreatingWaylandEmbeddedCompositorRequired(displayConfig))
        {
            Platform_EGL<Window_Wayland>::createTextureUploadingAdapter(displayConfig);
        }
        else
        {
            const auto* platformWindow = static_cast<const Window_Wayland*>(m_window.get());
            const auto* platformEmbeddedCompositor = static_cast<EmbeddedCompositor_Wayland*>(m_embeddedCompositor.get());

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
