//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Platform/Wayland/Platform_Wayland_EGL.h"

namespace ramses::internal
{
    class Platform_Wayland_Shell_EGL_ES_3_0 : public Platform_Wayland_EGL
    {
    public:
        explicit Platform_Wayland_Shell_EGL_ES_3_0(const RendererConfigData& rendererConfig);

    protected:
        bool                         createWindow(const DisplayConfigData& displayConfig, IWindowEventHandler& windowEventHandler) override;
    };
}

