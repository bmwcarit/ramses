//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PLATFORM_WAYLAND_SHELL_EGL_ES_3_0_H
#define RAMSES_PLATFORM_WAYLAND_SHELL_EGL_ES_3_0_H

#include "Platform_Wayland_Shell_EGL/Platform_Wayland_Shell_EGL.h"

namespace ramses_internal
{
    class Platform_Wayland_Shell_EGL_ES_3_0 : public Platform_Wayland_Shell_EGL
    {
    public:
        explicit Platform_Wayland_Shell_EGL_ES_3_0(const RendererConfig& rendererConfig);

    protected:
        std::vector<EGLint> getSurfaceAttributes(UInt32 msaaSampleCount) const override final;
    };
}

#endif
