//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PLATFORMFACTORY_WAYLAND_EGL_H
#define RAMSES_PLATFORMFACTORY_WAYLAND_EGL_H

#include "Platform_Base/PlatformFactory_Base.h"
#include "Context_EGL/Context_EGL.h"
#include "WindowEventsPollingManager_Wayland/WindowEventsPollingManager_Wayland.h"

namespace ramses_internal
{
    class PlatformFactory_Wayland_EGL : public PlatformFactory_Base
    {
    public:
        const IWindowEventsPollingManager* getWindowEventsPollingManager() const override final;

    protected:
        PlatformFactory_Wayland_EGL(const RendererConfig& rendererConfig);
        ~PlatformFactory_Wayland_EGL();

        IContext*   createContext(IWindow& window) override final;
        ISurface*   createSurface(IWindow& window, IContext& context) override final;
        IEmbeddedCompositor*         createEmbeddedCompositor() override final;
        ITextureUploadingAdapter*    createTextureUploadingAdapter(IDevice& device, IEmbeddedCompositor& embeddedCompositor, IWindow& window) override final;

        virtual void getContextAttributes(std::vector<EGLint>& attributes) const = 0;
        virtual void getSurfaceAttributes(UInt32 msaaSampleCount, std::vector<EGLint>& attributes) const = 0;

        //TODO Mohamed: remove use of EC dummy as soon as it is possible to create multiple displays on wayland
        Bool isCreatingWaylandEmbeddedCompositorRequired() const;

        WindowEventsPollingManager_Wayland m_windowEventsPollingManager;
    };
}

#endif

