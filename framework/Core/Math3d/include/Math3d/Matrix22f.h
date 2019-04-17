//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_MATRIX22F_H
#define RAMSES_MATRIX22F_H

#include <PlatformAbstraction/PlatformTypes.h>
#include <PlatformAbstraction/PlatformMemory.h>
#include <Math3d/Vector2.h>

namespace ramses_internal
{
    /**
    * Class for matrix22 computations
    */
    class Matrix22f
    {
    public:
        union
        {
            struct
            {
                Float m11;
                Float m21;
                Float m12;
                Float m22;
            };

            Float data[4];
        };

        /**
        * Special Identity matrix
        * 1 0
        * 0 1
        */
        static const Matrix22f Identity;

        /**
        * Special Empty matrix
        * 0 0
        * 0 0
        */
        static const Matrix22f Empty;

        /**
        * Default constructor for matrix. Initializes with identity matrix
        */
        Matrix22f();

        /**
        * Constructor for initialize matrix with 4 single values
        */
        Matrix22f(const Float _m11, const Float _m12, const Float _m21, const Float _m22);

        /**
        * Constructor to initialize matrix with one vector for each row
        * @param v1 Vector2 for first row
        * @param v2 Vector2 for second row
        */
        Matrix22f(const Vector2& v1, const Vector2& v2);

        Matrix22f(const Matrix22f& other) = default;
        Matrix22f(Matrix22f&& other) = default;
        Matrix22f& operator=(const Matrix22f& other) = default;
        Matrix22f& operator=(Matrix22f&& other) = default;

        /**
        * Sets matrix elements to the given values
        */
        void set(const Float _m11, const Float _m12, const Float _m21, const Float _m22);

        /**
        *  Sets all matrix elements to the given value
        */
        void set(const Float val);

        /**
        * Multiplies all elements of the matrix with given value
        * @param value to multiply the Matrix22f with
        * @return resulting Matrix22f
        */
        Matrix22f operator*(const Float val) const;

        /**
        * Divides all elements of the matrix with given value
        * @param value to divide the Matrix22f with
        * @return resulting Matrix22f
        */
        Matrix22f operator/(const Float val) const;

        /**
        * Multiplies the matrix with another matrix
        * @param mat Matrix22f to multiply with the matrix
        * @return the resulting Matrix22f
        */
        Matrix22f operator*(const Matrix22f& mat) const;

        /**
        * Multiplies the matrix with another matrix and assigns the result
        * @param mat Matrix22f to multiply with the matrix
        */
        void operator*=(const Matrix22f& mat);

        /**
        * Check if two matrices are equal
        * @param other Matrix22f to compare with
        * @return true if matrices are equal false otherwise
        */
        Bool operator==(const Matrix22f& other) const;

        /**
        * Check if two matrices are not equal
        * @param other Matrix22f to compare with
        * @return true if matrices are equal false otherwise
        */
        Bool operator!=(const Matrix22f& other) const;

        /**
        * Multiplies the matrix with the given Vector
        * @param vec Vector2 to multiply with the matrix
        * @return the resulting Vector2
        */
        Vector2 operator*(const Vector2& vec) const;
    };

    inline
        Matrix22f::Matrix22f()
        : m11(1.f), m21(0.f)
        , m12(0.f), m22(1.f)
    {
    }

    inline
        Matrix22f::Matrix22f(const Float _m11, const Float _m12, const Float _m21, const Float _m22)
        : m11(_m11), m21(_m21)
        , m12(_m12), m22(_m22)
    {
    }

    inline
        Matrix22f::Matrix22f(const Vector2& v1, const Vector2& v2)
    {
        m11 = v1[0];
        m12 = v1[1];
        m21 = v2[0];
        m22 = v2[1];
    }

    inline
        void
        Matrix22f::set(const Float _m11, const Float _m12, const Float _m21, const Float _m22)
    {
        m11 = _m11;
        m12 = _m12;
        m21 = _m21;
        m22 = _m22;
    }

    inline
        void
        Matrix22f::set(Float val)
    {
        m11 = m12 = m21 = m22 = val;
    }

    inline
        Matrix22f
        Matrix22f::operator*(const Float val) const
    {
        return Matrix22f(m11 * val, m12 * val, m21 * val, m22 * val);
    }

    inline
        Matrix22f
        Matrix22f::operator/(const Float val) const
    {
        return Matrix22f(m11 / val, m12 / val, m21 / val, m22 / val);
    }

    inline
        Matrix22f
        Matrix22f::operator*(const Matrix22f& mat) const
    {
        return Matrix22f(
            m11 * mat.m11 + m12 * mat.m21, m11 * mat.m12 + m12 * mat.m22,
            m21 * mat.m11 + m22 * mat.m21, m21 * mat.m12 + m22 * mat.m22);
    }

    inline
        void
        Matrix22f::operator*=(const Matrix22f& mat)
    {
        const Matrix22f& result = operator*(mat);
        PlatformMemory::Copy(&m11, &result.m11, 4 * sizeof(Float));
    }

    inline
        Vector2
        Matrix22f::operator*(const Vector2& vec) const
    {
        return Vector2(
            m11 * vec.x + m12 * vec.y,
            m21 * vec.x + m22 * vec.y);
    }

    inline
        Bool
        Matrix22f::operator==(const Matrix22f& other) const
    {
        return PlatformMemory::Compare(&m11, &other.m11, 4 * sizeof(Float)) == 0;
    }

    inline
        Bool
        Matrix22f::operator!=(const Matrix22f& other) const
    {
        return !operator==(other);
    }

    static_assert(std::is_nothrow_move_constructible<Matrix22f>::value &&
        std::is_nothrow_move_assignable<Matrix22f>::value, "Matrix22f must be movable");
}

#endif
