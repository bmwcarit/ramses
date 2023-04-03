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
#include "ramses-client-api/DataVector2f.h"
#include "ramses-client-api/DataVector4f.h"
#include "Math3d/CameraMatrixHelper.h"
#include "DataObjectImpl.h"
#include "CameraNodeImpl.h"
#include "ramses-utils.h"

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

    using CameraTypes = ::testing::Types
        <
        PerspectiveCamera,
        OrthographicCamera
        >;

    TYPED_TEST_SUITE(ACamera, CameraTypes);

    TYPED_TEST(ACamera, hasDefaultParams)
    {
        EXPECT_EQ(-1.0f, this->camera->getLeftPlane());
        EXPECT_EQ(1.0f, this->camera->getRightPlane());
        EXPECT_EQ(-1.0f, this->camera->getBottomPlane());
        EXPECT_EQ(1.0f, this->camera->getTopPlane());
        EXPECT_EQ(0.1f, this->camera->getNearPlane());
        EXPECT_EQ(1.0f, this->camera->getFarPlane());
        EXPECT_EQ(0, this->camera->getViewportX());
        EXPECT_EQ(0, this->camera->getViewportY());
        EXPECT_EQ(16u, this->camera->getViewportWidth());
        EXPECT_EQ(16u, this->camera->getViewportHeight());
    }

    TYPED_TEST(ACamera, canDestroyCamera)
    {
        EXPECT_EQ(StatusOK, this->m_scene.destroy(*this->camera));
    }

    TYPED_TEST(ACamera, reportsErrorWhenValidatingInitialState)
    {
        EXPECT_NE(StatusOK, this->camera->validate());
    }

    TYPED_TEST(ACamera, reportsErrorWhenViewportWidthExceedsMaximum)
    {
        this->camera->setFrustum(-0.01f, 0.01f, -0.01f, 0.01f, 0.1f, 1000.f);
        EXPECT_NE(StatusOK, this->camera->setViewport(0u, 0u, 32768 + 1, 16u));
        // Keeps default values
        EXPECT_EQ(16u, this->camera->getViewportWidth());
        EXPECT_EQ(16u, this->camera->getViewportHeight());
    }

    TYPED_TEST(ACamera, reportsErrorWhenViewportHeightExceedsMaximum)
    {
        this->camera->setFrustum(-0.01f, 0.01f, -0.01f, 0.01f, 0.1f, 1000.f);
        EXPECT_NE(StatusOK, this->camera->setViewport(0u, 0u, 16u, 32768 + 1));
        // Keeps default values
        EXPECT_EQ(16u, this->camera->getViewportWidth());
        EXPECT_EQ(16u, this->camera->getViewportHeight());
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

        const auto projType = (this->camera->getType() == ERamsesObjectType_PerspectiveCamera ? ramses_internal::ECameraProjectionType::Perspective : ramses_internal::ECameraProjectionType::Orthographic);
        const ramses_internal::Matrix44f expectedMatrix = ramses_internal::CameraMatrixHelper::ProjectionMatrix(ramses_internal::ProjectionParams::Frustum(projType, 0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f));

        float projMatData[16] = { 0.f };
        EXPECT_EQ(StatusOK, this->camera->getProjectionMatrix(projMatData));

        for (uint32_t i = 0u; i < 16u; ++i)
        {
            EXPECT_FLOAT_EQ(expectedMatrix.data[i], projMatData[i]);
        }
    }

    TYPED_TEST(ACamera, cameraDataNotBoundInitially)
    {
        EXPECT_FALSE(this->camera->isViewportOffsetBound());
        EXPECT_FALSE(this->camera->isViewportSizeBound());
        EXPECT_FALSE(this->camera->isFrustumPlanesBound());
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

    TYPED_TEST(ACamera, canBindFrustumPlanesData)
    {
        const auto do1 = this->m_scene.createDataVector4f();
        const auto do2 = this->m_scene.createDataVector2f();
        EXPECT_EQ(StatusOK, this->camera->bindFrustumPlanes(*do1, *do2));
        EXPECT_TRUE(this->camera->isFrustumPlanesBound());
    }

    TYPED_TEST(ACamera, failsToBindViewportDataIfDataObjectFromDifferentScene)
    {
        auto otherScene = this->client.createScene(sceneId_t{ this->m_scene.getSceneId().getValue() + 1 });
        const auto do1 = otherScene->createDataVector2i();
        const auto do2 = otherScene->createDataVector2i();

        EXPECT_NE(StatusOK, this->camera->bindViewportOffset(*do1));
        EXPECT_NE(StatusOK, this->camera->bindViewportSize(*do2));

        EXPECT_FALSE(this->camera->isViewportOffsetBound());
        EXPECT_FALSE(this->camera->isViewportSizeBound());
    }

    TYPED_TEST(ACamera, failsToBindFrustumDataIfDataObjectFromDifferentScene)
    {
        auto otherScene = this->client.createScene(sceneId_t{ this->m_scene.getSceneId().getValue() + 1 });
        const auto do1 = otherScene->createDataVector4f();
        const auto do2 = otherScene->createDataVector2f();

        EXPECT_NE(StatusOK, this->camera->bindFrustumPlanes(*do1, *do2));
        EXPECT_FALSE(this->camera->isFrustumPlanesBound());
    }

    TYPED_TEST(ACamera, canUnbindViewportData)
    {
        const auto do1 = this->m_scene.createDataVector2i();
        const auto do2 = this->m_scene.createDataVector2i();

        EXPECT_EQ(StatusOK, this->camera->bindViewportOffset(*do1));
        EXPECT_EQ(StatusOK, this->camera->bindViewportSize(*do2));
        EXPECT_EQ(do1->impl.getDataReference(), this->camera->impl.getViewportOffsetHandle());
        EXPECT_EQ(do2->impl.getDataReference(), this->camera->impl.getViewportSizeHandle());

        EXPECT_TRUE(this->camera->isViewportOffsetBound());
        EXPECT_TRUE(this->camera->isViewportSizeBound());

        EXPECT_EQ(StatusOK, this->camera->unbindViewportOffset());
        EXPECT_EQ(StatusOK, this->camera->unbindViewportSize());
        EXPECT_NE(do1->impl.getDataReference(), this->camera->impl.getViewportOffsetHandle());
        EXPECT_NE(do2->impl.getDataReference(), this->camera->impl.getViewportSizeHandle());

        EXPECT_FALSE(this->camera->isViewportOffsetBound());
        EXPECT_FALSE(this->camera->isViewportSizeBound());
    }

    TYPED_TEST(ACamera, canUnbindFrustumPlanesData)
    {
        const auto do1 = this->m_scene.createDataVector4f();
        const auto do2 = this->m_scene.createDataVector2f();

        EXPECT_EQ(StatusOK, this->camera->bindFrustumPlanes(*do1, *do2));
        EXPECT_TRUE(this->camera->isFrustumPlanesBound());
        EXPECT_EQ(do1->impl.getDataReference(), this->camera->impl.getFrustrumPlanesHandle());
        EXPECT_EQ(do2->impl.getDataReference(), this->camera->impl.getFrustrumNearFarPlanesHandle());

        EXPECT_EQ(StatusOK, this->camera->unbindFrustumPlanes());
        EXPECT_FALSE(this->camera->isFrustumPlanesBound());
        EXPECT_NE(do1->impl.getDataReference(), this->camera->impl.getFrustrumPlanesHandle());
        EXPECT_NE(do2->impl.getDataReference(), this->camera->impl.getFrustrumNearFarPlanesHandle());
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

    TYPED_TEST(ACamera, getsReferencedFrustumPlanesValuesWhenBound)
    {
        const auto do1 = this->m_scene.createDataVector4f();
        const auto do2 = this->m_scene.createDataVector2f();
        do1->setValue(1, 2, 3, 4);
        do2->setValue(5, 6);

        EXPECT_EQ(StatusOK, this->camera->setFrustum(-1.f, 1.f, -1.f, 1.f, 0.1f, 1.f));

        EXPECT_EQ(StatusOK, this->camera->bindFrustumPlanes(*do1, *do2));
        EXPECT_TRUE(this->camera->isFrustumPlanesBound());

        EXPECT_EQ(1.f, this->camera->getLeftPlane());
        EXPECT_EQ(2.f, this->camera->getRightPlane());
        EXPECT_EQ(3.f, this->camera->getBottomPlane());
        EXPECT_EQ(4.f, this->camera->getTopPlane());
        EXPECT_EQ(5.f, this->camera->getNearPlane());
        EXPECT_EQ(6.f, this->camera->getFarPlane());
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

    TYPED_TEST(ACamera, getsCorrectFrustumValuesWhenBoundFromOneDataObjectToAnother)
    {
        const auto do1 = this->m_scene.createDataVector4f();
        const auto do2 = this->m_scene.createDataVector2f();
        do1->setValue(1, 2, 3, 4);
        do2->setValue(5, 6);

        EXPECT_EQ(StatusOK, this->camera->setFrustum(-1.f, 1.f, -1.f, 1.f, 0.1f, 1.f));

        EXPECT_EQ(StatusOK, this->camera->bindFrustumPlanes(*do1, *do2));
        EXPECT_TRUE(this->camera->isFrustumPlanesBound());

        const auto do3 = this->m_scene.createDataVector4f();
        const auto do4 = this->m_scene.createDataVector2f();
        do3->setValue(-1, -2, -3, -4);
        do4->setValue(-5, -6);

        EXPECT_EQ(StatusOK, this->camera->bindFrustumPlanes(*do3, *do4));
        EXPECT_TRUE(this->camera->isFrustumPlanesBound());

        EXPECT_EQ(-1.f, this->camera->getLeftPlane());
        EXPECT_EQ(-2.f, this->camera->getRightPlane());
        EXPECT_EQ(-3.f, this->camera->getBottomPlane());
        EXPECT_EQ(-4.f, this->camera->getTopPlane());
        EXPECT_EQ(-5.f, this->camera->getNearPlane());
        EXPECT_EQ(-6.f, this->camera->getFarPlane());
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

    TYPED_TEST(ACamera, getsReferencedFrustumValuesWhenBound_alsoWhenChangedInDataObject)
    {
        const auto do1 = this->m_scene.createDataVector4f();
        const auto do2 = this->m_scene.createDataVector2f();
        do1->setValue(1, 2, 3, 4);
        do2->setValue(5, 6);

        EXPECT_EQ(StatusOK, this->camera->setFrustum(-1.f, 1.f, -1.f, 1.f, 0.1f, 1.f));

        EXPECT_EQ(StatusOK, this->camera->bindFrustumPlanes(*do1, *do2));
        EXPECT_TRUE(this->camera->isFrustumPlanesBound());

        do1->setValue(-1, -2, -3, -4);
        do2->setValue(-5, -6);

        EXPECT_EQ(-1.f, this->camera->getLeftPlane());
        EXPECT_EQ(-2.f, this->camera->getRightPlane());
        EXPECT_EQ(-3.f, this->camera->getBottomPlane());
        EXPECT_EQ(-4.f, this->camera->getTopPlane());
        EXPECT_EQ(-5.f, this->camera->getNearPlane());
        EXPECT_EQ(-6.f, this->camera->getFarPlane());
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

    TYPED_TEST(ACamera, canBindDataToTwoCamerasFrustum)
    {
        const auto do1 = this->m_scene.createDataVector4f();
        const auto do2 = this->m_scene.createDataVector2f();
        do1->setValue(1, 2, 3, 4);
        do2->setValue(5, 6);

        EXPECT_EQ(StatusOK, this->camera->setFrustum(-1.f, 1.f, -1.f, 1.f, 0.1f, 1.f));
        auto camera2 = &this->template createObject<TypeParam>("camera");
        EXPECT_EQ(StatusOK, camera2->setFrustum(-10.f, 10.f, -10.f, 10.f, 0.01f, 10.f));

        EXPECT_EQ(StatusOK, this->camera->bindFrustumPlanes(*do1, *do2));
        EXPECT_EQ(StatusOK, camera2->bindFrustumPlanes(*do1, *do2));
        EXPECT_TRUE(this->camera->isFrustumPlanesBound());
        EXPECT_TRUE(camera2->isFrustumPlanesBound());

        EXPECT_EQ(1.f, this->camera->getLeftPlane());
        EXPECT_EQ(2.f, this->camera->getRightPlane());
        EXPECT_EQ(3.f, this->camera->getBottomPlane());
        EXPECT_EQ(4.f, this->camera->getTopPlane());
        EXPECT_EQ(5.f, this->camera->getNearPlane());
        EXPECT_EQ(6.f, this->camera->getFarPlane());
        EXPECT_EQ(1.f, camera2->getLeftPlane());
        EXPECT_EQ(2.f, camera2->getRightPlane());
        EXPECT_EQ(3.f, camera2->getBottomPlane());
        EXPECT_EQ(4.f, camera2->getTopPlane());
        EXPECT_EQ(5.f, camera2->getNearPlane());
        EXPECT_EQ(6.f, camera2->getFarPlane());

        do1->setValue(-1, -2, -3, -4);
        do2->setValue(-5, -6);
        EXPECT_EQ(-1.f, this->camera->getLeftPlane());
        EXPECT_EQ(-2.f, this->camera->getRightPlane());
        EXPECT_EQ(-3.f, this->camera->getBottomPlane());
        EXPECT_EQ(-4.f, this->camera->getTopPlane());
        EXPECT_EQ(-5.f, this->camera->getNearPlane());
        EXPECT_EQ(-6.f, this->camera->getFarPlane());
        EXPECT_EQ(-1.f, camera2->getLeftPlane());
        EXPECT_EQ(-2.f, camera2->getRightPlane());
        EXPECT_EQ(-3.f, camera2->getBottomPlane());
        EXPECT_EQ(-4.f, camera2->getTopPlane());
        EXPECT_EQ(-5.f, camera2->getNearPlane());
        EXPECT_EQ(-6.f, camera2->getFarPlane());
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

    TYPED_TEST(ACamera, getsOriginalFrustumValuesWhenUnbound)
    {
        const auto do1 = this->m_scene.createDataVector4f();
        const auto do2 = this->m_scene.createDataVector2f();
        do1->setValue(1, 2, 3, 4);
        do2->setValue(5, 6);

        EXPECT_EQ(StatusOK, this->camera->setFrustum(-1.f, 1.f, -1.f, 1.f, 0.1f, 1.f));

        EXPECT_EQ(StatusOK, this->camera->bindFrustumPlanes(*do1, *do2));
        EXPECT_TRUE(this->camera->isFrustumPlanesBound());

        EXPECT_EQ(StatusOK, this->camera->unbindFrustumPlanes());

        EXPECT_EQ(-1.f, this->camera->getLeftPlane());
        EXPECT_EQ(1.f, this->camera->getRightPlane());
        EXPECT_EQ(-1.f, this->camera->getBottomPlane());
        EXPECT_EQ(1.f, this->camera->getTopPlane());
        EXPECT_EQ(0.1f, this->camera->getNearPlane());
        EXPECT_EQ(1.f, this->camera->getFarPlane());
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

    TYPED_TEST(ACamera, frustumValueSetIfBoundOnlyAffectOriginalValuesThatBecomeActiveAfterUnbound)
    {
        const auto do1 = this->m_scene.createDataVector4f();
        const auto do2 = this->m_scene.createDataVector2f();
        do1->setValue(1, 2, 3, 4);
        do2->setValue(5, 6);

        EXPECT_EQ(StatusOK, this->camera->setFrustum(-1.f, 1.f, -1.f, 1.f, 0.1f, 1.f));

        EXPECT_EQ(StatusOK, this->camera->bindFrustumPlanes(*do1, *do2));
        EXPECT_TRUE(this->camera->isFrustumPlanesBound());

        // frustum planes currently bound so these values have no effect (yet)
        EXPECT_EQ(StatusOK, this->camera->setFrustum(-10.f, 10.f, -10.f, 10.f, 0.01f, 10.f));

        EXPECT_EQ(1.f, this->camera->getLeftPlane());
        EXPECT_EQ(2.f, this->camera->getRightPlane());
        EXPECT_EQ(3.f, this->camera->getBottomPlane());
        EXPECT_EQ(4.f, this->camera->getTopPlane());
        EXPECT_EQ(5.f, this->camera->getNearPlane());
        EXPECT_EQ(6.f, this->camera->getFarPlane());

        EXPECT_EQ(StatusOK, this->camera->unbindFrustumPlanes());

        // only now when unbounded values fall back to last directly set values
        EXPECT_EQ(-10.f, this->camera->getLeftPlane());
        EXPECT_EQ(10.f, this->camera->getRightPlane());
        EXPECT_EQ(-10.f, this->camera->getBottomPlane());
        EXPECT_EQ(10.f, this->camera->getTopPlane());
        EXPECT_EQ(0.01f, this->camera->getNearPlane());
        EXPECT_EQ(10.f, this->camera->getFarPlane());
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

    TYPED_TEST(ACamera, reportsValidationErrorWhenFrustumNotInitializedAndBoundToValidDataButUnboundAfterwards)
    {
        // make viewport valid - not testing viewport validity here
        EXPECT_EQ(StatusOK, this->camera->setViewport(0, 0, 16, 16));

        const auto do1 = this->m_scene.createDataVector4f();
        const auto do2 = this->m_scene.createDataVector2f();
        do1->setValue(-1, 1, -1, 1);
        do2->setValue(1, 10);
        EXPECT_EQ(StatusOK, this->camera->bindFrustumPlanes(*do1, *do2));
        // bound values valid
        EXPECT_EQ(StatusOK, this->camera->validate());

        EXPECT_EQ(StatusOK, this->camera->unbindFrustumPlanes());
        // no valid original values
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

    TYPED_TEST(ACamera, reportsValidationErrorWhenFrustumNotInitializedAndBoundToInvalidData)
    {
        // make viewport valid - not testing viewport validity here
        EXPECT_EQ(StatusOK, this->camera->setViewport(0, 0, 16, 16));

        const auto do1 = this->m_scene.createDataVector4f();
        const auto do2 = this->m_scene.createDataVector2f();
        do1->setValue(1, -1, 1, -1);
        do2->setValue(1, 10);
        EXPECT_EQ(StatusOK, this->camera->bindFrustumPlanes(*do1, *do2));
        EXPECT_NE(StatusOK, this->camera->validate());
    }

    TYPED_TEST(ACamera, reportsValidationErrorWhenViewportInitializedButBoundToInvalidData)
    {
        // make frustum valid - not testing frustum validity here
        EXPECT_EQ(StatusOK, this->camera->setFrustum(-1, 1, -1, 1, 0.1f, 0.2f));

        EXPECT_EQ(StatusOK, this->camera->setViewport(0, 0, 16, 16));

        const auto do1 = this->m_scene.createDataVector2i();
        const auto do2 = this->m_scene.createDataVector2i();
        do1->setValue(1, 2);
        do2->setValue(0, 4);
        EXPECT_EQ(StatusOK, this->camera->bindViewportOffset(*do1));
        EXPECT_EQ(StatusOK, this->camera->bindViewportSize(*do2));
        EXPECT_NE(StatusOK, this->camera->validate());
    }

    TYPED_TEST(ACamera, reportsValidationErrorWhenFrustumInitializedButBoundToInvalidData)
    {
        // make viewport valid - not testing viewport validity here
        EXPECT_EQ(StatusOK, this->camera->setViewport(0, 0, 16, 16));

        EXPECT_EQ(StatusOK, this->camera->setFrustum(-1, 1, -1, 1, 0.1f, 0.2f));

        const auto do1 = this->m_scene.createDataVector4f();
        const auto do2 = this->m_scene.createDataVector2f();
        do1->setValue(1, -1, 1, -1);
        do2->setValue(1, 10);
        EXPECT_EQ(StatusOK, this->camera->bindFrustumPlanes(*do1, *do2));
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

    TEST_F(APerspectiveCamera, canBindFrustumAndGetOriginalValuesWhenUnbound)
    {
        const auto do1 = this->m_scene.createDataVector4f();
        const auto do2 = this->m_scene.createDataVector2f();
        EXPECT_TRUE(RamsesUtils::SetPerspectiveCameraFrustumToDataObjects(45.f, 2.3f, 2.f, 20.f, *do1, *do2));

        EXPECT_EQ(StatusOK, this->camera->setFrustum(19.f, 1.33f, 0.1f, 1.f));

        EXPECT_EQ(StatusOK, this->camera->bindFrustumPlanes(*do1, *do2));
        EXPECT_TRUE(this->camera->isFrustumPlanesBound());

        EXPECT_FLOAT_EQ(45.f, this->camera->getVerticalFieldOfView());
        EXPECT_FLOAT_EQ(2.3f, this->camera->getAspectRatio());
        EXPECT_EQ(2.f, this->camera->getNearPlane());
        EXPECT_EQ(20.f, this->camera->getFarPlane());

        EXPECT_EQ(StatusOK, this->camera->unbindFrustumPlanes());

        EXPECT_FLOAT_EQ(19.f, this->camera->getVerticalFieldOfView());
        EXPECT_FLOAT_EQ(1.33f, this->camera->getAspectRatio());
        EXPECT_EQ(0.1f, this->camera->getNearPlane());
        EXPECT_EQ(1.f, this->camera->getFarPlane());
    }
}
