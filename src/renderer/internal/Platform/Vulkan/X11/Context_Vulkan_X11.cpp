//  -------------------------------------------------------------------------
//  Copyright (C) 2024 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#include "internal/Platform/X11/Window_X11.h"
#undef Status
#undef Bool
#undef None
#undef Always

#include "Context_Vulkan_X11.h"

namespace ramses::internal
{
    Context_Vulkan_X11::Context_Vulkan_X11(Window_X11& window)
        : Context_Vulkan_Base(VK_KHR_XLIB_SURFACE_EXTENSION_NAME, true, { "VK_LAYER_MESA_device_select" })
        , m_window(window)
    {
    }

    bool Context_Vulkan_X11::createSurface()
    {
        VkXlibSurfaceCreateInfoKHR surfaceCreationInfo = {};
        surfaceCreationInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
        surfaceCreationInfo.pNext = nullptr;
        surfaceCreationInfo.window = m_window.getNativeWindowHandle();
        surfaceCreationInfo.dpy = m_window.getNativeDisplayHandle();
        VK_CHECK_RETURN_ERR(vkCreateXlibSurfaceKHR(m_instance, &surfaceCreationInfo, nullptr, &m_surface));
        return true;
    }
}
