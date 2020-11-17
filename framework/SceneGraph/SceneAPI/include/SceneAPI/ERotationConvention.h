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
        Legacy_ZYX, //deprecated left-handed rotation using ZYX convention, all other conventions use right-handed rotation
        XYZ,
        XZY,
        YXZ,
        YZX,
        ZXY,
        ZYX,
        XYX,
        XZX,
        YXY,
        YZY,
        ZXZ,
        ZYZ,
    };

    static constexpr const char* const ERotationConventionNames[] = {
        "LEGACY_EULER_ROTATION_ZYX",
        "EULER_ROTATION_XYZ",
        "EULER_ROTATION_XZY",
        "EULER_ROTATION_YXZ",
        "EULER_ROTATION_YZX",
        "EULER_ROTATION_ZXY",
        "EULER_ROTATION_ZYX",
        "EULER_ROTATION_XYX",
        "EULER_ROTATION_XZX",
        "EULER_ROTATION_YXY",
        "EULER_ROTATION_YZY",
        "EULER_ROTATION_ZXZ",
        "EULER_ROTATION_ZYZ",
    };
}

MAKE_ENUM_CLASS_PRINTABLE_NO_EXTRA_LAST(ramses_internal::ERotationConvention,
                                        ramses_internal::ERotationConventionNames,
                                        ramses_internal::ERotationConvention::ZYZ);

#endif
