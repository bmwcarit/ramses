//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "Math3d/Matrix22f.h"
#include "Collections/StringOutputStream.h"

namespace ramses_internal
{
    class Matrix22fTest : public testing::Test
    {
    public:
        Matrix22fTest();

    protected:
        ramses_internal::Matrix22f mat1;
    };

    Matrix22fTest::Matrix22fTest()
        : mat1(1.0f, 2.0f, 4.0f, 5.0f)
    {
    }

    TEST_F(Matrix22fTest, DefaultConstructor)
    {
        const Matrix22f mat2;
        EXPECT_EQ(1.0f, mat2.m11);
        EXPECT_EQ(0.0f, mat2.m12);
        EXPECT_EQ(0.0f, mat2.m21);
        EXPECT_EQ(1.0f, mat2.m22);
    }

    TEST_F(Matrix22fTest, CopyConstructor)
    {
        const Matrix22f mat2 = mat1;
        EXPECT_EQ(1.0f, mat2.m11);
        EXPECT_EQ(2.0f, mat2.m12);
        EXPECT_EQ(4.0f, mat2.m21);
        EXPECT_EQ(5.0f, mat2.m22);
    }

    TEST_F(Matrix22fTest, ValueConstructor)
    {
        EXPECT_EQ(1.0f, mat1.m11);
        EXPECT_EQ(2.0f, mat1.m12);
        EXPECT_EQ(4.0f, mat1.m21);
        EXPECT_EQ(5.0f, mat1.m22);
    }

    TEST_F(Matrix22fTest, VectorConstructor)
    {
        const Vector2 row1(8.f, 7.f);
        const Vector2 row2(5.f, 4.f);
        const Matrix22f mat2(row1, row2);

        EXPECT_EQ(8.0f, mat2.m11);
        EXPECT_EQ(7.0f, mat2.m12);
        EXPECT_EQ(5.0f, mat2.m21);
        EXPECT_EQ(4.0f, mat2.m22);
    }

    TEST_F(Matrix22fTest, IdentityMatrix)
    {
        const Matrix22f identity = Matrix22f::Identity;

        EXPECT_EQ(1.0f, identity.m11);
        EXPECT_EQ(0.0f, identity.m12);
        EXPECT_EQ(0.0f, identity.m21);
        EXPECT_EQ(1.0f, identity.m22);
    }

    TEST_F(Matrix22fTest, EmptyMatrix)
    {
        const Matrix22f identity = Matrix22f::Empty;

        EXPECT_EQ(0.0f, identity.m11);
        EXPECT_EQ(0.0f, identity.m12);
        EXPECT_EQ(0.0f, identity.m21);
        EXPECT_EQ(0.0f, identity.m22);
    }

    TEST_F(Matrix22fTest, MatrixMultiplication)
    {
        const Matrix22f mat2(9.f, 8.f, 6.f, 5.f);
        const Matrix22f mat3 = mat1 * mat2;

        EXPECT_EQ(21.0f, mat3.m11);
        EXPECT_EQ(18.0f, mat3.m12);
        EXPECT_EQ(66.0f, mat3.m21);
        EXPECT_EQ(57.0f, mat3.m22);
    }

    TEST_F(Matrix22fTest, MatrixMultiplicationAndAssign)
    {
        const Matrix22f mat2(16.f, 15.f, 12.f, 11.f);
        mat1 *= mat2;

        EXPECT_EQ(40.0f, mat1.m11);
        EXPECT_EQ(37.0f, mat1.m12);
        EXPECT_EQ(124.0f, mat1.m21);
        EXPECT_EQ(115.0f, mat1.m22);
    }

    TEST_F(Matrix22fTest, Equality)
    {
        const Matrix22f mat2(1.0f, 2.0f, 4.0f, 5.0f);
        const bool equal = (mat1 == mat2);
        EXPECT_EQ(true, equal);
    }

    TEST_F(Matrix22fTest, UnEquality)
    {
        const Matrix22f mat2(1.0f, 2.0f, 3.0f, 5.0f);
        const bool unequal = (mat1 != mat2);
        EXPECT_EQ(true, unequal);
    }

    TEST_F(Matrix22fTest, SetSingleValues)
    {
        mat1.set(1.0f, 2.0f, 8.0f, 16.0f);

        EXPECT_EQ(1.0f, mat1.m11);
        EXPECT_EQ(2.0f, mat1.m12);
        EXPECT_EQ(8.0f, mat1.m21);
        EXPECT_EQ(16.0f, mat1.m22);
    }

    TEST_F(Matrix22fTest, SetValues)
    {
        mat1.set(42.0f);

        EXPECT_EQ(42.0f, mat1.m11);
        EXPECT_EQ(42.0f, mat1.m12);
        EXPECT_EQ(42.0f, mat1.m21);
        EXPECT_EQ(42.0f, mat1.m22);
    }

    TEST_F(Matrix22fTest, CanPrintToString)
    {
        EXPECT_EQ("[1.0 2.0; 4.0 5.0]", fmt::to_string(mat1));
        EXPECT_EQ("[1.0 2.0; 4.0 5.0]", StringOutputStream::ToString(mat1));
    }
}
