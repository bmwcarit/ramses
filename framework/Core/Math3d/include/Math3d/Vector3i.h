//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#ifndef RAMSES_VECTOR3I_H
#define RAMSES_VECTOR3I_H

#include "Collections/IInputStream.h"
#include "Collections/IOutputStream.h"
#include "PlatformAbstraction/FmtBase.h"

namespace ramses_internal
{
    class Vector3i
    {
    public:
        union
        {
            struct
            {
                Int32 x;
                Int32 y;
                Int32 z;
            };
            Int32 data[3];
        };

        constexpr Vector3i();
        constexpr Vector3i(const Int32 _x, const Int32 _y, const Int32 _z);
        explicit constexpr Vector3i(const Int32 value);

        constexpr Vector3i(const Vector3i& other) = default;
        constexpr Vector3i& operator=(const Vector3i& other) = default;

        constexpr void set(const Int32 _x, const Int32 _y, const Int32 _z);
        constexpr void set(const Int32 xyz);

        constexpr Vector3i operator+(const Vector3i& other) const;
        constexpr void operator+=(const Vector3i& other);
        constexpr Vector3i operator-(const Vector3i& other) const;
        constexpr void operator-=(const Vector3i& other);

        constexpr Vector3i operator-() const;

        constexpr Vector3i operator*(const Int32 scalar) const;
        constexpr Vector3i operator*(const Vector3i& vec) const;
        constexpr void operator*=(const Int32 scalar);
        constexpr void operator*=(const Vector3i& vec);
        constexpr void operator/=(const Int32 scalar);

        constexpr bool operator==(const Vector3i& other) const;
        constexpr bool operator!=(const Vector3i& other) const;

        constexpr Int32& operator[](const UInt32 index);
        constexpr const Int32& operator[](const UInt32 index) const;

        constexpr Int32 dot(const Vector3i& other) const;
        constexpr Vector3i cross(const Vector3i& other) const;
        Float length() const;
        Float angle(const Vector3i& other) const; ///< in radians

        friend constexpr Vector3i operator*(const Int32 scalar, const Vector3i&);
    };

    constexpr inline Vector3i::Vector3i()
        : x(0)
        , y(0)
        , z(0)
    {
    }

    constexpr inline
    Vector3i::Vector3i(const Int32 value)
        : x(value)
        , y(value)
        , z(value)
    {

    }

    constexpr inline Vector3i::Vector3i(const Int32 _x, const Int32 _y, const Int32 _z)
        : x(_x)
        , y(_y)
        , z(_z)
    {
    }

    constexpr inline void Vector3i::set(const Int32 _x, const Int32 _y, const Int32 _z)
    {
        x = _x;
        y = _y;
        z = _z;
    }

    constexpr inline void Vector3i::set(const Int32 xyz)
    {
        x = y = z = xyz;
    }

    inline Float Vector3i::length() const
    {
        return std::sqrt(static_cast<Float>(x*x + y*y + z*z));
    }

    constexpr inline Vector3i Vector3i::operator+(const Vector3i& other) const
    {
        return Vector3i( x + other.x
                        , y + other.y
                        , z + other.z);
    }

    constexpr inline void Vector3i::operator+=(const Vector3i& other)
    {
        x += other.x;
        y += other.y;
        z += other.z;
    }

    constexpr inline Vector3i Vector3i::operator-(const Vector3i& other) const
    {
        return Vector3i( x - other.x
                        , y - other.y
                        , z - other.z);
    }

    constexpr inline void Vector3i::operator-=(const Vector3i& other)
    {
        x -= other.x;
        y -= other.y;
        z -= other.z;
    }

    constexpr inline bool Vector3i::operator==(const Vector3i& other) const
    {
        return x == other.x && y == other.y && z == other.z;
    }

    constexpr inline bool Vector3i::operator!=(const Vector3i& other) const
    {
        return !operator==(other);
    }

    constexpr inline Int32 Vector3i::dot(const Vector3i& other) const
    {
        return x * other.x + y * other.y + z * other.z;
    }

    constexpr inline Vector3i Vector3i::cross(const Vector3i& other) const
    {
        return Vector3i( y * other.z - z * other.y
                        , z * other.x - x * other.z
                        , x * other.y - y * other.x);
    }

    constexpr inline void Vector3i::operator*=(const Int32 scalar)
    {
        x *= scalar;
        y *= scalar;
        z *= scalar;
    }

    constexpr inline void Vector3i::operator/=(const Int32 scalar)
    {
        x /= scalar;
        y /= scalar;
        z /= scalar;
    }

    constexpr inline Vector3i Vector3i::operator*(const Int32 scalar) const
    {
        return Vector3i( x * scalar
                        , y * scalar
                        , z * scalar);
    }

    constexpr inline Vector3i Vector3i::operator*(const Vector3i& vec) const
    {
        return Vector3i(x * vec.x, y * vec.y, z * vec.z);
    }

    constexpr inline void Vector3i::operator*=(const Vector3i& vec)
    {
        x *= vec.x;
        y *= vec.y;
        z *= vec.z;
    }

    inline Float Vector3i::angle(const Vector3i& other) const
    {
        return std::acos(dot(other) / (length() * other.length()));
    }

    constexpr inline
    Int32& Vector3i::operator[](const UInt32 index)
    {
        return data[index];
    }

    constexpr inline
    const Int32& Vector3i::operator[](const UInt32 index) const
    {
        return data[index];
    }

    constexpr inline
    Vector3i Vector3i::operator-() const
    {
        return Vector3i(-x, -y, -z);
    }

    inline
    IOutputStream&
    operator<<(IOutputStream& outputStream, const Vector3i& vector)
    {
        return outputStream.write(vector.data, sizeof(vector.data));
    }

    inline
    IInputStream&
    operator>>(IInputStream& inputStream, Vector3i& vector)
    {
        return inputStream.read(reinterpret_cast<Char*>(vector.data), sizeof(vector.data));
    }

    constexpr inline Vector3i operator*(const Int32 scalar, const Vector3i& vec)
    {
        return Vector3i(vec.x * scalar
            , vec.y * scalar
            , vec.z * scalar);
    }
}

template <>
struct fmt::formatter<ramses_internal::Vector3i> : public ramses_internal::SimpleFormatterBase
{
    template<typename FormatContext>
    auto format(const ramses_internal::Vector3i& m, FormatContext& ctx)
    {
        return fmt::format_to(ctx.out(), "[{} {} {}]", m.data[0], m.data[1], m.data[2]);
    }
};


#endif
