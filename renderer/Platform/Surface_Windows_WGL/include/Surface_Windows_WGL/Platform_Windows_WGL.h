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
#include "Context_WGL/WglExtensions.h"

namespace ramses_internal
{
    class Platform_Windows_WGL : public Platform_Base
    {
    protected:
        Platform_Windows_WGL(const RendererConfig& rendererConfig);
        ISystemCompositorController* createSystemCompositorController() override final;
        IWindow*    createWindow(const DisplayConfig& displayConfig, IWindowEventHandler& windowEventHandler) override;
        IContext*   createContext(IWindow& window) override;
        IEmbeddedCompositor*    createEmbeddedCompositor(const DisplayConfig& displayConfig, IContext& context) override;

        WglExtensions m_wglExtensions;

    private:
        // must be implemented by sub-classes
        virtual const Int32* getContextAttributes() = 0;
    };
}

#endif
