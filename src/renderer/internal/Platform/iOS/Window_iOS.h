//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/PlatformBase/Window_Base.h"

namespace ramses::internal
{
    class Window_iOS : public Window_Base
    {
    public:
        Window_iOS(const DisplayConfigData& displayConfig, IWindowEventHandler& windowEventHandler, uint32_t id);
        ~Window_iOS() override;

        bool init() override;

        void handleEvents() override;

        int getNativeDisplayHandle() const;
        void* getNativeWindowHandle() const;

        bool hasTitle() const override
        {
            return false;
        }

        bool setFullscreen(bool fullscreen) override;
        bool setExternallyOwnedWindowSize(uint32_t width, uint32_t height) override;

    private:
        void* m_metalLayer;
    };
}
