//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Core/Utils/LoggingUtils.h"
#include "ramses/framework/ERotationType.h"

#include <cstdint>

namespace ramses::internal
{
    using ramses::ERotationType;

    const std::array ERotationTypeNames = {
        "Euler_XYZ",
        "Euler_XZY",
        "Euler_YXZ",
        "Euler_YZX",
        "Euler_ZXY",
        "Euler_ZYX",
        "Euler_XYX",
        "Euler_XZX",
        "Euler_YXY",
        "Euler_YZY",
        "Euler_ZXZ",
        "Euler_ZYZ",
        "Quaternion",
    };
}

MAKE_ENUM_CLASS_PRINTABLE(ramses::ERotationType,
                                        "ERotationType",
                                        ramses::internal::ERotationTypeNames,
                                        ramses::ERotationType::Quaternion);

