//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Platform_Windows_WGL/Platform_Windows_WGL.h"
#include "Context_WGL/Context_WGL.h"
#include "Window_Windows/Window_Windows.h"
#include "Platform_Base/EmbeddedCompositor_Dummy.h"
#include "RendererLib/DisplayConfig.h"
#include "Utils/ThreadLocalLogForced.h"
#include "Device_GL/Device_GL.h"

namespace ramses_internal
{
    Platform_Windows_WGL::Platform_Windows_WGL(const RendererConfig& rendererConfig, EDeviceTypeId deviceType)
        : Platform_Base(rendererConfig)
        , m_wglExtensions()
    {
        switch (deviceType)
        {
        case EDeviceTypeId_GL_ES_3_0:
            m_contextConfig = {3, {2, 1, 0}, true};
            break;
        case EDeviceTypeId_GL_4_5:
            m_contextConfig = {4, {5}, false};
            break;
        case EDeviceTypeId_GL_4_2_CORE:
            m_contextConfig = {4, {2}, false};
            break;
        case EDeviceTypeId_ALL:
        case EDeviceTypeId_INVALID:
            assert(false);
        }
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
        assert(!m_contextConfig.minorVersions.empty());
        if (m_contextMinorVersion.has_value())
        {
            m_context = createContextInternal(displayConfig, *m_contextMinorVersion);
        }
        else
        {
            const auto& minorVersions = m_contextConfig.minorVersions;
            for (auto minor : minorVersions)
            {
                m_context = createContextInternal(displayConfig, minor);
                if (m_context)
                {
                    m_contextMinorVersion = minor;
                    break;
                }
                LOG_ERROR_P(CONTEXT_RENDERER,
                    "Windows_WGL::createContext failed: {}.{}. Ramses will crash if any scene uses features of this version.",
                    getMajorVersionString(),
                    minor);
            }
        }

        return m_context != nullptr;
    }

    bool Platform_Windows_WGL::createContextUploading()
    {
        assert(m_window);
        assert(m_context);
        assert(m_contextMinorVersion.has_value());
        Window_Windows* platformWindow = static_cast<Window_Windows*>(m_window.get());
        const auto contextAttributes = getContextAttributes(*m_contextMinorVersion);
        auto context = std::make_unique<Context_WGL>(
            static_cast<Context_WGL&>(*m_context),
            platformWindow->getNativeDisplayHandle(),
            m_wglExtensions, contextAttributes.data(),
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
        assert(m_contextMinorVersion.has_value());
        auto device = std::make_unique<Device_GL>(*m_context, m_contextConfig.majorVersion, *m_contextMinorVersion, m_contextConfig.embedded, nullptr);
        if (device->init())
            m_device = std::move(device);

        return m_device.get() != nullptr;
    }

    bool Platform_Windows_WGL::createDeviceUploading()
    {
        assert(m_contextUploading);
        assert(m_contextMinorVersion.has_value());
        auto device = std::make_unique<Device_GL>(*m_contextUploading, m_contextConfig.majorVersion, *m_contextMinorVersion, m_contextConfig.embedded, nullptr);
        if (device->init())
            m_deviceUploading = std::move(device);

        return m_deviceUploading.get() != nullptr;
    }

    std::unique_ptr<IContext> Platform_Windows_WGL::createContextInternal(const DisplayConfig& displayConfig, UInt8 minorVersion)
    {
        assert(m_window);
        Window_Windows* platformWindow = static_cast<Window_Windows*>(m_window.get());
        const auto contextAttributes = getContextAttributes(minorVersion);
        auto context = std::make_unique<Context_WGL>(
            displayConfig.getDepthStencilBufferType(),
            platformWindow->getNativeDisplayHandle(),
            m_wglExtensions, contextAttributes.data(),
            platformWindow->getMSAASampleCount());

        if (context->init())
        {
            LOG_INFO_P(CONTEXT_RENDERER, "Windows_WGL::createContext: {}.{}", getMajorVersionString(), minorVersion);
            return context;
        }

        return {};
    }

    std::vector<Int32> Platform_Windows_WGL::getContextAttributes(Int32 minorVersion)
    {
        if (m_wglExtensions.isExtensionAvailable("create_context_profile"))
        {
            if (m_contextConfig.embedded)
            {
                return {
                    WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
                    WGL_CONTEXT_MINOR_VERSION_ARB, minorVersion,
                    WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_ES_PROFILE_BIT_EXT, 0,
                    0
                };
            }

            return {
                WGL_CONTEXT_MAJOR_VERSION_ARB, m_contextConfig.majorVersion,
                WGL_CONTEXT_MINOR_VERSION_ARB, minorVersion,
                //WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
                WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
                0
            };
        }

        LOG_ERROR(CONTEXT_RENDERER, "Platform_Windows_WGL::getContextAttributes: could not load WGL context attributes");
        return {};
    }

    std::string Platform_Windows_WGL::getMajorVersionString() const
    {
        return fmt::format("WGL{} {}", m_contextConfig.embedded ? " ES" : "", m_contextConfig.majorVersion);
    }


}
