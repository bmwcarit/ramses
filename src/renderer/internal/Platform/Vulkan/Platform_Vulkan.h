//  -------------------------------------------------------------------------
//  Copyright (C) 2024 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#if defined(ramses_sdk_ENABLE_WINDOW_TYPE_WINDOWS)
#include "internal/Platform/Windows/Window_Windows.h"
#include "internal/Platform/Vulkan/Windows/Context_Vulkan_Windows.h"
#endif
#if defined(ramses_sdk_ENABLE_WINDOW_TYPE_X11)
#include "internal/Platform/X11/Window_X11.h"
#include "internal/Platform/Vulkan/X11/Context_Vulkan_X11.h"
#endif

#include "internal/RendererLib/PlatformBase/Platform_Base.h"
#include "internal/Platform/Vulkan/Device_Vulkan.h"

namespace ramses::internal
{
    template<typename WindowT>
    struct Platform_Vulkan_Type_Traits
    {
    };

#if defined(ramses_sdk_ENABLE_WINDOW_TYPE_WINDOWS)
    template<> struct Platform_Vulkan_Type_Traits<Window_Windows>
    {
        using ContextT = Context_Vulkan_Windows;
    };
#endif
#if defined(ramses_sdk_ENABLE_WINDOW_TYPE_X11)
    template<> struct Platform_Vulkan_Type_Traits<Window_X11>
    {
        using ContextT = Context_Vulkan_X11;
    };
#endif


    template<typename WindowT>
    class Platform_Vulkan : public Platform_Base
    {
        using ContextT = typename Platform_Vulkan_Type_Traits<WindowT>::ContextT;

    public:
        explicit Platform_Vulkan(const RendererConfigData& rendererConfig)
            : Platform_Base(rendererConfig)
        {
        }

        bool createWindow(const DisplayConfigData& displayConfig, IWindowEventHandler& windowEventHandler) override
        {
            auto window = std::make_unique<WindowT>(displayConfig, windowEventHandler, 0u);
            if (window->init())
            {
                m_window = std::move(window);
                return true;
            }

            return false;
        }

        bool createContext([[maybe_unused]] const DisplayConfigData& displayConfig) override
        {
            assert(m_window);
            auto* platformWindow = static_cast<WindowT*>(m_window.get());

            auto context = std::make_unique<ContextT>(*platformWindow);

            if (context->init())
            {
                m_context = std::move(context);
                return true;
            }

            return false;
        }

        bool createContextUploading() override
        {
            LOG_ERROR(CONTEXT_RENDERER, "createContextUploading(): Platform_Vulkan does not support async shader upload");
            return false;
        }

        bool createDevice() override
        {
            assert(m_context);
            auto* platformContext = static_cast<ContextT*>(m_context.get());

            auto device = std::make_unique<Device_Vulkan>(*platformContext, platformContext->m_instance, platformContext->m_surface);

            if (device->init())
            {
                m_device = std::move(device);
                return true;
            }

            return false;
        }

        bool createDeviceUploading() override
        {
            LOG_ERROR(CONTEXT_RENDERER, "createDeviceUploading(): Platform_Vulkan does not support async shader upload");
            return false;
        }
    };
}
