//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Platform/X11/Platform_X11_EGL.h"
#include "internal/RendererLib/RendererConfigData.h"
#include "internal/RendererLib/PlatformBase/EmbeddedCompositor_Dummy.h"

namespace ramses::internal
{
    Platform_X11_EGL::Platform_X11_EGL(const RendererConfigData& rendererConfig)
        : Platform_EGL<Window_X11>(rendererConfig)
    {
    }

    bool Platform_X11_EGL::createWindow(const DisplayConfigData& displayConfig, IWindowEventHandler& windowEventHandler)
    {
        auto window = std::make_unique<Window_X11>(displayConfig, windowEventHandler, 0u);
        if (window->init())
        {
            m_window = std::move(window);
            return true;
        }

        return false;
    }

    uint32_t Platform_X11_EGL::getSwapInterval() const
    {
        return 1u;
    }
}
