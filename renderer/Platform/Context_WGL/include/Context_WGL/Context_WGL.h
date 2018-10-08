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

#include "ramses-capu/os/Windows/MinimalWindowsH.h"

namespace ramses_internal
{
    class Context_WGL : public Context_Base
    {
    public:
        Context_WGL(HDC displayHandle, WglExtensions procs, const Int32* contextAttributes, UInt32 msaaSampleCount, Context_WGL* sharedContext = NULL);
        ~Context_WGL() override;

        Bool init();

        void* getProcAddress(const Char* name) const final;

        // Platform stuff used by other platform modules
        HGLRC getNativeContextHandle() const;
        Bool setEnabled(Bool active);

    private:
        Bool initCustomPixelFormat();

        HDC m_displayHandle;
        WglExtensions m_ext;
        // Type is broken in WGL - it has no type abstraction
        const Int32* m_contextAttributes;
        UInt32 m_msaaSampleCount;

        HGLRC m_wglSharedContextHandle;
        HGLRC m_wglContextHandle;
    };

}

#endif
