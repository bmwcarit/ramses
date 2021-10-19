//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Platform_Windows_WGL_ES_3_0/Platform_Windows_WGL_ES_3_0.h"

#include "Context_WGL/Context_WGL.h"
#include "Device_GL/Device_GL.h"
#include "Utils/ThreadLocalLogForced.h"

namespace ramses_internal
{
    IPlatform* Platform_Base::CreatePlatform(const RendererConfig& rendererConfig)
    {
        return new Platform_Windows_WGL_ES_3_0(rendererConfig);
    }

    Platform_Windows_WGL_ES_3_0::Platform_Windows_WGL_ES_3_0(const RendererConfig& rendererConfig)
        : Platform_Windows_WGL(rendererConfig)
    {
    }

    bool Platform_Windows_WGL_ES_3_0::createDevice()
    {
        assert(m_context);
        auto device = std::make_unique<Device_GL>(*m_context, uint8_t{ 3 }, uint8_t{ 0 }, true, nullptr);
        if (device->init())
            m_device = std::move(device);

        return m_device.get() != nullptr;
    }

    bool Platform_Windows_WGL_ES_3_0::createDeviceUploading()
    {
        assert(m_contextUploading);
        auto device = std::make_unique<Device_GL>(*m_contextUploading, uint8_t{ 3 }, uint8_t{ 0 }, true, nullptr);
        if (device->init())
            m_deviceUploading = std::move(device);

        return m_deviceUploading.get() != nullptr;
    }

    const Int32* Platform_Windows_WGL_ES_3_0::getContextAttributes()
    {
        const Int32* returnValue = NULL;

        if(m_wglExtensions.isExtensionAvailable("create_context_profile"))
        {
            static const Int32 attribs[] =
            {
                WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
                WGL_CONTEXT_MINOR_VERSION_ARB, 0,
                WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_ES_PROFILE_BIT_EXT, 0,
                0
            };
            returnValue = attribs;
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "Platform_Windows_WGL_ES_3_0::getContextAttributes:  could not load WGL context attributes");
        }

        return returnValue;
    }
}
