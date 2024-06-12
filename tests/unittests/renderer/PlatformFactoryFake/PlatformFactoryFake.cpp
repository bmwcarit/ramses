//  -------------------------------------------------------------------------
//  Copyright (C) 2024 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Platform/PlatformFactory.h"
#include <cassert>

namespace ramses::internal
{
    // This is a fake implementation of PlatformFactory, that basically does nothing, but allows
    // the tests to link and build without error
    // It is needed in order to allow unit testing of ramses-renderer
    // without dependency on Platform
    std::unique_ptr<IPlatform> PlatformFactory::createPlatform([[maybe_unused]] const RendererConfigData& rendererConfig, [[maybe_unused]] const DisplayConfigData& displayConfig)
    {
        assert(false);
        return {};
    }
}
