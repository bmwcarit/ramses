//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CLIENT_DATATYPEUTILS_H
#define RAMSES_CLIENT_DATATYPEUTILS_H

#include "ramses-client-api/EDataType.h"
#include "SceneAPI/EDataType.h"
#include "SceneAPI/EDataBufferType.h"
#include <assert.h>

namespace ramses
{
    class DataTypeUtils
    {
    public:
        static EDataType ConvertDataTypeFromInternal(ramses_internal::EDataType type)
        {
            switch (type)
            {
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
            case ramses_internal::EDataType::ByteBlob:
                return EDataType::ByteBlob;
            default:
                assert(false);
                return EDataType::UInt16;
            }
        }

        static ramses_internal::EDataType ConvertDataTypeToInternal(EDataType type)
        {
            switch (type)
            {
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
            case EDataType::ByteBlob:
                return ramses_internal::EDataType::ByteBlob;
            }
            assert(false);
            return ramses_internal::EDataType::UInt16;
        }

        static ramses_internal::EResourceType DeductResourceTypeFromDataType(EDataType type)
        {
            switch (type)
            {
            case ramses::EDataType::UInt16:
            case ramses::EDataType::UInt32:
                return ramses_internal::EResourceType_IndexArray;
            case ramses::EDataType::Float:
            case ramses::EDataType::Vector2F:
            case ramses::EDataType::Vector3F:
            case ramses::EDataType::Vector4F:
            case ramses::EDataType::ByteBlob:
                return ramses_internal::EResourceType_VertexArray;
            }
            assert(false);
            return ramses_internal::EResourceType_Invalid;
        }

        static ramses_internal::EDataBufferType DeductBufferTypeFromDataType(EDataType type)
        {
            switch (type)
            {
            case ramses::EDataType::UInt16:
            case ramses::EDataType::UInt32:
                return ramses_internal::EDataBufferType::IndexBuffer;
            case ramses::EDataType::Float:
            case ramses::EDataType::Vector2F:
            case ramses::EDataType::Vector3F:
            case ramses::EDataType::Vector4F:
            case ramses::EDataType::ByteBlob:
                return ramses_internal::EDataBufferType::VertexBuffer;
            }
            assert(false);
            return ramses_internal::EDataBufferType::Invalid;
        }

        static bool IsValidIndicesType(EDataType type)
        {
            return type == EDataType::UInt16 ||
                type == EDataType::UInt32;
        }

        static bool IsValidVerticesType(EDataType type)
        {
            return type == EDataType::Float ||
                type == EDataType::Vector2F ||
                type == EDataType::Vector3F ||
                type == EDataType::Vector4F ||
                type == EDataType::ByteBlob;
        }
    };
}

#endif
