//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PLATFORMFACTORY_H
#define RAMSES_PLATFORMFACTORY_H

#include "RendererAPI/IPlatformFactory.h"

namespace ramses_internal
{
    class PlatformFactory : public IPlatformFactory
    {
    public:
        std::unique_ptr<IPlatform> createPlatform(const RendererConfig& rendererConfig, const DisplayConfig& displayConfig) override;
    };
}

#endif
