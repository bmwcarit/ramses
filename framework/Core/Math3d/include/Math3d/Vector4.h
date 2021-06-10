//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_VECTOR4_H
#define RAMSES_VECTOR4_H

#include "Collections/IInputStream.h"
#include "Collections/IOutputStream.h"
#include "Math3d/Vector3.h"
#include "PlatformAbstraction/FmtBase.h"

namespace ramses_internal
{
    class Vector4
    {
    public:
        union
        {
            struct
            {
                Float x;
                Float y;
                Float z;
                Float w;
            };
            struct
            {
                Float r;
                Float g;
                Float b;
                Float a;
            };
            Float data[4];
        };

        constexpr Vector4();
        explicit constexpr Vector4(const Float value);
        explicit constexpr Vector4(const Vector3& other);
        constexpr Vector4(Float _x, Float _y, Float _z, Float _w);

        constexpr Vector4(const Vector4& other) = default;
        constexpr Vector4& operator=(const Vector4& other) = default;

        constexpr void set(Float _x, Float _y, Float _z, Float _w);
        constexpr void set(Float xyzw);

        constexpr Vector4 operator+(const Vector4& other) const;
        constexpr void operator+=(const Vector4& other);
        constexpr Vector4 operator-(const Vector4& other) const;
        constexpr void operator-=(const Vector4& other);

        constexpr Vector4 operator*(const Float scalar) const;
        constexpr Vector4 operator*(const Vector4& vec) const;
        constexpr void operator*=(const Float scalar);
        constexpr void operator*=(const Vector4& vec);
        constexpr Vector4 operator/(const Float scalar) const;
        constexpr Vector4 operator/(const Vector4& vec) const;
        constexpr void operator/=(const Float scalar);

        constexpr bool operator==(const Vector4& other) const;
        constexpr bool operator!=(const Vector4& other) const;

        constexpr Float& operator[](const UInt32 index);
        constexpr const Float& operator[](const UInt32 index) const;

        constexpr Float dot(const Vector4& other) const;
        constexpr Vector4 cross(const Vector4& other) const;
        Float length() const;

        friend constexpr Vector4 operator*(const Float scalar, const Vector4&);
    };

    constexpr inline Vector4::Vector4()
        : x(0)
        , y(0)
        , z(0)
        , w(0)
    {
    }

    constexpr inline Vector4::Vector4(Float _x, Float _y, Float _z, Float _w)
        : x(_x)
        , y(_y)
        , z(_z)
        , w(_w)
    {
    }

    constexpr inline
        Vector4::Vector4(const Float value)
        : x(value)
        , y(value)
        , z(value)
        , w(value)
    {
    }

    constexpr inline Vector4::Vector4(const Vector3& other)
        : x(other.x)
        , y(other.y)
        , z(other.z)
        , w(1.0f)
    {
    }

    constexpr inline void Vector4::set(Float _x, Float _y, Float _z, Float _w)
    {
        x = _x;
        y = _y;
        z = _z;
        w = _w;
    }

    constexpr inline void Vector4::set(Float xyzw)
    {
        x = y = z = w = xyzw;
    }

    inline Float Vector4::length() const
    {
        return std::sqrt(x*x + y*y + z*z + w*w);
    }

    constexpr inline Vector4 Vector4::operator+(const Vector4& other) const
    {
        return Vector4(x + other.x
            , y + other.y
            , z + other.z
            , w + other.w);
    }

    constexpr inline void Vector4::operator+=(const Vector4& other)
    {
        x += other.x;
        y += other.y;
        z += other.z;
        w += other.w;
    }

    constexpr inline Vector4 Vector4::operator-(const Vector4& other) const
    {
        return Vector4(x - other.x
            , y - other.y
            , z - other.z
            , w - other.w);
    }

    constexpr inline void Vector4::operator-=(const Vector4& other)
    {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        w -= other.w;
    }

    constexpr inline Vector4 Vector4::operator*(const Vector4& vec) const
    {
        return Vector4(x * vec.x, y * vec.y, z * vec.z, w * vec.w);
    }

    constexpr inline void Vector4::operator*=(const Vector4& vec)
    {
        x *= vec.x;
        y *= vec.y;
        z *= vec.z;
        w *= vec.w;
    }

    constexpr inline bool Vector4::operator==(const Vector4& other) const
    {
        return x == other.x && y == other.y && z == other.z && w == other.w;
    }

    constexpr inline bool Vector4::operator!=(const Vector4& other) const
    {
        return !operator==(other);
    }

    constexpr inline Float Vector4::dot(const Vector4& other) const
    {
        return x * other.x + y * other.y + z * other.z + w * other.w;
    }

    constexpr inline Vector4 Vector4::cross(const Vector4& other) const
    {
        return Vector4(y * other.z - z * other.y + z * other.w - w * other.z + y * other.w - w * other.y
            , z * other.x - x * other.z + w * other.x - x * other.w + z * other.w - w * other.z
            , x * other.y - y * other.x + w * other.y - y * other.w + w * other.x - x * other.w
            , x * other.y - y * other.x + y * other.z - z * other.y + x * other.z - z * other.x
            );
    }

    constexpr inline void Vector4::operator*=(const Float scalar)
    {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        w *= scalar;
    }

    constexpr inline Vector4 Vector4::operator*(const Float scalar) const
    {
        return Vector4(x * scalar
            , y * scalar
            , z * scalar
            , w * scalar);
    }

    constexpr inline Vector4 Vector4::operator/(const Float scalar) const
    {
        return Vector4(x / scalar
            , y / scalar
            , z / scalar
            , w / scalar);
    }

    constexpr inline Vector4 Vector4::operator/(const Vector4& other) const
    {
        return Vector4(x / other.x
            , y / other.y
            , z / other.z
            , w / other.w);
    }

    constexpr inline void Vector4::operator/=(const Float scalar)
    {
        x /= scalar;
        y /= scalar;
        z /= scalar;
        w /= scalar;
    }

    inline
    IOutputStream&
    operator<<(IOutputStream& outputStream, const Vector4& vector)
    {
            return outputStream.write(vector.data, sizeof(vector.data));
    }

    inline
    IInputStream&
    operator>>(IInputStream& inputStream, Vector4& vector)
    {
            return inputStream.read(reinterpret_cast<Char*>(vector.data), sizeof(vector.data));
    }

    constexpr inline
    Float& Vector4::operator[](const UInt32 index)
    {
            return data[index];
    }

    constexpr inline
    const Float& Vector4::operator[](const UInt32 index) const
    {
            return data[index];
    }

    constexpr inline
    Vector4 operator*(const Float scalar, const Vector4& vec)
    {
        return Vector4(vec.x * scalar
            , vec.y * scalar
            , vec.z * scalar
            , vec.w * scalar);
    }

    static_assert(std::is_nothrow_move_constructible<Vector4>::value, "Vector4 must be movable");
    static_assert(std::is_nothrow_move_assignable<Vector4>::value, "Vector4 must be movable");
}

template <>
struct fmt::formatter<ramses_internal::Vector4> : public ramses_internal::SimpleFormatterBase
{
    template<typename FormatContext>
    auto format(const ramses_internal::Vector4& m, FormatContext& ctx)
    {
        return fmt::format_to(ctx.out(), "[{} {} {} {}]", m.data[0], m.data[1], m.data[2], m.data[3]);
    }
};

#endif
