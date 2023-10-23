//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/renderer/Types.h"
#include "internal/RendererLib/PlatformBase/Platform_Base.h"
#include "internal/Platform/Windows/Context_WGL.h"
#include "internal/Platform/Windows/WglExtensions.h"
#include <optional>

namespace ramses::internal
{
    class Platform_Windows_WGL final : public Platform_Base
    {
    public:
        explicit Platform_Windows_WGL(const RendererConfig& rendererConfig);

    private:
        virtual bool createWindow(const DisplayConfig& displayConfig, IWindowEventHandler& windowEventHandler) override;
        virtual bool createContext(const DisplayConfig& displayConfig) override;
        virtual bool createContextUploading() override;
        virtual bool createDevice() override;
        virtual bool createDeviceUploading() override;

        std::unique_ptr<IContext> createContextInternal(const DisplayConfig& displayConfig, const Context_WGL::Config& contextConfig);

        static std::string GetVersionString(const Context_WGL::Config& config);

        WglExtensions m_wglExtensions;
        std::optional<Context_WGL::Config> m_contextConfig;
    };
}
