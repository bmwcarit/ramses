//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "internals/Math.h"
#include <numeric>

namespace rlogic::internal::math
{
    class Vector4Test : public testing::Test
    {
    protected:
        Vector4 m_vec1234{ 1.f, 2.f, 3.f, 4.f };
    };

    TEST_F(Vector4Test, DefaultConstructor)
    {
        const Vector4 vec;
        EXPECT_EQ(0.f, vec.x);
        EXPECT_EQ(0.f, vec.y);
        EXPECT_EQ(0.f, vec.z);
        EXPECT_EQ(0.f, vec.w);
    }

    TEST_F(Vector4Test, CopyConstructor)
    {
        const Vector4 vec = m_vec1234;
        EXPECT_EQ(1.f, vec.x);
        EXPECT_EQ(2.f, vec.y);
        EXPECT_EQ(3.f, vec.z);
        EXPECT_EQ(4.f, vec.w);
    }

    TEST_F(Vector4Test, ValueConstructor)
    {
        EXPECT_EQ(1.f, m_vec1234.x);
        EXPECT_EQ(2.f, m_vec1234.y);
        EXPECT_EQ(3.f, m_vec1234.z);
        EXPECT_EQ(4.f, m_vec1234.w);
    }

    TEST_F(Vector4Test, AssignmentOperator)
    {
        Vector4 vec;
        vec = m_vec1234;
        EXPECT_EQ(1.f, vec.x);
        EXPECT_EQ(2.f, vec.y);
        EXPECT_EQ(3.f, vec.z);
        EXPECT_EQ(4.f, vec.w);
    }

    TEST_F(Vector4Test, AddOperator)
    {
        const Vector4 vec = m_vec1234 + 1.f;
        EXPECT_EQ(2.f, vec.x);
        EXPECT_EQ(3.f, vec.y);
        EXPECT_EQ(4.f, vec.z);
        EXPECT_EQ(5.f, vec.w);
    }

    TEST_F(Vector4Test, SubOperator)
    {
        const Vector4 vec = m_vec1234 - 1.f;
        EXPECT_EQ(0.f, vec.x);
        EXPECT_EQ(1.f, vec.y);
        EXPECT_EQ(2.f, vec.z);
        EXPECT_EQ(3.f, vec.w);
    }

    TEST_F(Vector4Test, MulOperator)
    {
        const Vector4 vec = m_vec1234 * 2.f;
        EXPECT_EQ(2.f, vec.x);
        EXPECT_EQ(4.f, vec.y);
        EXPECT_EQ(6.f, vec.z);
        EXPECT_EQ(8.f, vec.w);
    }

    TEST_F(Vector4Test, DivOperator)
    {
        const Vector4 vec = m_vec1234 / 2.f;
        EXPECT_FLOAT_EQ(.5f, vec.x);
        EXPECT_FLOAT_EQ(1.f, vec.y);
        EXPECT_FLOAT_EQ(1.5f, vec.z);
        EXPECT_FLOAT_EQ(2.f, vec.w);
    }

    TEST_F(Vector4Test, MulVector)
    {
        const Vector4 vec = m_vec1234 * m_vec1234;
        EXPECT_EQ(1.f, vec.x);
        EXPECT_EQ(4.f, vec.y);
        EXPECT_EQ(9.f, vec.z);
        EXPECT_EQ(16.f, vec.w);
    }

    class Matrix44Test : public testing::Test
    {
    protected:
        Matrix44f m_mat{
            1.0f, 2.0f, 3.0f, 4.0f,
            5.0f, 6.0f, 7.0f, 8.0f,
            9.0f, 10.0f, 11.0f, 12.0f,
            13.0f, 14.0f, 15.0f, 16.0f };
    };

    TEST_F(Matrix44Test, DefaultConstructor)
    {
        const Matrix44f mat;
        EXPECT_EQ(1.0f, mat.m11);
        EXPECT_EQ(0.0f, mat.m12);
        EXPECT_EQ(0.0f, mat.m13);
        EXPECT_EQ(0.0f, mat.m14);
        EXPECT_EQ(0.0f, mat.m21);
        EXPECT_EQ(1.0f, mat.m22);
        EXPECT_EQ(0.0f, mat.m23);
        EXPECT_EQ(0.0f, mat.m24);
        EXPECT_EQ(0.0f, mat.m31);
        EXPECT_EQ(0.0f, mat.m32);
        EXPECT_EQ(1.0f, mat.m33);
        EXPECT_EQ(0.0f, mat.m34);
        EXPECT_EQ(0.0f, mat.m41);
        EXPECT_EQ(0.0f, mat.m42);
        EXPECT_EQ(0.0f, mat.m43);
        EXPECT_EQ(1.0f, mat.m44);
    }

    TEST_F(Matrix44Test, CopyConstructor)
    {
        const Matrix44f mat = m_mat;
        EXPECT_EQ(1.0f, mat.m11);
        EXPECT_EQ(2.0f, mat.m12);
        EXPECT_EQ(3.0f, mat.m13);
        EXPECT_EQ(4.0f, mat.m14);
        EXPECT_EQ(5.0f, mat.m21);
        EXPECT_EQ(6.0f, mat.m22);
        EXPECT_EQ(7.0f, mat.m23);
        EXPECT_EQ(8.0f, mat.m24);
        EXPECT_EQ(9.0f, mat.m31);
        EXPECT_EQ(10.0f, mat.m32);
        EXPECT_EQ(11.0f, mat.m33);
        EXPECT_EQ(12.0f, mat.m34);
        EXPECT_EQ(13.0f, mat.m41);
        EXPECT_EQ(14.0f, mat.m42);
        EXPECT_EQ(15.0f, mat.m43);
        EXPECT_EQ(16.0f, mat.m44);
    }

    TEST_F(Matrix44Test, ValueConstructor)
    {
        EXPECT_EQ(1.0f, m_mat.m11);
        EXPECT_EQ(2.0f, m_mat.m12);
        EXPECT_EQ(3.0f, m_mat.m13);
        EXPECT_EQ(4.0f, m_mat.m14);
        EXPECT_EQ(5.0f, m_mat.m21);
        EXPECT_EQ(6.0f, m_mat.m22);
        EXPECT_EQ(7.0f, m_mat.m23);
        EXPECT_EQ(8.0f, m_mat.m24);
        EXPECT_EQ(9.0f, m_mat.m31);
        EXPECT_EQ(10.0f, m_mat.m32);
        EXPECT_EQ(11.0f, m_mat.m33);
        EXPECT_EQ(12.0f, m_mat.m34);
        EXPECT_EQ(13.0f, m_mat.m41);
        EXPECT_EQ(14.0f, m_mat.m42);
        EXPECT_EQ(15.0f, m_mat.m43);
        EXPECT_EQ(16.0f, m_mat.m44);
    }

    TEST_F(Matrix44Test, ArrayConstructor)
    {
        float data[16]; // NOLINT(modernize-avoid-c-arrays) Ramses uses C array in matrix getters
        std::iota(data, data + 16, 1.f); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        const Matrix44f mat{ data };
        EXPECT_EQ(1.0f, mat.m11);
        EXPECT_EQ(2.0f, mat.m21);
        EXPECT_EQ(3.0f, mat.m31);
        EXPECT_EQ(4.0f, mat.m41);
        EXPECT_EQ(5.0f, mat.m12);
        EXPECT_EQ(6.0f, mat.m22);
        EXPECT_EQ(7.0f, mat.m32);
        EXPECT_EQ(8.0f, mat.m42);
        EXPECT_EQ(9.0f, mat.m13);
        EXPECT_EQ(10.0f, mat.m23);
        EXPECT_EQ(11.0f, mat.m33);
        EXPECT_EQ(12.0f, mat.m43);
        EXPECT_EQ(13.0f, mat.m14);
        EXPECT_EQ(14.0f, mat.m24);
        EXPECT_EQ(15.0f, mat.m34);
        EXPECT_EQ(16.0f, mat.m44);
    }

    TEST_F(Matrix44Test, StdArrayConstructor)
    {
        std::array<float, 16> data{};
        std::iota(data.begin(), data.end(), 1.f);
        const Matrix44f mat{ data };
        EXPECT_EQ(1.0f, mat.m11);
        EXPECT_EQ(2.0f, mat.m21);
        EXPECT_EQ(3.0f, mat.m31);
        EXPECT_EQ(4.0f, mat.m41);
        EXPECT_EQ(5.0f, mat.m12);
        EXPECT_EQ(6.0f, mat.m22);
        EXPECT_EQ(7.0f, mat.m32);
        EXPECT_EQ(8.0f, mat.m42);
        EXPECT_EQ(9.0f, mat.m13);
        EXPECT_EQ(10.0f, mat.m23);
        EXPECT_EQ(11.0f, mat.m33);
        EXPECT_EQ(12.0f, mat.m43);
        EXPECT_EQ(13.0f, mat.m14);
        EXPECT_EQ(14.0f, mat.m24);
        EXPECT_EQ(15.0f, mat.m34);
        EXPECT_EQ(16.0f, mat.m44);
    }

    TEST_F(Matrix44Test, ToStdArrayConversion)
    {
        std::array<float, 16> data{};
        std::iota(data.begin(), data.end(), 1.f);
        const Matrix44f mat{ data };
        EXPECT_EQ(data, mat.toStdArray());
    }

    TEST_F(Matrix44Test, VectorMultiplication)
    {
        const Vector4 vec1{ 1.f, 2.f, 3.f, 4.f };
        const Vector4 vec2 = m_mat * vec1;

        EXPECT_EQ(30.f, vec2.x);
        EXPECT_EQ(70.f, vec2.y);
        EXPECT_EQ(110.f, vec2.z);
        EXPECT_EQ(150.f, vec2.w);
    }

    TEST_F(Matrix44Test, MatrixMultiplication)
    {
        const Matrix44f mat{
            16.f, 15.f, 14.f, 13.f,
            12.f, 11.f, 10.f, 9.f,
            8.f, 7.f, 6.f, 5.f,
            4.f, 3.f, 2.f, 1.f };

        const Matrix44f mat3 = m_mat * mat;

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
}
