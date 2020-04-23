//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Math3d/ProjectionParams.h"
#include "PlatformAbstraction/PlatformMath.h"
#include <gtest/gtest.h>

namespace ramses_internal
{
    class AProjectionParams : public testing::Test
    {
    };

    TEST(AProjectionParams, EqualsToSimilarPerspectiveProjParams)
    {
        ProjectionParams params1 = ProjectionParams::Perspective(30.f, 1.0f, 0.1f, 1000.f);
        ProjectionParams params2 = ProjectionParams::Perspective(30.f, 1.0f, 0.1f, 1000.f);

        EXPECT_EQ(params1, params2);
    }

    TEST(AProjectionParams, EqualsToSimilarOrthographicProjParams)
    {
        ProjectionParams params1 = ProjectionParams::Frustum(ECameraProjectionType_Orthographic, 0.1f, 0.2f, 0.3f, 0.4f, 0.1f, 1000.f);
        ProjectionParams params2 = ProjectionParams::Frustum(ECameraProjectionType_Orthographic, 0.1f, 0.2f, 0.3f, 0.4f, 0.1f, 1000.f);

        EXPECT_EQ(params1, params2);
    }

    TEST(AProjectionParams, PerspectiveDoesNotEqualOrthographicWithSameCalculatedPlaneValues)
    {
        ProjectionParams params1 = ProjectionParams::Perspective(30.f, 1.0f, 0.1f, 1000.f);
        ProjectionParams params2 = ProjectionParams::Frustum(ECameraProjectionType_Orthographic, params1.leftPlane, params1.rightPlane,
            params1.bottomPlane, params1.topPlane, params1.nearPlane, params1.farPlane);

        EXPECT_EQ(params1.leftPlane, params2.leftPlane);
        EXPECT_EQ(params1.rightPlane, params2.rightPlane);
        EXPECT_EQ(params1.bottomPlane, params2.bottomPlane);
        EXPECT_EQ(params1.topPlane, params2.topPlane);
        EXPECT_EQ(params1.nearPlane, params2.nearPlane);
        EXPECT_EQ(params1.farPlane, params2.farPlane);

        EXPECT_NE(params1, params2);
    }

    TEST(AProjectionParams, PerspectiveDoesNotEqualOrthographicWithSamePlaneValues)
    {
        ProjectionParams params1 = ProjectionParams::Frustum(ECameraProjectionType_Perspective, 0.1f, 0.2f, 0.3f, 0.4f, 0.1f, 1000.f);
        ProjectionParams params2 = ProjectionParams::Frustum(ECameraProjectionType_Orthographic, 0.1f, 0.2f, 0.3f, 0.4f, 0.1f, 1000.f);

        EXPECT_EQ(params1.leftPlane, params2.leftPlane);
        EXPECT_EQ(params1.rightPlane, params2.rightPlane);
        EXPECT_EQ(params1.bottomPlane, params2.bottomPlane);
        EXPECT_EQ(params1.topPlane, params2.topPlane);
        EXPECT_EQ(params1.nearPlane, params2.nearPlane);
        EXPECT_EQ(params1.farPlane, params2.farPlane);

        EXPECT_NE(params1, params2);
    }

    TEST(AProjectionParams, CanBeConstructedFromOrthographicParameters)
    {
        ProjectionParams params(ProjectionParams::Frustum(ECameraProjectionType_Orthographic, 0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f));
        EXPECT_EQ(ECameraProjectionType_Orthographic, params.getProjectionType());
        EXPECT_EQ(0.1f, params.leftPlane);
        EXPECT_EQ(0.2f, params.rightPlane);
        EXPECT_EQ(0.3f, params.bottomPlane);
        EXPECT_EQ(0.4f, params.topPlane);
        EXPECT_EQ(0.5f, params.nearPlane);
        EXPECT_EQ(0.6f, params.farPlane);
    }

    TEST(AProjectionParams, CanUpdateOrthographicParameters)
    {
        ProjectionParams params(ProjectionParams::Frustum(ECameraProjectionType_Orthographic, 0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f));
        params.leftPlane = 0.5f;

        EXPECT_EQ(ECameraProjectionType_Orthographic, params.getProjectionType());
        EXPECT_EQ(0.5f, params.leftPlane);
        EXPECT_EQ(0.2f, params.rightPlane);
        EXPECT_EQ(0.3f, params.bottomPlane);
        EXPECT_EQ(0.4f, params.topPlane);
        EXPECT_EQ(0.5f, params.nearPlane);
        EXPECT_EQ(0.6f, params.farPlane);
    }

    TEST(AProjectionParams, CanBeConstructedFromPerspectiveParameters)
    {
        ProjectionParams params(ProjectionParams::Perspective(45.f, 1.5f, 0.1f, 1.0f));

        EXPECT_EQ(ECameraProjectionType_Perspective, params.getProjectionType());
        EXPECT_FLOAT_EQ(45.f, ProjectionParams::GetPerspectiveFovY(params));
        EXPECT_FLOAT_EQ(1.5f, ProjectionParams::GetAspectRatio(params));
        EXPECT_EQ(0.1f, params.nearPlane);
        EXPECT_EQ(1.0f, params.farPlane);

        EXPECT_GT(0.0f, params.leftPlane);
        EXPECT_LT(0.0f, params.rightPlane);
        EXPECT_GT(0.0f, params.bottomPlane);
        EXPECT_LT(0.0f, params.topPlane);

        EXPECT_FLOAT_EQ(params.rightPlane, -params.leftPlane);
        EXPECT_FLOAT_EQ(params.topPlane, -params.bottomPlane);
    }

    TEST(AProjectionParams, CanUpdateAspectRatioOnPerspectiveProjection)
    {
        ProjectionParams params(ProjectionParams::Perspective(45.f, 1.5f, 0.1f, 1.0f));

        ProjectionParams updateParams = ProjectionParams::Perspective(
            ProjectionParams::GetPerspectiveFovY(params),
            2.6f,
            params.nearPlane,
            params.farPlane
        );

        EXPECT_FLOAT_EQ(2.6f, ProjectionParams::GetAspectRatio(updateParams));

        EXPECT_NE(params.leftPlane, updateParams.leftPlane);
        EXPECT_NE(params.rightPlane, updateParams.rightPlane);

        EXPECT_EQ(params.bottomPlane, updateParams.bottomPlane);
        EXPECT_EQ(params.topPlane, updateParams.topPlane);
        EXPECT_EQ(params.nearPlane, updateParams.nearPlane);
        EXPECT_EQ(params.farPlane, updateParams.farPlane);
    }

    TEST(AProjectionParams, CanUpdateFovYOnPerspectiveProjection)
    {
        ProjectionParams params(ProjectionParams::Perspective(45.f, 1.5f, 0.1f, 1.0f));

        ProjectionParams updateParams = ProjectionParams::Perspective(
            10.f,
            ProjectionParams::GetAspectRatio(params),
            params.nearPlane,
            params.farPlane
        );

        EXPECT_FLOAT_EQ(10.f, ProjectionParams::GetPerspectiveFovY(updateParams));

        EXPECT_NE(params.bottomPlane, updateParams.bottomPlane);
        EXPECT_NE(params.topPlane, updateParams.topPlane);
        EXPECT_NE(params.leftPlane, updateParams.leftPlane);
        EXPECT_NE(params.rightPlane, updateParams.rightPlane);

        EXPECT_EQ(params.nearPlane, updateParams.nearPlane);
        EXPECT_EQ(params.farPlane, updateParams.farPlane);
    }

    TEST(AProjectionParams, GetFovYOnNonSymmectricPerspectiveProjection)
    {
        ProjectionParams params(ProjectionParams::Perspective(45.f, 1.5f, 0.1f, 1.0f)); // this is symmetric
        EXPECT_FLOAT_EQ(45.f, ProjectionParams::GetPerspectiveFovY(params));

        params.bottomPlane = params.bottomPlane / 2.0f; // halving the opening angle of the bottom plane
        EXPECT_GT(45.0f, ProjectionParams::GetPerspectiveFovY(params)); // total opening angle should be smaller now

        // calculate expected opening angle
        float expectedAngle = 45.f / 2.0f + PlatformMath::Rad2Deg(std::atan2(std::abs(params.bottomPlane), params.nearPlane));
        EXPECT_FLOAT_EQ(expectedAngle, ProjectionParams::GetPerspectiveFovY(params));
    }

    TEST(AProjectionParams, GetAspectRatioOnOrthographicProjection)
    {
        ProjectionParams params(ProjectionParams::Frustum(ECameraProjectionType_Orthographic, 0.1f, 0.3f, 0.3f, 0.4f, 0.1f, 1000.f));
        EXPECT_FLOAT_EQ(2.0f, ProjectionParams::GetAspectRatio(params));
    }

    TEST(AProjectionParams, GetAspectRatioOnUnsymmetricPerspectiveProjection)
    {
        ProjectionParams params(ProjectionParams::Perspective(45.f, 1.0f, 0.1f, 1.0f)); // this is symmetric in x and y

        params.leftPlane *= 2.0f;
        EXPECT_FLOAT_EQ(1.5f, ProjectionParams::GetAspectRatio(params));

        params.topPlane *= 0.5f;
        EXPECT_FLOAT_EQ(2.0f, ProjectionParams::GetAspectRatio(params));
    }
}
