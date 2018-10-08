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
    Matrix44f projMatrix(CameraMatrixHelper::ProjectionMatrix(ProjectionParams::Perspective(30.0f, 2.0f, 0.1f, 10.0f)));
    // Don't use matrix comparison operator here, but compare exact floats
    // Rationale: tests should be explicit, should not use internal behaviour of matrix class
    EXPECT_FLOAT_EQ(1.8660254f, projMatrix.m11);
    EXPECT_FLOAT_EQ(0, projMatrix.m12);
    EXPECT_FLOAT_EQ(0, projMatrix.m13);
    EXPECT_FLOAT_EQ(0, projMatrix.m14);
    EXPECT_FLOAT_EQ(0, projMatrix.m21);
    EXPECT_FLOAT_EQ(3.73205f, projMatrix.m22);
    EXPECT_FLOAT_EQ(0, projMatrix.m23);
    EXPECT_FLOAT_EQ(0, projMatrix.m24);
    EXPECT_FLOAT_EQ(0, projMatrix.m31);
    EXPECT_FLOAT_EQ(0, projMatrix.m32);
    EXPECT_FLOAT_EQ(-1.020202f, projMatrix.m33);
    EXPECT_FLOAT_EQ(-0.20202021f, projMatrix.m34);
    EXPECT_FLOAT_EQ(0, projMatrix.m41);
    EXPECT_FLOAT_EQ(0, projMatrix.m42);
    EXPECT_FLOAT_EQ(-1, projMatrix.m43);
    EXPECT_FLOAT_EQ(0, projMatrix.m44);
}

TEST(ACameraMatrixHelper, ComputesOrthographicMatrix)
{
    Matrix44f projMatrix(CameraMatrixHelper::ProjectionMatrix(ProjectionParams::Frustum(ECameraProjectionType_Orthographic, 30, 40, 50, 60, 10, 20)));
    // Don't use matrix comparison operator here, but compare exact floats
    // Rationale: tests should be explicit, should not use internal behaviour of matrix class
    EXPECT_FLOAT_EQ(0.2f, projMatrix.m11);
    EXPECT_FLOAT_EQ(0, projMatrix.m12);
    EXPECT_FLOAT_EQ(0, projMatrix.m13);
    EXPECT_FLOAT_EQ(-7.0f, projMatrix.m14);
    EXPECT_FLOAT_EQ(0, projMatrix.m21);
    EXPECT_FLOAT_EQ(0.2f, projMatrix.m22);
    EXPECT_FLOAT_EQ(0, projMatrix.m23);
    EXPECT_FLOAT_EQ(-11.0f, projMatrix.m24);
    EXPECT_FLOAT_EQ(0, projMatrix.m31);
    EXPECT_FLOAT_EQ(0, projMatrix.m32);
    EXPECT_FLOAT_EQ(-0.2f, projMatrix.m33);
    EXPECT_FLOAT_EQ(-3.0f, projMatrix.m34);
    EXPECT_FLOAT_EQ(0, projMatrix.m41);
    EXPECT_FLOAT_EQ(0, projMatrix.m42);
    EXPECT_FLOAT_EQ(0, projMatrix.m43);
    EXPECT_FLOAT_EQ(1, projMatrix.m44);
}

// Confidence test (above tests already test enough)
TEST(APerspectiveProjectionMatrix, ProjectsPointsCloserToViewerFartherFromCenterInNormalizedDeviceSpace)
{
    Matrix44f perpectivematrix = CameraMatrixHelper::ProjectionMatrix(ProjectionParams::Perspective(30.0f, 2.666f, 0.00001f, 100.f));
    // In right handed coordinate systems camera view ray is pointing towards -1, thus bigger negative Z means farther away from viewer
    Vector4 vecClose(1.0f, 12.0f, -50.0f, 1.0f);
    Vector4 vecFar(1.0f, 12.0f, -100.0f, 1.0f);

    Vector4 resultClose = perpectivematrix * vecClose;
    Vector4 resultFar = perpectivematrix * vecFar;
    EXPECT_GT(resultClose.x / resultClose.w, resultFar.x / resultFar.w);
    EXPECT_GT(resultClose.y / resultClose.w, resultFar.y / resultFar.w);
}

// Confidence test (above tests already test enough)
TEST(AOrthographicProjectionMatrix, ProjectsPointsWithDifferentDepthToTheSamePixelInNormalizedDeviceSpace)
{
    Matrix44f orthographicMatrix = CameraMatrixHelper::ProjectionMatrix(ProjectionParams::Frustum(ECameraProjectionType_Orthographic,
        -1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 100.0f));
    Vector4 vecA(1.0f, 2.0f, 0.0f, 1.f);
    Vector4 vecB(1.0f, 2.0f, -50.0f, 1.f);

    Vector4 resultA = orthographicMatrix* vecA;
    Vector4 resultB = orthographicMatrix* vecB;
    EXPECT_FLOAT_EQ(resultA.x / resultA.w, resultB.x / resultB.w);
    EXPECT_FLOAT_EQ(resultA.y / resultA.w, resultB.y / resultB.w);
}
