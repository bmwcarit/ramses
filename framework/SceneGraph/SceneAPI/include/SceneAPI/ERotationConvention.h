//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENEAPI_EROTATIONCONVENTION_H
#define RAMSES_SCENEAPI_EROTATIONCONVENTION_H

#include "Utils/LoggingUtils.h"
#include "PlatformAbstraction/PlatformTypes.h"

namespace ramses_internal
{
    enum class ERotationConvention : uint8_t
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

    static constexpr const char* const ERotationConventionNames[] = {
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

MAKE_ENUM_CLASS_PRINTABLE_NO_EXTRA_LAST(ramses_internal::ERotationConvention,
                                        "ERotationConvention",
                                        ramses_internal::ERotationConventionNames,
                                        ramses_internal::ERotationConvention::Quaternion);

#endif
