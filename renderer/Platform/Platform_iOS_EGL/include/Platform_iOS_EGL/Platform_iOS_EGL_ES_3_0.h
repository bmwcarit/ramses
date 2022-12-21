//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PLATFORM_IOS_EGL_H
#define RAMSES_PLATFORM_IOS_EGL_H


#include "Platform_iOS_EGL/Window_iOS.h"
#include "Platform_EGL/Platform_EGL.h"
#include "Context_EGL/Context_EGL.h"

namespace ramses_internal
{
    class Platform_iOS_EGL_ES_3_0 : public Platform_EGL<Window_iOS>
    {
    public:
        explicit Platform_iOS_EGL_ES_3_0(const RendererConfig& rendererConfig);
    protected:

        virtual bool createWindow(const DisplayConfig& displayConfig, IWindowEventHandler& windowEventHandler) override;
        virtual uint32_t getSwapInterval() const override;
    };
}

#endif
