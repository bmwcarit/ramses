//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Platform/Android/Platform_Android_EGL.h"
#include "internal/RendererLib/RendererConfigData.h"
#include "internal/Platform/EGL/Context_EGL.h"
#include "internal/RendererLib/PlatformBase/EmbeddedCompositor_Dummy.h"

namespace ramses::internal
{
    Platform_Android_EGL::Platform_Android_EGL(const RendererConfigData& rendererConfig)
        : Platform_EGL<Window_Android>(rendererConfig)
    {
    }

    bool Platform_Android_EGL::createWindow(const DisplayConfigData& displayConfig, IWindowEventHandler& windowEventHandler)
    {
        auto window = std::make_unique<Window_Android>(displayConfig, windowEventHandler, 0u);
        if (window->init())
        {
            m_window = std::move(window);
            return true;
        }

        return false;
    }

    uint32_t Platform_Android_EGL::getSwapInterval() const
    {
        return 1u;
    }
}
