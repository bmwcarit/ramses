//  -------------------------------------------------------------------------
//  Copyright (C) 2024 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include "internal/Platform/Vulkan/Context_Vulkan_Base.h"

namespace ramses::internal
{
    class Window_Windows;

    class Context_Vulkan_Windows : public Context_Vulkan_Base
    {
    public:
        explicit Context_Vulkan_Windows(Window_Windows& window);

    protected:
        bool createSurface() override;

    private:
        Window_Windows& m_window;
    };
}
