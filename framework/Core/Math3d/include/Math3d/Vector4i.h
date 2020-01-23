//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_VECTOR4I_H
#define RAMSES_VECTOR4I_H

#include "PlatformAbstraction/PlatformTypes.h"
#include "PlatformAbstraction/PlatformMemory.h"
#include "PlatformAbstraction/PlatformMath.h"
#include "Math3d/Vector3i.h"

namespace ramses_internal
{
    /**
    *   Allows storing of 4d-vector data and operations
    */
    class Vector4i
    {
    public:

        /**
         * Vector with all values set to 0
         */
        static const Vector4i Empty;

        /**
        *   Direct access to vector data in different ways
        */
        union
        {
            /**
            * Vector4i vec;
            * vec.x = 1;
            * vec.y = 2;
            * vec.z = 3;
            * vec.w = 4;
            */
            struct
            {
                Int32 x;
                Int32 y;
                Int32 z;
                Int32 w;
            };

            /**
            * Vector4i vec;
            * vec.data[0] = 1;
            * vec.data[1] = 2;
            * vec.data[2] = 3;
            * vec.data[3] = 4;
            */
            Int32 data[4];
        };

        /**
         *   Default constructor initializes elements with 0.0f
         */
        Vector4i();

        /**
        *   Copy constructor
        * @param other Vector4i to copy from
        */
        Vector4i(const Vector4i& other);

        /**
        *   Constructor to initialize the vector with single values
        * @param _x value of x element
        * @param _y value of y element
        * @param _z value of z element
        * @param _w value of w element
        */
        Vector4i(Int32 _x, Int32 _y, Int32 _z, Int32 _w);

        /**
         * Constructor to initialize the vector one value
         * @param value for all elements
         */
        explicit Vector4i(const Int32 value);

        /**
         * Constructs a Vector4i from Vector3. initializes the w component with 1.0
         * @param vector for construction
         */
        explicit Vector4i(const Vector3i& other);

        /**
         * Sets the vector elements
         * @param x value of x element
         * @param y value of y element
         * @param z value of z element
         * @param z value of w element
         */
        void set(Int32 _x, Int32 _y, Int32 _z, Int32 _w);

        /**
         * Sets all elements of the vector to the given value
         * @param val for all elements
         */
        void set(Int32 xyzw);

        /**
        *   Assignment operator to overwrite vector data with other vector data
        * @param other Vector4i to copy from
        */
        Vector4i& operator=(const Vector4i& other);

        /**
        *   Add operator to add two Vector4i by elements
        * @param other Vector4i with the elements to add
        * @return Vector3 with the result
        */
        Vector4i operator+(const Vector4i& other) const;

        /**
        *   Add and assign operator to add other Vector4i elements to local elements
        * @param other Vector4i with the elements to add
        */
        void operator+=(const Vector4i& other);

        /**
        *   Sub operator to substract two Vector4i by elements
        * @param other Vector4i with the elements to sub
        * @return Vector4i with the result
        */
        Vector4i operator-(const Vector4i& other) const;

        /**
        *   Sub and assign operator to sub other Vector4i elements from local elements
        * @param other Vector4i with the elements to sub
        */
        void operator-=(const Vector4i& other);

        /**
        *   Operator to multiply a scalar to the vector elements
        * @param the scalar for multiplication
        * @return Vector4i with the multiplied result
        */
        Vector4i operator*(const Int32 scalar) const;

        Vector4i operator/(const Vector4i& scalar) const;

        /**
        *   Operator to multiply a scalar and assign the result to the local vector elements
        * @param the scalar for multiplication
        */
        void operator*=(const Int32 scalar);

        /**
        * Operator to scale each vector element with the elements of the given vector
        * @param vec Vector4i to scale with
        * @return scaled Vector3
        */
        Vector4i operator*(const Vector4i& vec) const;

        /**
         * Operator to scale each vector element with the elements of the given vector
         * @param vec Vector4i to scale with
         */
        void operator*=(const Vector4i& vec);

        /**
         * Compares each element to check if two vectors are equal
         * @param other Vector3 to compare to
         * @return true if both vectors are equal false otherwise
         */
        bool operator==(const Vector4i& other) const;

        /**
         * Compares each element to check if two vectors are not equal
         * @param other Vector3 to compare to
         * @return false if both vectors are equal true otherwise
         */
        bool operator!=(const Vector4i& other) const;

        /**
        *   Computes the dot product of two Vector4i
        * @other Vector4i for the computation of the dot product
        * @return the resulting dot product
        */
        Int32 dot(const Vector4i& other) const;

        /**
        *   Computes the cross product of two Vector4i
        * @other Vector4i for the computation of the cross product
        * @return Vector4i with the result of the cross product
        */
        Vector4i cross(const Vector4i& other) const;

        /**
        *   Computes the euclidean length of the vector
        * @return the euclidean length of the vector
        */
        Float length() const;

        /**
        *   Computes the angle between two vectors in radians.
        * @param other Vector4i to compute the angle with
        * @return the angle in radians between the vectors
        */
        Float angle(const Vector4i& other) const;

        /**
        * Returns the element of the given index
        * @return the element of the given index
        */
        Int32& operator[](const UInt32 index);

        const Int32& operator[](const UInt32 index) const;

        friend Vector4i operator*(const Int32 scalar, const Vector4i&);

    protected:
    private:
    };

    inline Vector4i::Vector4i()
    {
        PlatformMemory::Set(data, 0, sizeof(data));
    }

    inline Vector4i::Vector4i(Int32 _x, Int32 _y, Int32 _z, Int32 _w)
        : x(_x)
        , y(_y)
        , z(_z)
        , w(_w)
    {
    }

    inline Vector4i::Vector4i(const Vector4i& other)
    {
        PlatformMemory::Copy(data, other.data, sizeof(data));
    }

    inline
        Vector4i::Vector4i(const Int32 value)
        : x(value)
        , y(value)
        , z(value)
        , w(value)
    {
    }

    inline Vector4i::Vector4i(const Vector3i& other)
    {
        PlatformMemory::Copy(data, other.data, sizeof(other.data));
        w = 255;
    }

    inline void Vector4i::set(Int32 _x, Int32 _y, Int32 _z, Int32 _w)
    {
        x = _x;
        y = _y;
        z = _z;
        w = _w;
    }

    inline void Vector4i::set(Int32 xyzw)
    {
        x = y = z = w = xyzw;
    }

    inline Float Vector4i::length() const
    {
        return std::sqrt(static_cast<Float>(x*x + y*y + z*z + w*w));
    }

    inline Vector4i& Vector4i::operator=(const Vector4i& other)
    {
        if (&other != this)
        {
            PlatformMemory::Copy(data, other.data, sizeof(data));
        }
        return *this;
    }

    inline Vector4i Vector4i::operator+(const Vector4i& other) const
    {
        return Vector4i(x + other.x
            , y + other.y
            , z + other.z
            , w + other.w);
    }

    inline void Vector4i::operator+=(const Vector4i& other)
    {
        x += other.x;
        y += other.y;
        z += other.z;
        w += other.w;
    }

    inline Vector4i Vector4i::operator-(const Vector4i& other) const
    {
        return Vector4i(x - other.x
            , y - other.y
            , z - other.z
            , w - other.w);
    }

    inline void Vector4i::operator-=(const Vector4i& other)
    {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        w -= other.w;
    }

    inline Vector4i Vector4i::operator*(const Vector4i& vec) const
    {
        return Vector4i(x * vec.x, y * vec.y, z * vec.z, w * vec.w);
    }

    inline void Vector4i::operator*=(const Vector4i& vec)
    {
        x *= vec.x;
        y *= vec.y;
        z *= vec.z;
        w *= vec.w;
    }

    inline bool Vector4i::operator==(const Vector4i& other) const
    {
        return PlatformMemory::Compare(data, other.data, sizeof(data)) == 0;
    }

    inline bool Vector4i::operator!=(const Vector4i& other) const
    {
        return !operator==(other);
    }

    inline Int32 Vector4i::dot(const Vector4i& other) const
    {
        return x * other.x + y * other.y + z * other.z + w * other.w;
    }

    inline Vector4i Vector4i::cross(const Vector4i& other) const
    {
        return Vector4i(y * other.z - z * other.y + z * other.w - w * other.z + y * other.w - w * other.y
            , z * other.x - x * other.z + w * other.x - x * other.w + z * other.w - w * other.z
            , x * other.y - y * other.x + w * other.y - y * other.w + w * other.x - x * other.w
            , x * other.y - y * other.x + y * other.z - z * other.y + x * other.z - z * other.x
            );
    }

    inline void Vector4i::operator*=(const Int32 scalar)
    {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        w *= scalar;
    }

    inline Vector4i Vector4i::operator*(const Int32 scalar) const
    {
        return Vector4i(x * scalar
            , y * scalar
            , z * scalar
            , w * scalar);
    }

    inline Vector4i Vector4i::operator/(const Vector4i& other) const
    {
        return Vector4i(x / other.x
            , y / other.y
            , z / other.z
            , w / other.w);
    }

    inline Float Vector4i::angle(const Vector4i& other) const
    {
        return std::acos(dot(other) / (length() * other.length()));
    }

    inline
        IOutputStream&
        operator<<(IOutputStream& outputStream, const Vector4i& vector)
    {
            return outputStream.write(reinterpret_cast<const Char*>(vector.data), sizeof(vector.data));
    }

    inline
        IInputStream&
        operator>>(IInputStream& inputStream, Vector4i& vector)
    {
            return inputStream.read(reinterpret_cast<Char*>(vector.data), sizeof(vector.data));
    }

    inline
        StringOutputStream& operator<<(StringOutputStream& outputStream, const Vector4i& vec4)
    {
            return outputStream << vec4.x << "/" << vec4.y << "/" << vec4.z << "/" << vec4.w;
    }

    inline
        Int32& Vector4i::operator[](const UInt32 index)
    {
            return data[index];
    }

    inline
        const Int32& Vector4i::operator[](const UInt32 index) const
    {
            return data[index];
    }

    inline
        Vector4i operator*(const Int32 scalar, const Vector4i& vec)
    {
        return Vector4i(vec.x * scalar
            , vec.y * scalar
            , vec.z * scalar
            , vec.w * scalar);
    }
}

#endif
