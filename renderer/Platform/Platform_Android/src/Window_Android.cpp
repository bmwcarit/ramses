//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/DisplayConfig.h"
#include "Utils/LogMacros.h"
#include "Utils/Warnings.h"

#include "RendererAPI/IWindowEventHandler.h"
#include "Platform_Android/Window_Android.h"

namespace ramses_internal
{
    Window_Android::Window_Android(const DisplayConfig& displayConfig, IWindowEventHandler &windowEventHandler, UInt32 id)
        : Window_Base(displayConfig, windowEventHandler, id)
        , m_nativeWindow(static_cast<ANativeWindow*>(displayConfig.getAndroidNativeWindow().getValue()))
    {
        LOG_INFO(CONTEXT_RENDERER, "Window_Android::Window_Android");
    }

    Window_Android::~Window_Android()
    {
        LOG_INFO(CONTEXT_RENDERER, "Window_Android::~Window_Android");
    }

    Bool Window_Android::init()
    {
        return true;
    }

    EGLNativeDisplayType Window_Android::getNativeDisplayHandle() const
    {
        return EGL_DEFAULT_DISPLAY;
    }

    ANativeWindow* Window_Android::getNativeWindowHandle() const
    {
        return m_nativeWindow;
    }

    Bool Window_Android::setFullscreen(Bool fullscreen)
    {
        UNUSED(fullscreen);
        return true;
    }

    void Window_Android::handleEvents()
    {
    }
}
