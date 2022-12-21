//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_WINDOW_IOS_H
#define RAMSES_WINDOW_IOS_H

#include "Platform_Base/Window_Base.h"
#include "RendererAPI/IWindowEventHandler.h"
#include <EGL/egl.h>

@class CAMetalLayer;

namespace ramses_internal
{
    class Window_iOS : public Window_Base
    {
    public:
        Window_iOS(const DisplayConfig& displayConfig, IWindowEventHandler& windowEventHandler, UInt32 id);
        ~Window_iOS() override;

        bool init() override;

        void handleEvents() override;

        EGLNativeDisplayType getNativeDisplayHandle() const;
        EGLNativeWindowType getNativeWindowHandle() const;

        bool hasTitle() const override
        {
            return false;
        }

        Bool setFullscreen(Bool fullscreen) override;
        bool setExternallyOwnedWindowSize(uint32_t width, uint32_t height) override;

    private:
        CAMetalLayer* mMetalLayer;
    };
}

#endif
