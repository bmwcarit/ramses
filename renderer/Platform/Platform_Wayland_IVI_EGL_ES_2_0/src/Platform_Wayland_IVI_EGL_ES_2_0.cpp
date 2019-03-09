//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Platform_Wayland_IVI_EGL_ES_2_0/Platform_Wayland_IVI_EGL_ES_2_0.h"
#include <EGL/egl.h>
#include "Context_EGL/Context_EGL.h"
#include "Device_GL/Device_GL.h"
#include "Utils/LogMacros.h"

namespace ramses_internal
{
    IPlatformFactory* PlatformFactory_Base::CreatePlatformFactory(const RendererConfig& rendererConfig)
    {
        return new Platform_Wayland_IVI_EGL_ES_2_0(rendererConfig);
    }

    Platform_Wayland_IVI_EGL_ES_2_0::Platform_Wayland_IVI_EGL_ES_2_0(const RendererConfig& rendererConfig)
        : PlatformFactory_Wayland_IVI_EGL(rendererConfig)
    {
    }

    IDevice* Platform_Wayland_IVI_EGL_ES_2_0::createDevice(IContext& context)
    {
        Context_EGL* platformContext = getPlatformContext<Context_EGL>(context);
        assert(0 != platformContext);
        Device_GL* device = new Device_GL(*platformContext, 2, 0, true);
        return addPlatformDevice(device);
    }

    void Platform_Wayland_IVI_EGL_ES_2_0::getSurfaceAttributes(UInt32 msaaSampleCount, Vector<EGLint>& attributes) const
    {
        attributes.clear();
        attributes.reserve(20u);

        attributes.push_back(EGL_SURFACE_TYPE);
        attributes.push_back(EGL_WINDOW_BIT);

        attributes.push_back(EGL_RENDERABLE_TYPE);
        attributes.push_back(EGL_OPENGL_ES2_BIT);

        attributes.push_back(EGL_RED_SIZE);
        attributes.push_back(8);

        attributes.push_back(EGL_ALPHA_SIZE);
        attributes.push_back(8);

        attributes.push_back(EGL_DEPTH_SIZE);
        attributes.push_back(24);

        attributes.push_back(EGL_STENCIL_SIZE);
        attributes.push_back(8);

        attributes.push_back(EGL_SAMPLE_BUFFERS);
        attributes.push_back((msaaSampleCount > 1) ? 1 : 0);

        attributes.push_back(EGL_SAMPLES);
        attributes.push_back((msaaSampleCount > 1) ? msaaSampleCount : 0);

        attributes.push_back(EGL_NONE);
    }

    void Platform_Wayland_IVI_EGL_ES_2_0::getContextAttributes(Vector<EGLint>& attributes) const
    {
        attributes.clear();
        attributes.reserve(2u);

        attributes.push_back(EGL_CONTEXT_CLIENT_VERSION);
        attributes.push_back(2);

        attributes.push_back(EGL_NONE);
    }
}
