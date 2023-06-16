//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PLATFORM_X11_EGL_H
#define RAMSES_PLATFORM_X11_EGL_H

#include "Platform_EGL/Platform_EGL.h"
#include "Platform_X11_EGL/Window_X11.h"

namespace ramses_internal
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

#endif
