//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_VECTOR4_H
#define RAMSES_VECTOR4_H

#include <PlatformAbstraction/PlatformTypes.h>
#include <PlatformAbstraction/PlatformMemory.h>
#include <PlatformAbstraction/PlatformMath.h>
#include <Math3d/Vector3.h>

namespace ramses_internal
{
    /**
    *   Allows storing of 4d-vector data and operations
    */
    class Vector4
    {
    public:

        /**
         * Vector with all values set to 0
         */
        static const Vector4 Empty;

        /**
        *   Direct access to vector data in different ways
        */
        union
        {
            /**
            * Vector4 vec;
            * vec.x = 1;
            * vec.y = 2;
            * vec.z = 3;
            * vec.w = 4;
            */
            IGNORE_UNNAMED_MEMBER_WARNING_START
            struct
            {
                Float x;
                Float y;
                Float z;
                Float w;
            };

            /**
            * Vector4 vec;
            * vec.r = 1;
            * vec.g = 2;
            * vec.b = 3;
            * vec.a = 4;
            */
            struct
            {
                Float r;
                Float g;
                Float b;
                Float a;
            };
            IGNORE_UNNAMED_MEMBER_WARNING_END

            /**
            * Vector4 vec;
            * vec.data[0] = 1;
            * vec.data[1] = 2;
            * vec.data[2] = 3;
            * vec.data[3] = 4;
            */
            Float data[4];
        };

        /**
         *   Default constructor initializes elements with 0.0f
         */
        Vector4();

        /**
        *   Copy constructor
        * @param other Vector4 to copy from
        */
        Vector4(const Vector4& other);

        /**
        *   Constructor to initialize the vector with single values
        * @param _x value of x element
        * @param _y value of y element
        * @param _z value of z element
        * @param _w value of w element
        */
        Vector4(Float _x, Float _y, Float _z, Float _w);

        Vector4(Vector4&& other) = default;
        Vector4& operator=(Vector4&& other) = default;

        /**
         * Constructor to initialize the vector one value
         * @param value for all elements
         */
        explicit Vector4(const Float value);

        /**
         * Constructs a Vector4 from Vector3. initializes the w component with 1.0
         * @param vector for construction
         */
        explicit Vector4(const Vector3& other);

        /**
         * Sets the vector elements
         * @param x value of x element
         * @param y value of y element
         * @param z value of z element
         * @param z value of w element
         */
        void set(Float _x, Float _y, Float _z, Float _w);

        /**
         * Sets all elements of the vector to the given value
         * @param val for all elements
         */
        void set(Float xyzw);

        /**
        *   Assignment operator to overwrite vector data with other vector data
        * @param other Vector4 to copy from
        */
        Vector4& operator=(const Vector4& other);

        /**
        *   Add operator to add two Vector4 by elements
        * @param other Vector4 with the elements to add
        * @return Vector3 with the result
        */
        Vector4 operator+(const Vector4& other) const;

        /**
        *   Add and assign operator to add other Vector4 elements to local elements
        * @param other Vector4 with the elements to add
        */
        void operator+=(const Vector4& other);

        /**
        *   Sub operator to substract two Vector4 by elements
        * @param other Vector4 with the elements to sub
        * @return Vector4 with the result
        */
        Vector4 operator-(const Vector4& other) const;

        /**
        *   Sub and assign operator to sub other Vector4 elements from local elements
        * @param other Vector4 with the elements to sub
        */
        void operator-=(const Vector4& other);

        /**
        *   Operator to multiply a scalar to the vector elements
        * @param the scalar for multiplication
        * @return Vector4 with the multiplied result
        */
        Vector4 operator*(const Float scalar) const;

        Vector4 operator/(const Vector4& scalar) const;

        /**
        *   Operator to multiply a scalar and assign the result to the local vector elements
        * @param the scalar for multiplication
        */
        void operator*=(const Float scalar);

        /**
        * Operator to scale each vector element with the elements of the given vector
        * @param vec Vector4 to scale with
        * @return scaled Vector3
        */
        Vector4 operator*(const Vector4& vec) const;

        /**
         * Operator to scale each vector element with the elements of the given vector
         * @param vec Vector4 to scale with
         */
        void operator*=(const Vector4& vec);

        /**
         * Compares each element to check if two vectors are equal
         * @param other Vector3 to compare to
         * @return true if both vectors are equal false otherwise
         */
        Bool operator==(const Vector4& other) const;

        /**
         * Compares each element to check if two vectors are not equal
         * @param other Vector3 to compare to
         * @return false if both vectors are equal true otherwise
         */
        Bool operator!=(const Vector4& other) const;

        /**
        *   Computes the dot product of two Vector4
        * @other Vector4 for the computation of the dot product
        * @return the resulting dot product
        */
        Float dot(const Vector4& other) const;

        /**
        *   Computes the cross product of two Vector4
        * @other Vector4 for the computation of the cross product
        * @return Vector4 with the result of the cross product
        */
        Vector4 cross(const Vector4& other) const;

        /**
        *   Computes the euclidean length of the vector
        * @return the euclidean length of the vector
        */
        Float length() const;

        /**
        *   Computes the angle between two vectors in radians.
        * @param other Vector4 to compute the angle with
        * @return the angle in radians between the vectors
        */
        Float angle(const Vector4& other) const;

        /**
        * Returns the element of the given index
        * @return the element of the given index
        */
        Float& operator[](const UInt32 index);

        const Float& operator[](const UInt32 index) const;

        friend Vector4 operator*(const Float scalar, const Vector4&);

    protected:
    private:
    };

    inline Vector4::Vector4()
    {
        PlatformMemory::Set(data, 0, sizeof(data));
    }

    inline Vector4::Vector4(Float _x, Float _y, Float _z, Float _w)
        : x(_x)
        , y(_y)
        , z(_z)
        , w(_w)
    {
    }

    inline Vector4::Vector4(const Vector4& other)
    {
        PlatformMemory::Copy(data, other.data, sizeof(data));
    }

    inline
        Vector4::Vector4(const Float value)
        : x(value)
        , y(value)
        , z(value)
        , w(value)
    {
    }

    inline Vector4::Vector4(const Vector3& other)
    {
        PlatformMemory::Copy(data, other.data, sizeof(other.data));
        w = 1.0f;
    }

    inline void Vector4::set(Float _x, Float _y, Float _z, Float _w)
    {
        x = _x;
        y = _y;
        z = _z;
        w = _w;
    }

    inline void Vector4::set(Float xyzw)
    {
        x = y = z = w = xyzw;
    }

    inline Float Vector4::length() const
    {
        return PlatformMath::Sqrt(PlatformMath::Pow2(x) + PlatformMath::Pow2(y) + PlatformMath::Pow2(z) + PlatformMath::Pow2(w));
    }

    inline Vector4& Vector4::operator=(const Vector4& other)
    {
        if (&other != this)
        {
            PlatformMemory::Copy(data, other.data, sizeof(data));
        }
        return *this;
    }

    inline Vector4 Vector4::operator+(const Vector4& other) const
    {
        return Vector4(x + other.x
            , y + other.y
            , z + other.z
            , w + other.w);
    }

    inline void Vector4::operator+=(const Vector4& other)
    {
        x += other.x;
        y += other.y;
        z += other.z;
        w += other.w;
    }

    inline Vector4 Vector4::operator-(const Vector4& other) const
    {
        return Vector4(x - other.x
            , y - other.y
            , z - other.z
            , w - other.w);
    }

    inline void Vector4::operator-=(const Vector4& other)
    {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        w -= other.w;
    }

    inline Vector4 Vector4::operator*(const Vector4& vec) const
    {
        return Vector4(x * vec.x, y * vec.y, z * vec.z, w * vec.w);
    }

    inline void Vector4::operator*=(const Vector4& vec)
    {
        x *= vec.x;
        y *= vec.y;
        z *= vec.z;
        w *= vec.w;
    }

    inline Bool Vector4::operator==(const Vector4& other) const
    {
        return PlatformMemory::Compare(data, other.data, sizeof(data)) == 0;
    }

    inline Bool Vector4::operator!=(const Vector4& other) const
    {
        return !operator==(other);
    }

    inline Float Vector4::dot(const Vector4& other) const
    {
        return x * other.x + y * other.y + z * other.z + w * other.w;
    }

    inline Vector4 Vector4::cross(const Vector4& other) const
    {
        return Vector4(y * other.z - z * other.y + z * other.w - w * other.z + y * other.w - w * other.y
            , z * other.x - x * other.z + w * other.x - x * other.w + z * other.w - w * other.z
            , x * other.y - y * other.x + w * other.y - y * other.w + w * other.x - x * other.w
            , x * other.y - y * other.x + y * other.z - z * other.y + x * other.z - z * other.x
            );
    }

    inline void Vector4::operator*=(const Float scalar)
    {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        w *= scalar;
    }

    inline Vector4 Vector4::operator*(const Float scalar) const
    {
        return Vector4(x * scalar
            , y * scalar
            , z * scalar
            , w * scalar);
    }

    inline Vector4 Vector4::operator/(const Vector4& other) const
    {
        return Vector4(x / other.x
            , y / other.y
            , z / other.z
            , w / other.w);
    }

    inline Float Vector4::angle(const Vector4& other) const
    {
        return PlatformMath::ArcCos(dot(other) / (length() * other.length()));
    }

    inline
        IOutputStream&
        operator<<(IOutputStream& outputStream, const Vector4& vector)
    {
            return outputStream.write(reinterpret_cast<const Char*>(vector.data), sizeof(vector.data));
    }

    inline
        IInputStream&
        operator>>(IInputStream& inputStream, Vector4& vector)
    {
            return inputStream.read(reinterpret_cast<Char*>(vector.data), sizeof(vector.data));
    }

    inline
        StringOutputStream& operator<<(StringOutputStream& outputStream, const Vector4& vec4)
    {
            return outputStream << vec4.x << "/" << vec4.y << "/" << vec4.z << "/" << vec4.w;
    }

    inline
        Float& Vector4::operator[](const UInt32 index)
    {
            return data[index];
    }

    inline
        const Float& Vector4::operator[](const UInt32 index) const
    {
            return data[index];
    }

    inline
        Vector4 operator*(const Float scalar, const Vector4& vec)
    {
        return Vector4(vec.x * scalar
            , vec.y * scalar
            , vec.z * scalar
            , vec.w * scalar);
    }

    static_assert(std::is_nothrow_move_constructible<Vector4>::value &&
        std::is_nothrow_move_assignable<Vector4>::value, "Vector4 must be movable");
}

#endif

