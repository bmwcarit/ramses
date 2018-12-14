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
#include "ramses-client-api/RemoteCamera.h"
#include "RamsesObjectTestTypes.h"

namespace ramses
{
    template <typename T>
    class NodeLazyTransformTest : public LocalTestClientWithScene, public testing::Test
    {
    protected:
        virtual void SetUp()
        {
            m_node = &this->template createObject<T>("node");
        }

        ramses_internal::UInt32 getActualTransformCount() const
        {
            ramses_internal::UInt32 count = 0u;
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

    TYPED_TEST_CASE(NodeLazyTransformTest, NodeTypes);

    TYPED_TEST(NodeLazyTransformTest, instantiationCreatesNoTransform)
    {
        EXPECT_EQ(this->getActualTransformCount(), 0u);
    }

    TYPED_TEST(NodeLazyTransformTest, callingSetterWithIdentityCreatesNoTransform)
    {
        EXPECT_EQ(this->m_node->setTranslation(0.0f, 0.0f, 0.0f), StatusOK);
        EXPECT_EQ(this->m_node->translate     (0.0f, 0.0f, 0.0f), StatusOK);
        EXPECT_EQ(this->m_node->setRotation   (0.0f, 0.0f, 0.0f), StatusOK);
        EXPECT_EQ(this->m_node->rotate        (0.0f, 0.0f, 0.0f), StatusOK);
        EXPECT_EQ(this->m_node->setScaling    (1.0f, 1.0f, 1.0f), StatusOK);
        EXPECT_EQ(this->m_node->scale         (1.0f, 1.0f, 1.0f), StatusOK);

        EXPECT_EQ(this->getActualTransformCount(), 0u);
    }

    TYPED_TEST(NodeLazyTransformTest, callingGettersWithoutTransformReturnIdentity)
    {
        float x;
        float y;
        float z;

        EXPECT_EQ(this->m_node->getTranslation(x, y, z), StatusOK);
        EXPECT_EQ(x, 0.0f);
        EXPECT_EQ(y, 0.0f);
        EXPECT_EQ(z, 0.0f);

        EXPECT_EQ(this->m_node->getRotation(x, y, z), StatusOK);
        EXPECT_EQ(x, 0.0f);
        EXPECT_EQ(y, 0.0f);
        EXPECT_EQ(z, 0.0f);

        EXPECT_EQ(this->m_node->getScaling(x, y, z), StatusOK);
        EXPECT_EQ(x, 1.0f);
        EXPECT_EQ(y, 1.0f);
        EXPECT_EQ(z, 1.0f);
    }

    TYPED_TEST(NodeLazyTransformTest, callingSetTranslationWithValueCreatesTransform)
    {
        EXPECT_EQ(this->getActualTransformCount(), 0u);
        EXPECT_EQ(this->m_node->setTranslation(1.0f, 2.0f, 3.0f), StatusOK);
        EXPECT_EQ(this->getActualTransformCount(), 1u);
    }

    TYPED_TEST(NodeLazyTransformTest, callingTranslateWithValueCreatesTransform)
    {
        EXPECT_EQ(this->getActualTransformCount(), 0u);
        EXPECT_EQ(this->m_node->translate(1.0f, 2.0f, 3.0f), StatusOK);
        EXPECT_EQ(this->getActualTransformCount(), 1u);
    }

    TYPED_TEST(NodeLazyTransformTest, callingSetRotationWithValueCreatesTransform)
    {
        EXPECT_EQ(this->getActualTransformCount(), 0u);
        EXPECT_EQ(this->m_node->setRotation(1.0f, 2.0f, 3.0f), StatusOK);
        EXPECT_EQ(this->getActualTransformCount(), 1u);
    }

    TYPED_TEST(NodeLazyTransformTest, callingRotateWithValueCreatesTransform)
    {
        EXPECT_EQ(this->getActualTransformCount(), 0u);
        EXPECT_EQ(this->m_node->rotate(1.0f, 2.0f, 3.0f), StatusOK);
        EXPECT_EQ(this->getActualTransformCount(), 1u);
    }

    TYPED_TEST(NodeLazyTransformTest, callingSetScalingWithValueCreatesTransform)
    {
        EXPECT_EQ(this->getActualTransformCount(), 0u);
        EXPECT_EQ(this->m_node->setScaling(1.0f, 2.0f, 3.0f), StatusOK);
        EXPECT_EQ(this->getActualTransformCount(), 1u);
    }

    TYPED_TEST(NodeLazyTransformTest, callingScaleWithValueCreatesTransform)
    {
        EXPECT_EQ(this->getActualTransformCount(), 0u);
        EXPECT_EQ(this->m_node->scale(1.0f, 2.0f, 3.0f), StatusOK);
        EXPECT_EQ(this->getActualTransformCount(), 1u);
    }

    TYPED_TEST(NodeLazyTransformTest, destructionDestroysTransform)
    {
        EXPECT_EQ(this->m_node->setTranslation(1.0f, 2.0f, 3.0f), StatusOK);

        EXPECT_EQ(this->getActualTransformCount(), 1u);
        this->m_scene.destroy(*this->m_node);
        EXPECT_EQ(this->getActualTransformCount(), 0u);
    }
}
