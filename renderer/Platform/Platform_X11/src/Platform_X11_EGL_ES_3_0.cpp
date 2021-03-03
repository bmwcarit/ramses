//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Platform_X11/Platform_X11_EGL_ES_3_0.h"

namespace ramses_internal
{
    IPlatform* Platform_Base::CreatePlatform(const RendererConfig& rendererConfig)
    {
        return new Platform_X11_EGL_ES_3_0(rendererConfig);
    }

    Platform_X11_EGL_ES_3_0::Platform_X11_EGL_ES_3_0(const RendererConfig& rendererConfig)
        : Platform_X11_EGL(rendererConfig)
    {
    }

    uint32_t Platform_X11_EGL_ES_3_0::getSwapInterval() const
    {
        return 1u;
    }
}
