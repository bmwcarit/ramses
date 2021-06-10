//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#ifndef RAMSES_VECTOR3_H
#define RAMSES_VECTOR3_H

#include "Collections/IInputStream.h"
#include "Collections/IOutputStream.h"
#include "PlatformAbstraction/FmtBase.h"

namespace ramses_internal
{
    class Vector4;

    class Vector3
    {
    public:
        union
        {
            struct
            {
                Float x;
                Float y;
                Float z;
            };
            Float data[3];
        };

        constexpr Vector3();
        constexpr Vector3(const Float _x, const Float _y, const Float _z);
        explicit constexpr Vector3(const Float value);
        explicit Vector3(const Vector4& other);

        constexpr Vector3(const Vector3& other) = default;
        constexpr Vector3& operator=(const Vector3& other) = default;

        constexpr void set(const Float _x, const Float _y, const Float _z);
        constexpr void set(const Float xyz);

        constexpr Vector3 operator+(const Vector3& other) const;
        constexpr Vector3 operator-(const Vector3& other) const;
        constexpr void operator+=(const Vector3& other);
        constexpr void operator-=(const Vector3& other);

        constexpr Vector3 operator-() const;

        constexpr Vector3 operator*(const Float scalar) const;
        constexpr Vector3 operator*(const Vector3& vec) const;
        constexpr void operator*=(const Float scalar);
        constexpr void operator*=(const Vector3& vec);
        constexpr void operator/=(const Float scalar);

        constexpr bool operator==(const Vector3& other) const;
        constexpr bool operator!=(const Vector3& other) const;

        constexpr Float& operator[](const UInt32 index);
        constexpr const Float& operator[](const UInt32 index) const;

        constexpr Float dot(const Vector3& other) const;
        constexpr Vector3 inverse() const;
        constexpr Vector3 cross(const Vector3& other) const;

        Float length() const;
        Vector3 normalize() const;

        friend constexpr Vector3 operator*(const Float scalar, const Vector3&);
    };

    constexpr inline Vector3::Vector3()
        : x(0)
        , y(0)
        , z(0)
    {
    }

    constexpr inline
    Vector3::Vector3(const Float value)
    : x(value)
    , y(value)
    , z(value)
    {

    }

    constexpr inline Vector3::Vector3(const Float _x, const Float _y, const Float _z)
        : x(_x)
        , y(_y)
        , z(_z)
    {
    }

    constexpr inline void Vector3::set(const Float _x, const Float _y, const Float _z)
    {
        x = _x;
        y = _y;
        z = _z;
    }

    constexpr inline void Vector3::set(const Float xyz)
    {
        x = y = z = xyz;
    }

    inline Float Vector3::length() const
    {
        return std::sqrt(x*x + y*y + z*z);
    }

    constexpr inline Vector3 Vector3::operator+(const Vector3& other) const
    {
        return Vector3(x + other.x, y + other.y, z + other.z);
    }

    constexpr inline void Vector3::operator+=(const Vector3& other)
    {
        x += other.x;
        y += other.y;
        z += other.z;
    }

    constexpr inline Vector3 Vector3::operator-(const Vector3& other) const
    {
        return Vector3( x - other.x
                        , y - other.y
                        , z - other.z);
    }

    constexpr inline void Vector3::operator-=(const Vector3& other)
    {
        x -= other.x;
        y -= other.y;
        z -= other.z;
    }

    constexpr inline bool Vector3::operator==(const Vector3& other) const
    {
        return x == other.x && y == other.y && z == other.z;
    }

    constexpr inline bool Vector3::operator!=(const Vector3& other) const
    {
        return !operator==(other);
    }

    constexpr inline Float Vector3::dot(const Vector3& other) const
    {
        return x * other.x + y * other.y + z * other.z;
    }

    constexpr inline Vector3 Vector3::inverse() const
    {
        return Vector3(1.0f / x, 1.0f / y, 1.0f / z);
    }

    constexpr inline Vector3 Vector3::cross(const Vector3& other) const
    {
        return Vector3( y * other.z - z * other.y
                        , z * other.x - x * other.z
                        , x * other.y - y * other.x);
    }

    constexpr inline void Vector3::operator*=(const Float scalar)
    {
        x *= scalar;
        y *= scalar;
        z *= scalar;
    }

    constexpr inline void Vector3::operator/=(const Float scalar)
    {
        x /= scalar;
        y /= scalar;
        z /= scalar;
    }

    constexpr inline Vector3 Vector3::operator*(const Float scalar) const
    {
        return Vector3( x * scalar
                        , y * scalar
                        , z * scalar);
    }

    constexpr inline Vector3 Vector3::operator*(const Vector3& vec) const
    {
        return Vector3(x * vec.x, y * vec.y, z * vec.z);
    }

    constexpr inline void Vector3::operator*=(const Vector3& vec)
    {
        x *= vec.x;
        y *= vec.y;
        z *= vec.z;
    }

    inline
    Vector3
    Vector3::normalize() const
    {
        const Float len = length();
        return Vector3(x / len, y / len, z / len);
    }

    constexpr inline
    Float& Vector3::operator[](const UInt32 index)
    {
        return data[index];
    }

    constexpr inline
    const Float& Vector3::operator[](const UInt32 index) const
    {
        return data[index];
    }

    constexpr inline
    Vector3 Vector3::operator-() const
    {
        return Vector3(-x, -y, -z);
    }

    inline
    IOutputStream&
    operator<<(IOutputStream& outputStream, const Vector3& vector)
    {
        return outputStream.write(vector.data, sizeof(vector.data));
    }

    inline
    IInputStream&
    operator>>(IInputStream& inputStream, Vector3& vector)
    {
        return inputStream.read(reinterpret_cast<Char*>(vector.data), sizeof(vector.data));
    }

    constexpr inline Vector3 operator*(const Float scalar, const Vector3& vec)
    {
        return Vector3(vec.x * scalar
            , vec.y * scalar
            , vec.z * scalar);
    }

    static_assert(std::is_nothrow_move_constructible<Vector3>::value, "Vector3 must be movable");
    static_assert(std::is_nothrow_move_assignable<Vector3>::value, "Vector3 must be movable");
}

template <>
struct fmt::formatter<ramses_internal::Vector3> : public ramses_internal::SimpleFormatterBase
{
    template<typename FormatContext>
    auto format(const ramses_internal::Vector3& m, FormatContext& ctx)
    {
        return fmt::format_to(ctx.out(), "[{} {} {}]", m.data[0], m.data[1], m.data[2]);
    }
};


#endif
