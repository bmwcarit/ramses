//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/PlatformInterface/IPlatformFactory.h"

namespace ramses::internal
{
    class PlatformFactory : public IPlatformFactory
    {
    public:
        std::unique_ptr<IPlatform> createPlatform(const RendererConfigData& rendererConfig, const DisplayConfigData& displayConfig) override;

    private:
        static std::unique_ptr<IPlatform> CreatePlatformWithOpenGL(const RendererConfigData& rendererConfig, const DisplayConfigData& displayConfig);
        static std::unique_ptr<IPlatform> CreatePlatformWithVulkan(const RendererConfigData& rendererConfig, const DisplayConfigData& displayConfig);
    };
}
