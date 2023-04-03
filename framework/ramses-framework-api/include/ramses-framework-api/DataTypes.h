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

namespace ramses
{
    /// Data type to hold vector2 float values
    struct vec2f : std::array<float, 2>
    {
        template <typename... Args> explicit vec2f(Args &&... args) : std::array<float, 2>{std::forward<Args>(args)...} {}
    };

    /// Data type to hold vector3 float values
    struct vec3f : std::array<float, 3>
    {
        template <typename... Args> explicit vec3f(Args &&... args) : std::array<float, 3>{std::forward<Args>(args)...} {}
    };

    /// Data type to hold vector4 float values
    struct vec4f : std::array<float, 4>
    {
        template <typename... Args> explicit vec4f(Args &&... args) : std::array<float, 4>{std::forward<Args>(args)...} {}
    };

    /// Data type to hold vector2 integer values
    struct vec2i : std::array<int32_t, 2>
    {
        template <typename... Args> explicit vec2i(Args &&... args) : std::array<int32_t, 2>{std::forward<Args>(args)...} {}
    };

    /// Data type to hold vector3 integer values
    struct vec3i : std::array<int32_t, 3>
    {
        template <typename... Args> explicit vec3i(Args &&... args) : std::array<int32_t, 3>{std::forward<Args>(args)...} {}
    };

    /// Data type to hold vector4 integer values
    struct vec4i : std::array<int32_t, 4>
    {
        template <typename... Args> explicit vec4i(Args &&... args) : std::array<int32_t, 4>{std::forward<Args>(args)...} {}
    };

    /// Data type to hold float matrix 2x2 values. Stored in column-major shape (the first 2 values correspond to the first matrix column)
    struct matrix22f : std::array<float, 4>
    {
        template <typename... Args> explicit matrix22f(Args &&... args) : std::array<float, 4>{std::forward<Args>(args)...} {}
    };

    /// Data type to hold float matrix 3x3 values. Stored in column-major shape (the first 3 values correspond to the first matrix column)
    struct matrix33f : std::array<float, 9>
    {
        template <typename... Args> explicit matrix33f(Args &&... args) : std::array<float, 9>{std::forward<Args>(args)...} {}
    };

    /// Data type to hold float matrix 4x4 values. Stored in column-major shape (the first 4 values correspond to the first matrix column)
    struct matrix44f : std::array<float, 16>
    {
        template <typename... Args> explicit matrix44f(Args &&... args) : std::array<float, 16>{std::forward<Args>(args)...} {}
    };

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

    /**
    * Data type to hold a quaternion
    */
    struct quat
    {
        /**
        * Creates an identity quaternion
        */
        quat()
            : quat(1.f, 0.f, 0.f, 0.f)
        {
        }

        /**
        * Creates a quaternion with the vector (qx, qy, qz) and the scalar qw
        */
        quat(float qw, float qx, float qy, float qz)
            : x(qx)
            , y(qy)
            , z(qz)
            , w(qw)
        {
        }

        bool operator==(const quat& rhs) const
        {
            return (x == rhs.x) && (y == rhs.y) && (z == rhs.z) && (w == rhs.w);
        }

        bool operator!=(const quat& rhs) const
        {
            return !(*this == rhs);
        }

        float x; ///< x coordinate of the quaternion's vector
        float y; ///< y coordinate of the quaternion's vector
        float z; ///< z coordinate of the quaternion's vector
        float w; ///< scalar component of the quaternion
    };
}
