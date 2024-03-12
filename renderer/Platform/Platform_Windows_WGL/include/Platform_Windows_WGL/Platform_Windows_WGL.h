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
#include "absl/types/optional.h"
#include "RendererAPI/EDeviceTypeId.h"

namespace ramses_internal
{
    class Platform_Windows_WGL final : public Platform_Base
    {
    public:
        explicit Platform_Windows_WGL(const RendererConfig& rendererConfig, EDeviceTypeId deviceType);

    protected:
        virtual bool createWindow(const DisplayConfig& displayConfig, IWindowEventHandler& windowEventHandler) override;
        virtual bool createContext(const DisplayConfig& displayConfig) override;
        virtual bool createContextUploading() override;

        virtual bool createDevice() override;
        virtual bool createDeviceUploading() override;

        WglExtensions m_wglExtensions;

    private:
        struct ContextConfig
        {
            UInt8              majorVersion  = 4;
            std::vector<UInt8> minorVersions = {2};
            bool               embedded      = false;
        };

        std::unique_ptr<IContext> createContextInternal(const DisplayConfig& displayConfig, UInt8 minorVersion);

        std::vector<Int32> getContextAttributes(Int32 minorVersion);

        std::string getMajorVersionString() const;

        ContextConfig         m_contextConfig;
        absl::optional<UInt8> m_contextMinorVersion;
    };
}

#endif
