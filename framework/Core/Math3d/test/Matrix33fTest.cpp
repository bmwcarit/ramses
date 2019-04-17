//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------
#include "gtest/gtest.h"
#include "Math3d/Matrix33f.h"

namespace ramses_internal
{
    class Matrix33fTest : public testing::Test
    {
    public:
        Matrix33fTest();
    protected:
        ramses_internal::Matrix33f mat1;
    };

    Matrix33fTest::Matrix33fTest()
        : mat1(Matrix33f(  1.0f,  2.0f,  3.0f
                        ,  4.0f,  5.0f,  6.0f
                        ,  7.0f,  8.0f,  9.0f))
    {
    }

    TEST_F(Matrix33fTest, DefaultConstructor)
    {
        Matrix33f mat2;
        EXPECT_EQ(1.0f, mat2.m11);
        EXPECT_EQ(0.0f, mat2.m12);
        EXPECT_EQ(0.0f, mat2.m13);
        EXPECT_EQ(0.0f, mat2.m21);
        EXPECT_EQ(1.0f, mat2.m22);
        EXPECT_EQ(0.0f, mat2.m23);
        EXPECT_EQ(0.0f, mat2.m31);
        EXPECT_EQ(0.0f, mat2.m32);
        EXPECT_EQ(1.0f, mat2.m33);
    }

    TEST_F(Matrix33fTest, CopyConstructor)
    {
        Matrix33f mat2 = mat1;
        EXPECT_EQ(1.0f, mat2.m11);
        EXPECT_EQ(2.0f, mat2.m12);
        EXPECT_EQ(3.0f, mat2.m13);
        EXPECT_EQ(4.0f, mat2.m21);
        EXPECT_EQ(5.0f, mat2.m22);
        EXPECT_EQ(6.0f, mat2.m23);
        EXPECT_EQ(7.0f, mat2.m31);
        EXPECT_EQ(8.0f, mat2.m32);
        EXPECT_EQ(9.0f, mat2.m33);
    }

    TEST_F(Matrix33fTest, ValueConstructor)
    {
        EXPECT_EQ(1.0f, mat1.m11);
        EXPECT_EQ(2.0f, mat1.m12);
        EXPECT_EQ(3.0f, mat1.m13);
        EXPECT_EQ(4.0f, mat1.m21);
        EXPECT_EQ(5.0f, mat1.m22);
        EXPECT_EQ(6.0f, mat1.m23);
        EXPECT_EQ(7.0f, mat1.m31);
        EXPECT_EQ(8.0f, mat1.m32);
        EXPECT_EQ(9.0f, mat1.m33);
    }

    TEST_F(Matrix33fTest, VectorConstructor)
    {
        Vector3 row1(8.f, 7.f, 6.f);
        Vector3 row2(5.f, 4.f,  3.f);
        Vector3 row3(2.f,  1.f,  0.f);

        Matrix33f mat2(row1, row2, row3);

        EXPECT_EQ(8.0f, mat2.m11);
        EXPECT_EQ(7.0f, mat2.m12);
        EXPECT_EQ(6.0f, mat2.m13);
        EXPECT_EQ(5.0f, mat2.m21);
        EXPECT_EQ(4.0f, mat2.m22);
        EXPECT_EQ(3.0f, mat2.m23);
        EXPECT_EQ(2.0f, mat2.m31);
        EXPECT_EQ(1.0f, mat2.m32);
        EXPECT_EQ(0.0f, mat2.m33);
    }

    TEST_F(Matrix33fTest, IdentityMatrix)
    {
        Matrix33f identity = Matrix33f::Identity;

        EXPECT_EQ(1.0f, identity.m11);
        EXPECT_EQ(0.0f, identity.m12);
        EXPECT_EQ(0.0f, identity.m13);
        EXPECT_EQ(0.0f, identity.m21);
        EXPECT_EQ(1.0f, identity.m22);
        EXPECT_EQ(0.0f, identity.m23);
        EXPECT_EQ(0.0f, identity.m31);
        EXPECT_EQ(0.0f, identity.m32);
        EXPECT_EQ(1.0f, identity.m33);
    }

    TEST_F(Matrix33fTest, EmptyMatrix)
    {
        Matrix33f identity = Matrix33f::Empty;

        EXPECT_EQ(0.0f, identity.m11);
        EXPECT_EQ(0.0f, identity.m12);
        EXPECT_EQ(0.0f, identity.m13);
        EXPECT_EQ(0.0f, identity.m21);
        EXPECT_EQ(0.0f, identity.m22);
        EXPECT_EQ(0.0f, identity.m23);
        EXPECT_EQ(0.0f, identity.m31);
        EXPECT_EQ(0.0f, identity.m32);
        EXPECT_EQ(0.0f, identity.m33);
    }

    TEST_F(Matrix33fTest, MatrixMultiplication)
    {
        Matrix33f mat2(9.f, 8.f, 7.f
                    , 6.f, 5.f, 4.f
                    ,  3.f,  2.f, 1.f);

        Matrix33f mat3 = mat1 * mat2;

        EXPECT_EQ(30.0f,  mat3.m11);
        EXPECT_EQ(24.0f,  mat3.m12);
        EXPECT_EQ(18.0f,  mat3.m13);
        EXPECT_EQ(84.0f,  mat3.m21);
        EXPECT_EQ(69.0f,  mat3.m22);
        EXPECT_EQ(54.0f,  mat3.m23);
        EXPECT_EQ(138.0f, mat3.m31);
        EXPECT_EQ(114.0f, mat3.m32);
        EXPECT_EQ(90.0f,  mat3.m33);
    }

    TEST_F(Matrix33fTest, MatrixMultiplicationAndAssign)
    {
        Matrix33f mat2(16.f, 15.f, 14.f
                    , 12.f, 11.f, 10.f
                    ,  8.f,  7.f,  6.f);

        mat1 *= mat2;

        EXPECT_EQ(64.0f,  mat1.m11);
        EXPECT_EQ(58.0f,  mat1.m12);
        EXPECT_EQ(52.0f,  mat1.m13);
        EXPECT_EQ(172.0f, mat1.m21);
        EXPECT_EQ(157.0f, mat1.m22);
        EXPECT_EQ(142.0f, mat1.m23);
        EXPECT_EQ(280.0f, mat1.m31);
        EXPECT_EQ(256.0f, mat1.m32);
        EXPECT_EQ(232.0f, mat1.m33);
    }

    TEST_F(Matrix33fTest, Equality)
    {
        Matrix33f mat2(1.0f,  2.0f,  3.0f
                    ,  4.0f,  5.0f,  6.0f
                    ,  7.0f, 8.0f, 9.0f);

        Bool equal = mat1 == mat2;
        EXPECT_EQ(true, equal);
    }

    TEST_F(Matrix33fTest, UnEquality)
    {
        Matrix33f mat2(1.0f,  2.0f,  3.0f
                    ,  4.0f,  5.0f,  6.0f
                    ,  8.0f, 8.0f, 9.0f);

        Bool unequal = mat1 != mat2;
        EXPECT_EQ(true, unequal);
    }

    TEST_F(Matrix33fTest, SetSingleValues)
    {
        mat1.set(1.0f,    2.0f,     4.0f
                ,   8.0f, 16.0f,   32.0f
                , 64.0f,   128.0f, 256.0f);

        EXPECT_EQ(1.0f, mat1.m11);
        EXPECT_EQ(2.0f, mat1.m12);
        EXPECT_EQ(4.0f, mat1.m13);
        EXPECT_EQ(8.0f, mat1.m21);
        EXPECT_EQ(16.0f, mat1.m22);
        EXPECT_EQ(32.0f, mat1.m23);
        EXPECT_EQ(64.0f, mat1.m31);
        EXPECT_EQ(128.0f, mat1.m32);
        EXPECT_EQ(256.0f, mat1.m33);
    }

    TEST_F(Matrix33fTest, SetValues)
    {
        mat1.set(42.0f);

        EXPECT_EQ(42.0f, mat1.m11);
        EXPECT_EQ(42.0f, mat1.m12);
        EXPECT_EQ(42.0f, mat1.m13);
        EXPECT_EQ(42.0f, mat1.m21);
        EXPECT_EQ(42.0f, mat1.m22);
        EXPECT_EQ(42.0f, mat1.m23);
        EXPECT_EQ(42.0f, mat1.m31);
        EXPECT_EQ(42.0f, mat1.m32);
        EXPECT_EQ(42.0f, mat1.m33);
    }

    class Matrix33fParamTest : public Matrix33fTest, public ::testing::WithParamInterface<Vector3>
    {
    };

    TEST_P(Matrix33fParamTest, SetAndRecreateRotationXYZ)
    {
        mat1 = Matrix33f::RotationEulerXYZ(GetParam());

        Vector3 resRotation;
        EXPECT_TRUE(mat1.toRotationEulerXYZ(resRotation));

        Matrix33f mat2 = Matrix33f::RotationEulerXYZ(resRotation);
        for (uint32_t i = 0u; i < 9u; i++)
        {
            EXPECT_NEAR(mat1.data[i], mat2.data[i], 1.0e-6f);
        }
    }

    TEST_P(Matrix33fParamTest, SetAndRecreateRotationZYX)
    {
        mat1 = Matrix33f::RotationEulerZYX(GetParam());

        Vector3 resRotation;
        EXPECT_TRUE(mat1.toRotationEulerZYX(resRotation));

        Matrix33f mat2 = Matrix33f::RotationEulerZYX(resRotation);
        for (uint32_t i = 0u; i < 9u; i++)
        {
            EXPECT_NEAR(mat1.data[i], mat2.data[i], 1.0e-6f);
        }
    }

    static Vector3 values[] = {
        Vector3(45.0f, 85.f, 15.0f),
        Vector3(-45.0f, -85.f, -15.0f),
        Vector3(-45.0f, 155.f, 345.0f),
        Vector3(320.0f, -155.f, -15.0f),
        Vector3(90.0f, -155.f, -15.0f)
    };

    INSTANTIATE_TEST_CASE_P(TestRotationConsistency, Matrix33fParamTest, ::testing::ValuesIn(values));

}
