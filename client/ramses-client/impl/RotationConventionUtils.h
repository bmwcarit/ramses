//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CLIENT_ROTATIONCONVENTIONUTILS_H
#define RAMSES_CLIENT_ROTATIONCONVENTIONUTILS_H

#include "ramses-client-api/ERotationConvention.h"
#include "SceneAPI/ERotationConvention.h"
#include <cassert>

namespace ramses
{
    namespace RotationConventionUtils
    {
        inline ERotationConvention ConvertDataTypeFromInternal(ramses_internal::ERotationConvention convention)
        {
            switch (convention)
            {
            case ramses_internal::ERotationConvention::Euler_ZYX: return ERotationConvention::Euler_ZYX;
            case ramses_internal::ERotationConvention::Euler_YZX: return ERotationConvention::Euler_YZX;
            case ramses_internal::ERotationConvention::Euler_ZXY: return ERotationConvention::Euler_ZXY;
            case ramses_internal::ERotationConvention::Euler_XZY: return ERotationConvention::Euler_XZY;
            case ramses_internal::ERotationConvention::Euler_YXZ: return ERotationConvention::Euler_YXZ;
            case ramses_internal::ERotationConvention::Euler_XYZ: return ERotationConvention::Euler_XYZ;
            case ramses_internal::ERotationConvention::Euler_XYX: return ERotationConvention::Euler_XYX;
            case ramses_internal::ERotationConvention::Euler_XZX: return ERotationConvention::Euler_XZX;
            case ramses_internal::ERotationConvention::Euler_YXY: return ERotationConvention::Euler_YXY;
            case ramses_internal::ERotationConvention::Euler_YZY: return ERotationConvention::Euler_YZY;
            case ramses_internal::ERotationConvention::Euler_ZXZ: return ERotationConvention::Euler_ZXZ;
            case ramses_internal::ERotationConvention::Euler_ZYZ: return ERotationConvention::Euler_ZYZ;
            case ramses_internal::ERotationConvention::Quaternion: return ERotationConvention::Quaternion;
            }

            assert(false);
            return ERotationConvention::Euler_XYZ;
        }

        inline ramses_internal::ERotationConvention ConvertRotationConventionToInternal(ERotationConvention convention)
        {
            switch (convention)
            {
            case ERotationConvention::Euler_ZYX: return ramses_internal::ERotationConvention::Euler_ZYX;
            case ERotationConvention::Euler_YZX: return ramses_internal::ERotationConvention::Euler_YZX;
            case ERotationConvention::Euler_ZXY: return ramses_internal::ERotationConvention::Euler_ZXY;
            case ERotationConvention::Euler_XZY: return ramses_internal::ERotationConvention::Euler_XZY;
            case ERotationConvention::Euler_YXZ: return ramses_internal::ERotationConvention::Euler_YXZ;
            case ERotationConvention::Euler_XYZ: return ramses_internal::ERotationConvention::Euler_XYZ;
            case ERotationConvention::Euler_XYX: return ramses_internal::ERotationConvention::Euler_XYX;
            case ERotationConvention::Euler_XZX: return ramses_internal::ERotationConvention::Euler_XZX;
            case ERotationConvention::Euler_YXY: return ramses_internal::ERotationConvention::Euler_YXY;
            case ERotationConvention::Euler_YZY: return ramses_internal::ERotationConvention::Euler_YZY;
            case ERotationConvention::Euler_ZXZ: return ramses_internal::ERotationConvention::Euler_ZXZ;
            case ERotationConvention::Euler_ZYZ: return ramses_internal::ERotationConvention::Euler_ZYZ;
            case ERotationConvention::Quaternion: return ramses_internal::ERotationConvention::Quaternion;
            }

            assert(false);
            return ramses_internal::ERotationConvention::Euler_ZYZ;
        }
    };
}

#endif
