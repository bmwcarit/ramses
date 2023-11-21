//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Platform/iOS/Window_iOS.h"
#include "internal/RendererLib/DisplayConfig.h"
#include "internal/Core/Utils/LogMacros.h"
#include "internal/Core/Utils/Warnings.h"

#include "QuartzCore/CAMetalLayer.h"
#include "EGL/egl.h"

namespace ramses::internal
{
    Window_iOS::Window_iOS(const DisplayConfig& displayConfig, IWindowEventHandler& windowEventHandler, UInt32 id)
        : Window_Base(displayConfig, windowEventHandler, id)
        , m_metalLayer(static_cast<CAMetalLayer*>(displayConfig.getIOSNativeWindow().getValue()))
    {
        LOG_INFO(CONTEXT_RENDERER, "Window_iOS::Window_iOS");
    }

    Window_iOS::~Window_iOS()
    {
        LOG_INFO(CONTEXT_RENDERER, "Window_iOS::~Window_iOS");
    }

    bool Window_iOS::init()
    {
        return true;
    }

    int Window_iOS::getNativeDisplayHandle() const
    {
        return EGL_DEFAULT_DISPLAY;
    }

    void* Window_iOS::getNativeWindowHandle() const
    {
        return m_metalLayer;
    }

    bool Window_iOS::setFullscreen(bool fullscreen)
    {
        (void)fullscreen;
        return true;
    }

    bool Window_iOS::setExternallyOwnedWindowSize(uint32_t width, uint32_t height)
    {
        m_width = width;
        m_height = height;
        return true;
    }

    void Window_iOS::handleEvents()
    {
    }
}
