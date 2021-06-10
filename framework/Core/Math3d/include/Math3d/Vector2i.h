//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#ifndef RAMSES_VECTOR2I_H
#define RAMSES_VECTOR2I_H

#include "Collections/IInputStream.h"
#include "Collections/IOutputStream.h"
#include "PlatformAbstraction/FmtBase.h"

namespace ramses_internal
{
    class Vector2i
    {
    public:
        union
        {
            struct
            {
                Int32 x;
                Int32 y;
            };
            Int32 data[2];
        };

        constexpr Vector2i();
        constexpr Vector2i(const Int32 _x, const Int32 _y);
        explicit constexpr Vector2i(const Int32 value);

        constexpr Vector2i(const Vector2i& other) = default;
        constexpr Vector2i& operator=(const Vector2i& other) = default;

        constexpr void set(const Int32 _x, const Int32 _y);
        constexpr void set(const Int32 xy);

        constexpr Vector2i operator+(const Vector2i& other) const;
        constexpr void operator+=(const Vector2i& other);
        constexpr Vector2i operator-(const Vector2i& other) const;
        constexpr void operator-=(const Vector2i& other);

        constexpr Vector2i operator-() const;

        constexpr Vector2i operator*(const Int32 scalar) const;
        constexpr Vector2i operator*(const Vector2i& vec) const;
        constexpr void operator*=(const Int32 scalar);
        constexpr void operator*=(const Vector2i& vec);
        constexpr void operator/=(const Int32 scalar);

        constexpr bool operator==(const Vector2i& other) const;
        constexpr bool operator!=(const Vector2i& other) const;

        constexpr Int32& operator[](const UInt32 index);
        constexpr const Int32& operator[](const UInt32 index) const;

        constexpr Int32 dot(const Vector2i& other) const;
        Float length() const;

        friend constexpr Vector2i operator*(const Int32 scalar, const Vector2i&);
    };

    constexpr inline Vector2i::Vector2i()
        : x(0)
        , y(0)
    {
    }

    constexpr inline Vector2i::Vector2i(const Int32 _x, const Int32 _y)
        : x(_x)
        , y(_y)
    {
    }

    constexpr inline Vector2i::Vector2i(const Int32 xy)
        : x(xy)
        , y(xy)
    {
    }

    constexpr inline void Vector2i::set(const Int32 _x, const Int32 _y)
    {
        x = _x;
        y = _y;
    }

    constexpr inline void Vector2i::set(const Int32 xy)
    {
        x = y = xy;
    }

    inline Float Vector2i::length() const
    {
        return std::sqrt(static_cast<Float>(x*x + y*y));
    }

    constexpr inline Vector2i Vector2i::operator+(const Vector2i& other) const
    {
        return Vector2i(x + other.x, y + other.y);
    }

    constexpr inline void Vector2i::operator+=(const Vector2i& other)
    {
        x += other.x;
        y += other.y;
    }

    constexpr inline Vector2i Vector2i::operator-(const Vector2i& other) const
    {
        return Vector2i(x - other.x, y - other.y);
    }

    constexpr inline void Vector2i::operator-=(const Vector2i& other)
    {
        x -= other.x;
        y -= other.y;
    }

    constexpr inline bool Vector2i::operator==(const Vector2i& other) const
    {
        return x == other.x && y == other.y;
    }

    constexpr inline bool Vector2i::operator!=(const Vector2i& other) const
    {
        return !operator==(other);
    }

    constexpr inline Int32 Vector2i::dot(const Vector2i& other) const
    {
        return x * other.x + y * other.y;
    }

    constexpr inline void Vector2i::operator*=(const Int32 scalar)
    {
        x *= scalar;
        y *= scalar;
    }

    constexpr inline void Vector2i::operator/=(const Int32 scalar)
    {
        x /= scalar;
        y /= scalar;
    }

    constexpr inline Vector2i Vector2i::operator*(const Int32 scalar) const
    {
        return Vector2i( x * scalar
                        , y * scalar);
    }

    constexpr inline Vector2i Vector2i::operator*(const Vector2i& vec) const
    {
        return Vector2i(x * vec.x, y * vec.y);
    }

    constexpr inline void Vector2i::operator*=(const Vector2i& vec)
    {
        x *= vec.x;
        y *= vec.y;
    }

    constexpr inline
    Int32& Vector2i::operator[](const UInt32 index)
    {
        return data[index];
    }

    constexpr inline
    const Int32& Vector2i::operator[](const UInt32 index) const
    {
        return data[index];
    }

    constexpr inline
    Vector2i Vector2i::operator-() const
    {
        return Vector2i(-x, -y);
    }

    inline
    IOutputStream&
    operator<<(IOutputStream& outputStream, const Vector2i& vector)
    {
        return outputStream.write(vector.data, sizeof(vector.data));
    }

    inline
    IInputStream&
    operator>>(IInputStream& inputStream, Vector2i& vector)
    {
        return inputStream.read(reinterpret_cast<Char*>(vector.data), sizeof(vector.data));
    }

    constexpr inline Vector2i operator*(const Int32 scalar, const Vector2i& vec)
    {
        return Vector2i(vec.x * scalar, vec.y * scalar);
    }
}

template <>
struct fmt::formatter<ramses_internal::Vector2i> : public ramses_internal::SimpleFormatterBase
{
    template<typename FormatContext>
    auto format(const ramses_internal::Vector2i& m, FormatContext& ctx)
    {
        return fmt::format_to(ctx.out(), "[{} {}]", m.data[0], m.data[1]);
    }
};

#endif
