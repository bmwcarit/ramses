//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_MATH3D_MATRIX44F_H
#define RAMSES_MATH3D_MATRIX44F_H

#include "PlatformAbstraction/PlatformTypes.h"
#include "PlatformAbstraction/PlatformMemory.h"
#include "Math3d/Matrix33f.h"
#include "Math3d/Vector4.h"
#include "Math3d/Vector3.h"
#include "Collections/IOutputStream.h"
#include "Collections/IInputStream.h"

namespace ramses_internal
{
    /**
     * Class for matrix computations
     */
    class Matrix44f
    {
    public:
        union
        {
            struct
            {
                // Matrix is stored column-wise (to fit GLSL convention), but elements are named row-wise
                // E.g. m1x is the first row, and m23 is the 3rd element on the 2nd row
                Float m11;
                Float m21;
                Float m31;
                Float m41;
                Float m12;
                Float m22;
                Float m32;
                Float m42;
                Float m13;
                Float m23;
                Float m33;
                Float m43;
                Float m14;
                Float m24;
                Float m34;
                Float m44;
            };
            Float data[16];
        };

        /**
        * Special Identity matrix
        * 1 0 0 0
        * 0 1 0 0
        * 0 0 1 0
        * 0 0 0 1
        */
        static const Matrix44f Identity;

        /**
         * Special Empty matrix
         * 0 0 0 0
         * 0 0 0 0
         * 0 0 0 0
         * 0 0 0 0
         */
        static const Matrix44f Empty;

        static Matrix44f Translation(const Vector3& translation);
        static Matrix44f Translation(const Float x, const Float y, const Float z);
        static Matrix44f RotationEulerXYZ(const Vector3& rotationXYZ);
        static Matrix44f RotationEulerXYZ(const Float x, const Float y, const Float z);
        static Matrix44f RotationEulerZYX(const Vector3& rotationXYZ);
        static Matrix44f RotationEulerZYX(const Float x, const Float y, const Float z);
        static Matrix44f Scaling(const Vector3& scaling);
        static Matrix44f Scaling(const Float x, const Float y, const Float z);
        static Matrix44f Scaling(const Float uniScale);

        /**
        * Default constructor for matrix. Initializes with identity matrix
        */
        Matrix44f();

        /**
         * Constructor for initialize matrix with 16 single values
         */
        Matrix44f(  const Float _m11, const Float _m12, const Float _m13, const Float _m14,
                    const Float _m21, const Float _m22, const Float _m23, const Float _m24,
                    const Float _m31, const Float _m32, const Float _m33, const Float _m34,
                    const Float _m41, const Float _m42, const Float _m43, const Float _m44);

        /**
        * Constructor for initialize matrix with 16 values array (row-wise)
        */
        explicit Matrix44f(const float (&matrixElements)[16]);

        /**
         * Constructor to initialize matrix with one vector for each row
         * @param v1 Vector4 for first row
         * @param v2 Vector4 for second row
         * @param v3 Vector4 for third row
         * @param v4 Vector4 for forth row
         */
        Matrix44f(const Vector4& v1, const Vector4& v2, const Vector4& v3, const Vector4& v4);

        Matrix44f(const Matrix44f& other) = default;
        Matrix44f(Matrix44f&& other) = default;
        Matrix44f& operator=(const Matrix44f& other);
        Matrix44f& operator=(Matrix44f&& other) = default;

        /**
         * Constructor to create Matrix44 from other matrix33.
         * Initializes element m44 with 1.f, all the rest untouched elements with 0.f
         * @param other Matrix33 to copy data from
         */
        explicit Matrix44f(const Matrix33f& otherMat33);

        /**
         * Sets matrix elements to the given values
         */
        void set(     const Float _m11, const Float _m12, const Float _m13, const Float _m14
                    , const Float _m21, const Float _m22, const Float _m23, const Float _m24
                    , const Float _m31, const Float _m32, const Float _m33, const Float _m34
                    , const Float _m41, const Float _m42, const Float _m43, const Float _m44);

        /**
         *  Sets all matrix elements to the given value
         */
        void set(const Float val);

        /**
         * Multiplies the matrix with the given Vector
         * @param vec Vector4 to multiply with the matrix
         * @return the resulting Vector4
         */
        Vector4 operator*(const Vector4& vec) const;

        /**
         * Multiplies the matrix with another matrix
         * @param mat Matrix44 to multiply with the matrix
         * @return the resulting Matrix44
         */
        Matrix44f operator*(const Matrix44f& mat) const;

        /**
         * Multiplies the matrix with another matrix and assigns the result
         * @param mat Matrix44 to multiply with the matrix
         */
        void operator*=(const Matrix44f& mat);

        /**
         * Check if two matrices are equal
         * @param other Matrix44 to compare with
         * @return true if matrices are equal false otherwise
         */
        Bool operator==(const Matrix44f& other) const;

        /**
         * Check if two matrices are not equal
         * @param other Matrix44 to compare with
         * @return true if matrices are equal false otherwise
         */
        Bool operator!=(const Matrix44f& other) const;

        /**
         * Returns a pointer to the row specified by index
         * @param the index of the row to return
         * @return a pointer to the row specified by index
         */
        Float& m(const UInt32 i, const UInt32 j);
        //Float* operator[](const UInt32 index);

        /**
         * Returns a const pointer to the row specified by index
         * @param the index of the row to return
         * @return a pointer to the row specified by index
         */
        const Float& m(const UInt32 i, const UInt32 j) const;
        //const Float* operator[](const UInt32 index) const;

        /**
         * Transposes and returns a matrix
         * @return A transposed version of the current Matrix
         */
        Matrix44f transpose() const;

        /**
         * Computes the determinant of the matrix
         * @ return the determinant of the matrix
         */
        Float determinant() const;

        /**
         * Computes the inverse matrix and returns it as result
         * @ return the inverse matrix
         */
        Matrix44f inverse() const;

        /**
         * Rotates the given point with the rotation information from upper 3x3 matrix
         * @param point Reference to the point which should be rotated
         * @return The rotated point
         */
        Vector3 rotate(const Vector3& point) const;

    protected:
    private:
    };

    inline IOutputStream& operator<<(IOutputStream& stream, const Matrix44f& value)
    {
        return stream.write(value.data, sizeof(value.data));
    }

    inline IInputStream& operator>>(IInputStream& stream, Matrix44f& value)
    {
        return stream.read(reinterpret_cast<Char*>(value.data), sizeof(value.data));
    }

    inline
    Matrix44f::Matrix44f()
    : m11(1.f), m21(0.f), m31(0.f), m41(0.f)
    , m12(0.f), m22(1.f), m32(0.f), m42(0.f)
    , m13(0.f), m23(0.f), m33(1.f), m43(0.f)
    , m14(0.f), m24(0.f), m34(0.f), m44(1.f)
    {
    }

    inline
    Matrix44f::Matrix44f(   const Float _m11, const Float _m12, const Float _m13, const Float _m14
                        , const Float _m21, const Float _m22, const Float _m23, const Float _m24
                        , const Float _m31, const Float _m32, const Float _m33, const Float _m34
                        , const Float _m41, const Float _m42, const Float _m43, const Float _m44)

        : m11(_m11) , m21(_m21) , m31(_m31) , m41(_m41)
        , m12(_m12) , m22(_m22) , m32(_m32) , m42(_m42)
        , m13(_m13) , m23(_m23) , m33(_m33) , m43(_m43)
        , m14(_m14) , m24(_m24) , m34(_m34) , m44(_m44)
    {
    }

    inline
    Matrix44f::Matrix44f(const float(&matrixElements)[16])
        : m11(matrixElements[0]), m21(matrixElements[4]), m31(matrixElements[8]),  m41(matrixElements[12])
        , m12(matrixElements[1]), m22(matrixElements[5]), m32(matrixElements[9]),  m42(matrixElements[13])
        , m13(matrixElements[2]), m23(matrixElements[6]), m33(matrixElements[10]), m43(matrixElements[14])
        , m14(matrixElements[3]), m24(matrixElements[7]), m34(matrixElements[11]), m44(matrixElements[15])
    {
    }

    inline
    Matrix44f::Matrix44f(const Vector4& v1, const Vector4& v2, const Vector4& v3, const Vector4& v4)
    {
        m11 = v1.x;
        m12 = v1.y;
        m13 = v1.z;
        m14 = v1.w;

        m21 = v2.x;
        m22 = v2.y;
        m23 = v2.z;
        m24 = v2.w;

        m31 = v3.x;
        m32 = v3.y;
        m33 = v3.z;
        m34 = v3.w;

        m41 = v4.x;
        m42 = v4.y;
        m43 = v4.z;
        m44 = v4.w;
    }

    inline
    void
    Matrix44f::set(const Float _m11, const Float _m12, const Float _m13, const Float _m14
                , const Float _m21, const Float _m22, const Float _m23, const Float _m24
                , const Float _m31, const Float _m32, const Float _m33, const Float _m34
                , const Float _m41, const Float _m42, const Float _m43, const Float _m44)
    {
        m11 = _m11;
        m12 = _m12;
        m13 = _m13;
        m14 = _m14;
        m21 = _m21;
        m22 = _m22;
        m23 = _m23;
        m24 = _m24;
        m31 = _m31;
        m32 = _m32;
        m33 = _m33;
        m34 = _m34;
        m41 = _m41;
        m42 = _m42;
        m43 = _m43;
        m44 = _m44;
    }

    inline
    void
    Matrix44f::set(Float val)
    {
        m11 = m12 = m13 = m14
        = m21 = m22 = m23 = m24
        = m31 = m32 = m33 = m34
        = m41 = m42 = m43 = m44
        = val;
    }

    inline
    Matrix44f&
    Matrix44f::operator=(const Matrix44f& other)
    {
        PlatformMemory::Copy(&m11, &other.m11, 16 * sizeof(Float));
        return *this;
    }

    inline
    Vector4
    Matrix44f::operator*(const Vector4& vec) const
    {
        return Vector4(   m11 * vec.x + m12 * vec.y + m13 * vec.z + m14 * vec.w
                        , m21 * vec.x + m22 * vec.y + m23 * vec.z + m24 * vec.w
                        , m31 * vec.x + m32 * vec.y + m33 * vec.z + m34 * vec.w
                        , m41 * vec.x + m42 * vec.y + m43 * vec.z + m44 * vec.w);
    }

    inline
    Matrix44f
    Matrix44f::operator*(const Matrix44f& mat) const
    {
        return Matrix44f(  m11 * mat.m11 + m12 * mat.m21 + m13 * mat.m31 + m14 * mat.m41, m11 * mat.m12 + m12 * mat.m22 + m13 * mat.m32 + m14 * mat.m42, m11 * mat.m13 + m12 * mat.m23 + m13 * mat.m33 + m14 * mat.m43, m11 * mat.m14 + m12 * mat.m24 + m13 * mat.m34 + m14 * mat.m44
                        , m21 * mat.m11 + m22 * mat.m21 + m23 * mat.m31 + m24 * mat.m41, m21 * mat.m12 + m22 * mat.m22 + m23 * mat.m32 + m24 * mat.m42, m21 * mat.m13 + m22 * mat.m23 + m23 * mat.m33 + m24 * mat.m43, m21 * mat.m14 + m22 * mat.m24 + m23 * mat.m34 + m24 * mat.m44
                        , m31 * mat.m11 + m32 * mat.m21 + m33 * mat.m31 + m34 * mat.m41, m31 * mat.m12 + m32 * mat.m22 + m33 * mat.m32 + m34 * mat.m42, m31 * mat.m13 + m32 * mat.m23 + m33 * mat.m33 + m34 * mat.m43, m31 * mat.m14 + m32 * mat.m24 + m33 * mat.m34 + m34 * mat.m44
                        , m41 * mat.m11 + m42 * mat.m21 + m43 * mat.m31 + m44 * mat.m41, m41 * mat.m12 + m42 * mat.m22 + m43 * mat.m32 + m44 * mat.m42, m41 * mat.m13 + m42 * mat.m23 + m43 * mat.m33 + m44 * mat.m43, m41 * mat.m14 + m42 * mat.m24 + m43 * mat.m34 + m44 * mat.m44);
    }

    inline
    void
    Matrix44f::operator*=(const Matrix44f& mat)
    {
        const Matrix44f& result = operator*(mat);
        PlatformMemory::Copy(&m11, &result.m11, 16 * sizeof(Float));
    }

    inline
    Matrix44f
    Matrix44f::transpose() const
    {
        return Matrix44f(  m11, m21, m31, m41
                        , m12, m22, m32, m42
                        , m13, m23, m33, m43
                        , m14, m24, m34, m44);
    }

    inline
    Float
    Matrix44f::determinant() const
    {
        const Float m13m21 = m13 * m21;
        const Float m13m22 = m13 * m22;
        const Float m13m24 = m13 * m24;
        const Float m11m22 = m11 * m22;
        const Float m11m23 = m11 * m23;
        const Float m11m24 = m11 * m24;
        const Float m12m21 = m12 * m21;
        const Float m14m21 = m12 * m21;
        const Float m12m23 = m12 * m23;
        const Float m14m22 = m14 * m22;
        const Float m12m24 = m12 * m24;
        const Float m14m23 = m14 * m23;
        const Float m33m44 = m33 * m44;
        const Float m34m43 = m34 * m43;
        const Float m32m44 = m32 * m44;
        const Float m33m42 = m33 * m42;
        const Float m34m42 = m34 * m42;
        const Float m31m44 = m31 * m44;
        const Float m34m41 = m34 * m41;
        const Float m32m43 = m32 * m43;
        const Float m31m43 = m31 * m43;
        const Float m33m41 = m33 * m41;
        const Float m31m42 = m31 * m42;
        const Float m32m41 = m32 * m41;

        return  m11m22 * m33m44 + m11m23 * m34m42 + m11m24 * m32m43
                + m12m21 * m34m43 + m12m23 * m31m44 + m12m24 * m33m41
                + m13m21 * m32m44 + m13m22 * m34m41 + m13m24 * m31m42
                + m14m21 * m33m42 + m14m22 * m31m43 + m14m23 * m32m41
                - m11m22 * m34m43 - m11m23 * m32m44 - m11m24 * m33m42
                - m12m21 * m33m44 - m12m23 * m34m41 - m12m24 * m31m43
                - m13m21 * m34m42 - m13m22 * m31m44 - m13m24 * m32m41
                - m14m21 * m32m43 - m14m22 * m33m41 - m14m23 * m31m42;
    }

    inline
    Matrix44f
    Matrix44f::inverse() const
    {
        const Float det = determinant();

        if (det == 0.0f)
        {
            return Matrix44f::Empty;
        }

        const Float invDet = 1.0f / det;

        return Matrix44f( ( m22 * m44 * m33 - m22 * m43 * m34 - m44 * m32 * m23 + m43 * m32 * m24 - m42 * m24 * m33 + m42 * m23 * m34) * invDet,
                        -( m43 * m32 * m14 - m43 * m34 * m12 + m42 * m13 * m34 - m42 * m14 * m33 - m44 * m32 * m13 + m44 * m33 * m12) * invDet,
                        (-m22 * m44 * m13 + m22 * m43 * m14 - m43 * m12 * m24 + m42 * m24 * m13 + m44 * m12 * m23 - m42 * m23 * m14) * invDet,
                        -( m13 * m32 * m24 + m12 * m23 * m34 + m22 * m14 * m33 - m12 * m24 * m33 - m22 * m13 * m34 - m14 * m32 * m23) * invDet,
                        -( m21 * m44 * m33 - m21 * m43 * m34 + m41 * m23 * m34 - m44 * m31 * m23 - m41 * m24 * m33 + m24 * m43 * m31) * invDet,
                        ( m11 * m44 * m33 - m11 * m43 * m34 - m44 * m31 * m13 + m41 * m13 * m34 - m41 * m14 * m33 + m43 * m31 * m14) * invDet,
                        (-m11 * m44 * m23 + m11 * m43 * m24 - m21 * m14 * m43 + m44 * m21 * m13 + m41 * m14 * m23 - m41 * m13 * m24) * invDet,
                        ( m11 * m23 * m34 - m11 * m24 * m33 + m21 * m14 * m33 - m23 * m31 * m14 - m21 * m13 * m34 + m24 * m31 * m13) * invDet,
                        -( m31 * m22 * m44 - m42 * m24 * m31 - m41 * m34 * m22 - m44 * m32 * m21 + m41 * m32 * m24 + m42 * m21 * m34) * invDet,
                        (-m42 * m31 * m14 + m31 * m44 * m12 + m14 * m41 * m32 - m12 * m41 * m34 + m11 * m42 * m34 - m32 * m11 * m44) * invDet,
                        -( m22 * m41 * m14 - m11 * m22 * m44 - m42 * m21 * m14 + m44 * m21 * m12 - m41 * m12 * m24 + m11 * m42 * m24) * invDet,
                        (-m34 * m11 * m22 + m34 * m21 * m12 + m31 * m14 * m22 + m32 * m11 * m24 - m32 * m21 * m14 - m31 * m12 * m24) * invDet,
                        (-m22 * m41 * m33 + m22 * m43 * m31 + m41 * m32 * m23 - m43 * m32 * m21 + m42 * m21 * m33 - m42 * m23 * m31) * invDet,
                        -( m11 * m42 * m33 - m11 * m43 * m32 - m42 * m31 * m13 + m41 * m13 * m32 - m41 * m12 * m33 + m43 * m31 * m12) * invDet,
                        -( m43 * m11 * m22 - m43 * m21 * m12 - m41 * m13 * m22 - m42 * m11 * m23 + m42 * m21 * m13 + m41 * m12 * m23) * invDet,
                        -(-m33 * m11 * m22 + m33 * m21 * m12 + m31 * m13 * m22 + m32 * m11 * m23 - m32 * m21 * m13 - m31 * m12 * m23) * invDet);
    }

    inline
    Bool
    Matrix44f::operator==(const Matrix44f& other) const
    {
        return PlatformMemory::Compare(&m11, &other.m11, 16 * sizeof(Float)) == 0;
    }

    inline
    Bool
    Matrix44f::operator!=(const Matrix44f& other) const
    {
        return !operator==(other);
    }

    inline
    Float&
    Matrix44f::m(const UInt32 i, const UInt32 j)
    {
        return (&m11)[4 * j + i];
    }

    inline
    const Float&
    Matrix44f::m(const UInt32 i, const UInt32 j) const
    {
        return (&m11)[4 * j + i];
    }

    inline
    Matrix44f Matrix44f::RotationEulerZYX(const Vector3& rotationXYZ)
    {
        return Matrix44f(Matrix33f::RotationEulerZYX(rotationXYZ));
    }

    inline
    Matrix44f Matrix44f::RotationEulerZYX(const Float x, const Float y, const Float z)
    {
        return RotationEulerZYX(Vector3(x, y, z));
    }

    static_assert(std::is_nothrow_move_constructible<Matrix44f>::value &&
        std::is_nothrow_move_assignable<Matrix44f>::value, "Matrix44f must be movable");
}

#endif
