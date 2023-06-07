//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>
#include "ClientTestUtils.h"

#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/PerspectiveCamera.h"
#include "ramses-client-api/OrthographicCamera.h"
#include "ramses-client-api/PickableObject.h"
#include "RamsesObjectTestTypes.h"

namespace ramses
{
    template <typename T>
    class NodeLazyTransformTest : public LocalTestClientWithScene, public testing::Test
    {
    protected:
        void SetUp() override
        {
            m_node = &this->template createObject<T>("node");
        }

        uint32_t getActualTransformCount() const
        {
            uint32_t count = 0u;
            for (ramses_internal::TransformHandle handle(0u); handle < this->m_internalScene.getTransformCount(); ++handle)
            {
                if (this->m_internalScene.isTransformAllocated(handle))
                {
                    ++count;
                }
            }

            return count;
        }

        T* m_node;
    };

    TYPED_TEST_SUITE(NodeLazyTransformTest, NodeTypes);

    TYPED_TEST(NodeLazyTransformTest, instantiationCreatesNoTransform)
    {
        EXPECT_EQ(this->getActualTransformCount(), 0u);
    }

    TYPED_TEST(NodeLazyTransformTest, callingSetterWithIdentityCreatesNoTransform)
    {
        EXPECT_EQ(this->m_node->setTranslation({0.0f, 0.0f, 0.0f}), StatusOK);
        EXPECT_EQ(this->m_node->translate({0.0f, 0.0f, 0.0f}), StatusOK);
        EXPECT_EQ(this->m_node->setRotation({0.0f, 0.0f, 0.0f}, ERotationType::Euler_ZYX), StatusOK);
        EXPECT_EQ(this->m_node->setRotation({0.0f, 0.0f, 0.0f}, ERotationType::Euler_XYZ), StatusOK);
        EXPECT_EQ(this->m_node->setRotation({0.0f, 0.0f, 0.0f}, ERotationType::Euler_ZYZ), StatusOK);
        EXPECT_EQ(this->m_node->setScaling({1.0f, 1.0f, 1.0f}), StatusOK);
        EXPECT_EQ(this->m_node->scale({1.0f, 1.0f, 1.0f}), StatusOK);

        EXPECT_EQ(this->getActualTransformCount(), 0u);
    }

    TYPED_TEST(NodeLazyTransformTest, callingGettersWithoutTransformReturnIdentity)
    {
        vec3f value;

        EXPECT_EQ(this->m_node->getTranslation(value), StatusOK);
        EXPECT_EQ(value, vec3f(0.f));

        EXPECT_EQ(this->m_node->getRotation(value), StatusOK);
        EXPECT_EQ(value, vec3f(0.f));

        quat q;
        EXPECT_EQ(this->m_node->getRotation(q), StatusOK);
        EXPECT_EQ(q, glm::identity<quat>());

        EXPECT_EQ(this->m_node->getScaling(value), StatusOK);
        EXPECT_EQ(value, vec3f(1.f));
    }

    TYPED_TEST(NodeLazyTransformTest, callingSetTranslationWithValueCreatesTransform)
    {
        EXPECT_EQ(this->getActualTransformCount(), 0u);
        EXPECT_EQ(this->m_node->setTranslation({1.0f, 2.0f, 3.0f}), StatusOK);
        EXPECT_EQ(this->getActualTransformCount(), 1u);
    }

    TYPED_TEST(NodeLazyTransformTest, callingTranslateWithValueCreatesTransform)
    {
        EXPECT_EQ(this->getActualTransformCount(), 0u);
        EXPECT_EQ(this->m_node->translate({1.0f, 2.0f, 3.0f}), StatusOK);
        EXPECT_EQ(this->getActualTransformCount(), 1u);
    }

    TYPED_TEST(NodeLazyTransformTest, callingSetRotationWithValueCreatesTransform)
    {
        EXPECT_EQ(this->getActualTransformCount(), 0u);
        EXPECT_EQ(this->m_node->setRotation({1.0f, 2.0f, 3.0f}, ERotationType::Euler_YXZ), StatusOK);
        EXPECT_EQ(this->getActualTransformCount(), 1u);
    }

    TYPED_TEST(NodeLazyTransformTest, callingSetScalingWithValueCreatesTransform)
    {
        EXPECT_EQ(this->getActualTransformCount(), 0u);
        EXPECT_EQ(this->m_node->setScaling({1.0f, 2.0f, 3.0f}), StatusOK);
        EXPECT_EQ(this->getActualTransformCount(), 1u);
    }

    TYPED_TEST(NodeLazyTransformTest, callingScaleWithValueCreatesTransform)
    {
        EXPECT_EQ(this->getActualTransformCount(), 0u);
        EXPECT_EQ(this->m_node->scale({1.0f, 2.0f, 3.0f}), StatusOK);
        EXPECT_EQ(this->getActualTransformCount(), 1u);
    }

    TYPED_TEST(NodeLazyTransformTest, destructionDestroysTransform)
    {
        EXPECT_EQ(this->m_node->setTranslation({1.0f, 2.0f, 3.0f}), StatusOK);

        EXPECT_EQ(this->getActualTransformCount(), 1u);
        this->m_scene.destroy(*this->m_node);
        EXPECT_EQ(this->getActualTransformCount(), 0u);
    }
}
