//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Platform/PlatformFactory.h"
#include "internal/RendererLib/DisplayConfigData.h"
#include "internal/RendererLib/RendererConfigData.h"
#include "internal/Core/Utils/LogMacros.h"

#if defined(ramses_sdk_ENABLE_WINDOW_TYPE_WINDOWS)
#include "internal/Platform/Windows/Platform_Windows_WGL.h"
#endif

#if defined(ramses_sdk_ENABLE_WINDOW_TYPE_X11)
#include "internal/Platform/X11/Platform_X11_EGL.h"
#endif

#if defined(ramses_sdk_ENABLE_WINDOW_TYPE_WAYLAND_IVI)
#include "internal/Platform/Wayland/IVI/Platform_Wayland_IVI_EGL_ES_3_0.h"
#endif

#if defined(ramses_sdk_ENABLE_WINDOW_TYPE_WAYLAND_WL_SHELL)
#include "internal/Platform/Wayland/WlShell/Platform_Wayland_Shell_EGL_ES_3_0.h"
#endif
#if defined(ramses_sdk_ENABLE_WINDOW_TYPE_ANDROID)
#include "internal/Platform/Android/Platform_Android_EGL.h"
#endif
#if defined(ramses_sdk_ENABLE_WINDOW_TYPE_IOS)
#include "internal/Platform/iOS/Platform_iOS_EGL.h"
#endif

#if defined(ramses_sdk_ENABLE_DEVICE_TYPE_VULKAN)
#include "internal/Platform/Vulkan/Platform_Vulkan.h"
#endif

namespace ramses::internal
{
    std::unique_ptr<IPlatform> PlatformFactory::createPlatform(const RendererConfigData& rendererConfig, const DisplayConfigData& displayConfig)
    {
        switch (displayConfig.getDeviceType())
        {
        case EDeviceType::GLES_3_0:
        case EDeviceType::GL_4_2:
        case EDeviceType::GL_4_5:
            return CreatePlatformWithOpenGL(rendererConfig, displayConfig);
        case EDeviceType::Vulkan:
            return CreatePlatformWithVulkan(rendererConfig, displayConfig);
        }

        return {};
    }

    std::unique_ptr<IPlatform> PlatformFactory::CreatePlatformWithOpenGL(const RendererConfigData& rendererConfig, const DisplayConfigData& displayConfig)
    {
        switch(displayConfig.getWindowType())
        {
        case EWindowType::Windows:
#if defined(ramses_sdk_ENABLE_WINDOW_TYPE_WINDOWS)
            return std::make_unique<Platform_Windows_WGL>(rendererConfig);
#endif
            break;
        case EWindowType::X11:
#if defined(ramses_sdk_ENABLE_WINDOW_TYPE_X11)
            return std::make_unique<Platform_X11_EGL>(rendererConfig);
#endif
            break;
        case EWindowType::Android:
#if defined(ramses_sdk_ENABLE_WINDOW_TYPE_ANDROID)
            return std::make_unique<Platform_Android_EGL>(rendererConfig);
#endif
            break;
        case ramses::EWindowType::iOS:
#if defined(ramses_sdk_ENABLE_WINDOW_TYPE_IOS)
            return std::make_unique<Platform_iOS_EGL>(rendererConfig);
#endif
            break;
        case EWindowType::Wayland_IVI:
#if defined(ramses_sdk_ENABLE_WINDOW_TYPE_WAYLAND_IVI)
            return std::make_unique<Platform_Wayland_IVI_EGL_ES_3_0>(rendererConfig);
#endif
            break;
        case EWindowType::Wayland_Shell:
#if defined(ramses_sdk_ENABLE_WINDOW_TYPE_WAYLAND_WL_SHELL)
            return std::make_unique<Platform_Wayland_Shell_EGL_ES_3_0>(rendererConfig);
#endif
            break;
        }

        LOG_ERROR(CONTEXT_RENDERER, "PlatformFactory::createPlatformWithOpenGL: trying to create an unsupported platform");
        return {};
    }

    std::unique_ptr<IPlatform> PlatformFactory::CreatePlatformWithVulkan([[maybe_unused]] const RendererConfigData& rendererConfig, [[maybe_unused]] const DisplayConfigData& displayConfig)
    {
        switch (displayConfig.getWindowType())
        {
        case EWindowType::Windows:
#if defined(ramses_sdk_ENABLE_WINDOW_TYPE_WINDOWS) && defined(ramses_sdk_ENABLE_DEVICE_TYPE_VULKAN)
            return std::make_unique<Platform_Vulkan<Window_Windows>>(rendererConfig);
#endif
            break;
        case EWindowType::X11:
#if defined(ramses_sdk_ENABLE_WINDOW_TYPE_X11) && defined(ramses_sdk_ENABLE_DEVICE_TYPE_VULKAN)
            return std::make_unique<Platform_Vulkan<Window_X11>>(rendererConfig);
#endif
            break;
        case EWindowType::Android:
#if defined(ramses_sdk_ENABLE_WINDOW_TYPE_ANDROID) && defined(ramses_sdk_ENABLE_DEVICE_TYPE_VULKAN)
#endif
            break;
        case ramses::EWindowType::iOS:
#if defined(ramses_sdk_ENABLE_WINDOW_TYPE_IOS) && defined(ramses_sdk_ENABLE_DEVICE_TYPE_VULKAN)
#endif
            break;
        case EWindowType::Wayland_IVI:
#if defined(ramses_sdk_ENABLE_WINDOW_TYPE_WAYLAND_IVI) && defined(ramses_sdk_ENABLE_DEVICE_TYPE_VULKAN)
#endif
            break;
        case EWindowType::Wayland_Shell:
#if defined(ramses_sdk_ENABLE_WINDOW_TYPE_WAYLAND_WL_SHELL) && defined(ramses_sdk_ENABLE_DEVICE_TYPE_VULKAN)
#endif
            break;
        }

        LOG_ERROR(CONTEXT_RENDERER, "PlatformFactory::createPlatformWithVulkan: trying to create an unsupported platform");
        return {};
    }
}
