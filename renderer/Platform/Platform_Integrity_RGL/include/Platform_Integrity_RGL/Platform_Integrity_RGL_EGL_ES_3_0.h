//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PLATFORM_INTEGRITY_RGL_EGL_ES_3_0_H
#define RAMSES_PLATFORM_INTEGRITY_RGL_EGL_ES_3_0_H

#include "Platform_Integrity_RGL/Window_Integrity_RGL.h"
#include "Platform_EGL/Platform_EGL.h"

namespace ramses_internal
{
    class Platform_Integrity_RGL_EGL_ES_3_0 : public Platform_EGL<Window_Integrity_RGL>
    {
    public:
        Platform_Integrity_RGL_EGL_ES_3_0(const RendererConfig& rendererConfig);

    protected:
        ISystemCompositorController* createSystemCompositorController() override final;
        IWindow*    createWindow(const DisplayConfig& displayConfig, IWindowEventHandler& windowEventHandler) final;
        IEmbeddedCompositor* createEmbeddedCompositor(const DisplayConfig& displayConfig, IContext& context) final;

        virtual uint32_t getSwapInterval() const override;
        virtual std::vector<EGLint> getSurfaceAttributes(UInt32 msaaSampleCount) const override final;
    };
}

#endif
