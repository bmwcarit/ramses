//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_VECTOR4I_H
#define RAMSES_VECTOR4I_H

#include "Collections/IInputStream.h"
#include "Collections/IOutputStream.h"
#include "Math3d/Vector3i.h"
#include "PlatformAbstraction/FmtBase.h"

namespace ramses_internal
{
    class Vector4i
    {
    public:
        union
        {
            struct
            {
                Int32 x;
                Int32 y;
                Int32 z;
                Int32 w;
            };
            Int32 data[4];
        };

        constexpr Vector4i();
        constexpr Vector4i(Int32 _x, Int32 _y, Int32 _z, Int32 _w);
        explicit constexpr Vector4i(const Int32 value);
        explicit constexpr Vector4i(const Vector3i& other);

        constexpr Vector4i(const Vector4i& other) = default;
        constexpr Vector4i& operator=(const Vector4i& other) = default;

        constexpr void set(Int32 _x, Int32 _y, Int32 _z, Int32 _w);
        constexpr void set(Int32 xyzw);

        constexpr Vector4i operator+(const Vector4i& other) const;
        constexpr void operator+=(const Vector4i& other);
        constexpr Vector4i operator-(const Vector4i& other) const;
        constexpr void operator-=(const Vector4i& other);

        constexpr Vector4i operator*(const Int32 scalar) const;
        constexpr Vector4i operator*(const Vector4i& vec) const;
        constexpr void operator*=(const Int32 scalar);
        constexpr void operator*=(const Vector4i& vec);
        constexpr Vector4i operator/(const Vector4i& scalar) const;

        constexpr bool operator==(const Vector4i& other) const;
        constexpr bool operator!=(const Vector4i& other) const;

        constexpr Int32 dot(const Vector4i& other) const;
        Float length() const;

        constexpr Int32& operator[](const UInt32 index);
        constexpr const Int32& operator[](const UInt32 index) const;

        friend constexpr Vector4i operator*(const Int32 scalar, const Vector4i&);
    };

    constexpr inline Vector4i::Vector4i()
        : x(0)
        , y(0)
        , z(0)
        , w(0)
    {
    }

    constexpr inline Vector4i::Vector4i(Int32 _x, Int32 _y, Int32 _z, Int32 _w)
        : x(_x)
        , y(_y)
        , z(_z)
        , w(_w)
    {
    }

    constexpr inline
    Vector4i::Vector4i(const Int32 value)
        : x(value)
        , y(value)
        , z(value)
        , w(value)
    {
    }

    constexpr inline Vector4i::Vector4i(const Vector3i& other)
        : x(other.x)
        , y(other.y)
        , z(other.z)
        , w(255)
    {
    }

    constexpr inline void Vector4i::set(Int32 _x, Int32 _y, Int32 _z, Int32 _w)
    {
        x = _x;
        y = _y;
        z = _z;
        w = _w;
    }

    constexpr inline void Vector4i::set(Int32 xyzw)
    {
        x = y = z = w = xyzw;
    }

    inline Float Vector4i::length() const
    {
        return std::sqrt(static_cast<Float>(x*x + y*y + z*z + w*w));
    }

    constexpr inline Vector4i Vector4i::operator+(const Vector4i& other) const
    {
        return Vector4i(x + other.x
            , y + other.y
            , z + other.z
            , w + other.w);
    }

    constexpr inline void Vector4i::operator+=(const Vector4i& other)
    {
        x += other.x;
        y += other.y;
        z += other.z;
        w += other.w;
    }

    constexpr inline Vector4i Vector4i::operator-(const Vector4i& other) const
    {
        return Vector4i(x - other.x
            , y - other.y
            , z - other.z
            , w - other.w);
    }

    constexpr inline void Vector4i::operator-=(const Vector4i& other)
    {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        w -= other.w;
    }

    constexpr inline Vector4i Vector4i::operator*(const Vector4i& vec) const
    {
        return Vector4i(x * vec.x, y * vec.y, z * vec.z, w * vec.w);
    }

    constexpr inline void Vector4i::operator*=(const Vector4i& vec)
    {
        x *= vec.x;
        y *= vec.y;
        z *= vec.z;
        w *= vec.w;
    }

    constexpr inline bool Vector4i::operator==(const Vector4i& other) const
    {
        return x == other.x && y == other.y && z == other.z && w == other.w;
    }

    constexpr inline bool Vector4i::operator!=(const Vector4i& other) const
    {
        return !operator==(other);
    }

    constexpr inline Int32 Vector4i::dot(const Vector4i& other) const
    {
        return x * other.x + y * other.y + z * other.z + w * other.w;
    }

    constexpr inline void Vector4i::operator*=(const Int32 scalar)
    {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        w *= scalar;
    }

    constexpr inline Vector4i Vector4i::operator*(const Int32 scalar) const
    {
        return Vector4i(x * scalar
            , y * scalar
            , z * scalar
            , w * scalar);
    }

    constexpr inline Vector4i Vector4i::operator/(const Vector4i& other) const
    {
        return Vector4i(x / other.x
            , y / other.y
            , z / other.z
            , w / other.w);
    }

    inline
    IOutputStream&
    operator<<(IOutputStream& outputStream, const Vector4i& vector)
    {
            return outputStream.write(vector.data, sizeof(vector.data));
    }

    inline
    IInputStream&
    operator>>(IInputStream& inputStream, Vector4i& vector)
    {
            return inputStream.read(vector.data, sizeof(vector.data));
    }

    constexpr inline
    Int32& Vector4i::operator[](const UInt32 index)
    {
            return data[index];
    }

    constexpr inline
    const Int32& Vector4i::operator[](const UInt32 index) const
    {
            return data[index];
    }

    constexpr inline
    Vector4i operator*(const Int32 scalar, const Vector4i& vec)
    {
        return Vector4i(vec.x * scalar
            , vec.y * scalar
            , vec.z * scalar
            , vec.w * scalar);
    }
}

template <>
struct fmt::formatter<ramses_internal::Vector4i> : public ramses_internal::SimpleFormatterBase
{
    template<typename FormatContext>
    constexpr auto format(const ramses_internal::Vector4i& m, FormatContext& ctx)
    {
        return fmt::format_to(ctx.out(), "[{} {} {} {}]", m.data[0], m.data[1], m.data[2], m.data[3]);
    }
};

#endif
