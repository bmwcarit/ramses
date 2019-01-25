//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#ifndef RAMSES_VECTOR3I_H
#define RAMSES_VECTOR3I_H

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
     *  Allows storing of 3d-vector data and operations
     */
    class Vector3i
    {
    public:

        /**
         * Vector with all values set to 0.0
         */
        static const Vector3i Empty;

        /*
         *  Vector with all values set to 1.0
         */
        static const Vector3i Identity;

        /**
         *  Direct access to vector data in different ways
         */
        union
        {
            /**
             * Vector3i vec;
             * vec.x = 1;
             * vec.y = 2;
             * vec.z = 3;
             */
            struct
            {
                Int32 x;
                Int32 y;
                Int32 z;
            };

            /**
            * Vector3i vec;
            * vec.data[0] = 1;
            * vec.data[1] = 2;
            * vec.data[2] = 3;
             */
            Int32 data[3];
        };

        /**
         *  Default constructor initializes elements with 0.0f
         */
        Vector3i();

        /**
         *  Copy constructor
         * @param other Vector3i to copy from
         */
        Vector3i(const Vector3i& other);

        /**
         *  Constructor to initialize the vector with single values
         * @param _x value of x element
         * @param _y value of y element
         * @param _z value of z element
         */
        Vector3i(const Int32 _x, const Int32 _y, const Int32 _z);

        /**
         *  Constructor to initialize the vector with one value
         * @param value for all elements
         */
        explicit Vector3i(const Int32 value);

        /**
         * Sets the vector elements
         * @param x value of x element
         * @param y value of y element
         * @param z value of z element
         */
        void set(const Int32 _x, const Int32 _y, const Int32 _z);

        /**
         * Sets all elements of the vector to the given value
         * @param val for all elements
         */
        void set(const Int32 xyz);

        /**
         *  Assignment operator to overwrite vector data with other vector data
         * @param other Vector3i to copy from
         */
        Vector3i& operator=(const Vector3i& other);

        /**
         *  Add operator to add two Vector3i by elements
         * @param other Vector3i with the elements to add
         * @return Vector3i with the result
         */
        Vector3i operator+(const Vector3i& other) const;

        /**
         *  Add and assign operator to add other Vector3i elements to local elements
         * @param other Vector3i with the elements to add
         */
        void operator+=(const Vector3i& other);

        /**
         *  Sub operator to substract two Vector3i by elements
         * @param other Vector3i with the elements to sub
         * @return Vector3i with the result
         */
        Vector3i operator-(const Vector3i& other) const;

        /**
         * Returns the inverse of the Vector
         * @return the inverse of the Vector
         */
        Vector3i operator-() const;

        /**
         *  Sub and assign operator to sub other Vector3i elements from local elements
         * @param other Vector3i with the elements to sub
         */
        void operator-=(const Vector3i& other);

        /**
         *  Operator to multiply a scalar to the vector elements
         * @param the scalar for multiplication
         * @return Vector3i with the multiplied result
         */
        Vector3i operator*(const Int32 scalar) const;

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
         * @param vec Vector3i to scale with
         * @return scaled Vector3i
         */
        Vector3i operator*(const Vector3i& vec) const;

        /**
         * Operator to scale each vector element with the elements of the given vector
         * @param vec Vector3i to scale with
         */
        void operator*=(const Vector3i& vec);

        /**
         * Compares each element to check if two vectors are equal
         * @param other Vector3i to compare to
         * @return true if both vectors are equal false otherwise
         */
        Bool operator==(const Vector3i& other) const;

        /**
         * Compares each element to check if two vectors are not equal
         * @param other Vector3i to compare to
         * @return false if both vectors are equal true otherwise
         */
        Bool operator!=(const Vector3i& other) const;

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
         *  Computes the dot product of two Vector3i
         * @other Vector3i for the computation of the dot product
         * @return the resulting dot product
         */
        Int32 dot(const Vector3i& other) const;

        /**
         *  Computes the cross product of two Vector3i
         * @other Vector3i for the computation of the cross product
         * @return Vector3i with the result of the cross product
         */
        Vector3i cross(const Vector3i& other) const;

        /**
         *  Computes the euclidean length of the vector
         * @return the euclidean length of the vector
         */
        Float length() const;

        /**
         *  Computes the angle between two vectors in radians.
         * @param other Vector3i to compute the angle with
         * @return the angle in radians between the vectors
         */
        Float angle(const Vector3i& other) const;

        String toString() const;

        friend Vector3i operator*(const Int32 scalar, const Vector3i&);

    protected:
    private:
    };

    inline Vector3i::Vector3i()
    {
        PlatformMemory::Set(data, 0, sizeof(data));
    }

    inline
    Vector3i::Vector3i(const Int32 value)
    : x(value)
    , y(value)
    , z(value)
    {

    }

    inline Vector3i::Vector3i(const Int32 _x, const Int32 _y, const Int32 _z)
        : x(_x)
        , y(_y)
        , z(_z)
    {
    }

    inline Vector3i::Vector3i(const Vector3i& other)
    {
        PlatformMemory::Copy(data, other.data, sizeof(data));
    }

    inline void Vector3i::set(const Int32 _x, const Int32 _y, const Int32 _z)
    {
        x = _x;
        y = _y;
        z = _z;
    }

    inline void Vector3i::set(const Int32 xyz)
    {
        x = y = z = xyz;
    }

    inline Float Vector3i::length() const
    {
        return PlatformMath::Sqrt(static_cast<Float>(x*x + y*y + z*z));
    }

    inline Vector3i& Vector3i::operator=(const Vector3i& other)
    {
        PlatformMemory::Copy(data, other.data, sizeof(data));
        return *this;
    }

    inline Vector3i Vector3i::operator+(const Vector3i& other) const
    {
        return Vector3i( x + other.x
                        , y + other.y
                        , z + other.z);
    }

    inline void Vector3i::operator+=(const Vector3i& other)
    {
        x += other.x;
        y += other.y;
        z += other.z;
    }

    inline Vector3i Vector3i::operator-(const Vector3i& other) const
    {
        return Vector3i( x - other.x
                        , y - other.y
                        , z - other.z);
    }

    inline void Vector3i::operator-=(const Vector3i& other)
    {
        x -= other.x;
        y -= other.y;
        z -= other.z;
    }

    inline Bool Vector3i::operator==(const Vector3i& other) const
    {
        return PlatformMemory::Compare(data, other.data, sizeof(data)) == 0;
    }

    inline Bool Vector3i::operator!=(const Vector3i& other) const
    {
        return !operator==(other);
    }

    inline Int32 Vector3i::dot(const Vector3i& other) const
    {
        return x * other.x + y * other.y + z * other.z;
    }

    inline Vector3i Vector3i::cross(const Vector3i& other) const
    {
        return Vector3i( y * other.z - z * other.y
                        , z * other.x - x * other.z
                        , x * other.y - y * other.x);
    }

    inline void Vector3i::operator*=(const Int32 scalar)
    {
        x *= scalar;
        y *= scalar;
        z *= scalar;
    }

    inline void Vector3i::operator/=(const Int32 scalar)
    {
        x /= scalar;
        y /= scalar;
        z /= scalar;
    }

    inline Vector3i Vector3i::operator*(const Int32 scalar) const
    {
        return Vector3i( x * scalar
                        , y * scalar
                        , z * scalar);
    }

    inline Vector3i Vector3i::operator*(const Vector3i& vec) const
    {
        return Vector3i(x * vec.x, y * vec.y, z * vec.z);
    }

    inline void Vector3i::operator*=(const Vector3i& vec)
    {
        x *= vec.x;
        y *= vec.y;
        z *= vec.z;
    }

    inline Float Vector3i::angle(const Vector3i& other) const
    {
        return PlatformMath::ArcCos(dot(other) / (length() * other.length()));
    }

    inline
    Int32& Vector3i::operator[](const UInt32 index)
    {
        return data[index];
    }

    inline
    const Int32& Vector3i::operator[](const UInt32 index) const
    {
        return data[index];
    }

    inline
    Vector3i Vector3i::operator-() const
    {
        return Vector3i(-x, -y, -z);
    }


    inline
    IOutputStream&
    operator<<(IOutputStream& outputStream, const Vector3i& vector)
    {
        return outputStream.write(reinterpret_cast<const Char*>(vector.data), sizeof(vector.data));
    }

    inline
    IInputStream&
    operator>>(IInputStream& inputStream, Vector3i& vector)
    {
        return inputStream.read(reinterpret_cast<Char*>(vector.data), sizeof(vector.data));
    }

    inline
        StringOutputStream& operator<<(StringOutputStream& outputStream, const Vector3i& vec3)
    {
        return outputStream << vec3.x << "/" << vec3.y << "/" << vec3.z;
    }



    inline
        String Vector3i::toString() const
    {
        StringOutputStream s;
        s << *this;
        return s.c_str();
    }

    inline Vector3i operator*(const Int32 scalar, const Vector3i& vec)
    {
        return Vector3i(vec.x * scalar
            , vec.y * scalar
            , vec.z * scalar);
    }
}

#endif
