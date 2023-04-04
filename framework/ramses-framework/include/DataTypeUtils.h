//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_FRAMEWORK_DATATYPEUTILS_H
#define RAMSES_FRAMEWORK_DATATYPEUTILS_H

#include "ramses-framework-api/EDataType.h"
#include "SceneAPI/EDataType.h"
#include "SceneAPI/EDataBufferType.h"
#include <cassert>

namespace ramses
{
    class DataTypeUtils
    {
    public:
        static EDataType ConvertDataTypeFromInternal(ramses_internal::EDataType type)
        {
            switch (type)
            {
            case ramses_internal::EDataType::Int32:
                return EDataType::Int32;
            case ramses_internal::EDataType::UInt16:
                return EDataType::UInt16;
            case ramses_internal::EDataType::UInt32:
                return EDataType::UInt32;
            case ramses_internal::EDataType::Float:
                return EDataType::Float;
            case ramses_internal::EDataType::Vector2F:
                return EDataType::Vector2F;
            case ramses_internal::EDataType::Vector3F:
                return EDataType::Vector3F;
            case ramses_internal::EDataType::Vector4F:
                return EDataType::Vector4F;
            case ramses_internal::EDataType::Vector2I:
                return EDataType::Vector2I;
            case ramses_internal::EDataType::Vector3I:
                return EDataType::Vector3I;
            case ramses_internal::EDataType::Vector4I:
                return EDataType::Vector4I;
            case ramses_internal::EDataType::Matrix22F:
                return EDataType::Matrix22F;
            case ramses_internal::EDataType::Matrix33F:
                return EDataType::Matrix33F;
            case ramses_internal::EDataType::Matrix44F:
                return EDataType::Matrix44F;
            case ramses_internal::EDataType::ByteBlob:
                return EDataType::ByteBlob;

            default:
                assert(false);
                return EDataType::ByteBlob;
            }
        }

        static constexpr ramses_internal::EDataType ConvertDataTypeToInternal(EDataType type)
        {
            switch (type)
            {
            case EDataType::Int32:
                return ramses_internal::EDataType::Int32;
            case EDataType::UInt16:
                return ramses_internal::EDataType::UInt16;
            case EDataType::UInt32:
                return ramses_internal::EDataType::UInt32;
            case EDataType::Float:
                return ramses_internal::EDataType::Float;
            case EDataType::Vector2F:
                return ramses_internal::EDataType::Vector2F;
            case EDataType::Vector3F:
                return ramses_internal::EDataType::Vector3F;
            case EDataType::Vector4F:
                return ramses_internal::EDataType::Vector4F;
            case EDataType::Vector2I:
                return ramses_internal::EDataType::Vector2I;
            case EDataType::Vector3I:
                return ramses_internal::EDataType::Vector3I;
            case EDataType::Vector4I:
                return ramses_internal::EDataType::Vector4I;
            case EDataType::Matrix22F:
                return ramses_internal::EDataType::Matrix22F;
            case EDataType::Matrix33F:
                return ramses_internal::EDataType::Matrix33F;
            case EDataType::Matrix44F:
                return ramses_internal::EDataType::Matrix44F;
            case EDataType::ByteBlob:
                return ramses_internal::EDataType::ByteBlob;
            }

            assert(false);
            return ramses_internal::EDataType::Invalid;
        }

        static ramses_internal::EResourceType DeductResourceTypeFromDataType(EDataType type)
        {
            if (IsValidIndicesType(type))
                return ramses_internal::EResourceType_IndexArray;
            else if (IsValidVerticesType(type))
                return ramses_internal::EResourceType_VertexArray;

            assert(false);
            return ramses_internal::EResourceType_Invalid;
        }

        static ramses_internal::EDataBufferType DeductBufferTypeFromDataType(EDataType type)
        {
            if (IsValidIndicesType(type))
                return ramses_internal::EDataBufferType::IndexBuffer;
            else if (IsValidVerticesType(type))
                return ramses_internal::EDataBufferType::VertexBuffer;

            assert(false);
            return ramses_internal::EDataBufferType::Invalid;
        }

        static bool IsValidIndicesType(EDataType type)
        {
            return
                type == EDataType::UInt16 ||
                type == EDataType::UInt32;
        }

        static bool IsValidVerticesType(EDataType type)
        {
            return
                type == EDataType::Float ||
                type == EDataType::Vector2F ||
                type == EDataType::Vector3F ||
                type == EDataType::Vector4F ||
                type == EDataType::ByteBlob;
        }
    };
}

#endif
