//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CONTEXT_WGL_H
#define RAMSES_CONTEXT_WGL_H

#include "Platform_Base/Context_Base.h"
#include "Context_WGL/WglExtensions.h"

#include "PlatformAbstraction/MinimalWindowsH.h"
#include "SceneAPI/TextureEnums.h"

namespace ramses_internal
{
    class Context_WGL : public Context_Base
    {
    public:
        Context_WGL(ERenderBufferType depthStencilBufferType, HDC displayHandle, WglExtensions procs, const Int32* contextAttributes, UInt32 msaaSampleCount);
        Context_WGL(Context_WGL& sharedContext, HDC displayHandle, WglExtensions procs, const Int32* contextAttributes, UInt32 msaaSampleCount);
        ~Context_WGL() override;

        Bool init();

        void* getProcAddress(const Char* name) const override;

        // Platform stuff used by other platform modules
        HGLRC getNativeContextHandle() const;

        virtual bool swapBuffers() override;
        virtual bool enable() override;
        virtual bool disable() override;

    private:
        Bool initCustomPixelFormat();

        HDC m_displayHandle;
        WglExtensions m_ext;
        // Type is broken in WGL - it has no type abstraction
        const Int32* m_contextAttributes;
        UInt32 m_msaaSampleCount;

        const HGLRC m_wglSharedContextHandle = 0;
        HGLRC m_wglContextHandle = 0;
        const ERenderBufferType m_depthStencilBufferType;
    };

}

#endif
