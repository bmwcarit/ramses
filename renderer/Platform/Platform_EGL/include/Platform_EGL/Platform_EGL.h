//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PLATFORM_EGL_H
#define RAMSES_PLATFORM_EGL_H

#include "Platform_Base/Platform_Base.h"
#include "RendererLib/RendererConfig.h"
#include "Context_EGL/Context_EGL.h"
#include "Device_GL/Device_GL.h"

#include <EGL/eglext.h>

namespace ramses_internal
{
    template <typename WindowT>
    class Platform_EGL : public Platform_Base
    {
    protected:
        explicit Platform_EGL(const RendererConfig& rendererConfig)
            : Platform_Base(rendererConfig)
        {
        }

        virtual IContext* createContext(const DisplayConfig& displayConfig, IWindow& window, IContext* sharedContext) override final
        {
            WindowT* platformWindow = getPlatformWindow<WindowT>(window);
            assert(nullptr != platformWindow);

            const auto swapInterval = getSwapInterval();
            const std::vector<EGLint> contextAttributes = GetContextAttributes();
            const std::vector<EGLint> surfaceAttributes = GetSurfaceAttributes(platformWindow->getMSAASampleCount(), displayConfig.getDepthStencilBufferType());
            Context_EGL* platformSharedContext = nullptr;

            if(sharedContext)
            {
                platformSharedContext = getPlatformContext<Context_EGL>(*sharedContext);
                assert(platformSharedContext);
            }


            Context_EGL* platformContext = new Context_EGL(
                        platformWindow->getNativeDisplayHandle(),
                        reinterpret_cast<Context_EGL::Generic_EGLNativeWindowType>(platformWindow->getNativeWindowHandle()),
                        contextAttributes.data(),
                        surfaceAttributes.data(),
                        nullptr,
                        swapInterval,
                        platformSharedContext);

            return addPlatformContext(platformContext);
        }

        virtual IDevice* createDevice(IContext& context) override final
        {
            Context_EGL* platformContext = getPlatformContext<Context_EGL>(context);
            assert(nullptr != platformContext);
            Device_GL* device = new Device_GL(*platformContext, 3, 0, true);
            return addPlatformDevice(device);
        }

        virtual uint32_t getSwapInterval() const = 0;

    private:
        static std::vector<EGLint> GetContextAttributes()
        {
            return {
                EGL_CONTEXT_CLIENT_VERSION,
                3,

                EGL_NONE
            };
        }

        static std::vector<EGLint> GetSurfaceAttributes(UInt32 msaaSampleCount, ERenderBufferType depthStencilBufferType)
        {
            EGLint depthBufferSize = 0;
            EGLint stencilBufferSize = 0;

            switch(depthStencilBufferType)
            {
            case ERenderBufferType_DepthStencilBuffer:
                depthBufferSize = 24;
                stencilBufferSize = 8;
                break;
            case ERenderBufferType_DepthBuffer:
                depthBufferSize = 24;
                stencilBufferSize = 0;
                break;
            case ERenderBufferType_InvalidBuffer:
                depthBufferSize = 0;
                stencilBufferSize = 0;
                break;
            case ERenderBufferType_ColorBuffer:
            case ERenderBufferType_NUMBER_OF_ELEMENTS:
                assert(false);
            }

            return std::vector<EGLint>
            {
                EGL_SURFACE_TYPE,
                EGL_WINDOW_BIT,

                EGL_RENDERABLE_TYPE,
                EGL_OPENGL_ES3_BIT_KHR,

                EGL_BUFFER_SIZE,
                32,

                EGL_RED_SIZE,
                8,

                EGL_GREEN_SIZE,
                8,

                EGL_BLUE_SIZE,
                8,

                EGL_ALPHA_SIZE,
                8,

                EGL_DEPTH_SIZE,
                depthBufferSize,

                EGL_STENCIL_SIZE,
                stencilBufferSize,

                EGL_SAMPLE_BUFFERS,
                (msaaSampleCount > 1) ? 1 : 0,

                EGL_SAMPLES,
                static_cast<EGLint>(msaaSampleCount > 1 ? msaaSampleCount : 0),

                EGL_NONE
            };
        }
    };
}

#endif
