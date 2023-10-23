//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/PlatformBase/Window_Base.h"
#include "internal/RendererLib/PlatformInterface/IWindowEventHandler.h"
#include <android/native_window.h>
#include <EGL/egl.h>

namespace ramses::internal
{
    class Window_Android : public Window_Base
    {
    public:
        Window_Android(const DisplayConfig& displayConfig, IWindowEventHandler& windowEventHandler, uint32_t id);
        ~Window_Android() override;

        bool init() override;

        void handleEvents() override;

        EGLNativeDisplayType getNativeDisplayHandle() const;
        ANativeWindow* getNativeWindowHandle() const;

        bool hasTitle() const override
        {
            return false;
        }

        bool setFullscreen(bool fullscreen) override;
        bool setExternallyOwnedWindowSize(uint32_t width, uint32_t height) override;

    private:
        ANativeWindow* m_nativeWindow;
    };
}
