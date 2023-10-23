//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses/client/RenderGroupMeshIterator.h"
#include "ramses/client/RenderGroup.h"
#include "ClientTestUtils.h"

using namespace testing;

namespace ramses::internal
{
    class ARenderGroupMeshIterator : public LocalTestClientWithScene, public testing::Test
    {
    public:
        ARenderGroupMeshIterator()
            : renderGroup(*m_scene.createRenderGroup())
            , mesh1(*m_scene.createMeshNode())
            , mesh2(*m_scene.createMeshNode())
        {
        }

    protected:
        ramses::RenderGroup& renderGroup;
        const MeshNode& mesh1;
        const MeshNode& mesh2;
    };

    TEST_F(ARenderGroupMeshIterator, returnsNextNULLWhenRenderGroupEmpty)
    {
        RenderGroupMeshIterator iterator(renderGroup);
        EXPECT_TRUE(nullptr == iterator.getNext());
    }

    TEST_F(ARenderGroupMeshIterator, canIterateOverAddedMeshNodes)
    {
        renderGroup.addMeshNode(mesh1);
        renderGroup.addMeshNode(mesh2);
        RenderGroupMeshIterator iterator(renderGroup);

        EXPECT_TRUE(&mesh1 == iterator.getNext());
        EXPECT_TRUE(&mesh2 == iterator.getNext());
        EXPECT_TRUE(nullptr == iterator.getNext());
    }
}
