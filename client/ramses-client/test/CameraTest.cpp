//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>

#include "ClientTestUtils.h"
#include "ramses-client-api/PerspectiveCamera.h"
#include "ramses-client-api/OrthographicCamera.h"
#include "ramses-client-api/DataVector2i.h"
#include "Math3d/CameraMatrixHelper.h"

namespace ramses
{
    using namespace testing;

    template <typename CameraType>
    class ACamera : public LocalTestClientWithScene, public testing::Test
    {
    protected:
        ACamera()
        {
            camera = &this->template createObject<CameraType>("camera");
        }

        CameraType* camera;
    };

    typedef ::testing::Types
        <
        PerspectiveCamera,
        OrthographicCamera
        > CameraTypes;

    TYPED_TEST_CASE(ACamera, CameraTypes);

    TYPED_TEST(ACamera, hasDefaultParams)
    {
        EXPECT_EQ(-1.0f, this->camera->getLeftPlane());
        EXPECT_EQ(1.0f, this->camera->getRightPlane());
        EXPECT_EQ(-1.0f, this->camera->getBottomPlane());
        EXPECT_EQ(1.0f, this->camera->getTopPlane());
        EXPECT_EQ(0.1f, this->camera->getNearPlane());
        EXPECT_EQ(1.0f, this->camera->getFarPlane());
    }

    TYPED_TEST(ACamera, reportsErrorWhenValidatingInitialState)
    {
        EXPECT_NE(StatusOK, this->camera->validate());
    }

    TYPED_TEST(ACamera, canSetItsViewport)
    {
        EXPECT_EQ(StatusOK, this->camera->setViewport(1, -2, 100, 200));
        EXPECT_EQ(1, this->camera->getViewportX());
        EXPECT_EQ(-2, this->camera->getViewportY());
        EXPECT_EQ(100u, this->camera->getViewportWidth());
        EXPECT_EQ(200u, this->camera->getViewportHeight());
    }

    TYPED_TEST(ACamera, validatesWhenAllParamsSet)
    {
        this->camera->setFrustum(-0.01f, 0.01f, -0.01f, 0.01f, 0.1f, 1000.f);
        this->camera->setViewport(0u, 0u, 16u, 32u);
        EXPECT_EQ(StatusOK, this->camera->validate());
    }

    TYPED_TEST(ACamera, reportsErrorIfTryingToRetrieveProjectionMatrixWithoutBeingInitialized)
    {
        float projMat[16] = { 0.f };
        EXPECT_NE(StatusOK, this->camera->getProjectionMatrix(projMat));
    }

    TYPED_TEST(ACamera, canSetFrustum)
    {
        EXPECT_EQ(StatusOK, this->camera->setFrustum(0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f));

        EXPECT_EQ(0.1f, this->camera->getLeftPlane());
        EXPECT_EQ(0.2f, this->camera->getRightPlane());
        EXPECT_EQ(0.3f, this->camera->getBottomPlane());
        EXPECT_EQ(0.4f, this->camera->getTopPlane());
        EXPECT_EQ(0.5f, this->camera->getNearPlane());
        EXPECT_EQ(0.6f, this->camera->getFarPlane());
    }

    TYPED_TEST(ACamera, failsToSetFrustumIfInvalid)
    {
        // leftPlane >= rightPlane
        EXPECT_NE(StatusOK, this->camera->setFrustum(0.2f, 0.1f, 0.3f, 0.4f, 0.5f, 0.6f));
        EXPECT_NE(StatusOK, this->camera->setFrustum(0.1f, 0.1f, 0.3f, 0.4f, 0.5f, 0.6f));
        // bottomPlane >= topPlane
        EXPECT_NE(StatusOK, this->camera->setFrustum(0.1f, 0.2f, 0.4f, 0.3f, 0.5f, 0.6f));
        EXPECT_NE(StatusOK, this->camera->setFrustum(0.1f, 0.2f, 0.3f, 0.3f, 0.5f, 0.6f));
        // nearPlane >= farPlane
        EXPECT_NE(StatusOK, this->camera->setFrustum(0.1f, 0.2f, 0.3f, 0.4f, 0.6f, 0.5f));
        EXPECT_NE(StatusOK, this->camera->setFrustum(0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.5f));
    }

    TYPED_TEST(ACamera, failsToSetViewportIfInvalid)
    {
        EXPECT_NE(StatusOK, this->camera->setViewport(0, 0, 0u, 0u));
        EXPECT_NE(StatusOK, this->camera->setViewport(0, 0, 1u, 0u));
        EXPECT_NE(StatusOK, this->camera->setViewport(0, 0, 0u, 1u));
    }

    TYPED_TEST(ACamera, returnsProjectionMatrix)
    {
        EXPECT_EQ(StatusOK, this->camera->setFrustum(0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f));

        const auto projType = (this->camera->getType() == ERamsesObjectType_PerspectiveCamera ? ramses_internal::ECameraProjectionType_Perspective : ramses_internal::ECameraProjectionType_Orthographic);
        const ramses_internal::Matrix44f expectedMatrix = ramses_internal::CameraMatrixHelper::ProjectionMatrix(ramses_internal::ProjectionParams::Frustum(projType, 0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f));

        float projMatData[16] = { 0.f };
        EXPECT_EQ(StatusOK, this->camera->getProjectionMatrix(projMatData));

        for (uint32_t i = 0u; i < 16u; ++i)
        {
            EXPECT_FLOAT_EQ(expectedMatrix.data[i], projMatData[i]);
        }
    }

    TYPED_TEST(ACamera, viewportDataNotBoundInitially)
    {
        EXPECT_FALSE(this->camera->isViewportOffsetBound());
        EXPECT_FALSE(this->camera->isViewportSizeBound());
    }

    TYPED_TEST(ACamera, canBindViewportData)
    {
        const auto do1 = this->m_scene.createDataVector2i();
        const auto do2 = this->m_scene.createDataVector2i();

        EXPECT_EQ(StatusOK, this->camera->bindViewportOffset(*do1));
        EXPECT_EQ(StatusOK, this->camera->bindViewportSize(*do2));

        EXPECT_TRUE(this->camera->isViewportOffsetBound());
        EXPECT_TRUE(this->camera->isViewportSizeBound());
    }

    TYPED_TEST(ACamera, canUnbindViewportData)
    {
        const auto do1 = this->m_scene.createDataVector2i();
        const auto do2 = this->m_scene.createDataVector2i();

        EXPECT_EQ(StatusOK, this->camera->bindViewportOffset(*do1));
        EXPECT_EQ(StatusOK, this->camera->bindViewportSize(*do2));

        EXPECT_TRUE(this->camera->isViewportOffsetBound());
        EXPECT_TRUE(this->camera->isViewportSizeBound());

        EXPECT_EQ(StatusOK, this->camera->unbindViewportOffset());
        EXPECT_EQ(StatusOK, this->camera->unbindViewportSize());

        EXPECT_FALSE(this->camera->isViewportOffsetBound());
        EXPECT_FALSE(this->camera->isViewportSizeBound());
    }

    TYPED_TEST(ACamera, getsReferencedViewportValuesWhenBound)
    {
        const auto do1 = this->m_scene.createDataVector2i();
        const auto do2 = this->m_scene.createDataVector2i();
        do1->setValue(1, 2);
        do2->setValue(3, 4);

        EXPECT_EQ(StatusOK, this->camera->setViewport(-1, -2, 16, 32));

        EXPECT_EQ(StatusOK, this->camera->bindViewportOffset(*do1));
        EXPECT_EQ(StatusOK, this->camera->bindViewportSize(*do2));
        EXPECT_TRUE(this->camera->isViewportOffsetBound());
        EXPECT_TRUE(this->camera->isViewportSizeBound());

        EXPECT_EQ(1, this->camera->getViewportX());
        EXPECT_EQ(2, this->camera->getViewportY());
        EXPECT_EQ(3u, this->camera->getViewportWidth());
        EXPECT_EQ(4u, this->camera->getViewportHeight());
    }

    TYPED_TEST(ACamera, getsCorrectViewportValuesWhenBoundFromOneDataObjectToAnother)
    {
        const auto do1 = this->m_scene.createDataVector2i();
        const auto do2 = this->m_scene.createDataVector2i();
        do1->setValue(1, 2);
        do2->setValue(3, 4);

        EXPECT_EQ(StatusOK, this->camera->setViewport(-1, -2, 16, 32));

        EXPECT_EQ(StatusOK, this->camera->bindViewportOffset(*do1));
        EXPECT_EQ(StatusOK, this->camera->bindViewportSize(*do2));
        EXPECT_TRUE(this->camera->isViewportOffsetBound());
        EXPECT_TRUE(this->camera->isViewportSizeBound());

        EXPECT_EQ(1, this->camera->getViewportX());
        EXPECT_EQ(2, this->camera->getViewportY());
        EXPECT_EQ(3u, this->camera->getViewportWidth());
        EXPECT_EQ(4u, this->camera->getViewportHeight());

        EXPECT_EQ(StatusOK, this->camera->bindViewportOffset(*do2));
        EXPECT_EQ(StatusOK, this->camera->bindViewportSize(*do1));
        EXPECT_TRUE(this->camera->isViewportOffsetBound());
        EXPECT_TRUE(this->camera->isViewportSizeBound());

        EXPECT_EQ(3, this->camera->getViewportX());
        EXPECT_EQ(4, this->camera->getViewportY());
        EXPECT_EQ(1u, this->camera->getViewportWidth());
        EXPECT_EQ(2u, this->camera->getViewportHeight());
    }

    TYPED_TEST(ACamera, getsReferencedViewportValuesWhenBound_alsoWhenChangedInDataObject)
    {
        const auto do1 = this->m_scene.createDataVector2i();
        const auto do2 = this->m_scene.createDataVector2i();
        do1->setValue(1, 2);
        do2->setValue(3, 4);

        EXPECT_EQ(StatusOK, this->camera->setViewport(-1, -2, 16, 32));

        EXPECT_EQ(StatusOK, this->camera->bindViewportOffset(*do1));
        EXPECT_EQ(StatusOK, this->camera->bindViewportSize(*do2));
        EXPECT_TRUE(this->camera->isViewportOffsetBound());
        EXPECT_TRUE(this->camera->isViewportSizeBound());

        EXPECT_EQ(1, this->camera->getViewportX());
        EXPECT_EQ(2, this->camera->getViewportY());
        EXPECT_EQ(3u, this->camera->getViewportWidth());
        EXPECT_EQ(4u, this->camera->getViewportHeight());

        do1->setValue(11, 22);
        do2->setValue(33, 44);

        EXPECT_EQ(11, this->camera->getViewportX());
        EXPECT_EQ(22, this->camera->getViewportY());
        EXPECT_EQ(33u, this->camera->getViewportWidth());
        EXPECT_EQ(44u, this->camera->getViewportHeight());
    }

    TYPED_TEST(ACamera, canBindDataToTwoCamerasViewports)
    {
        auto camera2 = &this->template createObject<TypeParam>("camera");

        const auto do1 = this->m_scene.createDataVector2i();
        do1->setValue(3, 4);

        EXPECT_EQ(StatusOK, this->camera->setViewport(-1, -2, 16, 32));
        EXPECT_EQ(StatusOK, camera2->setViewport(-11, -22, 166, 322));

        EXPECT_EQ(StatusOK, this->camera->bindViewportOffset(*do1));
        EXPECT_EQ(StatusOK, camera2->bindViewportSize(*do1));
        EXPECT_TRUE(this->camera->isViewportOffsetBound());
        EXPECT_TRUE(camera2->isViewportSizeBound());

        EXPECT_EQ(3, this->camera->getViewportX());
        EXPECT_EQ(4, this->camera->getViewportY());
        EXPECT_EQ(16u, this->camera->getViewportWidth());
        EXPECT_EQ(32u, this->camera->getViewportHeight());
        EXPECT_EQ(-11, camera2->getViewportX());
        EXPECT_EQ(-22, camera2->getViewportY());
        EXPECT_EQ(3u, camera2->getViewportWidth());
        EXPECT_EQ(4u, camera2->getViewportHeight());

        do1->setValue(11, 22);

        EXPECT_EQ(11, this->camera->getViewportX());
        EXPECT_EQ(22, this->camera->getViewportY());
        EXPECT_EQ(16u, this->camera->getViewportWidth());
        EXPECT_EQ(32u, this->camera->getViewportHeight());
        EXPECT_EQ(-11, camera2->getViewportX());
        EXPECT_EQ(-22, camera2->getViewportY());
        EXPECT_EQ(11u, camera2->getViewportWidth());
        EXPECT_EQ(22u, camera2->getViewportHeight());
    }

    TYPED_TEST(ACamera, getsOriginalViewportValuesWhenUnbound)
    {
        const auto do1 = this->m_scene.createDataVector2i();
        const auto do2 = this->m_scene.createDataVector2i();
        do1->setValue(1, 2);
        do2->setValue(3, 4);

        EXPECT_EQ(StatusOK, this->camera->setViewport(-1, -2, 16, 32));

        EXPECT_EQ(StatusOK, this->camera->bindViewportOffset(*do1));
        EXPECT_EQ(StatusOK, this->camera->bindViewportSize(*do2));
        EXPECT_TRUE(this->camera->isViewportOffsetBound());
        EXPECT_TRUE(this->camera->isViewportSizeBound());

        EXPECT_EQ(1, this->camera->getViewportX());
        EXPECT_EQ(2, this->camera->getViewportY());
        EXPECT_EQ(3u, this->camera->getViewportWidth());
        EXPECT_EQ(4u, this->camera->getViewportHeight());

        EXPECT_EQ(StatusOK, this->camera->unbindViewportOffset());
        EXPECT_EQ(StatusOK, this->camera->unbindViewportSize());

        EXPECT_EQ(-1, this->camera->getViewportX());
        EXPECT_EQ(-2, this->camera->getViewportY());
        EXPECT_EQ(16u, this->camera->getViewportWidth());
        EXPECT_EQ(32u, this->camera->getViewportHeight());
    }

    TYPED_TEST(ACamera, viewportValueSetIfBoundOnlyAffectsOriginalValueThatBecomesActiveAfterUnbound)
    {
        const auto do1 = this->m_scene.createDataVector2i();
        do1->setValue(1, 2);

        EXPECT_EQ(StatusOK, this->camera->setViewport(-1, -2, 16, 32));

        EXPECT_EQ(StatusOK, this->camera->bindViewportOffset(*do1));

        EXPECT_EQ(1, this->camera->getViewportX());
        EXPECT_EQ(2, this->camera->getViewportY());
        EXPECT_EQ(16u, this->camera->getViewportWidth());
        EXPECT_EQ(32u, this->camera->getViewportHeight());

        EXPECT_EQ(StatusOK, this->camera->setViewport(-11, -22, 166, 322));

        EXPECT_EQ(1, this->camera->getViewportX());
        EXPECT_EQ(2, this->camera->getViewportY());
        // width and height are not bound therefore active right away
        EXPECT_EQ(166u, this->camera->getViewportWidth());
        EXPECT_EQ(322u, this->camera->getViewportHeight());

        EXPECT_EQ(StatusOK, this->camera->unbindViewportOffset());

        // offset set back to original value, which was updated while bound
        EXPECT_EQ(-11, this->camera->getViewportX());
        EXPECT_EQ(-22, this->camera->getViewportY());
        EXPECT_EQ(166u, this->camera->getViewportWidth());
        EXPECT_EQ(322u, this->camera->getViewportHeight());
    }

    TYPED_TEST(ACamera, doesNotReportValidationErrorWhenViewportNotInitializedButBoundToValidData)
    {
        // make frustum valid - not testing frustum validity here
        EXPECT_EQ(StatusOK, this->camera->setFrustum(-1, 1, -1, 1, 0.1f, 0.2f));

        const auto do1 = this->m_scene.createDataVector2i();
        const auto do2 = this->m_scene.createDataVector2i();
        do1->setValue(1, 2);
        do2->setValue(3, 4);
        EXPECT_EQ(StatusOK, this->camera->bindViewportOffset(*do1));
        EXPECT_EQ(StatusOK, this->camera->bindViewportSize(*do2));
        EXPECT_EQ(StatusOK, this->camera->validate());
    }

    TYPED_TEST(ACamera, reportsValidationErrorWhenViewportNotInitializedAndBoundToValidDataButUnboundAfterwards)
    {
        // make frustum valid - not testing frustum validity here
        EXPECT_EQ(StatusOK, this->camera->setFrustum(-1, 1, -1, 1, 0.1f, 0.2f));

        const auto do1 = this->m_scene.createDataVector2i();
        const auto do2 = this->m_scene.createDataVector2i();
        do1->setValue(1, 2);
        do2->setValue(3, 4);
        EXPECT_EQ(StatusOK, this->camera->bindViewportOffset(*do1));
        EXPECT_EQ(StatusOK, this->camera->bindViewportSize(*do2));
        EXPECT_EQ(StatusOK, this->camera->validate());

        EXPECT_EQ(StatusOK, this->camera->unbindViewportOffset());
        EXPECT_NE(StatusOK, this->camera->validate());

        EXPECT_EQ(StatusOK, this->camera->unbindViewportSize());
        EXPECT_NE(StatusOK, this->camera->validate());
    }

    TYPED_TEST(ACamera, reportsValidationErrorWhenViewportNotInitializedAndBoundToInvalidData)
    {
        // make frustum valid - not testing frustum validity here
        EXPECT_EQ(StatusOK, this->camera->setFrustum(-1, 1, -1, 1, 0.1f, 0.2f));

        const auto do1 = this->m_scene.createDataVector2i();
        const auto do2 = this->m_scene.createDataVector2i();
        do1->setValue(1, 2);
        do2->setValue(0, 4);
        EXPECT_EQ(StatusOK, this->camera->bindViewportOffset(*do1));
        EXPECT_EQ(StatusOK, this->camera->bindViewportSize(*do2));
        EXPECT_NE(StatusOK, this->camera->validate());
    }

    class APerspectiveCamera : public LocalTestClientWithScene, public testing::Test
    {
    protected:
        APerspectiveCamera()
        {
            camera = m_scene.createPerspectiveCamera("camera");
        }

        PerspectiveCamera* camera;
    };

    TEST_F(APerspectiveCamera, canSetFrustum)
    {
        const float fov = 30.f;
        const float aspectRatio = 640.f / 480.f;
        const float nearPlane = 0.1f;
        const float farPlane = 100.f;

        EXPECT_EQ(StatusOK, camera->setFrustum(fov, aspectRatio, nearPlane, farPlane));

        EXPECT_FLOAT_EQ(fov, camera->getVerticalFieldOfView());
        EXPECT_FLOAT_EQ(aspectRatio, camera->getAspectRatio());
        EXPECT_EQ(nearPlane, camera->getNearPlane());
        EXPECT_EQ(farPlane, camera->getFarPlane());
    }

    TEST_F(APerspectiveCamera, failsToSetFrustumIfInvalid)
    {
        const float fov = 30.f;
        const float aspectRatio = 640.f / 480.f;
        const float nearPlane = 0.1f;
        const float farPlane = 100.f;

        EXPECT_NE(StatusOK, camera->setFrustum(0.f, aspectRatio, nearPlane, farPlane));
        EXPECT_NE(StatusOK, camera->setFrustum(180.f, aspectRatio, nearPlane, farPlane));
        EXPECT_NE(StatusOK, camera->setFrustum(fov, 0.f, nearPlane, farPlane));
        EXPECT_NE(StatusOK, camera->setFrustum(fov, -1.f, nearPlane, farPlane));
        EXPECT_NE(StatusOK, camera->setFrustum(fov, aspectRatio, 0.f, farPlane));
        EXPECT_NE(StatusOK, camera->setFrustum(fov, aspectRatio, nearPlane, nearPlane));
    }
}
