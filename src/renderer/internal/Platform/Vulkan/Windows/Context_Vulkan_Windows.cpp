//  -------------------------------------------------------------------------
//  Copyright (C) 2024 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Context_Vulkan_Windows.h"
#include "internal/Platform/Windows/Window_Windows.h"

namespace ramses::internal
{
    Context_Vulkan_Windows::Context_Vulkan_Windows(Window_Windows& window)
        : Context_Vulkan_Base(VK_KHR_WIN32_SURFACE_EXTENSION_NAME, true, { "VK_LAYER_KHRONOS_validation" })
        , m_window(window)
    {
    }

    bool Context_Vulkan_Windows::createSurface()
    {
        VkWin32SurfaceCreateInfoKHR surfaceCreationInfo = {};
        surfaceCreationInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        surfaceCreationInfo.pNext = nullptr;
        surfaceCreationInfo.hinstance = m_window.getModuleHandle();
        surfaceCreationInfo.hwnd = m_window.getNativeWindowHandle();
        VK_CHECK_RETURN_ERR(vkCreateWin32SurfaceKHR(m_instance, &surfaceCreationInfo, nullptr, &m_surface));
        return true;
    }
}
