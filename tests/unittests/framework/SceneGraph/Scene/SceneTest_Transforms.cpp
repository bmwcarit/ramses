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

    TYPED_TEST(AScene, ContainsCreatedTransform)
    {
        const TransformHandle transform = this->m_scene.allocateTransform(this->m_scene.allocateNode(0, {}), {});
        EXPECT_TRUE(this->m_scene.isTransformAllocated(transform));
    }

    TYPED_TEST(AScene, ReleasesTransform)
    {
        const TransformHandle transform = this->m_scene.allocateTransform(this->m_scene.allocateNode(0, {}), {});
        this->m_scene.releaseTransform(transform);

        EXPECT_FALSE(this->m_scene.isTransformAllocated(transform));
    }

    TYPED_TEST(AScene, ContainsZeroTotalTransformsUponCreation)
    {
        EXPECT_EQ(0u, this->m_scene.getTransformCount());
    }

    TYPED_TEST(AScene, CreatingTransformsIncreasesTransformsCount)
    {
        this->m_scene.allocateTransform(this->m_scene.allocateNode(0, {}), {});

        EXPECT_EQ(1u, this->m_scene.getTransformCount());
    }

    TYPED_TEST(AScene, DoesNotContainUnexistingTransform)
    {
        EXPECT_FALSE(this->m_scene.isTransformAllocated(TransformHandle(1234u)));
    }

    TYPED_TEST(AScene, SetsTranslationForTransform)
    {
        const TransformHandle transform = this->m_scene.allocateTransform(this->m_scene.allocateNode(0, {}), {});

        const glm::vec3 translation(1, 2, 3);

        this->m_scene.setTranslation(transform, translation);
        EXPECT_EQ(translation, this->m_scene.getTranslation(transform));
    }

    TYPED_TEST(AScene, SetsRotationForTransform)
    {
        const TransformHandle transform = this->m_scene.allocateTransform(this->m_scene.allocateNode(0, {}), {});

        const glm::vec4 rotation(4, 5, 6, 1.f);

        this->m_scene.setRotation(transform, rotation, ERotationType::Euler_XYX);
        EXPECT_EQ(rotation, this->m_scene.getRotation(transform));
        EXPECT_EQ(ERotationType::Euler_XYX, this->m_scene.getRotationType(transform));
    }

    TYPED_TEST(AScene, SetsScalingForTransform)
    {
        const TransformHandle transform = this->m_scene.allocateTransform(this->m_scene.allocateNode(0, {}), {});

        const glm::vec3 scaling(7, 8, 9);

        this->m_scene.setScaling(transform, scaling);
        EXPECT_EQ(scaling, this->m_scene.getScaling(transform));
    }

    TYPED_TEST(AScene, CanGetAssignedNodeHandle)
    {
        const NodeHandle node = this->m_scene.allocateNode(0, {});
        const TransformHandle transHandle = this->m_scene.allocateTransform(node, {});

        EXPECT_EQ(node, this->m_scene.getTransformNode(transHandle));
    }
}
