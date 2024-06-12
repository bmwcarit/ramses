//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/PlatformInterface/IPlatform.h"
#include <memory>

namespace ramses::internal
{
    class RendererConfigData;
    class DisplayConfigData;

    class IPlatformFactory
    {
    public:
        virtual ~IPlatformFactory() = default;
        virtual std::unique_ptr<IPlatform> createPlatform(const RendererConfigData& rendererConfig, const DisplayConfigData& displayConfig) = 0;
    };
}
