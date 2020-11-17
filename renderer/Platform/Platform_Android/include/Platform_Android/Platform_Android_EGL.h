//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PLATFORM_ANDROID_EGL_H
#define RAMSES_PLATFORM_ANDROID_EGL_H

#include "Platform_Base/Platform_Base.h"

#include "Context_EGL/Context_EGL.h"

namespace ramses_internal
{
    class Platform_Android_EGL : public Platform_Base
    {
    protected:
        Platform_Android_EGL(const RendererConfig& rendererConfig);

        ISystemCompositorController* createSystemCompositorController() override final;
        IWindow*    createWindow(const DisplayConfig& displayConfig, IWindowEventHandler& windowEventHandler) override final;
        IContext*   createContext(IWindow& window) override final;
        IEmbeddedCompositor*    createEmbeddedCompositor(const DisplayConfig& displayConfig, IContext& context) override;

        virtual void getContextAttributes(std::vector<EGLint>& attributes) const = 0;
        virtual void getSurfaceAttributes(UInt32 msaaSampleCount, std::vector<EGLint>& attributes) const = 0;
    };
}

#endif
