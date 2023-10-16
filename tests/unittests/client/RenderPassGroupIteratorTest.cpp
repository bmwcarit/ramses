//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses/client/RenderPassGroupIterator.h"
#include "ramses/client/RenderPass.h"
#include "ClientTestUtils.h"

using namespace testing;

namespace ramses::internal
{
    class ARenderPassGroupIterator : public LocalTestClientWithScene, public testing::Test
    {
    public:
        ARenderPassGroupIterator()
            : renderPass(*m_scene.createRenderPass())
            , group1(*m_scene.createRenderGroup())
            , group2(*m_scene.createRenderGroup())
        {
        }

    protected:
        ramses::RenderPass& renderPass;
        const ramses::RenderGroup& group1;
        const ramses::RenderGroup& group2;
    };

    TEST_F(ARenderPassGroupIterator, getNextNULLForEmptyRenderGroup)
    {
        RenderPassGroupIterator iterator(renderPass);
        EXPECT_TRUE(nullptr == iterator.getNext());
    }

    TEST_F(ARenderPassGroupIterator, canIterateOverAddedMeshNodes)
    {
        renderPass.addRenderGroup(group1);
        renderPass.addRenderGroup(group2);
        RenderPassGroupIterator iterator(renderPass);

        EXPECT_TRUE(&group1 == iterator.getNext());
        EXPECT_TRUE(&group2 == iterator.getNext());
        EXPECT_TRUE(nullptr == iterator.getNext());
    }
}
