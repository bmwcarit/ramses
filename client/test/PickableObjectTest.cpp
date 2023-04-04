//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "ClientTestUtils.h"
#include "ramses-client-api/PickableObject.h"
#include "ramses-client-api/PerspectiveCamera.h"
#include "ramses-client-api/ArrayBuffer.h"

#include "PickableObjectImpl.h"
#include "CameraNodeImpl.h"
#include "ArrayBufferImpl.h"

using namespace testing;
using namespace ramses_internal;

namespace ramses
{
    class APickableObject : public LocalTestClientWithScene, public testing::Test
    {
    protected:
        APickableObject()
            : LocalTestClientWithScene()
            , geometryBuffer(*createGeometryBuffer())
            , pickableObject(*m_scene.createPickableObject(geometryBuffer, pickableObjectId_t(1u)))
            , pickableObjectHandle(pickableObject.impl.getPickableObjectHandle())
        {
        }

        void setPickableCamera()
        {
            pickableCamera = m_scene.createPerspectiveCamera("pickableCamera");
            pickableCamera->setFrustum(-1.4f, 1.4f, -1.4f, 1.4f, 1.f, 100.f);
            pickableCamera->setViewport(0, 0, 200, 200);
            pickableObject.setCamera(*pickableCamera);
        }

        ArrayBuffer* createGeometryBuffer()
        {
            return m_scene.createArrayBuffer(EDataType::Vector3F, 3u);
        }

        void setGeometryBufferData()
        {
            const vec3f dummyVec{ 0.f, 0.f, 0.f };
            geometryBuffer.updateData(0u, 1u, &dummyVec);
        }

        PerspectiveCamera* pickableCamera = nullptr;
        ArrayBuffer& geometryBuffer;
        PickableObject& pickableObject;
        const ramses_internal::PickableObjectHandle pickableObjectHandle;
    };

    TEST_F(APickableObject, CanSetAndGetPickableCamera)
    {
        setPickableCamera();
        ASSERT_NE(nullptr, pickableCamera);

        const auto cam = pickableObject.getCamera();
        EXPECT_EQ(cam, pickableObject.getCamera());
        EXPECT_EQ(cam->impl.getCameraHandle(), pickableCamera->impl.getCameraHandle());
    }

    TEST_F(APickableObject, CannotSetPickableCameraFromAnotherScene)
    {
        Scene* anotherScene = client.createScene(sceneId_t(214u));
        PerspectiveCamera* cameraFromAnotherScene = anotherScene->createPerspectiveCamera("pickableCamera");
        ASSERT_NE(nullptr, cameraFromAnotherScene);
        cameraFromAnotherScene->setFrustum(-1.4f, 1.4f, -1.4f, 1.4f, 1.f, 100.f);
        cameraFromAnotherScene->setViewport(0, 0, 200, 200);
        EXPECT_EQ(StatusOK, cameraFromAnotherScene->validate());

        EXPECT_NE(StatusOK, pickableObject.setCamera(*cameraFromAnotherScene));
    }

    TEST_F(APickableObject, CannotSetInvalidPickableCamera)
    {
        const PerspectiveCamera* invalidCamera = m_scene.createPerspectiveCamera();
        EXPECT_NE(StatusOK, pickableObject.setCamera(*invalidCamera));
    }

    TEST_F(APickableObject, CanGetGeometryBufferOfPickableObject)
    {
        const ArrayBuffer& geoBuffer = pickableObject.getGeometryBuffer();
        EXPECT_EQ(&geoBuffer, &geometryBuffer);
        EXPECT_EQ(geoBuffer.impl.getDataBufferHandle(), geometryBuffer.impl.getDataBufferHandle());
    }

    TEST_F(APickableObject, CanSetAndGetPickableObjectId)
    {
        const pickableObjectId_t testId(214);
        const status_t success = pickableObject.setPickableObjectId(testId);
        ASSERT_EQ(StatusOK, success);

        const pickableObjectId_t pickableId = pickableObject.getPickableObjectId();
        EXPECT_EQ(testId, pickableId);
    }

    TEST_F(APickableObject, PickableObjectIsEnabledByDefault)
    {
        EXPECT_TRUE(pickableObject.isEnabled());
    }

    TEST_F(APickableObject, CanEnableAndDisablePickableObject)
    {
        pickableObject.setEnabled(false);
        EXPECT_FALSE(pickableObject.isEnabled());

        pickableObject.setEnabled(true);
        EXPECT_TRUE(pickableObject.isEnabled());
    }

    TEST_F(APickableObject, CanValidatePickableObject)
    {
        setPickableCamera();
        setGeometryBufferData();
        EXPECT_EQ(StatusOK, pickableObject.validate());
    }

    TEST_F(APickableObject, CannotValidatePickableObjectWhenReferencingDeletedGeometryBuffer)
    {
        setPickableCamera();
        setGeometryBufferData();
        m_scene.destroy(geometryBuffer);
        EXPECT_NE(StatusOK, pickableObject.validate());
    }

    TEST_F(APickableObject, CannotValidatePickableObjectWhenReferencingInvalidGeometryBuffer)
    {
        setPickableCamera();
        EXPECT_NE(StatusOK, pickableObject.validate());
    }

    TEST_F(APickableObject, CannotValidatePickableObjectWithoutSettingCamera)
    {
        setGeometryBufferData();
        EXPECT_NE(StatusOK, pickableObject.validate());
    }

    TEST_F(APickableObject, CannotValidatePickableObjectWhenReferencingDeletedCamera)
    {
        setPickableCamera();
        setGeometryBufferData();
        m_scene.destroy(*pickableCamera);
        EXPECT_NE(StatusOK, pickableObject.validate());
    }
}
