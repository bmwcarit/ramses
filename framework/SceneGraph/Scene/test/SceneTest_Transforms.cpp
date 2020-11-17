//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#include "SceneTest.h"

using namespace testing;

namespace ramses_internal
{
    TYPED_TEST_SUITE(AScene, SceneTypes);

    TYPED_TEST(AScene, ContainsCreatedTransform)
    {
        const TransformHandle transform = this->m_scene.allocateTransform(this->m_scene.allocateNode());
        EXPECT_TRUE(this->m_scene.isTransformAllocated(transform));
    }

    TYPED_TEST(AScene, ReleasesTransform)
    {
        const TransformHandle transform = this->m_scene.allocateTransform(this->m_scene.allocateNode());
        this->m_scene.releaseTransform(transform);

        EXPECT_FALSE(this->m_scene.isTransformAllocated(transform));
    }

    TYPED_TEST(AScene, ContainsZeroTotalTransformsUponCreation)
    {
        EXPECT_EQ(0u, this->m_scene.getTransformCount());
    }

    TYPED_TEST(AScene, CreatingTransformsIncreasesTransformsCount)
    {
        this->m_scene.allocateTransform(this->m_scene.allocateNode());

        EXPECT_EQ(1u, this->m_scene.getTransformCount());
    }

    TYPED_TEST(AScene, DoesNotContainUnexistingTransform)
    {
        EXPECT_FALSE(this->m_scene.isTransformAllocated(TransformHandle(1234u)));
    }

    TYPED_TEST(AScene, SetsTranslationForTransform)
    {
        const TransformHandle transform = this->m_scene.allocateTransform(this->m_scene.allocateNode());

        const Vector3 translation(1, 2, 3);

        this->m_scene.setTranslation(transform, translation);
        EXPECT_EQ(translation, this->m_scene.getTranslation(transform));
    }

    TYPED_TEST(AScene, SetsRotationForTransform)
    {
        const TransformHandle transform = this->m_scene.allocateTransform(this->m_scene.allocateNode());

        const Vector3 rotation(4, 5, 6);

        this->m_scene.setRotation(transform, rotation, ERotationConvention::XYX);
        EXPECT_EQ(rotation, this->m_scene.getRotation(transform));
        EXPECT_EQ(ERotationConvention::XYX, this->m_scene.getRotationConvention(transform));
    }

    TYPED_TEST(AScene, SetsRotationForTransformUsingSetterForAnimations)
    {
        //skip test for ActionTestScene
        if (std::is_same<TypeParam, ActionTestScene>::value)
            return;

        const TransformHandle transform = this->m_scene.allocateTransform(this->m_scene.allocateNode());

        const Vector3 rotation(4, 5, 6);

        this->m_scene.setRotation(transform, { 0.f, 0.f, 0.f }, ERotationConvention::Legacy_ZYX);
        this->m_scene.setRotationForAnimation(transform, rotation);
        EXPECT_EQ(rotation, this->m_scene.getRotation(transform));
    }

    TYPED_TEST(AScene, FailsToSetRotationForTransformUsingSetterForAnimationsForNonLegacyConvention)
    {
        //skip test for ActionTestScene
        if (std::is_same<TypeParam, ActionTestScene>::value)
            return;

        const TransformHandle transform = this->m_scene.allocateTransform(this->m_scene.allocateNode());

        const Vector3 initialRotation{ 1.f, 2.f, 3.f };
        const Vector3 destRotation{ 4, 5, 6 };

        this->m_scene.setRotation(transform, initialRotation, ERotationConvention::ZYX);
        this->m_scene.setRotationForAnimation(transform, destRotation);
        EXPECT_EQ(initialRotation, this->m_scene.getRotation(transform));
    }

    TYPED_TEST(AScene, SetsScalingForTransform)
    {
        const TransformHandle transform = this->m_scene.allocateTransform(this->m_scene.allocateNode());

        const Vector3 scaling(7, 8, 9);

        this->m_scene.setScaling(transform, scaling);
        EXPECT_EQ(scaling, this->m_scene.getScaling(transform));
    }

    TYPED_TEST(AScene, CanGetAssignedNodeHandle)
    {
        const NodeHandle node = this->m_scene.allocateNode();
        const TransformHandle transHandle = this->m_scene.allocateTransform(node);

        EXPECT_EQ(node, this->m_scene.getTransformNode(transHandle));
    }
}
