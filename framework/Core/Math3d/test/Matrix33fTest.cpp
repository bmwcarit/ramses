//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "TestEqualHelper.h"
#include "Math3d/Matrix33f.h"
#include "Math3d/Vector4.h"
#include "Collections/StringOutputStream.h"
#include <vector>
#include <array>

using ::testing::Pointwise;
using ::testing::FloatNear;

namespace ramses_internal
{
    std::ostream& operator<<(std::ostream& os, const Vector3& value)
    {
        os << "xyz(" << value.x;
        os << "," << value.y;
        os << "," << value.z << ")";
        return os;
    }

    std::ostream& operator<<(std::ostream& os, const Vector4& value)
    {
        os << "xyzw(" << value.x;
        os << "," << value.y;
        os << "," << value.z;
        os << "," << value.w << ")";
        return os;
    }

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

        bool equal = mat1 == mat2;
        EXPECT_EQ(true, equal);
    }

    TEST_F(Matrix33fTest, UnEquality)
    {
        Matrix33f mat2(1.0f,  2.0f,  3.0f
                    ,  4.0f,  5.0f,  6.0f
                    ,  8.0f, 8.0f, 9.0f);

        bool unequal = mat1 != mat2;
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

    TEST_F(Matrix33fTest, CanPrintToString)
    {
        EXPECT_EQ("[1 2 3; 4 5 6; 7 8 9]", fmt::to_string(mat1));
    }

    class Matrix33fParamTest : public Matrix33fTest, public ::testing::WithParamInterface<Vector3>
    {
    protected:
        Matrix33f calculateRotationMatrixAroundAxis(uint8_t axis, float angle)
        {
            //Note: it does not matter which convention is used, since rotation around single axis
            //would result in same matrix using any convention
            switch (axis)
            {
            case 0: //x-axis
                return Matrix33f::Rotation({ angle, 0.f, 0.f, 1.f }, ERotationConvention::Euler_ZYX);
            case 1: //y-axis
                return Matrix33f::Rotation({ 0.f, angle, 0.f, 1.f }, ERotationConvention::Euler_ZYX);
            case 2: //z-axis
                return Matrix33f::Rotation({ 0.f, 0.f, angle, 1.f }, ERotationConvention::Euler_ZYX);
            }

            assert(false);
            return Matrix33f::Empty;
        };

        struct RotationConventionTest
        {
            ERotationConvention convention;
            std::array<uint8_t, 3> axesRepresentedByInputAngles; //what do [x,y,z] rotation angles mean in terms of axes? (they have different meaning for "Proper Euler conventions")
            std::array<uint8_t, 3> matrixMultiplicationOrder; //in which order should the matrices be multiplied (for verification)?
        };
        static std::vector<RotationConventionTest> rotationAxesPerConvention;
    };

    std::vector<Matrix33fParamTest::RotationConventionTest> Matrix33fParamTest::rotationAxesPerConvention
    {
        //Tait-Bryan angles' conventions:
        //The conventions always specify rotation around the three axes X, Y and Z. The order in which rotation around
        //the axes is applied differs from one convention to another.
        //i.e., Euler_ZYX input angles indicate rotation around X, Y, and Z axes respectively, and the order of matrix multiplication is
        //the same as the order of the angles in the convention (name).
        //Since input rotation angles in Tait-Bryan conventions always mean X, Y and Z, therefore all of the 6 Tait-Bryan conventions
        //take value {0, 1, 2} for "what the input angles' axes mean"
        { ERotationConvention::Euler_ZYX, {0, 1, 2}, {0, 1, 2} },
        { ERotationConvention::Euler_YZX, {0, 1, 2}, {0, 2, 1} },
        { ERotationConvention::Euler_ZXY, {0, 1, 2}, {1, 0, 2} },
        { ERotationConvention::Euler_XZY, {0, 1, 2}, {1, 2, 0} },
        { ERotationConvention::Euler_YXZ, {0, 1, 2}, {2, 0, 1} },
        { ERotationConvention::Euler_XYZ, {0, 1, 2}, {2, 1, 0} },

        //Proper Euler angles' conventions:
        //The conventions always specify only Two axes for rotation, but rotation around ONE of those axes is applied TWICE with DIFFERENT angles.
        //i.e., input Euler_ZYX angles indicate rotation angles around the axes as stated in the convention, and matrix multiplication
        //is always done in order RxRyRz (where x is the 1st angle, y is the 2nd angle and z is the 3rd angle)
        //Since matrix multiplication is always done in the same order for all of Proper Euler conventions, therefore the order of matrix multiplication
        //for all 6 of Proper Euler conventions is {0, 1, 2}
        { ERotationConvention::Euler_XYX, {0, 1, 0}, {0, 1, 2} },
        { ERotationConvention::Euler_XZX, {0, 2, 0}, {0, 1, 2} },
        { ERotationConvention::Euler_YXY, {1, 0, 1}, {0, 1, 2} },
        { ERotationConvention::Euler_YZY, {1, 2, 1}, {0, 1, 2} },
        { ERotationConvention::Euler_ZXZ, {2, 0, 2}, {0, 1, 2} },
        { ERotationConvention::Euler_ZYZ, {2, 1, 2}, {0, 1, 2} },
    };


    TEST_P(Matrix33fParamTest, SetAndRecreateRotationXYZ)
    {
        mat1 = Matrix33f::Rotation(Vector4(GetParam()), ERotationConvention::Euler_ZYX);

        Vector3 resRotation;
        EXPECT_TRUE(mat1.toRotationEuler(resRotation, ERotationConvention::Euler_ZYX));

        Matrix33f mat2 = Matrix33f::Rotation(Vector4(resRotation), ERotationConvention::Euler_ZYX);
        expectMatrixFloatEqual(mat1, mat2);
    }

    TEST_P(Matrix33fParamTest, RotationWithEulerConventions)
    {
        const float rotationAngle0 = GetParam().x;
        const float rotationAngle1 = GetParam().y;
        const float rotationAngle2 = GetParam().z;

        for (const auto& conventionTest : rotationAxesPerConvention)
        {
            const ERotationConvention convention = conventionTest.convention;
            const auto& rotationAxes = conventionTest.axesRepresentedByInputAngles;
            const auto& matrixOrder = conventionTest.matrixMultiplicationOrder;

            const auto rotA0 = calculateRotationMatrixAroundAxis(rotationAxes[0], rotationAngle0);
            const auto rotA1 = calculateRotationMatrixAroundAxis(rotationAxes[1], rotationAngle1);
            const auto rotA2 = calculateRotationMatrixAroundAxis(rotationAxes[2], rotationAngle2);

            const Matrix33f rotMatrices[] = { rotA0, rotA1, rotA2 };

            //calculate rotation matrix using the convention to be tested
            const Matrix33f rotationMatrixUsingConvention = Matrix33f::Rotation({ rotationAngle0, rotationAngle1, rotationAngle2, 1.f }, convention);

            //calculate rotation matrix which results from the multiplication of each axis separately
            const Matrix33f expectedRotationMatrix = rotMatrices[matrixOrder[0]] * rotMatrices[matrixOrder[1]] * rotMatrices[matrixOrder[2]];

            expectMatrixFloatEqual(rotationMatrixUsingConvention, expectedRotationMatrix);
        }
    }

    static Vector3 values[] = {
        Vector3(45.0f, 85.f, 15.0f),
        Vector3(-45.0f, -85.f, -15.0f),
        Vector3(-45.0f, 155.f, 345.0f),
        Vector3(320.0f, -155.f, -15.0f),
        Vector3(90.0f, -155.f, -15.0f)
    };

    INSTANTIATE_TEST_SUITE_P(TestRotationConsistency, Matrix33fParamTest, ::testing::ValuesIn(values));

    using RotationValues = std::vector<std::pair<Vector4, Vector3>>;

    const RotationValues rotationValues = {
        // identity
        {{0.f, 0.f, 0.f, 1.f}, {0.f, 0.f, 0.f}},
        // 45 degrees around single axis (X, Y, Z)
        {{0.3826834f, 0, 0, 0.9238795f}, {45.f, 0.f, 0.f}},
        {{0, 0.3826834f, 0, 0.9238795f}, {0.f, 45.f, 0.f}},
        {{0, 0, 0.3826834f, 0.9238795f}, {0.f, 0.f, 45.f}},
        // 90 degrees around single axis (X, Y, Z)
        {{0.7071068f, 0, 0, 0.7071068f}, {90.f, 0.f, 0.f}},
        {{0, 0.7071068f, 0, 0.7071068f}, {0.f, 90.f, 0.f}},
        {{0, 0, 0.7071068f, 0.7071068f}, {0.f, 0.f, 90.f}},
        // 135 degrees around single axis (X, Y, Z)
        {{0.9238795f, 0, 0, 0.3826834f}, {135.f, 0.f, 0.f}},
        {{0, 0.9238795f, 0, 0.3826834f}, {0.f, 135.f, 0.f}},
        {{0, 0.9238795f, 0, 0.3826834f}, {-180.f, 45.f, -180.f}}, // is equivalent to (0, +135, 0) rotation
        {{0, -0.9238795f, 0, -0.3826834f}, {-180.f, 45.f, -180.f}}, // is equivalent to (0, +135, 0) rotation
        {{0, 0, 0.9238795f, 0.3826834f}, {0.f, 0.f, 135.f}},
        // 90 degrees, 2 axes
        {{0.5f, -0.5f, 0.5f, 0.5f}, {90.f, 0.f, 90.f}},
        {{0.5f, 0.5f, 0.5f, 0.5f}, {90.f, 90.f, 0.f}},
        // More exotic combinations
        {{0.1276794f, 0.1448781f, 0.2685358f, 0.9437144f}, {10.f, 20.f, 30.f}},
        {{0.2317316f, 0.5668337f, 0.2478199f, 0.7507232f}, {15.f, 75.f, 25.f}},
        {{0.2705981f, -0.2705981f, 0.6532815f, 0.6532815f}, {45.f, 0.f, 90.f}},
        {{0.25f, 0.0669873f, 0.9330127f, 0.25f}, {0.f, 30.f, 150.f}},
    };

    class Matrix33fQuaternionTest : public ::testing::TestWithParam<std::pair<Vector4, Vector3>>
    {
    };

    TEST_P(Matrix33fQuaternionTest, rotationMatrixIsEquivalent)
    {
        auto [quat, euler] = GetParam();
        const auto matEuler = Matrix33f::Rotation(Vector4(euler), ERotationConvention::Euler_ZYX);
        const auto matQuat = Matrix33f::Rotation(quat, ERotationConvention::Quaternion);
        EXPECT_THAT(matQuat.data, Pointwise(FloatNear(0.000001f), matEuler.data));
    }

    INSTANTIATE_TEST_SUITE_P(Matrix33fQuaternionTest, Matrix33fQuaternionTest, ::testing::ValuesIn(rotationValues));
}
