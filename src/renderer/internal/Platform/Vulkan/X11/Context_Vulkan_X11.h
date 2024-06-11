//  -------------------------------------------------------------------------
//  Copyright (C) 2024 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#define VK_USE_PLATFORM_XLIB_KHR
#include "internal/Platform/Vulkan/Context_Vulkan_Base.h"

namespace ramses::internal
{
    class Window_X11;

    class Context_Vulkan_X11 : public Context_Vulkan_Base
    {
    public:
        explicit Context_Vulkan_X11(Window_X11 &window);

    protected:
        bool createSurface() override;

    private:
        Window_X11& m_window;
    };
}
