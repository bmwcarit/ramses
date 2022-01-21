//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_WINDOW_ANDROID_H
#define RAMSES_WINDOW_ANDROID_H

#include "Platform_Base/Window_Base.h"
#include "RendererAPI/IWindowEventHandler.h"
#include <android/native_window.h>
#include <EGL/egl.h>

namespace ramses_internal
{
    class Window_Android : public Window_Base
    {
    public:
        Window_Android(const DisplayConfig& displayConfig, IWindowEventHandler& windowEventHandler, UInt32 id);
        ~Window_Android() override;

        bool init() override;

        void handleEvents() override;

        EGLNativeDisplayType getNativeDisplayHandle() const;
        ANativeWindow* getNativeWindowHandle() const;

        bool hasTitle() const override
        {
            return false;
        }

        Bool setFullscreen(Bool fullscreen) override;

    private:
        ANativeWindow* m_nativeWindow;
    };
}

#endif
