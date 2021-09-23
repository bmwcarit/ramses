//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#ifndef RAMSES_VECTOR2_H
#define RAMSES_VECTOR2_H

#include "Collections/IInputStream.h"
#include "Collections/IOutputStream.h"
#include "PlatformAbstraction/FmtBase.h"
#include "Utils/AssertMovable.h"

namespace ramses_internal
{
    class Vector2
    {
    public:
        union
        {
            struct
            {
                Float x;
                Float y;
            };
            Float data[2];
        };

        constexpr Vector2();
        constexpr Vector2(const Float _x, const Float _y);
        explicit constexpr Vector2(const Float xy);

        constexpr Vector2(const Vector2& other) = default;
        constexpr Vector2& operator=(const Vector2& other) = default;

        constexpr void set(const Float _x, const Float _y);
        constexpr void set(const Float xy);

        constexpr Vector2 operator+(const Vector2& other) const;
        constexpr Vector2 operator-(const Vector2& other) const;
        constexpr void operator+=(const Vector2& other);
        constexpr void operator-=(const Vector2& other);

        constexpr Vector2 operator-() const;

        constexpr Vector2 operator*(const Float scalar) const;
        constexpr Vector2 operator*(const Vector2& vec) const;
        constexpr void operator*=(const Float scalar);
        constexpr void operator*=(const Vector2& vec);
        constexpr void operator/=(const Float scalar);

        constexpr bool operator==(const Vector2& other) const;
        constexpr bool operator!=(const Vector2& other) const;

        constexpr Float& operator[](const UInt32 index);
        constexpr const Float& operator[](const UInt32 index) const;

        constexpr Float dot(const Vector2& other) const;
        Float length() const;
        Vector2 normalize() const;

        friend constexpr Vector2 operator*(const Float scalar, const Vector2&);
    };

    constexpr inline Vector2::Vector2()
        : x(0)
        , y(0)
    {
    }

    constexpr inline Vector2::Vector2(const Float _x, const Float _y)
        : x(_x)
        , y(_y)
    {
    }

    constexpr inline Vector2::Vector2(const Float xy)
        : x(xy)
        , y(xy)
    {
    }

    constexpr inline void Vector2::set(const Float _x, const float _y)
    {
        x = _x;
        y = _y;
    }

    constexpr inline void Vector2::set(const Float xy)
    {
        x = y = xy;
    }

    inline Float Vector2::length() const
    {
        return std::sqrt(x*x + y*y);
    }

    constexpr inline Vector2 Vector2::operator+(const Vector2& other) const
    {
        return Vector2( x + other.x
                        , y + other.y);
    }

    constexpr inline void Vector2::operator+=(const Vector2& other)
    {
        x += other.x;
        y += other.y;
    }

    constexpr inline Vector2 Vector2::operator-(const Vector2& other) const
    {
        return Vector2(x - other.x, y - other.y);
    }

    constexpr inline void Vector2::operator-=(const Vector2& other)
    {
        x -= other.x;
        y -= other.y;
    }

    constexpr inline bool Vector2::operator==(const Vector2& other) const
    {
        return x == other.x && y == other.y;
    }

    constexpr inline bool Vector2::operator!=(const Vector2& other) const
    {
        return !operator==(other);
    }

    constexpr inline Float Vector2::dot(const Vector2& other) const
    {
        return x * other.x + y * other.y;
    }

    constexpr inline void Vector2::operator*=(const Float scalar)
    {
        x *= scalar;
        y *= scalar;
    }

    constexpr inline void Vector2::operator/=(const Float scalar)
    {
        x /= scalar;
        y /= scalar;
    }

    constexpr inline Vector2 Vector2::operator*(const Float scalar) const
    {
        return Vector2(x * scalar, y * scalar);
    }

    constexpr inline Vector2 Vector2::operator*(const Vector2& vec) const
    {
        return Vector2(x * vec.x, y * vec.y);
    }

    constexpr inline void Vector2::operator*=(const Vector2& vec)
    {
        x *= vec.x;
        y *= vec.y;
    }

    inline
    Vector2
    Vector2::normalize() const
    {
        const Float len = length();
        return Vector2(x / len, y / len);
    }

    constexpr inline
    Float& Vector2::operator[](const UInt32 index)
    {
        return data[index];
    }

    constexpr inline
    const Float& Vector2::operator[](const UInt32 index) const
    {
        return data[index];
    }

    constexpr inline
    Vector2 Vector2::operator-() const
    {
        return Vector2(-x, -y);
    }

    inline
    IOutputStream&
    operator<<(IOutputStream& outputStream, const Vector2& vector)
    {
        return outputStream.write(vector.data, sizeof(vector.data));
    }

    inline
    IInputStream&
    operator>>(IInputStream& inputStream, Vector2& vector)
    {
        return inputStream.read(vector.data, sizeof(vector.data));
    }

    constexpr inline Vector2 operator*(const Float scalar, const Vector2& vec)
    {
        return Vector2(vec.x * scalar, vec.y * scalar);
    }

    ASSERT_MOVABLE(Vector2)
}

template <>
struct fmt::formatter<ramses_internal::Vector2> : public ramses_internal::SimpleFormatterBase
{
    template<typename FormatContext>
    constexpr auto format(const ramses_internal::Vector2& m, FormatContext& ctx)
    {
        return fmt::format_to(ctx.out(), "[{} {}]", m.data[0], m.data[1]);
    }
};

#endif
