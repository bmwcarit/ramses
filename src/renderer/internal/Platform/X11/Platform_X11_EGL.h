//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Platform/EGL/Platform_EGL.h"
#include "internal/Platform/X11/Window_X11.h"

namespace ramses::internal
{
    class Platform_X11_EGL : public Platform_EGL<Window_X11>
    {
    public:
        explicit Platform_X11_EGL(const RendererConfig& rendererConfig);

    protected:
        bool createWindow(const DisplayConfig& displayConfig, IWindowEventHandler& windowEventHandler) override;
        [[nodiscard]] uint32_t getSwapInterval() const override;
    };
}
