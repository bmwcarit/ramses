//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_WINDOW_INTEGRITY_RGL_H
#define RAMSES_WINDOW_INTEGRITY_RGL_H

#include "Platform_Base/Window_Base.h"
#include "Math3d/Vector4.h"
#include <EGL/egl.h>
#include "INTEGRITY.h"
#include "r_ddb_api.h"
#include "r_mmgr_config.h"
#include "r_mmgr_api.h"
#include "r_config_wm.h"
#include "r_wm_api.h"

namespace ramses_internal
{
    class Window_Integrity_RGL : public Window_Base
    {
    public:
        Window_Integrity_RGL(const DisplayConfig& displayConfig, IWindowEventHandler& windowEventHandler);
        ~Window_Integrity_RGL();

        Bool init();

        const EGLNativeDisplayType    getNativeDisplayHandle() const;
        const EGLNativeWindowType     getNativeWindowHandle() const;
        const EGLint*                 getSurfaceAttributes() const;

        bool hasTitle() const override final;

    private:
        void handleEvents() override final;
        Bool setFullscreen(Bool fullscreen) override final;

        const EGLNativeDisplayType      m_nativeDisplayHandle = EGL_DEFAULT_DISPLAY;
        const UInt32                    m_deviceUnit;
        const UInt32                    m_layer = 0u;
        const UInt32                    m_numberOfBuffers = 3u;
        const r_wm_WinColorFmt_t        m_colorFormat = R_WM_COLORFMT_ARGB8888;
        const r_wm_WinBufAllocMode_t    m_bufferAllocationMode = R_WM_WINBUF_ALLOC_INTERNAL;
        const r_wm_SurfaceType_t        m_surfaceType = R_WM_SURFACE_FB;
        const Vector4                   m_backgroundColor;

        r_wm_Window_t                   m_rglWindow;
    };
}

#endif
