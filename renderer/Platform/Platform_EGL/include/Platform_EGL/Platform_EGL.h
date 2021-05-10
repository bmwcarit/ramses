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
#include "RendererLib/DisplayConfig.h"
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

        virtual bool createContext(const DisplayConfig& displayConfig) override final
        {
            m_context = createContextInternal(displayConfig, nullptr);
            return m_context != nullptr;
        }

        virtual bool createContextUploading() override final
        {
            assert(m_context);
            m_contextUploading = createContextInternal(DisplayConfig{}, static_cast<Context_EGL*>(m_context.get()));
            return m_contextUploading != nullptr;
        }

        virtual bool createDevice() override final
        {
            assert(m_context);
            m_device = createDeviceInternal(*m_context);
            return m_device != nullptr;
        }

        virtual bool createDeviceUploading() override final
        {
            assert(m_contextUploading);
            m_deviceUploading = createDeviceInternal(*m_contextUploading);
            return m_deviceUploading != nullptr;
        }

        virtual uint32_t getSwapInterval() const = 0;

    private:
        std::unique_ptr<IContext> createContextInternal(const DisplayConfig& displayConfig, Context_EGL* sharedContext)
        {
            assert(m_window);
            WindowT* platformWindow = static_cast<WindowT*>(m_window.get());

            const auto swapInterval = getSwapInterval();
            const std::vector<EGLint> contextAttributes = GetContextAttributes();
            const std::vector<EGLint> surfaceAttributes = GetSurfaceAttributes(platformWindow->getMSAASampleCount(), displayConfig.getDepthStencilBufferType());

            auto context = std::make_unique<Context_EGL>(
                platformWindow->getNativeDisplayHandle(),
                reinterpret_cast<Context_EGL::Generic_EGLNativeWindowType>(platformWindow->getNativeWindowHandle()),
                contextAttributes.data(),
                surfaceAttributes.data(),
                nullptr,
                swapInterval,
                sharedContext);

            if (context->init())
                return context;

            return {};
        }

        std::unique_ptr<Device_GL> createDeviceInternal(IContext& context)
        {
            auto device = std::make_unique<Device_GL>(context, uint8_t{ 3 }, uint8_t{ 0 }, true);
            if (device->init())
                return device;

            return {};
        }

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
