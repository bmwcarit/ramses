//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses-client-api/ERotationType.h"
#include "SceneAPI/ERotationType.h"
#include <cassert>

namespace ramses
{
    namespace RotationTypeUtils
    {
        inline ERotationType ConvertRotationTypeFromInternal(ramses_internal::ERotationType rotationType)
        {
            switch (rotationType)
            {
            case ramses_internal::ERotationType::Euler_ZYX: return ERotationType::Euler_ZYX;
            case ramses_internal::ERotationType::Euler_YZX: return ERotationType::Euler_YZX;
            case ramses_internal::ERotationType::Euler_ZXY: return ERotationType::Euler_ZXY;
            case ramses_internal::ERotationType::Euler_XZY: return ERotationType::Euler_XZY;
            case ramses_internal::ERotationType::Euler_YXZ: return ERotationType::Euler_YXZ;
            case ramses_internal::ERotationType::Euler_XYZ: return ERotationType::Euler_XYZ;
            case ramses_internal::ERotationType::Euler_XYX: return ERotationType::Euler_XYX;
            case ramses_internal::ERotationType::Euler_XZX: return ERotationType::Euler_XZX;
            case ramses_internal::ERotationType::Euler_YXY: return ERotationType::Euler_YXY;
            case ramses_internal::ERotationType::Euler_YZY: return ERotationType::Euler_YZY;
            case ramses_internal::ERotationType::Euler_ZXZ: return ERotationType::Euler_ZXZ;
            case ramses_internal::ERotationType::Euler_ZYZ: return ERotationType::Euler_ZYZ;
            case ramses_internal::ERotationType::Quaternion: return ERotationType::Quaternion;
            }

            assert(false);
            return ERotationType::Euler_XYZ;
        }

        inline ramses_internal::ERotationType ConvertRotationTypeToInternal(ERotationType rotationType)
        {
            switch (rotationType)
            {
            case ERotationType::Euler_ZYX: return ramses_internal::ERotationType::Euler_ZYX;
            case ERotationType::Euler_YZX: return ramses_internal::ERotationType::Euler_YZX;
            case ERotationType::Euler_ZXY: return ramses_internal::ERotationType::Euler_ZXY;
            case ERotationType::Euler_XZY: return ramses_internal::ERotationType::Euler_XZY;
            case ERotationType::Euler_YXZ: return ramses_internal::ERotationType::Euler_YXZ;
            case ERotationType::Euler_XYZ: return ramses_internal::ERotationType::Euler_XYZ;
            case ERotationType::Euler_XYX: return ramses_internal::ERotationType::Euler_XYX;
            case ERotationType::Euler_XZX: return ramses_internal::ERotationType::Euler_XZX;
            case ERotationType::Euler_YXY: return ramses_internal::ERotationType::Euler_YXY;
            case ERotationType::Euler_YZY: return ramses_internal::ERotationType::Euler_YZY;
            case ERotationType::Euler_ZXZ: return ramses_internal::ERotationType::Euler_ZXZ;
            case ERotationType::Euler_ZYZ: return ramses_internal::ERotationType::Euler_ZYZ;
            case ERotationType::Quaternion: return ramses_internal::ERotationType::Quaternion;
            }

            assert(false);
            return ramses_internal::ERotationType::Euler_ZYZ;
        }
    };
}

