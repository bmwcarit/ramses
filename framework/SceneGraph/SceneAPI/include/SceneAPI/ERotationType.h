//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "Utils/LoggingUtils.h"
#include "PlatformAbstraction/PlatformTypes.h"

namespace ramses_internal
{
    enum class ERotationType : uint8_t
    {
        Euler_XYZ,
        Euler_XZY,
        Euler_YXZ,
        Euler_YZX,
        Euler_ZXY,
        Euler_ZYX,
        Euler_XYX,
        Euler_XZX,
        Euler_YXY,
        Euler_YZY,
        Euler_ZXZ,
        Euler_ZYZ,
        Quaternion,
    };

    static constexpr const char* const ERotationTypeNames[] = {
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

MAKE_ENUM_CLASS_PRINTABLE_NO_EXTRA_LAST(ramses_internal::ERotationType,
                                        "ERotationType",
                                        ramses_internal::ERotationTypeNames,
                                        ramses_internal::ERotationType::Quaternion);

