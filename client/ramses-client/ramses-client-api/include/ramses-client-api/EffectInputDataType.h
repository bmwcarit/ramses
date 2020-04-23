//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_EFFECTINPUTDATATYPE_H
#define RAMSES_EFFECTINPUTDATATYPE_H

namespace ramses
{
    /**
    * @brief Data type of effect input
    */
    enum EEffectInputDataType
    {
        EEffectInputDataType_Invalid = 0,    ///< Invalid or Unknown data type
        EEffectInputDataType_Int32,          ///< Integer 32bit data type
        EEffectInputDataType_UInt16,         ///< Unsigned integer 16bit data type
        EEffectInputDataType_UInt32,         ///< Unsigned integer 32bit data type
        EEffectInputDataType_Float,          ///< Float data type
        EEffectInputDataType_Vector2F,       ///< Vector2 float data type
        EEffectInputDataType_Vector3F,       ///< Vector3 float data type
        EEffectInputDataType_Vector4F,       ///< Vector4 float data type
        EEffectInputDataType_Vector2I,       ///< Vector2 integer data type
        EEffectInputDataType_Vector3I,       ///< Vector3 integer data type
        EEffectInputDataType_Vector4I,       ///< Vector4 integer data type
        EEffectInputDataType_Matrix22F,      ///< Matrix2x2 data type
        EEffectInputDataType_Matrix33F,      ///< Matrix3x3 data type
        EEffectInputDataType_Matrix44F,      ///< Matrix4x4 data type
        EEffectInputDataType_TextureSampler  ///< Texture sampler data type
    };
}

#endif
