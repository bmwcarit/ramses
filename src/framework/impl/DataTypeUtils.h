//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/EDataType.h"
#include "internal/SceneGraph/SceneAPI/EDataType.h"
#include "internal/SceneGraph/SceneAPI/EDataBufferType.h"
#include <cassert>

namespace ramses
{
    class DataTypeUtils
    {
    public:
        static ramses::EDataType ConvertDataTypeFromInternal(ramses::internal::EDataType type)
        {
            switch (type)
            {
            case ramses::internal::EDataType::Bool:
                return EDataType::Bool;
            case ramses::internal::EDataType::Int32:
                return EDataType::Int32;
            case ramses::internal::EDataType::UInt16:
                return EDataType::UInt16;
            case ramses::internal::EDataType::UInt32:
                return EDataType::UInt32;
            case ramses::internal::EDataType::Float:
                return EDataType::Float;
            case ramses::internal::EDataType::Vector2F:
                return EDataType::Vector2F;
            case ramses::internal::EDataType::Vector3F:
                return EDataType::Vector3F;
            case ramses::internal::EDataType::Vector4F:
                return EDataType::Vector4F;
            case ramses::internal::EDataType::Vector2I:
                return EDataType::Vector2I;
            case ramses::internal::EDataType::Vector3I:
                return EDataType::Vector3I;
            case ramses::internal::EDataType::Vector4I:
                return EDataType::Vector4I;
            case ramses::internal::EDataType::Matrix22F:
                return EDataType::Matrix22F;
            case ramses::internal::EDataType::Matrix33F:
                return EDataType::Matrix33F;
            case ramses::internal::EDataType::Matrix44F:
                return EDataType::Matrix44F;
            case ramses::internal::EDataType::ByteBlob:
                return EDataType::ByteBlob;

            // internal attribure array types are converted back to their element type on public API
            case ramses::internal::EDataType::UInt16Buffer:
                return EDataType::UInt16;
            case ramses::internal::EDataType::FloatBuffer:
                return EDataType::Float;
            case ramses::internal::EDataType::Vector2Buffer:
                return EDataType::Vector2F;
            case ramses::internal::EDataType::Vector3Buffer:
                return EDataType::Vector3F;
            case ramses::internal::EDataType::Vector4Buffer:
                return EDataType::Vector4F;

            case ramses::internal::EDataType::TextureSampler2D:
                return EDataType::TextureSampler2D;
            case ramses::internal::EDataType::TextureSampler2DMS:
                return EDataType::TextureSampler2DMS;
            case ramses::internal::EDataType::TextureSampler3D:
                return EDataType::TextureSampler3D;
            case ramses::internal::EDataType::TextureSamplerCube:
                return EDataType::TextureSamplerCube;
            case ramses::internal::EDataType::TextureSamplerExternal:
                return EDataType::TextureSamplerExternal;

            default:
                assert(false);
                return EDataType::ByteBlob;
            }
        }

        static constexpr ramses::internal::EDataType ConvertDataTypeToInternal(EDataType type)
        {
            switch (type)
            {
            case EDataType::Bool:
                return ramses::internal::EDataType::Bool;
            case EDataType::Int32:
                return ramses::internal::EDataType::Int32;
            case EDataType::UInt16:
                return ramses::internal::EDataType::UInt16;
            case EDataType::UInt32:
                return ramses::internal::EDataType::UInt32;
            case EDataType::Float:
                return ramses::internal::EDataType::Float;
            case EDataType::Vector2F:
                return ramses::internal::EDataType::Vector2F;
            case EDataType::Vector3F:
                return ramses::internal::EDataType::Vector3F;
            case EDataType::Vector4F:
                return ramses::internal::EDataType::Vector4F;
            case EDataType::Vector2I:
                return ramses::internal::EDataType::Vector2I;
            case EDataType::Vector3I:
                return ramses::internal::EDataType::Vector3I;
            case EDataType::Vector4I:
                return ramses::internal::EDataType::Vector4I;
            case EDataType::Matrix22F:
                return ramses::internal::EDataType::Matrix22F;
            case EDataType::Matrix33F:
                return ramses::internal::EDataType::Matrix33F;
            case EDataType::Matrix44F:
                return ramses::internal::EDataType::Matrix44F;
            case EDataType::ByteBlob:
                return ramses::internal::EDataType::ByteBlob;
            case EDataType::TextureSampler2D:
                return ramses::internal::EDataType::TextureSampler2D;
            case EDataType::TextureSampler2DMS:
                return ramses::internal::EDataType::TextureSampler2DMS;
            case EDataType::TextureSampler3D:
                return ramses::internal::EDataType::TextureSampler3D;
            case EDataType::TextureSamplerCube:
                return ramses::internal::EDataType::TextureSamplerCube;
            case EDataType::TextureSamplerExternal:
                return ramses::internal::EDataType::TextureSamplerExternal;
            }

            assert(false);
            return ramses::internal::EDataType::Invalid;
        }

        static ramses::internal::EResourceType DeductResourceTypeFromDataType(EDataType type)
        {
            if (IsValidIndicesType(type))
                return ramses::internal::EResourceType::IndexArray;
            if (IsValidVerticesType(type))
                return ramses::internal::EResourceType::VertexArray;

            assert(false);
            return ramses::internal::EResourceType::Invalid;
        }

        static ramses::internal::EDataBufferType DeductBufferTypeFromDataType(EDataType type)
        {
            if (IsValidIndicesType(type))
                return ramses::internal::EDataBufferType::IndexBuffer;
            if (IsValidVerticesType(type))
                return ramses::internal::EDataBufferType::VertexBuffer;

            assert(false);
            return ramses::internal::EDataBufferType::Invalid;
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
