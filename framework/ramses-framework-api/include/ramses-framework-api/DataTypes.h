//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses-framework-api/EDataType.h"
#include <array>
#include <cassert>
#include "glm/mat4x4.hpp"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "glm/gtc/quaternion.hpp"

namespace ramses
{
    using vec2f = glm::vec2;
    using vec3f = glm::vec3;
    using vec4f = glm::vec4;
    using vec2i = glm::ivec2;
    using vec3i = glm::ivec3;
    using vec4i = glm::ivec4;
    using matrix22f = glm::mat2;
    using matrix33f = glm::mat3;
    using matrix44f = glm::mat4;
    using quat = glm::quat;

    using Byte = unsigned char;

    /**
    * @brief Static query of data type compatibility with uniform inputs (e.g. #ramses::Appearance::setInputValue).
    * @return true if data type can be used with uniform inputs
    */
    template <typename T> constexpr bool IsUniformInputDataType()
    {
        using RawType = std::remove_cv_t<std::remove_reference_t<T>>;
        return std::is_same_v<RawType, int32_t>
            || std::is_same_v<RawType, float>
            || std::is_same_v<RawType, vec2i>
            || std::is_same_v<RawType, vec3i>
            || std::is_same_v<RawType, vec4i>
            || std::is_same_v<RawType, vec2f>
            || std::is_same_v<RawType, vec3f>
            || std::is_same_v<RawType, vec4f>
            || std::is_same_v<RawType, matrix22f>
            || std::is_same_v<RawType, matrix33f>
            || std::is_same_v<RawType, matrix44f>;
    }

    /**
    * @brief Static query of data type compatibility with #ramses::ArrayResource and #ramses::ArrayBuffer.
    * @return true if data type can be used with #ramses::ArrayResource and #ramses::ArrayBuffer
    */
    template <typename T> constexpr bool IsArrayResourceDataType()
    {
        return std::is_same_v<T, uint16_t>
            || std::is_same_v<T, uint32_t>
            || std::is_same_v<T, float>
            || std::is_same_v<T, vec2f>
            || std::is_same_v<T, vec3f>
            || std::is_same_v<T, vec4f>
            || std::is_same_v<T, Byte>;
    }

    /**
    * @brief Query of data type compatibility with #ramses::ArrayResource and #ramses::ArrayBuffer.
    * @param[in] dataType Data type to check if compatible
    * @return true if given #ramses::EDataType can be used with #ramses::ArrayResource and #ramses::ArrayBuffer
    */
    constexpr bool IsArrayResourceDataType(EDataType dataType)
    {
        switch (dataType)
        {
        case EDataType::UInt16:
        case EDataType::UInt32:
        case EDataType::Float:
        case EDataType::Vector2F:
        case EDataType::Vector3F:
        case EDataType::Vector4F:
        case EDataType::ByteBlob:
            return true;
        case EDataType::Int32:
        case EDataType::Vector2I:
        case EDataType::Vector3I:
        case EDataType::Vector4I:
        case EDataType::Matrix22F:
        case EDataType::Matrix33F:
        case EDataType::Matrix44F:
        case EDataType::TextureSampler2D:
        case EDataType::TextureSampler2DMS:
        case EDataType::TextureSampler3D:
        case EDataType::TextureSamplerCube:
        case EDataType::TextureSamplerExternal:
            return false;
        }

        assert(false);
        return false;
    }

    /**
    * @brief Query of data type compatibility with #ramses::DataObject.
    * @param[in] dataType Data type to check if compatible
    * @return true if given #ramses::EDataType can be used with #ramses::DataObject
    */
    constexpr bool IsDataObjectDataType(EDataType dataType)
    {
        switch (dataType)
        {
        case EDataType::Int32:
        case EDataType::Float:
        case EDataType::Vector2F:
        case EDataType::Vector3F:
        case EDataType::Vector4F:
        case EDataType::Vector2I:
        case EDataType::Vector3I:
        case EDataType::Vector4I:
        case EDataType::Matrix22F:
        case EDataType::Matrix33F:
        case EDataType::Matrix44F:
            return true;
        case EDataType::UInt16:
        case EDataType::UInt32:
        case EDataType::ByteBlob:
        case EDataType::TextureSampler2D:
        case EDataType::TextureSampler2DMS:
        case EDataType::TextureSampler3D:
        case EDataType::TextureSamplerCube:
        case EDataType::TextureSamplerExternal:
            return false;
        }

        assert(false);
        return false;
    }

    /**
    * @brief Static query of #ramses::EDataType that matches given template type.
    * @return #ramses::EDataType for given template type
    */
    template <typename T> constexpr EDataType GetEDataType()
    {
        if constexpr (std::is_same_v<T, uint16_t>)
            return EDataType::UInt16;
        else if constexpr (std::is_same_v<T, uint32_t>)
            return EDataType::UInt32;
        else if constexpr (std::is_same_v<T, float>)
            return EDataType::Float;
        else if constexpr (std::is_same_v<T, vec2f>)
            return EDataType::Vector2F;
        else if constexpr (std::is_same_v<T, vec3f>)
            return EDataType::Vector3F;
        else if constexpr (std::is_same_v<T, vec4f>)
            return EDataType::Vector4F;
        else if constexpr (std::is_same_v<T, Byte>)
            return EDataType::ByteBlob;
        else if constexpr (std::is_same_v<T, int32_t>)
            return EDataType::Int32;
        else if constexpr (std::is_same_v<T, vec2i>)
            return EDataType::Vector2I;
        else if constexpr (std::is_same_v<T, vec3i>)
            return EDataType::Vector3I;
        else if constexpr (std::is_same_v<T, vec4i>)
            return EDataType::Vector4I;
        else if constexpr (std::is_same_v<T, matrix22f>)
            return EDataType::Matrix22F;
        else if constexpr (std::is_same_v<T, matrix33f>)
            return EDataType::Matrix33F;
        else if constexpr (std::is_same_v<T, matrix44f>)
            return EDataType::Matrix44F;

        static_assert(IsArrayResourceDataType<T>() || IsUniformInputDataType<T>(), "This type has no corresponding EDataType");
        return EDataType::ByteBlob;
    }
}
