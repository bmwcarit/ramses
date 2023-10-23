//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once


#include "internal/Platform/Android/Window_Android.h"
#include "internal/Platform/EGL/Platform_EGL.h"
#include "internal/Platform/EGL/Context_EGL.h"

namespace ramses::internal
{
    class Platform_Android_EGL : public Platform_EGL<Window_Android>
    {
    public:
        explicit Platform_Android_EGL(const RendererConfig& rendererConfig);

    protected:
        virtual bool createWindow(const DisplayConfig& displayConfig, IWindowEventHandler& windowEventHandler) override;
        virtual uint32_t getSwapInterval() const override;
    };
}
