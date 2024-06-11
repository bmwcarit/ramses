//  -------------------------------------------------------------------------
//  Copyright (C) 2024 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include <vector>

namespace ramses::internal
{
    enum class EVulkanAPIVersion : uint8_t
    {
        Invalid,
        Version_1_0,
        Version_1_1,
        Version_1_2,
        Version_1_3,
    };

    enum class ESPIRVVersion : uint8_t
    {
        Invalid,
        Version_1_0,
        Version_1_1,
        Version_1_2,
        Version_1_3,
        Version_1_4,
        Version_1_5,
        Version_1_6,
    };

    static constexpr EVulkanAPIVersion TargetVulkanApiVersion = EVulkanAPIVersion::Version_1_0;
    static constexpr ESPIRVVersion TargetSPIRVVersion = ESPIRVVersion::Version_1_0;
}
