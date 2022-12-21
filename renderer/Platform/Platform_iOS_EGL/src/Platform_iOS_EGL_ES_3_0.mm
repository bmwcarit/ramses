//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Platform_iOS_EGL/Platform_iOS_EGL_ES_3_0.h"
#include "RendererLib/RendererConfig.h"
#include "Context_EGL/Context_EGL.h"
#include "Platform_Base/EmbeddedCompositor_Dummy.h"

namespace ramses_internal
{
    IPlatform* Platform_Base::CreatePlatform(const RendererConfig& rendererConfig)
    {
        return new Platform_iOS_EGL_ES_3_0(rendererConfig);
    }

    Platform_iOS_EGL_ES_3_0::Platform_iOS_EGL_ES_3_0(const RendererConfig& rendererConfig)
        : Platform_EGL<Window_iOS>(rendererConfig)
    {
    }

    bool Platform_iOS_EGL_ES_3_0::createWindow(const DisplayConfig& displayConfig, IWindowEventHandler& windowEventHandler)
    {
        auto window = std::make_unique<Window_iOS>(displayConfig, windowEventHandler, 0u);
        if (window->init())
        {
            m_window = std::move(window);
            return true;
        }

        return false;
    }

    uint32_t Platform_iOS_EGL_ES_3_0::getSwapInterval() const
    {
        return 1u;
    }
}
