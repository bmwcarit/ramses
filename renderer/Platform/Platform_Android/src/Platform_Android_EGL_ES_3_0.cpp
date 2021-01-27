//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Platform_Android/Platform_Android_EGL_ES_3_0.h"
#include <EGL/eglext.h>

namespace ramses_internal
{
    IPlatform* Platform_Base::CreatePlatform(const RendererConfig& rendererConfig)
    {
        return new Platform_Android_EGL_ES_3_0(rendererConfig);
    }

    Platform_Android_EGL_ES_3_0::Platform_Android_EGL_ES_3_0(const RendererConfig& rendererConfig)
        : Platform_Android_EGL(rendererConfig)
    {
    }

    std::vector<EGLint> Platform_Android_EGL_ES_3_0::getSurfaceAttributes(UInt32 msaaSampleCount) const
    {
        return std::vector<EGLint>
        {
            EGL_SURFACE_TYPE,
            EGL_WINDOW_BIT,

            EGL_RENDERABLE_TYPE,
            EGL_OPENGL_ES3_BIT_KHR,

            EGL_RED_SIZE,
            8,

            EGL_ALPHA_SIZE,
            8,

            EGL_DEPTH_SIZE,
            1,

            EGL_STENCIL_SIZE,
            8,

            EGL_SAMPLE_BUFFERS,
            (msaaSampleCount > 1) ? 1 : 0,

            EGL_SAMPLES,
            static_cast<EGLint>(msaaSampleCount > 1 ? msaaSampleCount : 0),

            EGL_NONE
        };
    }
}
