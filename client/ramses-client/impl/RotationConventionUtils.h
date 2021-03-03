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
            assert(convention != ramses_internal::ERotationConvention::Legacy_ZYX);

            switch (convention)
            {
            case ramses_internal::ERotationConvention::XYZ: return ERotationConvention::XYZ;
            case ramses_internal::ERotationConvention::XZY: return ERotationConvention::XZY;
            case ramses_internal::ERotationConvention::YXZ: return ERotationConvention::YXZ;
            case ramses_internal::ERotationConvention::YZX: return ERotationConvention::YZX;
            case ramses_internal::ERotationConvention::ZXY: return ERotationConvention::ZXY;
            case ramses_internal::ERotationConvention::ZYX: return ERotationConvention::ZYX;
            case ramses_internal::ERotationConvention::XYX: return ERotationConvention::XYX;
            case ramses_internal::ERotationConvention::XZX: return ERotationConvention::XZX;
            case ramses_internal::ERotationConvention::YXY: return ERotationConvention::YXY;
            case ramses_internal::ERotationConvention::YZY: return ERotationConvention::YZY;
            case ramses_internal::ERotationConvention::ZXZ: return ERotationConvention::ZXZ;
            case ramses_internal::ERotationConvention::ZYZ: return ERotationConvention::ZYZ;
            default:
                assert(false);
                return ERotationConvention::ZYZ;
            }
        }

        inline ramses_internal::ERotationConvention ConvertRotationConventionToInternal(ERotationConvention convention)
        {
            switch (convention)
            {
            case ERotationConvention::XYZ: return ramses_internal::ERotationConvention::XYZ;
            case ERotationConvention::XZY: return ramses_internal::ERotationConvention::XZY;
            case ERotationConvention::YXZ: return ramses_internal::ERotationConvention::YXZ;
            case ERotationConvention::YZX: return ramses_internal::ERotationConvention::YZX;
            case ERotationConvention::ZXY: return ramses_internal::ERotationConvention::ZXY;
            case ERotationConvention::ZYX: return ramses_internal::ERotationConvention::ZYX;
            case ERotationConvention::XYX: return ramses_internal::ERotationConvention::XYX;
            case ERotationConvention::XZX: return ramses_internal::ERotationConvention::XZX;
            case ERotationConvention::YXY: return ramses_internal::ERotationConvention::YXY;
            case ERotationConvention::YZY: return ramses_internal::ERotationConvention::YZY;
            case ERotationConvention::ZXZ: return ramses_internal::ERotationConvention::ZXZ;
            case ERotationConvention::ZYZ: return ramses_internal::ERotationConvention::ZYZ;
            }

            assert(false);
            return ramses_internal::ERotationConvention::ZYZ;
        }
    };
}

#endif
