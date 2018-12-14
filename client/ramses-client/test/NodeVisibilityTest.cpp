//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>

#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/PerspectiveCamera.h"
#include "ramses-client-api/OrthographicCamera.h"
#include "ramses-client-api/RemoteCamera.h"
#include "ClientTestUtils.h"
#include "RamsesObjectTestTypes.h"
#include "NodeImpl.h"
#include "MeshNodeImpl.h"
#include "CameraNodeImpl.h"

using namespace testing;
using namespace ramses_internal;

namespace ramses
{
    template <typename T>
    class ANodeVisibilityTest : public LocalTestClientWithScene, public testing::Test
    {
    protected:
        virtual void SetUp()
        {
            m_visibilityNode = &this->template createObject<T>("node");
            m_parentVisNode = &this->template createObject<T>("parent");
            m_visibilityNode->setParent(*m_parentVisNode);
            m_childMesh = &createObject<MeshNode>("mesh1");
            m_childMesh->setParent(*m_visibilityNode);
        }

        void testFlattenedVisibility(bool flag)
        {
            EXPECT_EQ(this->m_childMesh->impl.getFlattenedVisibility(), flag);
        }

        T* m_parentVisNode;
        T* m_visibilityNode;
        MeshNode* m_childMesh;
    };

    TYPED_TEST_CASE(ANodeVisibilityTest, NodeTypes);

    TYPED_TEST(ANodeVisibilityTest, isVisibleInitially)
    {
        EXPECT_TRUE(this->m_visibilityNode->impl.getVisibility());
    }

    TYPED_TEST(ANodeVisibilityTest, canChangeVisibility)
    {
        this->m_visibilityNode->setVisibility(false);
        EXPECT_FALSE(this->m_visibilityNode->impl.getVisibility());
    }

    TYPED_TEST(ANodeVisibilityTest, isMarkedDirtyWhenChangingVisibility)
    {
        this->m_visibilityNode->setVisibility(false);
        EXPECT_TRUE(this->m_visibilityNode->impl.isDirty());
    }

    TYPED_TEST(ANodeVisibilityTest, staysCleanWhenSettingTheSameVisibility)
    {
        this->m_visibilityNode->setVisibility(false);
        EXPECT_TRUE(this->m_visibilityNode->impl.isDirty());
        this->m_scene.flush(); // to clear dirty state

        this->m_visibilityNode->setVisibility(false);
        EXPECT_FALSE(this->m_visibilityNode->impl.isDirty());
    }

    TYPED_TEST(ANodeVisibilityTest, confidenceTest_isMarkedDirtyWhenChangingVisibilityMultipleTimes)
    {
        this->m_visibilityNode->setVisibility(false);
        EXPECT_TRUE(this->m_visibilityNode->impl.isDirty());
        this->m_scene.flush(); // to clear dirty state

        this->m_visibilityNode->setVisibility(true);
        EXPECT_TRUE(this->m_visibilityNode->impl.isDirty());
        this->m_scene.flush(); // to clear dirty state

        this->m_visibilityNode->setVisibility(false);
        EXPECT_TRUE(this->m_visibilityNode->impl.isDirty());
    }

    TYPED_TEST(ANodeVisibilityTest, hasFlattenedVisibilitySetInitially)
    {
        this->testFlattenedVisibility(true);
        this->m_scene.flush();
        this->testFlattenedVisibility(true);
    }

    TYPED_TEST(ANodeVisibilityTest, visibilityPropagatesFromParentToChildrenOnFlush)
    {
        this->m_visibilityNode->setVisibility(false);
        EXPECT_TRUE(this->m_visibilityNode->impl.isDirty());

        this->m_scene.flush();
        this->testFlattenedVisibility(false);
    }

    TYPED_TEST(ANodeVisibilityTest, visibilityPropagatesFromGrandParentToChildrenOnFlush)
    {
        this->m_parentVisNode->setVisibility(false);
        EXPECT_TRUE(this->m_visibilityNode->impl.isDirty());

        this->m_scene.flush();
        this->testFlattenedVisibility(false);
    }

    TYPED_TEST(ANodeVisibilityTest, visibilityIsFlattenedToMeshNodeItself)
    {
        this->m_childMesh->setVisibility(false);
        this->m_scene.flush();
        EXPECT_FALSE(this->m_childMesh->impl.getFlattenedVisibility());
    }
}
