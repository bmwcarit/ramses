//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_MATRIX33F_H
#define RAMSES_MATRIX33F_H

#include <PlatformAbstraction/PlatformTypes.h>
#include <PlatformAbstraction/PlatformMemory.h>
#include "Utils/Warnings.h"
#include <Math3d/Vector3.h>

namespace ramses_internal
{
    class Matrix44f;

    /**
        * Class for matrix33 computations
        */
    class Matrix33f
    {
    public:
        union
        {
            struct
            {
                Float m11;
                Float m21;
                Float m31;
                Float m12;
                Float m22;
                Float m32;
                Float m13;
                Float m23;
                Float m33;
            };
            Float data[9];
        };

        /**
        * Special Identity matrix
        * 1 0 0
        * 0 1 0
        * 0 0 1
        */
        static const Matrix33f Identity;

        /**
            * Special Empty matrix
            * 0 0 0
            * 0 0 0
            * 0 0 0
            */
        static const Matrix33f Empty;

        /**
        * Default constructor for matrix. Initializes with identity matrix
        */
        Matrix33f();

        /**
            * Constructor for initialize matrix with 9 single values
            */
        Matrix33f(  const Float _m11, const Float _m12, const Float _m13,
                    const Float _m21, const Float _m22, const Float _m23,
                    const Float _m31, const Float _m32, const Float _m33);

        /**
         * Constructor initializing matrix33 from matrix44 (taking only 3x3)
         */
        explicit Matrix33f(const Matrix44f& otherMat44);

        /**
         * Constructor to initialize matrix with one vector for each row
         * @param v1 Vector3 for first row
         * @param v2 Vector3 for second row
         * @param v3 Vector3 for third row
         */
        Matrix33f(const Vector3& v1, const Vector3& v2, const Vector3& v3);

        Matrix33f(const Matrix33f& other) = default;
        Matrix33f(Matrix33f&& other) = default;
        Matrix33f& operator=(const Matrix33f& other) = default;
        Matrix33f& operator=(Matrix33f&& other) = default;

        /**
         * Sets matrix elements to the given values
         */
        void set(   const Float _m11, const Float _m12, const Float _m13
                    , const Float _m21, const Float _m22, const Float _m23
                    , const Float _m31, const Float _m32, const Float _m33);

        /**
         *  Sets all matrix elements to the given value
         */
        void set(const Float val);

        /**
         * Multiplies all elements of the matrix with given value
         * @param value to multiply the Matrix33f with
         * @return resulting Matrix33f
         */
        Matrix33f operator*(const Float val) const;

        /**
         * Divides all elements of the matrix with given value
         * @param value to divide the Matrix33f with
         * @return resulting Matrix33f
         */
        Matrix33f operator/(const Float val) const;

        /**
         * Multiplies the matrix with another matrix
         * @param mat Matrix33f to multiply with the matrix
         * @return the resulting Matrix33f
         */
        Matrix33f operator*(const Matrix33f& mat) const;

        /**
         * Multiplies the matrix with another matrix and assigns the result
         * @param mat Matrix33f to multiply with the matrix
         */
        void operator*=(const Matrix33f& mat);

        /**
         * Check if two matrices are equal
         * @param other Matrix33f to compare with
         * @return true if matrices are equal false otherwise
         */
        Bool operator==(const Matrix33f& other) const;

        /**
         * Check if two matrices are not equal
         * @param other Matrix33f to compare with
         * @return true if matrices are equal false otherwise
         */
        Bool operator!=(const Matrix33f& other) const;

        /**
         * Multiplies the matrix with the given Vector
         * @param vec Vector3 to multiply with the matrix
         * @return the resulting Vector3
         */
        Vector3 operator*(const Vector3& vec) const;

        /**
         * Extract XYZ-order Euler angles
         * @param rotationXYZ vector to store angles
         * @return true if there is a unique solution
         */
        Bool toRotationEulerXYZ(Vector3& rotationXYZ) const;

        /**
         * Extract XYZ-order Euler angles
         * @param x angle
         * @param y angle
         * @param z angle
         * @return true if there is a unique solution
         */
        Bool toRotationEulerXYZ(Float& x, Float& y, Float& z) const;

        /**
         * Generate Matrix33f from Euler XYZ-order angles
         * @param rotationXYZ vector carrying Euler angles
         * @return Matrix33f generated by Euler angles
         */
        static Matrix33f RotationEulerXYZ(const Vector3& rotationXYZ);

        /**
        * Extract ZYX-order Euler angles
        * @param rotationXYZ vector to store angles
        * @return true if there is a unique solution
        */
        Bool toRotationEulerZYX(Vector3& rotationXYZ) const;

        /**
        * Extract ZYX-order Euler angles
        * @param x angle
        * @param y angle
        * @param z angle
        * @return true if there is a unique solution
        */
        Bool toRotationEulerZYX(Float& x, Float& y, Float& z) const;

        /**
        * Generate Matrix33f from Euler ZYX-order angles
        * @param rotationXYZ vector carrying Euler angles
        * @return Matrix33f generated by Euler angles
        */
        static Matrix33f RotationEulerZYX(const Vector3& rotationXYZ);

        /**
        * Generate Matrix33f from Euler ZYX-order angles
        * @param x angle
        * @param y angle
        * @param z angle
        * @return Matrix33f generated by Euler angles
        */
        static Matrix33f RotationEulerZYX(Float x, Float y, Float z);

        /**
         * Returns the ref value at given column (i) and row (j) index
         * @param column for value
         * @param row for value
         * @return ref to the value at given row and column
         */
        Float& m(UInt32 column, UInt32 row);

        /**
         * Returns the const ref value at given column (i) and row (j) index
         * @param column for value
         * @param row for value
         * @return const ref value at given row and column
         */
        const Float& m(UInt32 column, UInt32 row) const;

    protected:
    private:
    };

    inline
    Matrix33f::Matrix33f()
    : m11(1.f), m21(0.f), m31(0.f)
    , m12(0.f), m22(1.f), m32(0.f)
    , m13(0.f), m23(0.f), m33(1.f)
    {
    }

    inline
    Matrix33f::Matrix33f(const Float _m11, const Float _m12, const Float _m13
                        , const Float _m21, const Float _m22, const Float _m23
                        , const Float _m31, const Float _m32, const Float _m33)

        : m11(_m11), m21(_m21), m31(_m31)
        , m12(_m12), m22(_m22), m32(_m32)
        , m13(_m13), m23(_m23), m33(_m33)
    {
    }

    inline
    Matrix33f::Matrix33f(const Vector3& v1, const Vector3& v2, const Vector3& v3)
    {
        m11 = v1[0];
        m12 = v1[1];
        m13 = v1[2];
        m21 = v2[0];
        m22 = v2[1];
        m23 = v2[2];
        m31 = v3[0];
        m32 = v3[1];
        m33 = v3[2];
    }

    inline
    void
    Matrix33f::set(const Float _m11, const Float _m12, const Float _m13
                , const Float _m21, const Float _m22, const Float _m23
                , const Float _m31, const Float _m32, const Float _m33)
    {
        m11 = _m11;
        m12 = _m12;
        m13 = _m13;
        m21 = _m21;
        m22 = _m22;
        m23 = _m23;
        m31 = _m31;
        m32 = _m32;
        m33 = _m33;
    }

    inline
    void
    Matrix33f::set(Float val)
    {
        m11 = m12 = m13
        = m21 = m22 = m23
        = m31 = m32 = m33
        = val;
    }

    inline
    Matrix33f
    Matrix33f::operator*(const Float val) const
    {
        return Matrix33f(   m11 * val, m12 * val, m13 * val,
                            m21 * val, m22 * val, m23 * val,
                            m31 * val, m32 * val, m33 * val);
    }

    inline
    Matrix33f
    Matrix33f::operator/(const Float val) const
    {
        return Matrix33f(   m11 / val, m12 / val, m13 / val,
                            m21 / val, m22 / val, m23 / val,
                            m31 / val, m32 / val, m33 / val);
    }

    inline
    Matrix33f
    Matrix33f::operator*(const Matrix33f& mat) const
    {
        return Matrix33f( m11 * mat.m11 + m12 * mat.m21 + m13 * mat.m31, m11 * mat.m12 + m12 * mat.m22 + m13 * mat.m32, m11 * mat.m13 + m12 * mat.m23 + m13 * mat.m33
                        , m21 * mat.m11 + m22 * mat.m21 + m23 * mat.m31, m21 * mat.m12 + m22 * mat.m22 + m23 * mat.m32, m21 * mat.m13 + m22 * mat.m23 + m23 * mat.m33
                        , m31 * mat.m11 + m32 * mat.m21 + m33 * mat.m31, m31 * mat.m12 + m32 * mat.m22 + m33 * mat.m32, m31 * mat.m13 + m32 * mat.m23 + m33 * mat.m33);
    }

    inline
    void
    Matrix33f::operator*=(const Matrix33f& mat)
    {
        const Matrix33f& result = operator*(mat);
        PlatformMemory::Copy(&m11, &result.m11, 9 * sizeof(Float));
    }

    inline
    Vector3
    Matrix33f::operator*(const Vector3& vec) const
    {
        return Vector3(  m11 * vec.x + m12 * vec.y + m13 * vec.z
                        , m21 * vec.x + m22 * vec.y + m23 * vec.z
                        , m31 * vec.x + m32 * vec.y + m33 * vec.z);
    }

    inline
    Bool
    Matrix33f::operator==(const Matrix33f& other) const
    {
        return PlatformMemory::Compare(&m11, &other.m11, 9 * sizeof(Float)) == 0;
    }

    inline
    Bool
    Matrix33f::operator!=(const Matrix33f& other) const
    {
        return !operator==(other);
    }

    static_assert(std::is_nothrow_move_constructible<Matrix33f>::value &&
        std::is_nothrow_move_assignable<Matrix33f>::value, "Matrix33f must be movable");
}

#endif
