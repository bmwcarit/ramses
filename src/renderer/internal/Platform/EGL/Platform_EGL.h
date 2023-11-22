//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/PlatformBase/Platform_Base.h"
#include "internal/RendererLib/RendererConfig.h"
#include "internal/RendererLib/DisplayConfig.h"
#include "internal/Platform/EGL/Context_EGL.h"
#include "internal/Platform/OpenGL/Device_GL.h"
#include "internal/Core/Utils/LogMacros.h"
#include <optional>

#ifdef DEVICE_EGL_EXTENSION_SUPPORTED
#include "internal/Platform/Device_EGL_Extension/Device_EGL_Extension.h"
#endif

#include <EGL/eglext.h>

namespace ramses::internal
{
    template <typename WindowT>
    class Platform_EGL : public Platform_Base
    {
    protected:
        explicit Platform_EGL(const RendererConfig& rendererConfig)
            : Platform_Base(rendererConfig)
        {
        }

        bool createContext(const DisplayConfig& displayConfig) override
        {
            if (m_glesMinorVersion.has_value())
            {
                m_context = createContextInternal(displayConfig, nullptr, *m_glesMinorVersion);
            }
            else
            {
                const std::array<EGLint, 3> minorVersions = {2, 1, 0};
                for (auto minor : minorVersions)
                {
                    m_context = createContextInternal(displayConfig, nullptr, minor);
                    if (m_context)
                    {
                        m_glesMinorVersion = minor;
                        break;
                    }
                    LOG_ERROR(
                        CONTEXT_RENDERER, "Context_EGL::init(): Failed to create GLES 3.{} context. Ramses will crash if any scene uses GLES 3.{} features.", minor, minor);
                }
            }

            return m_context != nullptr;
        }

        bool createContextUploading() override
        {
            assert(m_context);
            assert(m_glesMinorVersion.has_value());
            m_contextUploading = createContextInternal(DisplayConfig{}, static_cast<Context_EGL*>(m_context.get()), *m_glesMinorVersion);
            return m_contextUploading != nullptr;
        }

        bool createDevice() override
        {
            assert(m_context);
            m_device = createDeviceInternal(*m_context, m_deviceExtension.get());
            return m_device != nullptr;
        }

        bool createDeviceUploading() override
        {
            assert(m_contextUploading);
            m_deviceUploading = createDeviceInternal(*m_contextUploading, nullptr);
            return m_deviceUploading != nullptr;
        }

        bool createDeviceExtension(const DisplayConfig& displayConfig) override
        {
            const auto platformRenderNode = displayConfig.getPlatformRenderNode();
            if (platformRenderNode.empty())
                return true;

#ifdef DEVICE_EGL_EXTENSION_SUPPORTED
            auto& context = static_cast<Context_EGL&>(*m_context);
            auto deviceExtension = std::make_unique<Device_EGL_Extension>(context, platformRenderNode);
            if(deviceExtension->init())
                m_deviceExtension = std::move(deviceExtension);
#endif

            return m_deviceExtension != nullptr;
        }

        /**
         * gets the platform specific default swap interval
         */
        [[nodiscard]] virtual uint32_t getSwapInterval() const = 0;

    private:
        std::unique_ptr<IContext> createContextInternal(const DisplayConfig& displayConfig, Context_EGL* sharedContext, EGLint minorVersion)
        {
            if(displayConfig.getDeviceType() != EDeviceType::GLES_3_0)
            {
                LOG_ERROR(CONTEXT_RENDERER, "Platform_EGL::createContext: Unsupported device type!");
                return {};
            }

            assert(m_window);
            auto* platformWindow = static_cast<WindowT*>(m_window.get());

            EGLint swapInterval = displayConfig.getSwapInterval();
            if (swapInterval < 0)
            {
                swapInterval = getSwapInterval();
            }
            const std::vector<EGLint> contextAttributes = GetContextAttributes(minorVersion);
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
            {
                LOG_INFO(CONTEXT_RENDERER, "Context_EGL::init(): EGL 3.{} context creation succeeded (swap interval:{})", minorVersion, swapInterval);
                return context;
            }

            return {};
        }

        std::unique_ptr<Device_GL> createDeviceInternal(IContext& context, IDeviceExtension* deviceExtension)
        {
            auto device = std::make_unique<Device_GL>(context, deviceExtension);
            if (device->init())
                return device;

            return {};
        }

        static std::vector<EGLint> GetContextAttributes(EGLint minorVersion)
        {
            return {
                EGL_CONTEXT_CLIENT_VERSION,
                3,
                EGL_CONTEXT_MINOR_VERSION,
                minorVersion,
                EGL_NONE
            };
        }

        static std::vector<EGLint> GetSurfaceAttributes(uint32_t msaaSampleCount, EDepthBufferType depthStencilBufferType)
        {
            EGLint depthBufferSize = 0;
            EGLint stencilBufferSize = 0;

            switch(depthStencilBufferType)
            {
            case EDepthBufferType::DepthStencil:
                depthBufferSize = 24;
                stencilBufferSize = 8;
                break;
            case EDepthBufferType::Depth:
                depthBufferSize = 24;
                stencilBufferSize = 0;
                break;
            case EDepthBufferType::None:
                depthBufferSize = 0;
                stencilBufferSize = 0;
                break;
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

        std::optional<EGLint> m_glesMinorVersion;
    };
}
