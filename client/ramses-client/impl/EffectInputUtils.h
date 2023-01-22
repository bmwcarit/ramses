//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_EFFECTINPUTUTILS_H
#define RAMSES_EFFECTINPUTUTILS_H

#include "ramses-client-api/EffectInputDataType.h"
#include "SceneAPI/EDataType.h"
#include <cassert>

namespace ramses
{
    class EffectInputUtils
    {
    public:

        static EEffectInputDataType GetEffectInputDataType(ramses_internal::EDataType dataType)
        {
            switch (dataType)
            {
            case ramses_internal::EDataType::Int32              : return EEffectInputDataType_Int32;
            case ramses_internal::EDataType::UInt16             : return EEffectInputDataType_UInt16;
            case ramses_internal::EDataType::UInt32             : return EEffectInputDataType_UInt32;
            case ramses_internal::EDataType::Float              : return EEffectInputDataType_Float;
            case ramses_internal::EDataType::Vector2F           : return EEffectInputDataType_Vector2F;
            case ramses_internal::EDataType::Vector3F           : return EEffectInputDataType_Vector3F;
            case ramses_internal::EDataType::Vector4F           : return EEffectInputDataType_Vector4F;
            case ramses_internal::EDataType::Vector2I           : return EEffectInputDataType_Vector2I;
            case ramses_internal::EDataType::Vector3I           : return EEffectInputDataType_Vector3I;
            case ramses_internal::EDataType::Vector4I           : return EEffectInputDataType_Vector4I;
            case ramses_internal::EDataType::Matrix22F          : return EEffectInputDataType_Matrix22F;
            case ramses_internal::EDataType::Matrix33F          : return EEffectInputDataType_Matrix33F;
            case ramses_internal::EDataType::Matrix44F          : return EEffectInputDataType_Matrix44F;
            case ramses_internal::EDataType::TextureSampler2D   : return EEffectInputDataType_TextureSampler2D;
            case ramses_internal::EDataType::TextureSampler2DMS   : return EEffectInputDataType_TextureSampler2DMS;
            case ramses_internal::EDataType::TextureSamplerExternal : return EEffectInputDataType_TextureSamplerExternal;
            case ramses_internal::EDataType::TextureSampler3D   : return EEffectInputDataType_TextureSampler3D;
            case ramses_internal::EDataType::TextureSamplerCube : return EEffectInputDataType_TextureSamplerCube;
            case ramses_internal::EDataType::Invalid            : return EEffectInputDataType_Invalid;
            default:
                assert(false);
                return EEffectInputDataType_Invalid;
            }
        }

        static EEffectInputDataType GetEffectInputDataTypeFromBuffer(ramses_internal::EDataType dataType)
        {
            switch (dataType)
            {
            case ramses_internal::EDataType::UInt16Buffer               : return EEffectInputDataType_UInt16;
            case ramses_internal::EDataType::FloatBuffer                : return EEffectInputDataType_Float;
            case ramses_internal::EDataType::Vector2Buffer              : return EEffectInputDataType_Vector2F;
            case ramses_internal::EDataType::Vector3Buffer              : return EEffectInputDataType_Vector3F;
            case ramses_internal::EDataType::Vector4Buffer              : return EEffectInputDataType_Vector4F;
            case ramses_internal::EDataType::Invalid                    : return EEffectInputDataType_Invalid;
            default:
                assert(false);
                return EEffectInputDataType_Invalid;
            }
        }
    };
}

#endif
