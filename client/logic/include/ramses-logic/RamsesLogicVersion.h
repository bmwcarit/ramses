//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses-framework-api/APIExport.h"
#include <string_view>
#include <cstdint>

namespace rlogic
{
    /**
     * @brief Ramses Logic version information
     */
    struct RamsesLogicVersion
    {
        /// Version information as string in format major.minor.patch with an optional arbitrary suffix
        const std::string_view string;

        /// Major version
        uint32_t major;
        /// Minor version
        uint32_t minor;
        /// Patch version
        uint32_t patch;
    };

    /**
     * @brief Retrieve currently used Ramses Logic version information
     * @returns the Ramses Logic version of the currently used build
     */
    [[nodiscard]] RAMSES_API RamsesLogicVersion GetRamsesLogicVersion();
}
