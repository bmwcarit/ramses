//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENEAPI_EDATATYPE_H
#define RAMSES_SCENEAPI_EDATATYPE_H

#include "PlatformAbstraction/PlatformTypes.h"
#include "SceneAPI/ResourceField.h"
#include "SceneAPI/Handles.h"
#include "Utils/LoggingUtils.h"

namespace ramses_internal
{
    enum EDataType
    {
        EDataType_Invalid = 0,

        EDataType_Int32,
        EDataType_UInt16,
        EDataType_UInt32,
        EDataType_Float,
        EDataType_Vector2F,
        EDataType_Vector3F,
        EDataType_Vector4F,
        EDataType_Vector2I,
        EDataType_Vector3I,
        EDataType_Vector4I,
        EDataType_Matrix22F,
        EDataType_Matrix33F,
        EDataType_Matrix44F,

        // non-POD types
        EDataType_DataReference,
        EDataType_TextureSampler,
        EDataType_Indices,        // special type that is not strictly typed by effect and both 16/32bit integer can be used
        EDataType_UInt16Buffer,
        EDataType_FloatBuffer,
        EDataType_Vector2Buffer,
        EDataType_Vector3Buffer,
        EDataType_Vector4Buffer,

        EDataType_NUMBER_OF_ELEMENTS // must be last, used for checking
    };

    static const char* DataTypeNames[] =
    {
        "DATATYPE_INVALID",
        "DATATYPE_INT32",
        "DATATYPE_UINT16",
        "DATATYPE_UINT32",
        "DATATYPE_FLOAT",
        "DATATYPE_VECTOR2F",
        "DATATYPE_VECTOR3F",
        "DATATYPE_VECTOR4F",
        "DATATYPE_VECTOR2I",
        "DATATYPE_VECTOR3I",
        "DATATYPE_VECTOR4I",
        "DATATYPE_MATRIX22F",
        "DATATYPE_MATRIX33F",
        "DATATYPE_MATRIX44F",
        "DATATYPE_DATAREFERENCE",
        "DATATYPE_TEXTURESAMPLER",
        "DATATYPE_INDICES",
        "DATATYPE_UINT16BUFFER",
        "DATATYPE_FLOATBUFFER",
        "DATATYPE_VECTOR2BUFFER",
        "DATATYPE_VECTOR3BUFFER",
        "DATATYPE_VECTOR4BUFFER"
    };

    ENUM_TO_STRING(EDataType, DataTypeNames, EDataType_NUMBER_OF_ELEMENTS);

    inline UInt32 EnumToNumComponents(EDataType type)
    {
        switch (type)
        {
        case EDataType_Int32:
        case EDataType_UInt16:
        case EDataType_UInt32:
        case EDataType_Float:
            return 1u;
        case EDataType_Vector2F:
        case EDataType_Vector2I:
            return 2u;
        case EDataType_Vector3F:
        case EDataType_Vector3I:
            return 3u;
        case EDataType_Vector4F:
        case EDataType_Vector4I:
            return 4u;
        case EDataType_Matrix22F:
            return 4u;
        case EDataType_Matrix33F:
            return 9u;
        case EDataType_Matrix44F:
            return 16u;
        default:
            assert(false);
            return 0;
        };
    }

    inline UInt32 EnumToSize(EDataType type)
    {
        switch (type)
        {
        case EDataType_Int32            : return sizeof(Int32);
        case EDataType_UInt16           : return sizeof(UInt16);
        case EDataType_UInt32           : return sizeof(UInt32);
        case EDataType_Float            : return sizeof(Float);
        case EDataType_Vector2F         : return sizeof(Float) * EnumToNumComponents(EDataType_Vector2F);
        case EDataType_Vector3F         : return sizeof(Float) * EnumToNumComponents(EDataType_Vector3F);
        case EDataType_Vector4F         : return sizeof(Float) * EnumToNumComponents(EDataType_Vector4F);
        case EDataType_Vector2I         : return sizeof(Int32) * EnumToNumComponents(EDataType_Vector2I);
        case EDataType_Vector3I         : return sizeof(Int32) * EnumToNumComponents(EDataType_Vector3I);
        case EDataType_Vector4I         : return sizeof(Int32) * EnumToNumComponents(EDataType_Vector4I);
        case EDataType_Matrix22F        : return sizeof(Float) * EnumToNumComponents(EDataType_Matrix22F);
        case EDataType_Matrix33F        : return sizeof(Float) * EnumToNumComponents(EDataType_Matrix33F);
        case EDataType_Matrix44F        : return sizeof(Float) * EnumToNumComponents(EDataType_Matrix44F);

        case EDataType_DataReference    : return sizeof(DataInstanceHandle);
        case EDataType_TextureSampler   : return sizeof(TextureSamplerHandle);
        case EDataType_Indices          : return sizeof(ResourceField);
        case EDataType_UInt16Buffer     : return sizeof(ResourceField);
        case EDataType_FloatBuffer      : return sizeof(ResourceField);
        case EDataType_Vector2Buffer    : return sizeof(ResourceField);
        case EDataType_Vector3Buffer    : return sizeof(ResourceField);
        case EDataType_Vector4Buffer    : return sizeof(ResourceField);
        default:
            assert(false);
            return 0;
        };
    }

    inline UInt EnumToAlignment(EDataType type)
    {
        switch (type)
        {
        case EDataType_Int32            : return alignof(Int32);
        case EDataType_UInt16           : return alignof(UInt16);
        case EDataType_UInt32           : return alignof(UInt32);
        case EDataType_Float            : return alignof(Float);
        case EDataType_Vector2F         : return alignof(Float);
        case EDataType_Vector3F         : return alignof(Float);
        case EDataType_Vector4F         : return alignof(Float);
        case EDataType_Vector2I         : return alignof(Int32);
        case EDataType_Vector3I         : return alignof(Int32);
        case EDataType_Vector4I         : return alignof(Int32);
        case EDataType_Matrix22F        : return alignof(Float);
        case EDataType_Matrix33F        : return alignof(Float);
        case EDataType_Matrix44F        : return alignof(Float);

        case EDataType_DataReference    : return alignof(DataInstanceHandle);
        case EDataType_TextureSampler   : return alignof(TextureSamplerHandle);
        case EDataType_Indices          : return alignof(ResourceField);
        case EDataType_UInt16Buffer     : return alignof(ResourceField);
        case EDataType_FloatBuffer      : return alignof(ResourceField);
        case EDataType_Vector2Buffer    : return alignof(ResourceField);
        case EDataType_Vector3Buffer    : return alignof(ResourceField);
        case EDataType_Vector4Buffer    : return alignof(ResourceField);
        default:
            assert(false);
            return 0;
        };
    }

    inline static Bool IsBufferDataType(EDataType dataType)
    {
        return dataType == EDataType_Indices
            || dataType == EDataType_UInt16Buffer
            || dataType == EDataType_FloatBuffer
            || dataType == EDataType_Vector2Buffer
            || dataType == EDataType_Vector3Buffer
            || dataType == EDataType_Vector4Buffer;
    }

    class Vector2i;
    class Vector3i;
    class Vector4i;
    class Vector2;
    class Vector3;
    class Vector4;
    class Matrix22f;
    class Matrix33f;
    class Matrix44f;

    template <typename T>
    struct TypeToEDataTypeTraits
    {
    };

    template <>
    struct TypeToEDataTypeTraits < int32_t >
    {
        static const EDataType DataType = EDataType_Int32;
    };

    template <>
    struct TypeToEDataTypeTraits < uint16_t >
    {
        static const EDataType DataType = EDataType_UInt16;
    };

    template <>
    struct TypeToEDataTypeTraits < uint32_t >
    {
        static const EDataType DataType = EDataType_UInt32;
    };

    template <>
    struct TypeToEDataTypeTraits < float >
    {
        static const EDataType DataType = EDataType_Float;
    };

    template <>
    struct TypeToEDataTypeTraits < Vector2i >
    {
        static const EDataType DataType = EDataType_Vector2I;
    };

    template <>
    struct TypeToEDataTypeTraits < Vector3i >
    {
        static const EDataType DataType = EDataType_Vector3I;
    };

    template <>
    struct TypeToEDataTypeTraits < Vector4i >
    {
        static const EDataType DataType = EDataType_Vector4I;
    };

    template <>
    struct TypeToEDataTypeTraits < Vector2 >
    {
        static const EDataType DataType = EDataType_Vector2F;
    };

    template <>
    struct TypeToEDataTypeTraits < Vector3 >
    {
        static const EDataType DataType = EDataType_Vector3F;
    };

    template <>
    struct TypeToEDataTypeTraits < Vector4 >
    {
        static const EDataType DataType = EDataType_Vector4F;
    };

    template <>
    struct TypeToEDataTypeTraits < Matrix22f >
    {
        static const EDataType DataType = EDataType_Matrix22F;
    };

    template <>
    struct TypeToEDataTypeTraits < Matrix33f >
    {
        static const EDataType DataType = EDataType_Matrix33F;
    };

    template <>
    struct TypeToEDataTypeTraits < Matrix44f >
    {
        static const EDataType DataType = EDataType_Matrix44F;
    };
}

#endif
