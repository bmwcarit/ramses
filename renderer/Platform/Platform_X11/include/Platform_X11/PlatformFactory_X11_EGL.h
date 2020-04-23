//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PLATFORMFACTORY_X11_EGL_H
#define RAMSES_PLATFORMFACTORY_X11_EGL_H

#include "Platform_Base/PlatformFactory_Base.h"

#include "Context_EGL/Context_EGL.h"

namespace ramses_internal
{
    class PlatformFactory_X11_EGL : public PlatformFactory_Base
    {
    protected:
        PlatformFactory_X11_EGL(const RendererConfig& rendererConfig);

        ISystemCompositorController* createSystemCompositorController() override final;
        IWindow*    createWindow(const DisplayConfig& displayConfig, IWindowEventHandler& windowEventHandler) override final;
        IContext*   createContext(IWindow& window) override final;
        ISurface*   createSurface(IWindow& window, IContext& context) override final;
        IEmbeddedCompositor*    createEmbeddedCompositor() override;

        virtual void getContextAttributes(std::vector<EGLint>& attributes) const = 0;
        virtual void getSurfaceAttributes(UInt32 msaaSampleCount, std::vector<EGLint>& attributes) const = 0;
    };
}

#endif
