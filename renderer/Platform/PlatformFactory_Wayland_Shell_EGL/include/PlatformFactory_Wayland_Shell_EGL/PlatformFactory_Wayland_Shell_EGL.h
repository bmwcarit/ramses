//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PLATFORMFACTORY_WAYLAND_SHELL_EGL_H
#define RAMSES_PLATFORMFACTORY_WAYLAND_SHELL_EGL_H

#include "PlatformFactory_Wayland_EGL/PlatformFactory_Wayland_EGL.h"

namespace ramses_internal
{
    class PlatformFactory_Wayland_Shell_EGL : public PlatformFactory_Wayland_EGL
    {
    protected:
        PlatformFactory_Wayland_Shell_EGL(const RendererConfig& rendererConfig);

        ISystemCompositorController* createSystemCompositorController() override final;
        IWindow*                     createWindow(const DisplayConfig& displayConfig, IWindowEventHandler& windowEventHandler) override final;
        Bool                         destroyWindow(IWindow& window) override final;
    };
}

#endif

