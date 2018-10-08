//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#ifndef RAMSES_VECTOR2_H
#define RAMSES_VECTOR2_H

#include <PlatformAbstraction/PlatformTypes.h>
#include <PlatformAbstraction/PlatformMemory.h>
#include <PlatformAbstraction/PlatformMath.h>

#include "Collections/IInputStream.h"
#include "Collections/IOutputStream.h"
#include "Collections/StringOutputStream.h"
#include "Utils/Warnings.h"

namespace ramses_internal
{
    /**
     *  Allows storing of 2d-vector data and operations
     */
    class Vector2
    {
    public:

        /**
         * Vector with all values set to 0.0
         */
        static const Vector2 Empty;

        /*
         *  Vector with all values set to 1.0
         */
        static const Vector2 Identity;

        /**
         *  Direct access to vector data in different ways
         */
        union
        {
            /**
             * Vector2 vec;
             * vec.x = 1;
             * vec.y = 2;
             */
            IGNORE_UNNAMED_MEMBER_WARNING_START
            struct
            {
                Float x;
                Float y;
            };
            IGNORE_UNNAMED_MEMBER_WARNING_END
            /**
            * Vector2 vec;
            * vec.data[0] = 1;
            * vec.data[1] = 2;
             */
            Float data[2];
        };

        /**
         *  Default constructor initializes elements with 0.0f
         */
        Vector2();

        /**
         *  Copy constructor
         * @param other Vector2 to copy from
         */
        Vector2(const Vector2& other) = default;

        /**
         *  Constructor to initialize the vector with single values
         * @param _x value of x element
         * @param _y value of y element
         */
        Vector2(const Float _x, const Float _y);

        Vector2(Vector2&& other) = default;
        Vector2& operator=(Vector2&& other) = default;

        /**
        *  Constructor to initialize the vector with one value
        * @param value for all elements
        */
        explicit Vector2(const Float value);

        /**
         * Sets the vector elements
         * @param x value of x element
         * @param y value of y element
         */
        void set(const Float _x, const Float _y);

        /**
         * Sets all elements of the vector to the given value
         * @param val for all elements
         */
        void set(const Float xy);

        /**
         *  Assignment operator to overwrite vector data with other vector data
         * @param other Vector2 to copy from
         */
        Vector2& operator=(const Vector2& other);

        /**
         *  Add operator to add two Vector2 by elements
         * @param other Vector2 with the elements to add
         * @return Vector2 with the result
         */
        Vector2 operator+(const Vector2& other) const;

        /**
         *  Add and assign operator to add other Vector2 elements to local elements
         * @param other Vector2 with the elements to add
         */
        void operator+=(const Vector2& other);

        /**
         *  Sub operator to substract two Vector2 by elements
         * @param other Vector2 with the elements to sub
         * @return Vector2 with the result
         */
        Vector2 operator-(const Vector2& other) const;

        /**
         * Returns the inverse of the Vector
         * @return the inverse of the Vector
         */
        Vector2 operator-() const;

        /**
         *  Sub and assign operator to sub other Vector2 elements from local elements
         * @param other Vector2 with the elements to sub
         */
        void operator-=(const Vector2& other);

        /**
         *  Operator to multiply a scalar to the vector elements
         * @param the scalar for multiplication
         * @return Vector2 with the multiplied result
         */
        Vector2 operator*(const Float scalar) const;

        /**
        *  Operator to multiply a scalar and assign the result to the local vector elements
        * @param the scalar for multiplication
        */
        void operator*=(const Float scalar);

        /**
         * Operator to devide a scalar and assign the result to the local vector elements
         * @param the scalar for devision
         */
        void operator/=(const Float scalar);

        /**
         * Operator to scale each vector element with the elements of the given vector
         * @param vec Vector2 to scale with
         * @return scaled Vector2
         */
        Vector2 operator*(const Vector2& vec) const;

        /**
         * Operator to scale each vector element with the elements of the given vector
         * @param vec Vector2 to scale with
         */
        void operator*=(const Vector2& vec);

        /**
         * Compares each element to check if two vectors are equal
         * @param other Vector2 to compare to
         * @return true if both vectors are equal false otherwise
         */
        Bool operator==(const Vector2& other) const;

        /**
         * Compares each element to check if two vectors are not equal
         * @param other Vector2 to compare to
         * @return false if both vectors are equal true otherwise
         */
        Bool operator!=(const Vector2& other) const;

        /**
         * Returns the element of the given index
         * @return the element of the given index
         */
        Float& operator[](const UInt32 index);

        /**
         * Returns the element of the given index
         * @return the element of the given index
         */
        const Float& operator[](const UInt32 index) const;


        /**
         *  Computes the dot product of two Vector2
         * @other Vector2 for the computation of the dot product
         * @return the resulting dot product
         */
        Float dot(const Vector2& other) const;

        /**
         *  Computes the euclidean length of the vector
         * @return the euclidean length of the vector
         */
        Float length() const;

        /**
         *  Computes the angle between two vectors in radians.
         * @param other Vector2 to compute the angle with
         * @return the angle in radians between the vectors
         */
        Float angle(const Vector2& other) const;

        /**
         * Normalizes the vector to the length of 1
         * @return Vector2 normalized vector
         */
        Vector2 normalize() const;

        friend Vector2 operator*(const Float scalar, const Vector2&);

    protected:
    private:
    };

    inline Vector2::Vector2()
    {
        PlatformMemory::Set(data, 0, sizeof(data));
    }

    inline Vector2::Vector2(const Float _x, const Float _y)
        : x(_x)
        , y(_y)
    {
    }

    inline Vector2::Vector2(const Float xy)
        : x(xy)
        , y(xy)
    {
    }

    inline void Vector2::set(const Float _x, const float _y)
    {
        x = _x;
        y = _y;
    }

    inline void Vector2::set(const Float xy)
    {
        x = y = xy;
    }

    inline Float Vector2::length() const
    {
        return PlatformMath::Sqrt(PlatformMath::Pow2(x) + PlatformMath::Pow2(y));
    }

    inline Vector2& Vector2::operator=(const Vector2& other)
    {
        PlatformMemory::Copy(data, other.data, sizeof(data));
        return *this;
    }

    inline Vector2 Vector2::operator+(const Vector2& other) const
    {
        return Vector2( x + other.x
                        , y + other.y);
    }

    inline void Vector2::operator+=(const Vector2& other)
    {
        x += other.x;
        y += other.y;
    }

    inline Vector2 Vector2::operator-(const Vector2& other) const
    {
        return Vector2( x - other.x
                        , y - other.y);
    }

    inline void Vector2::operator-=(const Vector2& other)
    {
        x -= other.x;
        y -= other.y;
    }

    inline Bool Vector2::operator==(const Vector2& other) const
    {
        return PlatformMemory::Compare(data, other.data, sizeof(data)) == 0;
    }

    inline Bool Vector2::operator!=(const Vector2& other) const
    {
        return !operator==(other);
    }

    inline Float Vector2::dot(const Vector2& other) const
    {
        return x * other.x + y * other.y;
    }

    inline void Vector2::operator*=(const Float scalar)
    {
        x *= scalar;
        y *= scalar;
    }

    inline void Vector2::operator/=(const Float scalar)
    {
        x /= scalar;
        y /= scalar;
    }

    inline Vector2 Vector2::operator*(const Float scalar) const
    {
        return Vector2( x * scalar
                        , y * scalar);
    }

    inline Vector2 Vector2::operator*(const Vector2& vec) const
    {
        return Vector2(x * vec.x, y * vec.y);
    }

    inline void Vector2::operator*=(const Vector2& vec)
    {
        x *= vec.x;
        y *= vec.y;
    }

    inline Float Vector2::angle(const Vector2& other) const
    {
        return PlatformMath::ArcCos(dot(other) / (length() * other.length()));
    }

    inline
    Vector2
    Vector2::normalize() const
    {
        const Float len = length();
        return Vector2(x / len, y / len);
    }

    inline
    Float& Vector2::operator[](const UInt32 index)
    {
        return data[index];
    }

    inline
    const Float& Vector2::operator[](const UInt32 index) const
    {
        return data[index];
    }

    inline
    Vector2 Vector2::operator-() const
    {
        return Vector2(-x, -y);
    }

    inline
    IOutputStream&
    operator<<(IOutputStream& outputStream, const Vector2& vector)
    {
        return outputStream.write(reinterpret_cast<const Char*>(vector.data), sizeof(vector.data));
    }

    inline
    IInputStream&
    operator>>(IInputStream& inputStream, Vector2& vector)
    {
        return inputStream.read(reinterpret_cast<Char*>(vector.data), sizeof(vector.data));
    }

    inline
    StringOutputStream& operator<<(StringOutputStream& outputStream, const Vector2& vec4)
    {
        return outputStream << vec4.x << "/" << vec4.y;
    }

    inline Vector2 operator*(const Float scalar, const Vector2& vec)
    {
        return Vector2(vec.x * scalar, vec.y * scalar);
    }

    static_assert(std::is_nothrow_move_constructible<Vector2>::value &&
        std::is_nothrow_move_assignable<Vector2>::value, "Vector2 must be movable");
}

#endif
