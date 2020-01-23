//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Matrix44Test.h"

namespace ramses_internal
{
    Matrix44Test::Matrix44Test()
        : mat1(Matrix44f(  1.0f,  2.0f,  3.0f,  4.0f
                        ,  5.0f,  6.0f,  7.0f,  8.0f
                        ,  9.0f, 10.0f, 11.0f, 12.0f
                        , 13.0f, 14.0f, 15.0f, 16.0f))
    {
    }

    TEST_F(Matrix44Test, DefaultConstructor)
    {
        Matrix44f mat2;
        EXPECT_EQ(1.0f, mat2.m11);
        EXPECT_EQ(0.0f, mat2.m12);
        EXPECT_EQ(0.0f, mat2.m13);
        EXPECT_EQ(0.0f, mat2.m14);
        EXPECT_EQ(0.0f, mat2.m21);
        EXPECT_EQ(1.0f, mat2.m22);
        EXPECT_EQ(0.0f, mat2.m23);
        EXPECT_EQ(0.0f, mat2.m24);
        EXPECT_EQ(0.0f, mat2.m31);
        EXPECT_EQ(0.0f, mat2.m32);
        EXPECT_EQ(1.0f, mat2.m33);
        EXPECT_EQ(0.0f, mat2.m34);
        EXPECT_EQ(0.0f, mat2.m41);
        EXPECT_EQ(0.0f, mat2.m42);
        EXPECT_EQ(0.0f, mat2.m43);
        EXPECT_EQ(1.0f, mat2.m44);
    }

    TEST_F(Matrix44Test, CopyConstructor)
    {
        Matrix44f mat2 = mat1;
        EXPECT_EQ(1.0f, mat2.m11);
        EXPECT_EQ(2.0f, mat2.m12);
        EXPECT_EQ(3.0f, mat2.m13);
        EXPECT_EQ(4.0f, mat2.m14);
        EXPECT_EQ(5.0f, mat2.m21);
        EXPECT_EQ(6.0f, mat2.m22);
        EXPECT_EQ(7.0f, mat2.m23);
        EXPECT_EQ(8.0f, mat2.m24);
        EXPECT_EQ(9.0f, mat2.m31);
        EXPECT_EQ(10.0f, mat2.m32);
        EXPECT_EQ(11.0f, mat2.m33);
        EXPECT_EQ(12.0f, mat2.m34);
        EXPECT_EQ(13.0f, mat2.m41);
        EXPECT_EQ(14.0f, mat2.m42);
        EXPECT_EQ(15.0f, mat2.m43);
        EXPECT_EQ(16.0f, mat2.m44);
    }

    TEST_F(Matrix44Test, ValueConstructor)
    {
        EXPECT_EQ(1.0f, mat1.m11);
        EXPECT_EQ(2.0f, mat1.m12);
        EXPECT_EQ(3.0f, mat1.m13);
        EXPECT_EQ(4.0f, mat1.m14);
        EXPECT_EQ(5.0f, mat1.m21);
        EXPECT_EQ(6.0f, mat1.m22);
        EXPECT_EQ(7.0f, mat1.m23);
        EXPECT_EQ(8.0f, mat1.m24);
        EXPECT_EQ(9.0f, mat1.m31);
        EXPECT_EQ(10.0f, mat1.m32);
        EXPECT_EQ(11.0f, mat1.m33);
        EXPECT_EQ(12.0f, mat1.m34);
        EXPECT_EQ(13.0f, mat1.m41);
        EXPECT_EQ(14.0f, mat1.m42);
        EXPECT_EQ(15.0f, mat1.m43);
        EXPECT_EQ(16.0f, mat1.m44);
    }

    TEST_F(Matrix44Test, ValueArrayConstructor)
    {
        const float valueArray[16] =
        {
            1.0f, 2.0f, 3.0f, 4.0f,
            5.0f, 6.0f, 7.0f, 8.0f,
            9.0f, 10.0f, 11.0f, 12.0f,
            13.0f, 14.0f, 15.0f, 16.0f,
        };

        const Matrix44f mat2(valueArray);

        EXPECT_EQ(1.0f, mat2.m11);
        EXPECT_EQ(2.0f, mat2.m12);
        EXPECT_EQ(3.0f, mat2.m13);
        EXPECT_EQ(4.0f, mat2.m14);
        EXPECT_EQ(5.0f, mat2.m21);
        EXPECT_EQ(6.0f, mat2.m22);
        EXPECT_EQ(7.0f, mat2.m23);
        EXPECT_EQ(8.0f, mat2.m24);
        EXPECT_EQ(9.0f, mat2.m31);
        EXPECT_EQ(10.0f, mat2.m32);
        EXPECT_EQ(11.0f, mat2.m33);
        EXPECT_EQ(12.0f, mat2.m34);
        EXPECT_EQ(13.0f, mat2.m41);
        EXPECT_EQ(14.0f, mat2.m42);
        EXPECT_EQ(15.0f, mat2.m43);
        EXPECT_EQ(16.0f, mat2.m44);
    }

    TEST_F(Matrix44Test, VectorConstructor)
    {
        Vector4 row1(15.f, 14.f, 13.f, 12.f);
        Vector4 row2(11.f, 10.f,  9.f,  8.f);
        Vector4 row3(7.f,  6.f,  5.f,  4.f);
        Vector4 row4(3.f,  2.f,  1.f,  0.f);

        Matrix44f mat2(row1, row2, row3, row4);

        EXPECT_EQ(15.0f, mat2.m11);
        EXPECT_EQ(14.0f, mat2.m12);
        EXPECT_EQ(13.0f, mat2.m13);
        EXPECT_EQ(12.0f, mat2.m14);
        EXPECT_EQ(11.0f, mat2.m21);
        EXPECT_EQ(10.0f, mat2.m22);
        EXPECT_EQ(9.0f, mat2.m23);
        EXPECT_EQ(8.0f, mat2.m24);
        EXPECT_EQ(7.0f, mat2.m31);
        EXPECT_EQ(6.0f, mat2.m32);
        EXPECT_EQ(5.0f, mat2.m33);
        EXPECT_EQ(4.0f, mat2.m34);
        EXPECT_EQ(3.0f, mat2.m41);
        EXPECT_EQ(2.0f, mat2.m42);
        EXPECT_EQ(1.0f, mat2.m43);
        EXPECT_EQ(0.0f, mat2.m44);
    }

    TEST_F(Matrix44Test, IdentityMatrix)
    {
        Matrix44f identity = Matrix44f::Identity;

        EXPECT_EQ(1.0f, identity.m11);
        EXPECT_EQ(0.0f, identity.m12);
        EXPECT_EQ(0.0f, identity.m13);
        EXPECT_EQ(0.0f, identity.m14);
        EXPECT_EQ(0.0f, identity.m21);
        EXPECT_EQ(1.0f, identity.m22);
        EXPECT_EQ(0.0f, identity.m23);
        EXPECT_EQ(0.0f, identity.m24);
        EXPECT_EQ(0.0f, identity.m31);
        EXPECT_EQ(0.0f, identity.m32);
        EXPECT_EQ(1.0f, identity.m33);
        EXPECT_EQ(0.0f, identity.m34);
        EXPECT_EQ(0.0f, identity.m41);
        EXPECT_EQ(0.0f, identity.m42);
        EXPECT_EQ(0.0f, identity.m43);
        EXPECT_EQ(1.0f, identity.m44);
    }

    TEST_F(Matrix44Test, EulerRotationMatrixZYX)
    {
        Matrix44f noRotation = Matrix44f::RotationEulerZYX(0, 0, 0);

        EXPECT_NEAR(1.0f, noRotation.m11, 0.00001f);
        EXPECT_NEAR(0.0f, noRotation.m12, 0.00001f);
        EXPECT_NEAR(0.0f, noRotation.m13, 0.00001f);
        EXPECT_NEAR(0.0f, noRotation.m14, 0.00001f);
        EXPECT_NEAR(0.0f, noRotation.m21, 0.00001f);
        EXPECT_NEAR(1.0f, noRotation.m22, 0.00001f);
        EXPECT_NEAR(0.0f, noRotation.m23, 0.00001f);
        EXPECT_NEAR(0.0f, noRotation.m24, 0.00001f);
        EXPECT_NEAR(0.0f, noRotation.m31, 0.00001f);
        EXPECT_NEAR(0.0f, noRotation.m32, 0.00001f);
        EXPECT_NEAR(1.0f, noRotation.m33, 0.00001f);
        EXPECT_NEAR(0.0f, noRotation.m34, 0.00001f);
        EXPECT_NEAR(0.0f, noRotation.m41, 0.00001f);
        EXPECT_NEAR(0.0f, noRotation.m42, 0.00001f);
        EXPECT_NEAR(0.0f, noRotation.m43, 0.00001f);
        EXPECT_NEAR(1.0f, noRotation.m44, 0.00001f);
    }

    TEST_F(Matrix44Test, EulerRotationXMatrixZYX)
    {
        Matrix44f rotationX90 = Matrix44f::RotationEulerZYX(90, 0, 0);

        EXPECT_NEAR(1.0f, rotationX90.m11, 0.00001f);
        EXPECT_NEAR(0.0f, rotationX90.m12, 0.00001f);
        EXPECT_NEAR(0.0f, rotationX90.m13, 0.00001f);
        EXPECT_NEAR(0.0f, rotationX90.m14, 0.00001f);
        EXPECT_NEAR(0.0f, rotationX90.m21, 0.00001f);
        EXPECT_NEAR(0.0f, rotationX90.m22, 0.00001f);
        EXPECT_NEAR(1.0f, rotationX90.m23, 0.00001f);
        EXPECT_NEAR(0.0f, rotationX90.m24, 0.00001f);
        EXPECT_NEAR(0.0f, rotationX90.m31, 0.00001f);
        EXPECT_NEAR(-1.f, rotationX90.m32, 0.00001f);
        EXPECT_NEAR(0.0f, rotationX90.m33, 0.00001f);
        EXPECT_NEAR(0.0f, rotationX90.m34, 0.00001f);
        EXPECT_NEAR(0.0f, rotationX90.m41, 0.00001f);
        EXPECT_NEAR(0.0f, rotationX90.m42, 0.00001f);
        EXPECT_NEAR(0.0f, rotationX90.m43, 0.00001f);
        EXPECT_NEAR(1.0f, rotationX90.m44, 0.00001f);

        Matrix44f rotationX135 = Matrix44f::RotationEulerZYX(135, 0, 0);

        EXPECT_NEAR(1.0f, rotationX135.m11, 0.00001f);
        EXPECT_NEAR(0.0f, rotationX135.m12, 0.00001f);
        EXPECT_NEAR(0.0f, rotationX135.m13, 0.00001f);
        EXPECT_NEAR(0.0f, rotationX135.m14, 0.00001f);
        EXPECT_NEAR(0.0f, rotationX135.m21, 0.00001f);
        EXPECT_NEAR(-0.70710676f, rotationX135.m22, 0.00001f);
        EXPECT_NEAR(0.70710676f, rotationX135.m23, 0.00001f);
        EXPECT_NEAR(0.0f, rotationX135.m24, 0.00001f);
        EXPECT_NEAR(0.0f, rotationX135.m31, 0.00001f);
        EXPECT_NEAR(-0.70710676f, rotationX135.m32, 0.00001f);
        EXPECT_NEAR(-0.70710676f, rotationX135.m33, 0.00001f);
        EXPECT_NEAR(0.0f, rotationX135.m34, 0.00001f);
        EXPECT_NEAR(0.0f, rotationX135.m41, 0.00001f);
        EXPECT_NEAR(0.0f, rotationX135.m42, 0.00001f);
        EXPECT_NEAR(0.0f, rotationX135.m43, 0.00001f);
        EXPECT_NEAR(1.0f, rotationX135.m44, 0.00001f);
    }

    TEST_F(Matrix44Test, EulerRotationYMatrixZYX)
    {
        Matrix44f rotationY90 = Matrix44f::RotationEulerZYX(0, 90, 0);

        EXPECT_NEAR(0.0f, rotationY90.m11, 0.00001f);
        EXPECT_NEAR(0.0f, rotationY90.m12, 0.00001f);
        EXPECT_NEAR(-1.0f, rotationY90.m13, 0.00001f);
        EXPECT_NEAR(0.0f, rotationY90.m14, 0.00001f);
        EXPECT_NEAR(0.0f, rotationY90.m21, 0.00001f);
        EXPECT_NEAR(1.0f, rotationY90.m22, 0.00001f);
        EXPECT_NEAR(0.0f, rotationY90.m23, 0.00001f);
        EXPECT_NEAR(0.0f, rotationY90.m24, 0.00001f);
        EXPECT_NEAR(1.0f, rotationY90.m31, 0.00001f);
        EXPECT_NEAR(0.f, rotationY90.m32, 0.00001f);
        EXPECT_NEAR(0.0f, rotationY90.m33, 0.00001f);
        EXPECT_NEAR(0.0f, rotationY90.m34, 0.00001f);
        EXPECT_NEAR(0.0f, rotationY90.m41, 0.00001f);
        EXPECT_NEAR(0.0f, rotationY90.m42, 0.00001f);
        EXPECT_NEAR(0.0f, rotationY90.m43, 0.00001f);
        EXPECT_NEAR(1.0f, rotationY90.m44, 0.00001f);

        Matrix44f rotationY135 = Matrix44f::RotationEulerZYX(0, 135, 0);

        EXPECT_NEAR(-0.70710676f, rotationY135.m11, 0.00001f);
        EXPECT_NEAR(0.0f, rotationY135.m12, 0.00001f);
        EXPECT_NEAR(-0.70710676f, rotationY135.m13, 0.00001f);
        EXPECT_NEAR(0.0f, rotationY135.m14, 0.00001f);
        EXPECT_NEAR(0.0f, rotationY135.m21, 0.00001f);
        EXPECT_NEAR(1.0f, rotationY135.m22, 0.00001f);
        EXPECT_NEAR(0.0f, rotationY135.m23, 0.00001f);
        EXPECT_NEAR(0.0f, rotationY135.m24, 0.00001f);
        EXPECT_NEAR(0.70710676f, rotationY135.m31, 0.00001f);
        EXPECT_NEAR(0.0f, rotationY135.m32, 0.00001f);
        EXPECT_NEAR(-0.70710676f, rotationY135.m33, 0.00001f);
        EXPECT_NEAR(0.0f, rotationY135.m34, 0.00001f);
        EXPECT_NEAR(0.0f, rotationY135.m41, 0.00001f);
        EXPECT_NEAR(0.0f, rotationY135.m42, 0.00001f);
        EXPECT_NEAR(0.0f, rotationY135.m43, 0.00001f);
        EXPECT_NEAR(1.0f, rotationY135.m44, 0.00001f);
    }

    TEST_F(Matrix44Test, EulerRotationZMatrixZYX)
    {
        Matrix44f rotationZ90 = Matrix44f::RotationEulerZYX(0, 0, 90);

        EXPECT_NEAR(0.0f, rotationZ90.m11, 0.00001f);
        EXPECT_NEAR(1.0f, rotationZ90.m12, 0.00001f);
        EXPECT_NEAR(0.0f, rotationZ90.m13, 0.00001f);
        EXPECT_NEAR(0.0f, rotationZ90.m14, 0.00001f);
        EXPECT_NEAR(-1.0f, rotationZ90.m21, 0.00001f);
        EXPECT_NEAR(0.0f, rotationZ90.m22, 0.00001f);
        EXPECT_NEAR(0.0f, rotationZ90.m23, 0.00001f);
        EXPECT_NEAR(0.0f, rotationZ90.m24, 0.00001f);
        EXPECT_NEAR(0.0f, rotationZ90.m31, 0.00001f);
        EXPECT_NEAR(0.0f, rotationZ90.m32, 0.00001f);
        EXPECT_NEAR(1.0f, rotationZ90.m33, 0.00001f);
        EXPECT_NEAR(0.0f, rotationZ90.m34, 0.00001f);
        EXPECT_NEAR(0.0f, rotationZ90.m41, 0.00001f);
        EXPECT_NEAR(0.0f, rotationZ90.m42, 0.00001f);
        EXPECT_NEAR(0.0f, rotationZ90.m43, 0.00001f);
        EXPECT_NEAR(1.0f, rotationZ90.m44, 0.00001f);

        Matrix44f rotationZ135 = Matrix44f::RotationEulerZYX(0, 0, 135);

        EXPECT_NEAR(-0.70710676f, rotationZ135.m11, 0.00001f);
        EXPECT_NEAR(0.70710676f, rotationZ135.m12, 0.00001f);
        EXPECT_NEAR(0.0f, rotationZ135.m13, 0.00001f);
        EXPECT_NEAR(0.0f, rotationZ135.m14, 0.00001f);
        EXPECT_NEAR(-0.70710676f, rotationZ135.m21, 0.00001f);
        EXPECT_NEAR(-0.70710676f, rotationZ135.m22, 0.00001f);
        EXPECT_NEAR(0.0f, rotationZ135.m23, 0.00001f);
        EXPECT_NEAR(0.0f, rotationZ135.m24, 0.00001f);
        EXPECT_NEAR(0.0f, rotationZ135.m31, 0.00001f);
        EXPECT_NEAR(0.0f, rotationZ135.m32, 0.00001f);
        EXPECT_NEAR(1.0f, rotationZ135.m33, 0.00001f);
        EXPECT_NEAR(0.0f, rotationZ135.m34, 0.00001f);
        EXPECT_NEAR(0.0f, rotationZ135.m41, 0.00001f);
        EXPECT_NEAR(0.0f, rotationZ135.m42, 0.00001f);
        EXPECT_NEAR(0.0f, rotationZ135.m43, 0.00001f);
        EXPECT_NEAR(1.0f, rotationZ135.m44, 0.00001f);
    }

    TEST_F(Matrix44Test, EmptyMatrix)
    {
        Matrix44f identity = Matrix44f::Empty;

        EXPECT_EQ(0.0f, identity.m11);
        EXPECT_EQ(0.0f, identity.m12);
        EXPECT_EQ(0.0f, identity.m13);
        EXPECT_EQ(0.0f, identity.m14);
        EXPECT_EQ(0.0f, identity.m21);
        EXPECT_EQ(0.0f, identity.m22);
        EXPECT_EQ(0.0f, identity.m23);
        EXPECT_EQ(0.0f, identity.m24);
        EXPECT_EQ(0.0f, identity.m31);
        EXPECT_EQ(0.0f, identity.m32);
        EXPECT_EQ(0.0f, identity.m33);
        EXPECT_EQ(0.0f, identity.m34);
        EXPECT_EQ(0.0f, identity.m41);
        EXPECT_EQ(0.0f, identity.m42);
        EXPECT_EQ(0.0f, identity.m43);
        EXPECT_EQ(0.0f, identity.m44);
    }

    TEST_F(Matrix44Test, VectorMultiplication)
    {
        Vector4 vec1(1.f, 2.f, 3.f, 4.f);
        Vector4 vec2 = mat1 * vec1;

        EXPECT_EQ(30.f , vec2.x);
        EXPECT_EQ(70.f , vec2.y);
        EXPECT_EQ(110.f, vec2.z);
        EXPECT_EQ(150.f, vec2.w);
    }

    TEST_F(Matrix44Test, MatrixMultiplication)
    {
        Matrix44f mat2(16.f, 15.f, 14.f, 13.f
                    , 12.f, 11.f, 10.f,  9.f
                    ,  8.f,  7.f,  6.f,  5.f
                    ,  4.f,  3.f,  2.f,  1.f);

        Matrix44f mat3 = mat1 * mat2;

        EXPECT_EQ(80.0f, mat3.m11);
        EXPECT_EQ(70.0f, mat3.m12);
        EXPECT_EQ(60.0f, mat3.m13);
        EXPECT_EQ(50.0f, mat3.m14);
        EXPECT_EQ(240.0f, mat3.m21);
        EXPECT_EQ(214.0f, mat3.m22);
        EXPECT_EQ(188.0f, mat3.m23);
        EXPECT_EQ(162.0f, mat3.m24);
        EXPECT_EQ(400.0f, mat3.m31);
        EXPECT_EQ(358.0f, mat3.m32);
        EXPECT_EQ(316.0f, mat3.m33);
        EXPECT_EQ(274.0f, mat3.m34);
        EXPECT_EQ(560.0f, mat3.m41);
        EXPECT_EQ(502.0f, mat3.m42);
        EXPECT_EQ(444.0f, mat3.m43);
        EXPECT_EQ(386.0f, mat3.m44);
    }

    TEST_F(Matrix44Test, MatrixMultiplicationAndAssign)
    {
        Matrix44f mat2(16.f, 15.f, 14.f, 13.f
                    , 12.f, 11.f, 10.f,  9.f
                    ,  8.f,  7.f,  6.f,  5.f
                    ,  4.f,  3.f,  2.f,  1.f);

        mat1 *= mat2;

        EXPECT_EQ(80.0f, mat1.m11);
        EXPECT_EQ(70.0f, mat1.m12);
        EXPECT_EQ(60.0f, mat1.m13);
        EXPECT_EQ(50.0f, mat1.m14);
        EXPECT_EQ(240.0f, mat1.m21);
        EXPECT_EQ(214.0f, mat1.m22);
        EXPECT_EQ(188.0f, mat1.m23);
        EXPECT_EQ(162.0f, mat1.m24);
        EXPECT_EQ(400.0f, mat1.m31);
        EXPECT_EQ(358.0f, mat1.m32);
        EXPECT_EQ(316.0f, mat1.m33);
        EXPECT_EQ(274.0f, mat1.m34);
        EXPECT_EQ(560.0f, mat1.m41);
        EXPECT_EQ(502.0f, mat1.m42);
        EXPECT_EQ(444.0f, mat1.m43);
        EXPECT_EQ(386.0f, mat1.m44);
    }

    TEST_F(Matrix44Test, Equality)
    {
        Matrix44f mat2(  1.0f,  2.0f,  3.0f,  4.0f
                        ,  5.0f,  6.0f,  7.0f,  8.0f
                        ,  9.0f, 10.0f, 11.0f, 12.0f
                        , 13.0f, 14.0f, 15.0f, 16.0f);

        bool equal = mat1 == mat2;
        EXPECT_EQ(true, equal);
    }

    TEST_F(Matrix44Test, UnEquality)
    {
        Matrix44f mat2(     0.0f,  2.0f,  3.0f,  4.0f
                        ,  5.0f,  6.0f,  7.0f,  8.0f
                        ,  9.0f, 10.0f, 11.0f, 12.0f
                        , 13.0f, 14.0f, 15.0f, 16.0f);

        bool unequal = mat1 != mat2;
        EXPECT_EQ(true, unequal);
    }

    TEST_F(Matrix44Test, Transpose)
    {
        Matrix44f mat2 = mat1.transpose();

        EXPECT_EQ(1.0f, mat2.m11);
        EXPECT_EQ(5.0f, mat2.m12);
        EXPECT_EQ(9.0f, mat2.m13);
        EXPECT_EQ(13.0f, mat2.m14);
        EXPECT_EQ(2.0f, mat2.m21);
        EXPECT_EQ(6.0f, mat2.m22);
        EXPECT_EQ(10.0f, mat2.m23);
        EXPECT_EQ(14.0f, mat2.m24);
        EXPECT_EQ(3.0f, mat2.m31);
        EXPECT_EQ(7.0f, mat2.m32);
        EXPECT_EQ(11.0f, mat2.m33);
        EXPECT_EQ(15.0f, mat2.m34);
        EXPECT_EQ(4.0f, mat2.m41);
        EXPECT_EQ(8.0f, mat2.m42);
        EXPECT_EQ(12.0f, mat2.m43);
        EXPECT_EQ(16.0f, mat2.m44);
    }

    TEST_F(Matrix44Test, SetSingleValues)
    {
        mat1.set(        1.0f,    2.0f,     4.0f,     8.0f
                    ,   16.0f,   32.0f,    64.0f,   128.0f
                    ,  256.0f,  512.0f,  1024.0f,  2048.0f
                    , 4096.0f, 8096.0f, 16192.0f, 32384.0f);

        EXPECT_EQ(1.0f, mat1.m11);
        EXPECT_EQ(2.0f, mat1.m12);
        EXPECT_EQ(4.0f, mat1.m13);
        EXPECT_EQ(8.0f, mat1.m14);
        EXPECT_EQ(16.0f, mat1.m21);
        EXPECT_EQ(32.0f, mat1.m22);
        EXPECT_EQ(64.0f, mat1.m23);
        EXPECT_EQ(128.0f, mat1.m24);
        EXPECT_EQ(256.0f, mat1.m31);
        EXPECT_EQ(512.0f, mat1.m32);
        EXPECT_EQ(1024.0f, mat1.m33);
        EXPECT_EQ(2048.0f, mat1.m34);
        EXPECT_EQ(4096.0f, mat1.m41);
        EXPECT_EQ(8096.0f, mat1.m42);
        EXPECT_EQ(16192.0f, mat1.m43);
        EXPECT_EQ(32384.0f, mat1.m44);
    }

    TEST_F(Matrix44Test, SetValues)
    {
        mat1.set(42.0f);

        EXPECT_EQ(42.0f, mat1.m11);
        EXPECT_EQ(42.0f, mat1.m12);
        EXPECT_EQ(42.0f, mat1.m13);
        EXPECT_EQ(42.0f, mat1.m14);
        EXPECT_EQ(42.0f, mat1.m21);
        EXPECT_EQ(42.0f, mat1.m22);
        EXPECT_EQ(42.0f, mat1.m23);
        EXPECT_EQ(42.0f, mat1.m24);
        EXPECT_EQ(42.0f, mat1.m31);
        EXPECT_EQ(42.0f, mat1.m32);
        EXPECT_EQ(42.0f, mat1.m33);
        EXPECT_EQ(42.0f, mat1.m34);
        EXPECT_EQ(42.0f, mat1.m41);
        EXPECT_EQ(42.0f, mat1.m42);
        EXPECT_EQ(42.0f, mat1.m43);
        EXPECT_EQ(42.0f, mat1.m44);
    }

    TEST_F(Matrix44Test, AccessByIndex)
    {
        EXPECT_EQ(1.0f, mat1.m(0, 0));
        EXPECT_EQ(2.0f, mat1.m(0, 1));
        EXPECT_EQ(3.0f, mat1.m(0, 2));
        EXPECT_EQ(4.0f, mat1.m(0, 3));
        EXPECT_EQ(5.0f, mat1.m(1, 0));
        EXPECT_EQ(6.0f, mat1.m(1, 1));
        EXPECT_EQ(7.0f, mat1.m(1, 2));
        EXPECT_EQ(8.0f, mat1.m(1, 3));
        EXPECT_EQ(9.0f, mat1.m(2, 0));
        EXPECT_EQ(10.0f, mat1.m(2, 1));
        EXPECT_EQ(11.0f, mat1.m(2, 2));
        EXPECT_EQ(12.0f, mat1.m(2, 3));
        EXPECT_EQ(13.0f, mat1.m(3, 0));
        EXPECT_EQ(14.0f, mat1.m(3, 1));
        EXPECT_EQ(15.0f, mat1.m(3, 2));
        EXPECT_EQ(16.0f, mat1.m(3, 3));
    }

    TEST_F(Matrix44Test, Determinant)
    {
        mat1 = Matrix44f::Identity;
        Float det = mat1.determinant();

        EXPECT_EQ(1.0f, det);

        mat1.m11 =  0.88446331f;
        mat1.m12 =  0.00000000f;
        mat1.m13 =  0.46660975f;
        mat1.m14 =  0.00000000f;
        mat1.m21 =  0.00000000f;
        mat1.m22 =  1.0000000f;
        mat1.m23 =  0.00000000f;
        mat1.m24 =  0.00000000f;
        mat1.m31 = -0.46660975f;
        mat1.m32 =  0.00000000f;
        mat1.m33 =  0.88446331f;
        mat1.m34 =  0.00000000f;
        mat1.m41 =  0.00000000f;
        mat1.m42 =  0.00000000f;
        mat1.m43 =  0.00000000f;
        mat1.m44 =  1.0000000f;

        det =  mat1.determinant();

        EXPECT_FLOAT_EQ(1.00000f, det);
    }

    TEST_F(Matrix44Test, Inverse)
    {
        mat1 = Matrix44f::Identity;
        Matrix44f inv = mat1.inverse();

        EXPECT_EQ(1.0f, mat1.m11);
        EXPECT_EQ(0.0f, mat1.m12);
        EXPECT_EQ(0.0f, mat1.m13);
        EXPECT_EQ(0.0f, mat1.m14);
        EXPECT_EQ(0.0f, mat1.m21);
        EXPECT_EQ(1.0f, mat1.m22);
        EXPECT_EQ(0.0f, mat1.m23);
        EXPECT_EQ(0.0f, mat1.m24);
        EXPECT_EQ(0.0f, mat1.m31);
        EXPECT_EQ(0.0f, mat1.m32);
        EXPECT_EQ(1.0f, mat1.m33);
        EXPECT_EQ(0.0f, mat1.m34);
        EXPECT_EQ(0.0f, mat1.m41);
        EXPECT_EQ(0.0f, mat1.m42);
        EXPECT_EQ(0.0f, mat1.m43);
        EXPECT_EQ(1.0f, mat1.m44);

        mat1.m11 =  0.88446331f;
        mat1.m12 =  0.00000000f;
        mat1.m13 =  0.46660975f;
        mat1.m14 =  0.00000000f;
        mat1.m21 =  0.00000000f;
        mat1.m22 =  1.0000000f;
        mat1.m23 =  0.00000000f;
        mat1.m24 =  0.00000000f;
        mat1.m31 = -0.46660975f;
        mat1.m32 =  0.00000000f;
        mat1.m33 =  0.88446331f;
        mat1.m34 =  0.00000000f;
        mat1.m41 =  0.00000000f;
        mat1.m42 =  0.00000000f;
        mat1.m43 =  0.00000000f;
        mat1.m44 =  1.0000000f;

        inv = mat1.inverse();

        EXPECT_EQ(0.88446331f, inv.m11);
        EXPECT_EQ(0.0f, inv.m12);
        EXPECT_EQ(-0.46660975f, inv.m13);
        EXPECT_EQ(0.00000000f, inv.m14);
        EXPECT_EQ(0.00000000f, inv.m21);
        EXPECT_EQ(1.0f, inv.m22);
        EXPECT_EQ(0.00000000f, inv.m23);
        EXPECT_EQ(0.00000000f, inv.m24);
        EXPECT_EQ(0.46660975f, inv.m31);
        EXPECT_EQ(0.0f, inv.m32);
        EXPECT_EQ(0.88446331f, inv.m33);
        EXPECT_EQ(0.00000000f, inv.m34);
        EXPECT_EQ(0.00000000f, inv.m41);
        EXPECT_EQ(0.0f, inv.m42);
        EXPECT_EQ(0.00000000f, inv.m43);
        EXPECT_EQ(1.00000000f, inv.m44);

        Matrix44f identity = inv * mat1;

        EXPECT_EQ(1.0f, identity.m11);
        EXPECT_EQ(0.0f, identity.m12);
        EXPECT_EQ(0.0f, identity.m13);
        EXPECT_EQ(0.0f, identity.m14);
        EXPECT_EQ(0.0f, identity.m21);
        EXPECT_EQ(1.0f, identity.m22);
        EXPECT_EQ(0.0f, identity.m23);
        EXPECT_EQ(0.0f, identity.m24);
        EXPECT_EQ(0.0f, identity.m31);
        EXPECT_EQ(0.0f, identity.m32);
        EXPECT_EQ(1.0f, identity.m33);
        EXPECT_EQ(0.0f, identity.m34);
        EXPECT_EQ(0.0f, identity.m41);
        EXPECT_EQ(0.0f, identity.m42);
        EXPECT_EQ(0.0f, identity.m43);
        EXPECT_EQ(1.0f, identity.m44);
    }

    TEST_F(Matrix44Test, GetScalingMatrix)
    {
        Float factorX = 0.25f;
        Float factorY = 0.33f;
        Float factorZ = 0.5f;
        Matrix44f scale = Matrix44f::Scaling(factorX, factorY, factorZ);

        EXPECT_FLOAT_EQ(factorX, scale.m11);
        EXPECT_EQ(0.0f, scale.m12);
        EXPECT_EQ(0.0f, scale.m13);
        EXPECT_EQ(0.0f, scale.m14);
        EXPECT_EQ(0.0f, scale.m21);
        EXPECT_FLOAT_EQ(factorY, scale.m22);
        EXPECT_EQ(0.0f, scale.m23);
        EXPECT_EQ(0.0f, scale.m24);
        EXPECT_EQ(0.0f, scale.m31);
        EXPECT_EQ(0.0f, scale.m32);
        EXPECT_FLOAT_EQ(factorZ, scale.m33);
        EXPECT_EQ(0.0f, scale.m34);
        EXPECT_EQ(0.0f, scale.m41);
        EXPECT_EQ(0.0f, scale.m42);
        EXPECT_EQ(0.0f, scale.m43);
        EXPECT_EQ(1.0f, scale.m44);
    }

    TEST_F(Matrix44Test, RotateUsingMatrix44ZYX)
    {
        Matrix44f rotationZ90 = Matrix44f::RotationEulerZYX(0, 0, 90);
        Vector3 vec(1.f, 1.f, 0.f);

        Vector3 rotated = rotationZ90.rotate(vec);

        EXPECT_FLOAT_EQ(rotated.x, 1.0);
        EXPECT_FLOAT_EQ(rotated.y, -1.0);
        EXPECT_FLOAT_EQ(rotated.z, 0.0);
    }
}
