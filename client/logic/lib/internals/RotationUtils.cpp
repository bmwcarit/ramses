//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internals/RotationUtils.h"

#include <cassert>
#include <cmath>
#include <algorithm>

namespace rlogic::internal
{
    std::optional<ERotationType> RotationUtils::RamsesRotationConventionToRotationType(ramses::ERotationConvention convention)
    {
        switch (convention)
        {
        case ramses::ERotationConvention::Euler_ZYX:
            return ERotationType::Euler_ZYX;
        case ramses::ERotationConvention::Euler_YZX:
            return ERotationType::Euler_YZX;
        case ramses::ERotationConvention::Euler_ZXY:
            return ERotationType::Euler_ZXY;
        case ramses::ERotationConvention::Euler_XZY:
            return ERotationType::Euler_XZY;
        case ramses::ERotationConvention::Euler_YXZ:
            return ERotationType::Euler_YXZ;
        case ramses::ERotationConvention::Euler_XYZ:
            return ERotationType::Euler_XYZ;
            //
        case ramses::ERotationConvention::Euler_XYX:
        case ramses::ERotationConvention::Euler_XZX:
        case ramses::ERotationConvention::Euler_YXY:
        case ramses::ERotationConvention::Euler_YZY:
        case ramses::ERotationConvention::Euler_ZXZ:
        case ramses::ERotationConvention::Euler_ZYZ:
        case ramses::ERotationConvention::Quaternion:
            return std::nullopt;
        }

        return std::nullopt;
    }

    std::optional<ramses::ERotationConvention> RotationUtils::RotationTypeToRamsesRotationConvention(ERotationType rotationType)
    {
        switch (rotationType)
        {
        case ERotationType::Euler_ZYX:
            return ramses::ERotationConvention::Euler_ZYX;
        case ERotationType::Euler_YZX:
            return ramses::ERotationConvention::Euler_YZX;
        case ERotationType::Euler_ZXY:
            return ramses::ERotationConvention::Euler_ZXY;
        case ERotationType::Euler_XZY:
            return ramses::ERotationConvention::Euler_XZY;
        case ERotationType::Euler_YXZ:
            return ramses::ERotationConvention::Euler_YXZ;
        case ERotationType::Euler_XYZ:
            return ramses::ERotationConvention::Euler_XYZ;
        case ERotationType::Quaternion:
            // Ramses doesn't support native quaternions yet
            return std::nullopt;
        }

        return std::nullopt;
    }
}
