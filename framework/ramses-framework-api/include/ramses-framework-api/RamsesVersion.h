//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSESVERSION_H
#define RAMSES_RAMSESVERSION_H

#include "ramses-framework-api/APIExport.h"

namespace ramses
{
    /**
     * @brief Ramses version information
     */
    struct RamsesVersion
    {
        /// Version information as string in format major.minor.patch with an optional arbitrary suffix
        const char *const string;

        /// Major version
        int major;
        /// Minor version
        int minor;
        /// Patch version
        int patch;
    };

    /**
     * @brief Retrieve currently used ramses version information
     * @returns the ramses version of the currently used build
     */
    RAMSES_API RamsesVersion GetRamsesVersion();
}

#endif
