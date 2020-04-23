//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PLATFORM_INTEGRITY_RGL_EGL_ES_3_0_H
#define RAMSES_PLATFORM_INTEGRITY_RGL_EGL_ES_3_0_H

#include "Platform_Base/PlatformFactory_Base.h"

#include "Context_EGL/Context_EGL.h"

namespace ramses_internal
{
    class Platform_Integrity_RGL_EGL_ES_3_0 : public PlatformFactory_Base
    {
    public:
        Platform_Integrity_RGL_EGL_ES_3_0(const RendererConfig& rendererConfig);

    protected:
        ISystemCompositorController* createSystemCompositorController() override final;
        IWindow*    createWindow(const DisplayConfig& displayConfig, IWindowEventHandler& windowEventHandler) final;
        IContext*   createContext(IWindow& window) final;
        IDevice*    createDevice(IContext& context) final;
        ISurface*   createSurface(IWindow& window, IContext& context) final;
        IEmbeddedCompositor* createEmbeddedCompositor() final;

    private:
        const EGLint* getContextAttributes() const;
    };
}

#endif
