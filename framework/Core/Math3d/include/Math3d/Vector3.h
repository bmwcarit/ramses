//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#ifndef RAMSES_VECTOR3_H
#define RAMSES_VECTOR3_H

#include <PlatformAbstraction/PlatformTypes.h>
#include <PlatformAbstraction/PlatformMemory.h>
#include <PlatformAbstraction/PlatformMath.h>

#include "Collections/IInputStream.h"
#include "Collections/IOutputStream.h"
#include "Collections/StringOutputStream.h"
#include "Utils/Warnings.h"

namespace ramses_internal
{
    class Vector4;

    /**
     *  Allows storing of 3d-vector data and operations
     */
    class Vector3
    {
    public:

        /**
         * Vector with all values set to 0.0
         */
        static const Vector3 Empty;

        /*
         *  Vector with all values set to 1.0
         */
        static const Vector3 Identity;

        /**
         *  Direct access to vector data in different ways
         */
        union
        {
            /**
             * Vector3 vec;
             * vec.x = 1;
             * vec.y = 2;
             * vec.z = 3;
             */
            struct
            {
                Float x;
                Float y;
                Float z;
            };

            /**
            * Vector3 vec;
            * vec.data[0] = 1;
            * vec.data[1] = 2;
            * vec.data[2] = 3;
             */
            Float data[3];
        };

        /**
         *  Default constructor initializes elements with 0.0f
         */
        Vector3();

        /**
         *  Copy constructor
         * @param other Vector3 to copy from
         */
        Vector3(const Vector3& other);

        /**
         *  Constructor to initialize the vector with single values
         * @param _x value of x element
         * @param _y value of y element
         * @param _z value of z element
         */
        Vector3(const Float _x, const Float _y, const Float _z);

        Vector3(Vector3&& other) RNOEXCEPT = default;
        Vector3& operator=(Vector3&& other) RNOEXCEPT = default;

        /**
         *  Constructor to initialize the vector with one value
         * @param value for all elements
         */
        explicit Vector3(const Float value);

        /**
        * Constructs a Vector3 from Vector4. discards the w component
        * @param vector for construction
        */
        explicit Vector3(const Vector4& other);

        /**
         * Sets the vector elements
         * @param x value of x element
         * @param y value of y element
         * @param z value of z element
         */
        void set(const Float _x, const Float _y, const Float _z);

        /**
         * Sets all elements of the vector to the given value
         * @param val for all elements
         */
        void set(const Float xyz);

        /**
         *  Assignment operator to overwrite vector data with other vector data
         * @param other Vector3 to copy from
         */
        Vector3& operator=(const Vector3& other);

        /**
         *  Add operator to add two Vector3 by elements
         * @param other Vector3 with the elements to add
         * @return Vector3 with the result
         */
        Vector3 operator+(const Vector3& other) const;

        /**
         *  Add and assign operator to add other Vector3 elements to local elements
         * @param other Vector3 with the elements to add
         */
        void operator+=(const Vector3& other);

        /**
         *  Sub operator to substract two Vector3 by elements
         * @param other Vector3 with the elements to sub
         * @return Vector3 with the result
         */
        Vector3 operator-(const Vector3& other) const;

        /**
         * Returns the inverse of the Vector
         * @return the inverse of the Vector
         */
        Vector3 operator-() const;

        /**
         *  Sub and assign operator to sub other Vector3 elements from local elements
         * @param other Vector3 with the elements to sub
         */
        void operator-=(const Vector3& other);

        /**
         *  Operator to multiply a scalar to the vector elements
         * @param the scalar for multiplication
         * @return Vector3 with the multiplied result
         */
        Vector3 operator*(const Float scalar) const;

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
         * @param vec Vector3 to scale with
         * @return scaled Vector3
         */
        Vector3 operator*(const Vector3& vec) const;

        /**
         * Operator to scale each vector element with the elements of the given vector
         * @param vec Vector3 to scale with
         */
        void operator*=(const Vector3& vec);

        /**
         * Compares each element to check if two vectors are equal
         * @param other Vector3 to compare to
         * @return true if both vectors are equal false otherwise
         */
        bool operator==(const Vector3& other) const;

        /**
         * Compares each element to check if two vectors are not equal
         * @param other Vector3 to compare to
         * @return false if both vectors are equal true otherwise
         */
        bool operator!=(const Vector3& other) const;

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
         *  Computes the dot product of two Vector3
         * @other Vector3 for the computation of the dot product
         * @return the resulting dot product
         */
        Float dot(const Vector3& other) const;

        /**
        *  Inverts a Vector3
        * @return the resulting inverted vector
        */
        Vector3 inverse() const;

        /**
         *  Computes the cross product of two Vector3
         * @other Vector3 for the computation of the cross product
         * @return Vector3 with the result of the cross product
         */
        Vector3 cross(const Vector3& other) const;

        /**
         *  Computes the euclidean length of the vector
         * @return the euclidean length of the vector
         */
        Float length() const;

        /**
         *  Computes the angle between two vectors in radians.
         * @param other Vector3 to compute the angle with
         * @return the angle in radians between the vectors
         */
        Float angle(const Vector3& other) const;

        /**
         * Normalizes the vector to the length of 1
         * @return Vector3 normalized vector
         */
        Vector3 normalize() const;

        String toString() const;

        friend Vector3 operator*(const Float scalar, const Vector3&);

    protected:
    private:
    };

    inline Vector3::Vector3()
    {
        PlatformMemory::Set(data, 0, sizeof(data));
    }

    inline
    Vector3::Vector3(const Float value)
    : x(value)
    , y(value)
    , z(value)
    {

    }

    inline Vector3::Vector3(const Float _x, const Float _y, const Float _z)
        : x(_x)
        , y(_y)
        , z(_z)
    {
    }

    inline Vector3::Vector3(const Vector3& other)
    {
        PlatformMemory::Copy(data, other.data, sizeof(data));
    }

    inline void Vector3::set(const Float _x, const Float _y, const Float _z)
    {
        x = _x;
        y = _y;
        z = _z;
    }

    inline void Vector3::set(const Float xyz)
    {
        x = y = z = xyz;
    }

    inline Float Vector3::length() const
    {
        return std::sqrt(x*x + y*y + z*z);
    }

    inline Vector3& Vector3::operator=(const Vector3& other)
    {
        PlatformMemory::Copy(data, other.data, sizeof(data));
        return *this;
    }

    inline Vector3 Vector3::operator+(const Vector3& other) const
    {
        return Vector3( x + other.x
                        , y + other.y
                        , z + other.z);
    }

    inline void Vector3::operator+=(const Vector3& other)
    {
        x += other.x;
        y += other.y;
        z += other.z;
    }

    inline Vector3 Vector3::operator-(const Vector3& other) const
    {
        return Vector3( x - other.x
                        , y - other.y
                        , z - other.z);
    }

    inline void Vector3::operator-=(const Vector3& other)
    {
        x -= other.x;
        y -= other.y;
        z -= other.z;
    }

    inline bool Vector3::operator==(const Vector3& other) const
    {
        return PlatformMemory::Compare(data, other.data, sizeof(data)) == 0;
    }

    inline bool Vector3::operator!=(const Vector3& other) const
    {
        return !operator==(other);
    }

    inline Float Vector3::dot(const Vector3& other) const
    {
        return x * other.x + y * other.y + z * other.z;
    }

    inline Vector3 Vector3::inverse() const
    {
        return Vector3(1.0f / x, 1.0f / y, 1.0f / z);
    }

    inline Vector3 Vector3::cross(const Vector3& other) const
    {
        return Vector3( y * other.z - z * other.y
                        , z * other.x - x * other.z
                        , x * other.y - y * other.x);
    }

    inline void Vector3::operator*=(const Float scalar)
    {
        x *= scalar;
        y *= scalar;
        z *= scalar;
    }

    inline void Vector3::operator/=(const Float scalar)
    {
        x /= scalar;
        y /= scalar;
        z /= scalar;
    }

    inline Vector3 Vector3::operator*(const Float scalar) const
    {
        return Vector3( x * scalar
                        , y * scalar
                        , z * scalar);
    }

    inline Vector3 Vector3::operator*(const Vector3& vec) const
    {
        return Vector3(x * vec.x, y * vec.y, z * vec.z);
    }

    inline void Vector3::operator*=(const Vector3& vec)
    {
        x *= vec.x;
        y *= vec.y;
        z *= vec.z;
    }

    inline Float Vector3::angle(const Vector3& other) const
    {
        return std::acos(dot(other) / (length() * other.length()));
    }

    inline
    Vector3
    Vector3::normalize() const
    {
        const Float len = length();
        return Vector3(x / len, y / len, z / len);
    }

    inline
    Float& Vector3::operator[](const UInt32 index)
    {
        return data[index];
    }

    inline
    const Float& Vector3::operator[](const UInt32 index) const
    {
        return data[index];
    }

    inline
    Vector3 Vector3::operator-() const
    {
        return Vector3(-x, -y, -z);
    }


    inline
    IOutputStream&
    operator<<(IOutputStream& outputStream, const Vector3& vector)
    {
        return outputStream.write(reinterpret_cast<const Char*>(vector.data), sizeof(vector.data));
    }

    inline
    IInputStream&
    operator>>(IInputStream& inputStream, Vector3& vector)
    {
        return inputStream.read(reinterpret_cast<Char*>(vector.data), sizeof(vector.data));
    }

    inline
    StringOutputStream& operator<<(StringOutputStream& outputStream, const Vector3& vec3)
    {
        return outputStream << vec3.x << "/" << vec3.y << "/" << vec3.z;
    }

    inline
        String Vector3::toString() const
    {
        StringOutputStream s;
        s << *this;
        return s.c_str();
    }

    inline Vector3 operator*(const Float scalar, const Vector3& vec)
    {
        return Vector3(vec.x * scalar
            , vec.y * scalar
            , vec.z * scalar);
    }

    static_assert(std::is_nothrow_move_constructible<Vector3>::value, "Vector3 must be movable");
    static_assert(std::is_nothrow_move_assignable<Vector3>::value, "Vector3 must be movable");
}

#endif
