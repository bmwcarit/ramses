//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
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

    static DataInstanceHandle AllocDummyDataInstance(IScene& scene)
    {
        const auto dataLayout = scene.allocateDataLayout({DataFieldInfo{ramses::internal::EDataType::Vector2I}, DataFieldInfo{ramses::internal::EDataType::Vector2I}}, ResourceContentHash(123u, 0u), {});
        return scene.allocateDataInstance(dataLayout, {});
    }

    static CameraHandle AllocCamera(ECameraProjectionType type, IScene& scene)
    {
        return scene.allocateCamera(type, scene.allocateNode(0, {}), AllocDummyDataInstance(scene), {});
    }

    TYPED_TEST(AScene, DoesNotHaveAnyCamerasInitially)
    {
        EXPECT_EQ(0u, this->m_scene.getCameraCount());
    }

    TYPED_TEST(AScene, CountsItsCamerasCorrectly)
    {
        EXPECT_EQ(0u, this->m_scene.getCameraCount());

        AllocCamera(ECameraProjectionType::Perspective, this->m_scene);
        AllocCamera(ECameraProjectionType::Perspective, this->m_scene);

        EXPECT_EQ(2u, this->m_scene.getCameraCount());
    }

    TYPED_TEST(AScene, ContainsCreatedCamera)
    {
        AllocCamera(ECameraProjectionType::Perspective, this->m_scene);
        const CameraHandle camera2 = AllocCamera(ECameraProjectionType::Perspective, this->m_scene);

        EXPECT_TRUE(this->m_scene.isCameraAllocated(camera2));
    }

    TYPED_TEST(AScene, GetsAssignedCameraNode)
    {
        const NodeHandle node = this->m_scene.allocateNode(0, {});
        const CameraHandle camera = this->m_scene.allocateCamera(ECameraProjectionType::Perspective, node, AllocDummyDataInstance(this->m_scene), {});
        EXPECT_EQ(node, this->m_scene.getCamera(camera).node);
    }

    TYPED_TEST(AScene, GetsAssignedDataInstance)
    {
        const auto dataInst = AllocDummyDataInstance(this->m_scene);
        const CameraHandle camera = this->m_scene.allocateCamera(ECameraProjectionType::Perspective, this->m_scene.allocateNode(0, {}), dataInst, {});
        EXPECT_EQ(dataInst, this->m_scene.getCamera(camera).dataInstance);
    }

    TYPED_TEST(AScene, CreatesAPerspectiveTypeCameraByDefault)
    {
        const CameraHandle camera = AllocCamera(ECameraProjectionType::Perspective, this->m_scene);
        EXPECT_EQ(ECameraProjectionType::Perspective, this->m_scene.getCamera(camera).projectionType);
    }

    TYPED_TEST(AScene, ReleaseCamera)
    {
        const CameraHandle camera = AllocCamera(ECameraProjectionType::Perspective, this->m_scene);
        this->m_scene.releaseCamera(camera);
        EXPECT_FALSE(this->m_scene.isCameraAllocated(camera));
    }
}
