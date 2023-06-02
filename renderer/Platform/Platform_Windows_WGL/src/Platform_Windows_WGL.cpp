//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Platform_Windows_WGL/Platform_Windows_WGL.h"
#include "Context_WGL/Context_WGL.h"
#include "Device_GL/Device_GL.h"
#include "Window_Windows/Window_Windows.h"
#include "Platform_Base/EmbeddedCompositor_Dummy.h"
#include "RendererLib/DisplayConfig.h"

namespace ramses_internal
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
        assert(m_window);
        Window_Windows* platformWindow = static_cast<Window_Windows*>(m_window.get());

        auto context = std::make_unique<Context_WGL>(
            displayConfig.getDepthStencilBufferType(),
            platformWindow->getNativeDisplayHandle(),
            m_wglExtensions,
            displayConfig.getDeviceType(),
            platformWindow->getMSAASampleCount());

        if (context->init())
        {
            m_context = std::move(context);
            return true;
        }

        return false;
    }

    bool Platform_Windows_WGL::createContextUploading()
    {
        assert(m_window);
        assert(m_context);
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
        auto device = std::make_unique<Device_GL>(*m_context, nullptr);
        if (device->init())
            m_device = std::move(device);

        return m_device.get() != nullptr;
    }

    bool Platform_Windows_WGL::createDeviceUploading()
    {
        assert(m_contextUploading);
        auto device = std::make_unique<Device_GL>(*m_contextUploading, nullptr);
        if (device->init())
            m_deviceUploading = std::move(device);

        return m_deviceUploading.get() != nullptr;
    }
}
