//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "SceneTest.h"

using namespace testing;

namespace ramses::internal
{
    TYPED_TEST_SUITE(AScene, SceneTypes);

    TYPED_TEST(AScene, PickableObjectCreated)
    {
        EXPECT_EQ(0u, this->m_scene.getPickableObjectCount());

        PickableObjectHandle pickableHandle = this->m_scene.allocatePickableObject(DataBufferHandle(1u), NodeHandle(2u), PickableObjectId{3u}, {});

        EXPECT_EQ(1u, this->m_scene.getPickableObjectCount());
        EXPECT_TRUE(this->m_scene.isPickableObjectAllocated(pickableHandle));
    }

    TYPED_TEST(AScene, PickableObjectReleased)
    {
        PickableObjectHandle pickableHandle = this->m_scene.allocatePickableObject(DataBufferHandle(1u), NodeHandle(2u), PickableObjectId{3u}, {});
        this->m_scene.releasePickableObject(pickableHandle);

        EXPECT_FALSE(this->m_scene.isPickableObjectAllocated(pickableHandle));
    }

    TYPED_TEST(AScene, DoesNotContainPickableObjectWhichWasNotCreated)
    {
        EXPECT_FALSE(this->m_scene.isPickableObjectAllocated(PickableObjectHandle(1)));
    }

    TYPED_TEST(AScene, PickableObjectCanGetPropertiesGivenAtAllocationTime)
    {
        const DataBufferHandle geometryBuffer = this->m_scene.allocateDataBuffer(EDataBufferType::IndexBuffer, EDataType::UInt32, 10u, {});
        const NodeHandle nodeHandle = this->m_scene.allocateNode(0, {});
        const PickableObjectId id{ 3u };

        PickableObjectHandle pickableHandle = this->m_scene.allocatePickableObject(geometryBuffer, nodeHandle, id, {});
        EXPECT_EQ(geometryBuffer, this->m_scene.getPickableObject(pickableHandle).geometryHandle);
        EXPECT_EQ(nodeHandle, this->m_scene.getPickableObject(pickableHandle).nodeHandle);
        EXPECT_EQ(id, this->m_scene.getPickableObject(pickableHandle).id);
    }

    TYPED_TEST(AScene, PickableObjectCanSetId)
    {
        const PickableObjectHandle pickableHandle = this->m_scene.allocatePickableObject(DataBufferHandle(1u), NodeHandle(2u), PickableObjectId{ 3u }, {});
        EXPECT_EQ(1u, this->m_scene.getPickableObjectCount());

        const PickableObjectId id{ 666u };
        this->m_scene.setPickableObjectId(pickableHandle, id);
        EXPECT_EQ(id, this->m_scene.getPickableObject(pickableHandle).id);
    }

    TYPED_TEST(AScene, PickableObjectCanSetCamera)
    {
        const PickableObjectHandle pickableHandle = this->m_scene.allocatePickableObject(DataBufferHandle(1u), NodeHandle(2u), PickableObjectId{3u}, {});
        EXPECT_EQ(1u, this->m_scene.getPickableObjectCount());

        const CameraHandle cameraHandle(11u);
        this->m_scene.setPickableObjectCamera(pickableHandle, cameraHandle);
        EXPECT_EQ(cameraHandle, this->m_scene.getPickableObject(pickableHandle).cameraHandle);
    }

    TYPED_TEST(AScene, CanDisablePickableObject)
    {
        PickableObjectHandle pickableHandle = this->m_scene.allocatePickableObject(DataBufferHandle(1u), NodeHandle(2u), PickableObjectId{3u}, {});
        EXPECT_TRUE(this->m_scene.getPickableObject(pickableHandle).isEnabled);

        this->m_scene.setPickableObjectEnabled(pickableHandle, false);
        EXPECT_FALSE(this->m_scene.getPickableObject(pickableHandle).isEnabled);
    }

    TYPED_TEST(AScene, PickableObjectEnabledByDefault)
    {
        PickableObjectHandle pickableHandle = this->m_scene.allocatePickableObject(DataBufferHandle(1u), NodeHandle(2u), PickableObjectId{3u}, {});
        EXPECT_TRUE(this->m_scene.getPickableObject(pickableHandle).isEnabled);
    }
}
