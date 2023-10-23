//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "TestEqualHelper.h"
#include "internal/Core/Math3d/Rotation.h"
#include "internal/PlatformAbstraction/Collections/StringOutputStream.h"
#include "TestEqualHelper.h"
#include <vector>
#include <array>
#include "glm/gtx/transform.hpp"
#include "glm/gtx/euler_angles.hpp"
#include "glm/ext.hpp"

namespace ramses::internal
{
    class RotationEulerTest : public ::testing::TestWithParam<glm::vec3>
    {
    protected:
        static glm::mat4 CalculateRotationMatrixAroundAxis(uint8_t axis, float angle)
        {
            //Note: it does not matter which convention is used, since rotation around single axis
            //would result in same matrix using any convention
            switch (axis)
            {
            case 0: //x-axis
                return glm::eulerAngleX(glm::radians(angle));
            case 1: //y-axis
                return glm::eulerAngleY(glm::radians(angle));
            case 2: //z-axis
                return glm::eulerAngleZ(glm::radians(angle));
            default:
                break;
            }

            assert(false);
            return glm::mat4{0.f};
        };

        struct RotationConventionTest
        {
            ERotationType convention;
            std::array<uint8_t, 3> axesRepresentedByInputAngles; //what do [x,y,z] rotation angles mean in terms of axes? (they have different meaning for "Proper Euler conventions")
            std::array<uint8_t, 3> matrixMultiplicationOrder; //in which order should the matrices be multiplied (for verification)?
        };
        static std::vector<RotationConventionTest> rotationAxesPerConvention;
    };

    std::vector<RotationEulerTest::RotationConventionTest> RotationEulerTest::rotationAxesPerConvention
    {
        //Tait-Bryan angles' conventions:
        //The conventions always specify rotation around the three axes X, Y and Z. The order in which rotation around
        //the axes is applied differs from one convention to another.
        //i.e., Euler_ZYX input angles indicate rotation around X, Y, and Z axes respectively, and the order of matrix multiplication is
        //the same as the order of the angles in the convention (name).
        //Since input rotation angles in Tait-Bryan conventions always mean X, Y and Z, therefore all of the 6 Tait-Bryan conventions
        //take value {0, 1, 2} for "what the input angles' axes mean"
        { ERotationType::Euler_ZYX, {0, 1, 2}, {0, 1, 2} },
        { ERotationType::Euler_YZX, {0, 1, 2}, {0, 2, 1} },
        { ERotationType::Euler_ZXY, {0, 1, 2}, {1, 0, 2} },
        { ERotationType::Euler_XZY, {0, 1, 2}, {1, 2, 0} },
        { ERotationType::Euler_YXZ, {0, 1, 2}, {2, 0, 1} },
        { ERotationType::Euler_XYZ, {0, 1, 2}, {2, 1, 0} },

        //Proper Euler angles' conventions:
        //The conventions always specify only Two axes for rotation, but rotation around ONE of those axes is applied TWICE with DIFFERENT angles.
        //i.e., input Euler_ZYX angles indicate rotation angles around the axes as stated in the convention, and matrix multiplication
        //is always done in order RxRyRz (where x is the 1st angle, y is the 2nd angle and z is the 3rd angle)
        //Since matrix multiplication is always done in the same order for all of Proper Euler conventions, therefore the order of matrix multiplication
        //for all 6 of Proper Euler conventions is {0, 1, 2}
        { ERotationType::Euler_XYX, {0, 1, 0}, {0, 1, 2} },
        { ERotationType::Euler_XZX, {0, 2, 0}, {0, 1, 2} },
        { ERotationType::Euler_YXY, {1, 0, 1}, {0, 1, 2} },
        { ERotationType::Euler_YZY, {1, 2, 1}, {0, 1, 2} },
        { ERotationType::Euler_ZXZ, {2, 0, 2}, {0, 1, 2} },
        { ERotationType::Euler_ZYZ, {2, 1, 2}, {0, 1, 2} },
    };

    TEST_P(RotationEulerTest, RotationWithEulerConventions)
    {
        const float rotationAngle0 = GetParam().x;
        const float rotationAngle1 = GetParam().y;
        const float rotationAngle2 = GetParam().z;

        for (const auto& conventionTest : rotationAxesPerConvention)
        {
            const ERotationType convention = conventionTest.convention;
            const auto& rotationAxes = conventionTest.axesRepresentedByInputAngles;
            const auto& matrixOrder = conventionTest.matrixMultiplicationOrder;

            const auto rotA0 = CalculateRotationMatrixAroundAxis(rotationAxes[0], rotationAngle0);
            const auto rotA1 = CalculateRotationMatrixAroundAxis(rotationAxes[1], rotationAngle1);
            const auto rotA2 = CalculateRotationMatrixAroundAxis(rotationAxes[2], rotationAngle2);

            const std::array rotMatrices = { rotA0, rotA1, rotA2 };

            //calculate rotation matrix using the convention to be tested
            const auto rotationMatrixUsingConvention = Math3d::Rotation({ rotationAngle0, rotationAngle1, rotationAngle2, 1.f }, convention);

            //calculate rotation matrix which results from the multiplication of each axis separately
            const auto expectedRotationMatrix = rotMatrices[matrixOrder[0]] * rotMatrices[matrixOrder[1]] * rotMatrices[matrixOrder[2]];

            expectMatrixFloatEqual(rotationMatrixUsingConvention, expectedRotationMatrix);
        }
    }

    static glm::vec3 values[] = {
        glm::vec3(45.0f, 85.f, 15.0f),
        glm::vec3(-45.0f, -85.f, -15.0f),
        glm::vec3(-45.0f, 155.f, 345.0f),
        glm::vec3(320.0f, -155.f, -15.0f),
        glm::vec3(90.0f, -155.f, -15.0f)
    };

    INSTANTIATE_TEST_SUITE_P(RotationEulerTest, RotationEulerTest, ::testing::ValuesIn(values));

    using RotationValues = std::vector<std::pair<glm::vec4, glm::vec3>>;

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

    class RotationQuaternionTest : public ::testing::TestWithParam<std::pair<glm::vec4, glm::vec3>>
    {
    };

    TEST_P(RotationQuaternionTest, rotationMatrixIsEquivalent)
    {
        auto [quat, euler] = GetParam();
        const auto matEuler = Math3d::Rotation(glm::vec4(euler, 1.f), ERotationType::Euler_ZYX);
        const auto matQuat = Math3d::Rotation(quat, ERotationType::Quaternion);
        EXPECT_TRUE(glm::all(glm::equal(matQuat, matEuler, 0.000001f)));
    }

    INSTANTIATE_TEST_SUITE_P(RotationQuaternionTest, RotationQuaternionTest, ::testing::ValuesIn(rotationValues));
}
