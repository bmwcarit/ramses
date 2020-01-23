//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#ifndef RAMSES_VECTOR2I_H
#define RAMSES_VECTOR2I_H

#include "PlatformAbstraction/PlatformTypes.h"
#include "PlatformAbstraction/PlatformMemory.h"
#include "PlatformAbstraction/PlatformMath.h"

#include "Collections/IInputStream.h"
#include "Collections/IOutputStream.h"
#include "Collections/StringOutputStream.h"
#include "Utils/Warnings.h"

namespace ramses_internal
{
    /**
     *  Allows storing of 2d-vector data and operations
     */
    class Vector2i
    {
    public:

        /**
         * Vector with all values set to 0.0
         */
        static const Vector2i Empty;

        /*
         *  Vector with all values set to 1.0
         */
        static const Vector2i Identity;

        /**
         *  Direct access to vector data in different ways
         */
        union
        {
            /**
             * Vector2i vec;
             * vec.x = 1;
             * vec.y = 2;
             */
            struct
            {
                Int32 x;
                Int32 y;
            };

            /**
            * Vector2i vec;
            * vec.data[0] = 1;
            * vec.data[1] = 2;
             */
            Int32 data[2];
        };

        /**
         *  Default constructor initializes elements with 0.0f
         */
        Vector2i();

        /**
         *  Copy constructor
         * @param other Vector2i to copy from
         */
        Vector2i(const Vector2i& other);

        /**
         *  Constructor to initialize the vector with single values
         * @param _x value of x element
         * @param _y value of y element
         */
        Vector2i(const Int32 _x, const Int32 _y);

        /**
        *  Constructor to initialize the vector with one value
        * @param value for all elements
        */
        explicit Vector2i(const Int32 value);

        /**
         * Sets the vector elements
         * @param x value of x element
         * @param y value of y element
         */
        void set(const Int32 _x, const Int32 _y);

        /**
         * Sets all elements of the vector to the given value
         * @param val for all elements
         */
        void set(const Int32 xy);

        /**
         *  Assignment operator to overwrite vector data with other vector data
         * @param other Vector2i to copy from
         */
        Vector2i& operator=(const Vector2i& other);

        /**
         *  Add operator to add two Vector2i by elements
         * @param other Vector2i with the elements to add
         * @return Vector2i with the result
         */
        Vector2i operator+(const Vector2i& other) const;

        /**
         *  Add and assign operator to add other Vector2i elements to local elements
         * @param other Vector2i with the elements to add
         */
        void operator+=(const Vector2i& other);

        /**
         *  Sub operator to substract two Vector2i by elements
         * @param other Vector2i with the elements to sub
         * @return Vector2i with the result
         */
        Vector2i operator-(const Vector2i& other) const;

        /**
         * Returns the inverse of the Vector
         * @return the inverse of the Vector
         */
        Vector2i operator-() const;

        /**
         *  Sub and assign operator to sub other Vector2i elements from local elements
         * @param other Vector2i with the elements to sub
         */
        void operator-=(const Vector2i& other);

        /**
         *  Operator to multiply a scalar to the vector elements
         * @param the scalar for multiplication
         * @return Vector2i with the multiplied result
         */
        Vector2i operator*(const Int32 scalar) const;

        /**
        *  Operator to multiply a scalar and assign the result to the local vector elements
        * @param the scalar for multiplication
        */
        void operator*=(const Int32 scalar);

        /**
         * Operator to devide a scalar and assign the result to the local vector elements
         * @param the scalar for devision
         */
        void operator/=(const Int32 scalar);

        /**
         * Operator to scale each vector element with the elements of the given vector
         * @param vec Vector2i to scale with
         * @return scaled Vector2i
         */
        Vector2i operator*(const Vector2i& vec) const;

        /**
         * Operator to scale each vector element with the elements of the given vector
         * @param vec Vector2i to scale with
         */
        void operator*=(const Vector2i& vec);

        /**
         * Compares each element to check if two vectors are equal
         * @param other Vector2i to compare to
         * @return true if both vectors are equal false otherwise
         */
        bool operator==(const Vector2i& other) const;

        /**
         * Compares each element to check if two vectors are not equal
         * @param other Vector2i to compare to
         * @return false if both vectors are equal true otherwise
         */
        bool operator!=(const Vector2i& other) const;

        /**
         * Returns the element of the given index
         * @return the element of the given index
         */
        Int32& operator[](const UInt32 index);

        /**
         * Returns the element of the given index
         * @return the element of the given index
         */
        const Int32& operator[](const UInt32 index) const;


        /**
         *  Computes the dot product of two Vector2i
         * @other Vector2i for the computation of the dot product
         * @return the resulting dot product
         */
        Int32 dot(const Vector2i& other) const;

        /**
         *  Computes the euclidean length of the vector
         * @return the euclidean length of the vector
         */
        Float length() const;

        /**
         *  Computes the angle between two vectors in radians.
         * @param other Vector2i to compute the angle with
         * @return the angle in radians between the vectors
         */
        Float angle(const Vector2i& other) const;

        friend Vector2i operator*(const Int32 scalar, const Vector2i&);

    protected:
    private:
    };

    inline Vector2i::Vector2i()
    {
        PlatformMemory::Set(data, 0, sizeof(data));
    }

    inline Vector2i::Vector2i(const Int32 _x, const Int32 _y)
        : x(_x)
        , y(_y)
    {
    }

    inline Vector2i::Vector2i(const Vector2i& other)
    {
        PlatformMemory::Copy(data, other.data, sizeof(data));
    }

    inline Vector2i::Vector2i(const Int32 xy)
        : x(xy)
        , y(xy)
    {
    }

    inline void Vector2i::set(const Int32 _x, const Int32 _y)
    {
        x = _x;
        y = _y;
    }

    inline void Vector2i::set(const Int32 xy)
    {
        x = y = xy;
    }

    inline Float Vector2i::length() const
    {
        return std::sqrt(static_cast<Float>(x*x + y*y));
    }

    inline Vector2i& Vector2i::operator=(const Vector2i& other)
    {
        PlatformMemory::Copy(data, other.data, sizeof(data));
        return *this;
    }

    inline Vector2i Vector2i::operator+(const Vector2i& other) const
    {
        return Vector2i( x + other.x
                        , y + other.y);
    }

    inline void Vector2i::operator+=(const Vector2i& other)
    {
        x += other.x;
        y += other.y;
    }

    inline Vector2i Vector2i::operator-(const Vector2i& other) const
    {
        return Vector2i( x - other.x
                        , y - other.y);
    }

    inline void Vector2i::operator-=(const Vector2i& other)
    {
        x -= other.x;
        y -= other.y;
    }

    inline bool Vector2i::operator==(const Vector2i& other) const
    {
        return PlatformMemory::Compare(data, other.data, sizeof(data)) == 0;
    }

    inline bool Vector2i::operator!=(const Vector2i& other) const
    {
        return !operator==(other);
    }

    inline Int32 Vector2i::dot(const Vector2i& other) const
    {
        return x * other.x + y * other.y;
    }

    inline void Vector2i::operator*=(const Int32 scalar)
    {
        x *= scalar;
        y *= scalar;
    }

    inline void Vector2i::operator/=(const Int32 scalar)
    {
        x /= scalar;
        y /= scalar;
    }

    inline Vector2i Vector2i::operator*(const Int32 scalar) const
    {
        return Vector2i( x * scalar
                        , y * scalar);
    }

    inline Vector2i Vector2i::operator*(const Vector2i& vec) const
    {
        return Vector2i(x * vec.x, y * vec.y);
    }

    inline void Vector2i::operator*=(const Vector2i& vec)
    {
        x *= vec.x;
        y *= vec.y;
    }

    inline Float Vector2i::angle(const Vector2i& other) const
    {
        return std::acos(dot(other) / (length() * other.length()));
    }

    inline
    Int32& Vector2i::operator[](const UInt32 index)
    {
        return data[index];
    }

    inline
    const Int32& Vector2i::operator[](const UInt32 index) const
    {
        return data[index];
    }

    inline
    Vector2i Vector2i::operator-() const
    {
        return Vector2i(-x, -y);
    }

    inline
    IOutputStream&
    operator<<(IOutputStream& outputStream, const Vector2i& vector)
    {
        return outputStream.write(reinterpret_cast<const Char*>(vector.data), sizeof(vector.data));
    }

    inline
    IInputStream&
    operator>>(IInputStream& inputStream, Vector2i& vector)
    {
        return inputStream.read(reinterpret_cast<Char*>(vector.data), sizeof(vector.data));
    }

    inline
    StringOutputStream& operator<<(StringOutputStream& outputStream, const Vector2i& vec4)
    {
        return outputStream << vec4.x << "/" << vec4.y;
    }

    inline Vector2i operator*(const Int32 scalar, const Vector2i& vec)
    {
        return Vector2i(vec.x * scalar, vec.y * scalar);
    }
}

#endif
