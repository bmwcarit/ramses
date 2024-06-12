//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/PlatformBase/Context_Base.h"
#include "internal/Platform/Windows/WglExtensions.h"

#include "ramses/renderer/Types.h"

#include "internal/PlatformAbstraction/MinimalWindowsH.h"
#include "internal/SceneGraph/SceneAPI/TextureEnums.h"

namespace ramses::internal
{
    class Context_WGL : public Context_Base
    {
    public:
        struct Config
        {
            uint8_t majorVersion = 4;
            uint8_t minorVersion = 2;
            bool    gles = false;
        };

        Context_WGL(EDepthBufferType depthStencilBufferType, HDC displayHandle, WglExtensions& procs, const Config& config, uint32_t msaaSampleCount);
        Context_WGL(Context_WGL& sharedContext, HDC displayHandle, WglExtensions& procs, uint32_t msaaSampleCount);
        ~Context_WGL() override;

        bool init();

        [[nodiscard]] GlProcLoadFunc getGlProcLoadFunc() const override;

        // Platform stuff used by other platform modules
        HGLRC getNativeContextHandle() const;

        virtual bool swapBuffers() override;
        virtual bool enable() override;
        virtual bool disable() override;

    private:
        bool initCustomPixelFormat();
        std::vector<int32_t> createContextAttributes(const Config& config);

        HDC m_displayHandle;
        const WglExtensions& m_ext;
        // Type is broken in WGL - it has no type abstraction
        const std::vector<int32_t> m_contextAttributes;
        uint32_t m_msaaSampleCount;

        const HGLRC m_wglSharedContextHandle = 0;
        HGLRC m_wglContextHandle = 0;
        const EDepthBufferType m_depthStencilBufferType;
    };

}
