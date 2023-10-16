//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "impl/SceneImpl.h"
#include "internal/PlatformAbstraction/Collections/Vector.h"

#include <string>

namespace ramses::internal
{
    struct MemoryInfo
    {
        uint32_t memoryUsage{0};
        std::string logInfoMesage{};
    };
    using MemoryInfoVector = std::vector<MemoryInfo>;

    MemoryInfoVector GetMemoryInfoFromScene(const ramses::internal::SceneImpl& scene);
}
