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

    TYPED_TEST(AScene, ContainsCreatedNode)
    {
        this->m_scene.allocateNode();
        NodeHandle node2 = this->m_scene.allocateNode();

        EXPECT_TRUE(this->m_scene.isNodeAllocated(node2));
    }

    TYPED_TEST(AScene, ContainsZeroTotalNodesUponCreation)
    {
        EXPECT_EQ(0u, this->m_scene.getNodeCount());
    }

    TYPED_TEST(AScene, CreatingNodeIncreasesNodeCount)
    {
        this->m_scene.allocateNode();

        EXPECT_EQ(1u, this->m_scene.getNodeCount());
    }

}
