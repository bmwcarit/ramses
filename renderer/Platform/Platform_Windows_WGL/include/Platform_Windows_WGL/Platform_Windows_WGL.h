//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PLATFORM_WINDOWS_WGL_H
#define RAMSES_PLATFORM_WINDOWS_WGL_H

#include "Platform_Base/Platform_Base.h"
#include "RendererAPI/EDeviceType.h"
#include "Context_WGL/WglExtensions.h"

namespace ramses_internal
{
    class Platform_Windows_WGL : public Platform_Base
    {
    protected:
        explicit Platform_Windows_WGL(const RendererConfig& rendererConfig);

        virtual bool createWindow(const DisplayConfig& displayConfig, IWindowEventHandler& windowEventHandler) override final;
        virtual bool createContext(const DisplayConfig& displayConfig) override final;
        virtual bool createContextUploading() override final;
        virtual bool createDevice() override final;
        virtual bool createDeviceUploading() override final;

        WglExtensions m_wglExtensions;
    };
}

#endif
