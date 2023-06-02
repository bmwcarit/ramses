//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-framework-api/RamsesVersion.h"
#include "ramses-sdk-build-config.h"

namespace ramses
{
    RamsesVersion GetRamsesVersion()
    {
        return RamsesVersion{
            ramses_sdk::RAMSES_SDK_RAMSES_VERSION,
            ramses_sdk::RAMSES_SDK_PROJECT_VERSION_MAJOR_INT,
            ramses_sdk::RAMSES_SDK_PROJECT_VERSION_MINOR_INT,
            ramses_sdk::RAMSES_SDK_PROJECT_VERSION_PATCH_INT
        };
    }
}
