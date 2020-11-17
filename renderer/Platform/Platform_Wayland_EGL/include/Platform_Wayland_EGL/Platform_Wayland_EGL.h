//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PLATFORM_WAYLAND_EGL_H
#define RAMSES_PLATFORM_WAYLAND_EGL_H

#include "Platform_Base/Platform_Base.h"
#include "Context_EGL/Context_EGL.h"
#include "Window_Wayland/WindowEventsPollingManager_Wayland.h"

namespace ramses_internal
{
    class Platform_Wayland_EGL : public Platform_Base
    {
    public:
        const IWindowEventsPollingManager* getWindowEventsPollingManager() const override final;

    protected:
        explicit Platform_Wayland_EGL(const RendererConfig& rendererConfig);
        ~Platform_Wayland_EGL();

        IContext*   createContext(IWindow& window) override final;
        IEmbeddedCompositor*         createEmbeddedCompositor(const DisplayConfig& displayConfig, IContext& context) override final;
        ITextureUploadingAdapter*    createTextureUploadingAdapter(IDevice& device, IEmbeddedCompositor& embeddedCompositor, IWindow& window) override final;

        virtual void getContextAttributes(std::vector<EGLint>& attributes) const = 0;
        virtual void getSurfaceAttributes(UInt32 msaaSampleCount, std::vector<EGLint>& attributes) const = 0;

        //TODO Mohamed: remove use of EC dummy as soon as it is possible to create multiple displays on wayland
        Bool isCreatingWaylandEmbeddedCompositorRequired() const;

        WindowEventsPollingManager_Wayland m_windowEventsPollingManager;
    };
}

#endif

