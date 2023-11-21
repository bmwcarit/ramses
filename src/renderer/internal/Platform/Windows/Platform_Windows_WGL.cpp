//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Platform/Windows/Platform_Windows_WGL.h"
#include "internal/Platform/Windows/Context_WGL.h"
#include "internal/Platform/OpenGL/Device_GL.h"
#include "internal/Platform/Windows/Window_Windows.h"
#include "internal/RendererLib/PlatformBase/EmbeddedCompositor_Dummy.h"
#include "internal/RendererLib/DisplayConfig.h"
#include "internal/Core/Utils/LogMacros.h"

namespace ramses::internal
{
    Platform_Windows_WGL::Platform_Windows_WGL(const RendererConfig& rendererConfig)
        : Platform_Base(rendererConfig)
        , m_wglExtensions()
    {
    }

    bool Platform_Windows_WGL::createWindow(const DisplayConfig& displayConfig, IWindowEventHandler& windowEventHandler)
    {
        auto window = std::make_unique<Window_Windows>(displayConfig, windowEventHandler, 0u);
        if (window->init())
        {
            m_window = std::move(window);
            return true;
        }

        return false;
    }

    bool Platform_Windows_WGL::createContext(const DisplayConfig& displayConfig)
    {
        if (m_contextConfig.has_value())
        {
            m_context = createContextInternal(displayConfig, *m_contextConfig);
            return m_context != nullptr;
        }

        std::vector<uint8_t> minorVersions;
        Context_WGL::Config  contextConfig;
        switch (displayConfig.getDeviceType())
        {
        case EDeviceType::GLES_3_0:
            contextConfig.majorVersion = 3;
            minorVersions = {2, 1, 0};
            contextConfig.gles = true;
            break;
        case EDeviceType::GL_4_2:
            contextConfig.majorVersion = 4;
            minorVersions = {2};
            break;
        case EDeviceType::GL_4_5:
            contextConfig.majorVersion = 4;
            minorVersions = {5};
            break;
        }

        for (auto minor : minorVersions)
        {
            contextConfig.minorVersion = minor;
            m_context = createContextInternal(displayConfig, contextConfig);
            if (m_context)
            {
                m_contextConfig = contextConfig;
                break;
            }
            LOG_ERROR(CONTEXT_RENDERER, "Windows_WGL::createContext failed: {}. Ramses will crash if any scene uses features of this version.", GetVersionString(contextConfig));
        }
        return m_context != nullptr;
    }

    bool Platform_Windows_WGL::createContextUploading()
    {
        assert(m_window);
        assert(m_context);
        assert(m_contextConfig.has_value());
        Window_Windows* platformWindow = static_cast<Window_Windows*>(m_window.get());
        auto context = std::make_unique<Context_WGL>(
            static_cast<Context_WGL&>(*m_context),
            platformWindow->getNativeDisplayHandle(),
            m_wglExtensions,
            platformWindow->getMSAASampleCount());

        if (context->init())
        {
            m_contextUploading = std::move(context);
            return true;
        }

        return false;
    }

    bool Platform_Windows_WGL::createDevice()
    {
        assert(m_context);
        assert(m_contextConfig.has_value());
        auto device = std::make_unique<Device_GL>(*m_context, nullptr);
        if (device->init())
            m_device = std::move(device);

        return m_device.get() != nullptr;
    }

    bool Platform_Windows_WGL::createDeviceUploading()
    {
        assert(m_contextUploading);
        assert(m_contextConfig.has_value());
        auto device = std::make_unique<Device_GL>(*m_contextUploading, nullptr);
        if (device->init())
            m_deviceUploading = std::move(device);

        return m_deviceUploading.get() != nullptr;
    }

    std::unique_ptr<IContext> Platform_Windows_WGL::createContextInternal(const DisplayConfig& displayConfig, const Context_WGL::Config& contextConfig)
    {
        assert(m_window);
        Window_Windows* platformWindow = static_cast<Window_Windows*>(m_window.get());
        auto context = std::make_unique<Context_WGL>(
            displayConfig.getDepthStencilBufferType(),
            platformWindow->getNativeDisplayHandle(),
            m_wglExtensions, contextConfig,
            platformWindow->getMSAASampleCount());

        if (context->init())
        {
            LOG_INFO(CONTEXT_RENDERER, "Windows_WGL::createContext: {}", GetVersionString(contextConfig));
            return context;
        }

        return {};
    }

    std::string Platform_Windows_WGL::GetVersionString(const Context_WGL::Config& config)
    {
        return fmt::format("WGL{} {}.{}", config.gles ? " ES" : "", config.majorVersion, config.minorVersion);
    }
}
