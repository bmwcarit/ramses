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

        virtual void getContextAttributes(Vector<EGLint>& attributes) const = 0;
        virtual void getSurfaceAttributes(UInt32 msaaSampleCount, Vector<EGLint>& attributes) const = 0;

        WindowEventsPollingManager_Wayland m_windowEventsPollingManager;
    };
}

#endif

