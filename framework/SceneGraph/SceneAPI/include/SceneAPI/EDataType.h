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
#include "Utils/Warnings.h"

namespace ramses_internal
{

WARNINGS_PUSH
WARNING_DISABLE_GCC(-Wshadow)

    enum class EDataType
    {
        Invalid = 0,

        Int32,
        UInt16,
        UInt32,
        Float,
        Vector2F,
        Vector3F,
        Vector4F,
        Vector2I,
        Vector3I,
        Vector4I,
        Matrix22F,
        Matrix33F,
        Matrix44F,

        // non-POD types
        DataReference,
        TextureSampler2D,
        TextureSampler2DMS,
        TextureSampler3D,
        TextureSamplerCube,

        Indices, // special type that is not strictly typed by effect and both 16/32bit integer can be used

        // these are special internal types used for effect attributes,
        // essentially the type carries 2 bits of information - element type and that it is a buffer,
        // see GlslToEffectConverter::replaceVertexAttributeWithBufferVariant
        UInt16Buffer,
        FloatBuffer,
        Vector2Buffer,
        Vector3Buffer,
        Vector4Buffer,

        ByteBlob,
        TextureSamplerExternal,

        NUMBER_OF_ELEMENTS // must be last, used for checking
    };

WARNINGS_POP

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
        "DATATYPE_TEXTURESAMPLER2D",
        "DATATYPE_TEXTURESAMPLER2DMS",
        "DATATYPE_TEXTURESAMPLER3D",
        "DATATYPE_TEXTURESAMPLERCUBE",
        "DATATYPE_INDICES",
        "DATATYPE_UINT16BUFFER",
        "DATATYPE_FLOATBUFFER",
        "DATATYPE_VECTOR2BUFFER",
        "DATATYPE_VECTOR3BUFFER",
        "DATATYPE_VECTOR4BUFFER",
        "DATATYPE_BYTE_BLOB",
        "DATATYPE_TEXTURESAMPLEREXTERNAL"
    };

    ENUM_TO_STRING(EDataType, DataTypeNames, EDataType::NUMBER_OF_ELEMENTS);

    inline constexpr UInt32 EnumToNumComponents(EDataType type)
    {
        switch (type)
        {
        case EDataType::Int32:
        case EDataType::UInt16:
        case EDataType::UInt32:
        case EDataType::Float:
        case EDataType::ByteBlob:
            return 1u;
        case EDataType::Vector2F:
        case EDataType::Vector2I:
            return 2u;
        case EDataType::Vector3F:
        case EDataType::Vector3I:
            return 3u;
        case EDataType::Vector4F:
        case EDataType::Vector4I:
            return 4u;
        case EDataType::Matrix22F:
            return 4u;
        case EDataType::Matrix33F:
            return 9u;
        case EDataType::Matrix44F:
            return 16u;
        default:
            assert(false);
            return 0;
        };
    }

    inline EDataType BufferTypeToElementType(EDataType bufferType)
    {
        switch (bufferType)
        {
        case EDataType::FloatBuffer:
            return EDataType::Float;
        case EDataType::Vector2Buffer:
            return EDataType::Vector2F;
        case EDataType::Vector3Buffer:
            return EDataType::Vector3F;
        case EDataType::Vector4Buffer:
            return EDataType::Vector4F;
        default:
            assert(false);
            return EDataType::Invalid;
        }
    }

    inline constexpr UInt32 EnumToSize(EDataType type)
    {
        switch (type)
        {
        case EDataType::Int32            : return sizeof(Int32);
        case EDataType::UInt16           : return sizeof(UInt16);
        case EDataType::UInt32           : return sizeof(UInt32);
        case EDataType::Float            : return sizeof(Float);
        case EDataType::Vector2F         : return sizeof(Float) * EnumToNumComponents(EDataType::Vector2F);
        case EDataType::Vector3F         : return sizeof(Float) * EnumToNumComponents(EDataType::Vector3F);
        case EDataType::Vector4F         : return sizeof(Float) * EnumToNumComponents(EDataType::Vector4F);
        case EDataType::Vector2I         : return sizeof(Int32) * EnumToNumComponents(EDataType::Vector2I);
        case EDataType::Vector3I         : return sizeof(Int32) * EnumToNumComponents(EDataType::Vector3I);
        case EDataType::Vector4I         : return sizeof(Int32) * EnumToNumComponents(EDataType::Vector4I);
        case EDataType::Matrix22F        : return sizeof(Float) * EnumToNumComponents(EDataType::Matrix22F);
        case EDataType::Matrix33F        : return sizeof(Float) * EnumToNumComponents(EDataType::Matrix33F);
        case EDataType::Matrix44F        : return sizeof(Float) * EnumToNumComponents(EDataType::Matrix44F);
        case EDataType::ByteBlob         : return 1u;

        case EDataType::DataReference    : return sizeof(DataInstanceHandle);
        case EDataType::TextureSampler2D : return sizeof(TextureSamplerHandle);
        case EDataType::TextureSampler2DMS : return sizeof(TextureSamplerHandle);
        case EDataType::TextureSampler3D : return sizeof(TextureSamplerHandle);
        case EDataType::TextureSamplerCube:return sizeof(TextureSamplerHandle);
        case EDataType::TextureSamplerExternal: return sizeof(TextureSamplerHandle);
        case EDataType::Indices          : return sizeof(ResourceField);
        case EDataType::UInt16Buffer     : return sizeof(ResourceField);
        case EDataType::FloatBuffer      : return sizeof(ResourceField);
        case EDataType::Vector2Buffer    : return sizeof(ResourceField);
        case EDataType::Vector3Buffer    : return sizeof(ResourceField);
        case EDataType::Vector4Buffer    : return sizeof(ResourceField);

        case EDataType::Invalid:
        case EDataType::NUMBER_OF_ELEMENTS:
            break;
        };

        assert(false);
        return 0;
    }

    inline UInt EnumToAlignment(EDataType type)
    {
        switch (type)
        {
        case EDataType::Int32            : return alignof(Int32);
        case EDataType::UInt16           : return alignof(UInt16);
        case EDataType::UInt32           : return alignof(UInt32);
        case EDataType::Float            : return alignof(Float);
        case EDataType::Vector2F         : return alignof(Float);
        case EDataType::Vector3F         : return alignof(Float);
        case EDataType::Vector4F         : return alignof(Float);
        case EDataType::Vector2I         : return alignof(Int32);
        case EDataType::Vector3I         : return alignof(Int32);
        case EDataType::Vector4I         : return alignof(Int32);
        case EDataType::Matrix22F        : return alignof(Float);
        case EDataType::Matrix33F        : return alignof(Float);
        case EDataType::Matrix44F        : return alignof(Float);

        case EDataType::DataReference    : return alignof(DataInstanceHandle);
        case EDataType::TextureSampler2D : return alignof(TextureSamplerHandle);
        case EDataType::TextureSampler2DMS : return alignof(TextureSamplerHandle);
        case EDataType::TextureSamplerExternal:return alignof(TextureSamplerHandle);
        case EDataType::TextureSampler3D : return alignof(TextureSamplerHandle);
        case EDataType::TextureSamplerCube:return alignof(TextureSamplerHandle);
        case EDataType::Indices          : return alignof(ResourceField);
        case EDataType::UInt16Buffer     : return alignof(ResourceField);
        case EDataType::FloatBuffer      : return alignof(ResourceField);
        case EDataType::Vector2Buffer    : return alignof(ResourceField);
        case EDataType::Vector3Buffer    : return alignof(ResourceField);
        case EDataType::Vector4Buffer    : return alignof(ResourceField);
        default:
            assert(false);
            return 0;
        };
    }

    inline static bool IsBufferDataType(EDataType dataType)
    {
        return dataType == EDataType::Indices
            || dataType == EDataType::UInt16Buffer
            || dataType == EDataType::FloatBuffer
            || dataType == EDataType::Vector2Buffer
            || dataType == EDataType::Vector3Buffer
            || dataType == EDataType::Vector4Buffer;
    }

    inline static bool IsTextureSamplerType(EDataType dataType)
    {
        return dataType == EDataType::TextureSampler2D
            || dataType == EDataType::TextureSampler2DMS
            || dataType == EDataType::TextureSampler3D
            || dataType == EDataType::TextureSamplerCube
            || dataType == EDataType::TextureSamplerExternal;
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
        static const EDataType DataType = EDataType::Int32;
    };

    template <>
    struct TypeToEDataTypeTraits < uint16_t >
    {
        static const EDataType DataType = EDataType::UInt16;
    };

    template <>
    struct TypeToEDataTypeTraits < uint32_t >
    {
        static const EDataType DataType = EDataType::UInt32;
    };

    template <>
    struct TypeToEDataTypeTraits < float >
    {
        static const EDataType DataType = EDataType::Float;
    };

    template <>
    struct TypeToEDataTypeTraits < Vector2i >
    {
        static const EDataType DataType = EDataType::Vector2I;
    };

    template <>
    struct TypeToEDataTypeTraits < Vector3i >
    {
        static const EDataType DataType = EDataType::Vector3I;
    };

    template <>
    struct TypeToEDataTypeTraits < Vector4i >
    {
        static const EDataType DataType = EDataType::Vector4I;
    };

    template <>
    struct TypeToEDataTypeTraits < Vector2 >
    {
        static const EDataType DataType = EDataType::Vector2F;
    };

    template <>
    struct TypeToEDataTypeTraits < Vector3 >
    {
        static const EDataType DataType = EDataType::Vector3F;
    };

    template <>
    struct TypeToEDataTypeTraits < Vector4 >
    {
        static const EDataType DataType = EDataType::Vector4F;
    };

    template <>
    struct TypeToEDataTypeTraits < Matrix22f >
    {
        static const EDataType DataType = EDataType::Matrix22F;
    };

    template <>
    struct TypeToEDataTypeTraits < Matrix33f >
    {
        static const EDataType DataType = EDataType::Matrix33F;
    };

    template <>
    struct TypeToEDataTypeTraits < Matrix44f >
    {
        static const EDataType DataType = EDataType::Matrix44F;
    };

    template <typename T>
    static bool TypeMatchesEDataType(EDataType dataType)
    {
        return dataType == TypeToEDataTypeTraits<T>::DataType;
    }

    template <>
    inline bool TypeMatchesEDataType<DataInstanceHandle>(EDataType dataType)
    {
        return dataType == EDataType::DataReference;
    }

    template <>
    inline bool TypeMatchesEDataType<TextureSamplerHandle>(EDataType dataType)
    {
        return IsTextureSamplerType(dataType);
    }

    template <>
    inline bool TypeMatchesEDataType<ResourceField>(EDataType dataType)
    {
        return IsBufferDataType(dataType);
    }
}

#endif
