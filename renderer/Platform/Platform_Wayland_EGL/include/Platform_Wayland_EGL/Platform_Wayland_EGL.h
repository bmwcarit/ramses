//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PLATFORM_WAYLAND_EGL_H
#define RAMSES_PLATFORM_WAYLAND_EGL_H

#include "Window_Wayland/Window_Wayland.h"
#include "Platform_EGL/Platform_EGL.h"
#include "Window_Wayland/WindowEventsPollingManager_Wayland.h"

namespace ramses_internal
{
    class Platform_Wayland_EGL : public Platform_EGL<Window_Wayland>
    {
    public:
        const IWindowEventsPollingManager* getWindowEventsPollingManager() const override final;

    protected:
        explicit Platform_Wayland_EGL(const RendererConfig& rendererConfig);
        virtual ~Platform_Wayland_EGL();

        IEmbeddedCompositor*         createEmbeddedCompositor(const DisplayConfig& displayConfig, IContext& context) override final;
        ITextureUploadingAdapter*    createTextureUploadingAdapter(IDevice& device, IEmbeddedCompositor& embeddedCompositor, IWindow& window) override final;

        virtual uint32_t getSwapInterval() const override final;

        //TODO Mohamed: remove use of EC dummy as soon as it is possible to create multiple displays on wayland
        Bool isCreatingWaylandEmbeddedCompositorRequired() const;

        WindowEventsPollingManager_Wayland m_windowEventsPollingManager;
    };
}

#endif

