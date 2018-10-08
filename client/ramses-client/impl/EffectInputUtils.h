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
#include <assert.h>

namespace ramses
{
    class EffectInputUtils
    {
    public:
        static ramses_internal::EDataType GetDataTypeInternal(EEffectInputDataType dataType)
        {
            switch (dataType)
            {
            case EEffectInputDataType_Int32          : return ramses_internal::EDataType_Int32;
            case EEffectInputDataType_UInt16         : return ramses_internal::EDataType_UInt16;
            case EEffectInputDataType_UInt32         : return ramses_internal::EDataType_UInt32;
            case EEffectInputDataType_Float          : return ramses_internal::EDataType_Float;
            case EEffectInputDataType_Vector2F       : return ramses_internal::EDataType_Vector2F;
            case EEffectInputDataType_Vector3F       : return ramses_internal::EDataType_Vector3F;
            case EEffectInputDataType_Vector4F       : return ramses_internal::EDataType_Vector4F;
            case EEffectInputDataType_Vector2I       : return ramses_internal::EDataType_Vector2I;
            case EEffectInputDataType_Vector3I       : return ramses_internal::EDataType_Vector3I;
            case EEffectInputDataType_Vector4I       : return ramses_internal::EDataType_Vector4I;
            case EEffectInputDataType_Matrix22F      : return ramses_internal::EDataType_Matrix22F;
            case EEffectInputDataType_Matrix33F      : return ramses_internal::EDataType_Matrix33F;
            case EEffectInputDataType_Matrix44F      : return ramses_internal::EDataType_Matrix44F;
            case EEffectInputDataType_TextureSampler : return ramses_internal::EDataType_TextureSampler;
            default:
                assert(false);
                return ramses_internal::EDataType_Invalid;
            }
        }

        static EEffectInputDataType GetEffectInputDataType(ramses_internal::EDataType dataType)
        {
            switch (dataType)
            {
            case ramses_internal::EDataType_Int32              : return EEffectInputDataType_Int32;
            case ramses_internal::EDataType_UInt16             : return EEffectInputDataType_UInt16;
            case ramses_internal::EDataType_UInt32             : return EEffectInputDataType_UInt32;
            case ramses_internal::EDataType_Float              : return EEffectInputDataType_Float;
            case ramses_internal::EDataType_Vector2F           : return EEffectInputDataType_Vector2F;
            case ramses_internal::EDataType_Vector3F           : return EEffectInputDataType_Vector3F;
            case ramses_internal::EDataType_Vector4F           : return EEffectInputDataType_Vector4F;
            case ramses_internal::EDataType_Vector2I           : return EEffectInputDataType_Vector2I;
            case ramses_internal::EDataType_Vector3I           : return EEffectInputDataType_Vector3I;
            case ramses_internal::EDataType_Vector4I           : return EEffectInputDataType_Vector4I;
            case ramses_internal::EDataType_Matrix22F          : return EEffectInputDataType_Matrix22F;
            case ramses_internal::EDataType_Matrix33F          : return EEffectInputDataType_Matrix33F;
            case ramses_internal::EDataType_Matrix44F          : return EEffectInputDataType_Matrix44F;
            case ramses_internal::EDataType_TextureSampler     : return EEffectInputDataType_TextureSampler;
            case ramses_internal::EDataType_Invalid            : return EEffectInputDataType_Invalid;
            default:
                assert(false);
                return EEffectInputDataType_Invalid;
            }
        }

        static EEffectInputDataType GetEffectInputDataTypeFromBuffer(ramses_internal::EDataType dataType)
        {
            switch (dataType)
            {
            case ramses_internal::EDataType_UInt16Buffer               : return EEffectInputDataType_UInt16;
            case ramses_internal::EDataType_FloatBuffer                : return EEffectInputDataType_Float;
            case ramses_internal::EDataType_Vector2Buffer              : return EEffectInputDataType_Vector2F;
            case ramses_internal::EDataType_Vector3Buffer              : return EEffectInputDataType_Vector3F;
            case ramses_internal::EDataType_Vector4Buffer              : return EEffectInputDataType_Vector4F;
            case ramses_internal::EDataType_Invalid                    : return EEffectInputDataType_Invalid;
            default:
                assert(false);
                return EEffectInputDataType_Invalid;
            }
        }
    };
}

#endif
