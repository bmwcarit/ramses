//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "framework_common_gmock_header.h"
#include "gtest/gtest.h"
#include "Math3d/CameraMatrixHelper.h"

using namespace ramses_internal;

TEST(ACameraMatrixHelper, ComputesProjectionMatrix)
{
    const auto projMatrix = CameraMatrixHelper::ProjectionMatrix(ProjectionParams::Perspective(30.0f, 2.0f, 0.1f, 10.0f));
    // Don't use matrix comparison operator here, but compare exact floats
    // Rationale: tests should be explicit, should not use internal behaviour of matrix class
    EXPECT_FLOAT_EQ(1.8660254f, projMatrix[0][0]);
    EXPECT_FLOAT_EQ(0, projMatrix[1][0]);
    EXPECT_FLOAT_EQ(0, projMatrix[2][0]);
    EXPECT_FLOAT_EQ(0, projMatrix[3][0]);
    EXPECT_FLOAT_EQ(0, projMatrix[0][1]);
    EXPECT_FLOAT_EQ(3.73205f, projMatrix[1][1]);
    EXPECT_FLOAT_EQ(0, projMatrix[2][1]);
    EXPECT_FLOAT_EQ(0, projMatrix[3][1]);
    EXPECT_FLOAT_EQ(0, projMatrix[0][2]);
    EXPECT_FLOAT_EQ(0, projMatrix[1][2]);
    EXPECT_FLOAT_EQ(-1.020202f, projMatrix[2][2]);
    EXPECT_FLOAT_EQ(-0.20202021f, projMatrix[3][2]);
    EXPECT_FLOAT_EQ(0, projMatrix[0][3]);
    EXPECT_FLOAT_EQ(0, projMatrix[1][3]);
    EXPECT_FLOAT_EQ(-1, projMatrix[2][3]);
    EXPECT_FLOAT_EQ(0, projMatrix[3][3]);
}

TEST(ACameraMatrixHelper, ComputesOrthographicMatrix)
{
    const auto projMatrix = CameraMatrixHelper::ProjectionMatrix(ProjectionParams::Frustum(ECameraProjectionType::Orthographic, 30, 40, 50, 60, 10, 20));
    // Don't use matrix comparison operator here, but compare exact floats
    // Rationale: tests should be explicit, should not use internal behaviour of matrix class
    EXPECT_FLOAT_EQ(0.2f, projMatrix[0][0]);
    EXPECT_FLOAT_EQ(0, projMatrix[1][0]);
    EXPECT_FLOAT_EQ(0, projMatrix[2][0]);
    EXPECT_FLOAT_EQ(-7.0f, projMatrix[3][0]);
    EXPECT_FLOAT_EQ(0, projMatrix[0][1]);
    EXPECT_FLOAT_EQ(0.2f, projMatrix[1][1]);
    EXPECT_FLOAT_EQ(0, projMatrix[2][1]);
    EXPECT_FLOAT_EQ(-11.0f, projMatrix[3][1]);
    EXPECT_FLOAT_EQ(0, projMatrix[0][2]);
    EXPECT_FLOAT_EQ(0, projMatrix[1][2]);
    EXPECT_FLOAT_EQ(-0.2f, projMatrix[2][2]);
    EXPECT_FLOAT_EQ(-3.0f, projMatrix[3][2]);
    EXPECT_FLOAT_EQ(0, projMatrix[0][3]);
    EXPECT_FLOAT_EQ(0, projMatrix[1][3]);
    EXPECT_FLOAT_EQ(0, projMatrix[2][3]);
    EXPECT_FLOAT_EQ(1, projMatrix[3][3]);
}

// Confidence test (above tests already test enough)
TEST(APerspectiveProjectionMatrix, ProjectsPointsCloserToViewerFartherFromCenterInNormalizedDeviceSpace)
{
    const auto perpectivematrix = CameraMatrixHelper::ProjectionMatrix(ProjectionParams::Perspective(30.0f, 2.666f, 0.00001f, 100.f));
    // In right handed coordinate systems camera view ray is pointing towards -1, thus bigger negative Z means farther away from viewer
    glm::vec4 vecClose(1.0f, 12.0f, -50.0f, 1.0f);
    glm::vec4 vecFar(1.0f, 12.0f, -100.0f, 1.0f);

    glm::vec4 resultClose = perpectivematrix * vecClose;
    glm::vec4 resultFar = perpectivematrix * vecFar;
    EXPECT_GT(resultClose.x / resultClose.w, resultFar.x / resultFar.w);
    EXPECT_GT(resultClose.y / resultClose.w, resultFar.y / resultFar.w);
}

// Confidence test (above tests already test enough)
TEST(AOrthographicProjectionMatrix, ProjectsPointsWithDifferentDepthToTheSamePixelInNormalizedDeviceSpace)
{
    const auto orthographicMatrix = CameraMatrixHelper::ProjectionMatrix(ProjectionParams::Frustum(ECameraProjectionType::Orthographic,
        -1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 100.0f));
    glm::vec4 vecA(1.0f, 2.0f, 0.0f, 1.f);
    glm::vec4 vecB(1.0f, 2.0f, -50.0f, 1.f);

    glm::vec4 resultA = orthographicMatrix* vecA;
    glm::vec4 resultB = orthographicMatrix* vecB;
    EXPECT_FLOAT_EQ(resultA.x / resultA.w, resultB.x / resultB.w);
    EXPECT_FLOAT_EQ(resultA.y / resultA.w, resultB.y / resultB.w);
}
