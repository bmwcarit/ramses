//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "SceneTest.h"
#include "SceneAPI/RenderGroupUtils.h"

using namespace testing;

namespace ramses_internal
{
    TYPED_TEST_SUITE(AScene, SceneTypes);

    TYPED_TEST(AScene, canCreateRenderGroup)
    {
        EXPECT_EQ(0u, this->m_scene.getRenderGroupCount());

        const RenderGroupHandle group = this->m_scene.allocateRenderGroup();

        EXPECT_EQ(1u, this->m_scene.getRenderGroupCount());
        EXPECT_TRUE(this->m_scene.isRenderGroupAllocated(group));
    }

    TYPED_TEST(AScene, canReleaseRenderGroup)
    {
        const RenderGroupHandle group = this->m_scene.allocateRenderGroup();
        this->m_scene.releaseRenderGroup(group);

        EXPECT_FALSE(this->m_scene.isRenderGroupAllocated(group));
    }

    TYPED_TEST(AScene, canAddRenderableToRenderGroup)
    {
        const RenderGroupHandle renderGroup = this->m_scene.allocateRenderGroup();
        const RenderableHandle renderable = this->m_scene.allocateRenderable(this->m_scene.allocateNode());
        const RenderGroup& rg = this->m_scene.getRenderGroup(renderGroup);
        EXPECT_FALSE(RenderGroupUtils::ContainsRenderable(renderable, rg));
        EXPECT_TRUE(rg.renderables.empty());

        this->m_scene.addRenderableToRenderGroup(renderGroup, renderable, 3);

        EXPECT_TRUE(RenderGroupUtils::ContainsRenderable(renderable, rg));
        ASSERT_EQ(1u, rg.renderables.size());
        EXPECT_EQ(renderable, rg.renderables[0].renderable);
        EXPECT_EQ(3, rg.renderables[0].order);
    }

    TYPED_TEST(AScene, canRemoveRenderableFromRenderGroup)
    {
        const RenderGroupHandle renderGroup = this->m_scene.allocateRenderGroup();
        const RenderableHandle renderable = this->m_scene.allocateRenderable(this->m_scene.allocateNode());
        const RenderableHandle renderable2 = this->m_scene.allocateRenderable(this->m_scene.allocateNode());
        this->m_scene.addRenderableToRenderGroup(renderGroup, renderable, 1);
        this->m_scene.addRenderableToRenderGroup(renderGroup, renderable2, 2);

        const RenderGroup& rg = this->m_scene.getRenderGroup(renderGroup);
        EXPECT_TRUE(RenderGroupUtils::ContainsRenderable(renderable, rg));
        EXPECT_TRUE(RenderGroupUtils::ContainsRenderable(renderable2, rg));
        ASSERT_EQ(2u, rg.renderables.size());
        EXPECT_EQ(renderable, rg.renderables[0].renderable);
        EXPECT_EQ(renderable2, rg.renderables[1].renderable);

        this->m_scene.removeRenderableFromRenderGroup(renderGroup, renderable);

        EXPECT_FALSE(RenderGroupUtils::ContainsRenderable(renderable, rg));
        EXPECT_TRUE(RenderGroupUtils::ContainsRenderable(renderable2, rg));
        ASSERT_EQ(1u, rg.renderables.size());
        EXPECT_EQ(renderable2, rg.renderables[0].renderable);
    }

    TYPED_TEST(AScene, canAddRenderGroupToRenderGroup)
    {
        const RenderGroupHandle renderGroupParent = this->m_scene.allocateRenderGroup();
        const RenderGroupHandle renderGroupChild = this->m_scene.allocateRenderGroup();

        const RenderGroup& rg = this->m_scene.getRenderGroup(renderGroupParent);
        EXPECT_FALSE(RenderGroupUtils::ContainsRenderGroup(renderGroupChild, rg));

        this->m_scene.addRenderGroupToRenderGroup(renderGroupParent, renderGroupChild, 3);

        EXPECT_TRUE(RenderGroupUtils::ContainsRenderGroup(renderGroupChild, rg));
        ASSERT_EQ(1u, rg.renderGroups.size());
        EXPECT_EQ(renderGroupChild, rg.renderGroups[0].renderGroup);
        EXPECT_EQ(3, rg.renderGroups[0].order);
    }

    TYPED_TEST(AScene, canRemoveRenderGroupFromRenderGroup)
    {
        const RenderGroupHandle renderGroupParent = this->m_scene.allocateRenderGroup();
        const RenderGroupHandle renderGroupChild1 = this->m_scene.allocateRenderGroup();
        const RenderGroupHandle renderGroupChild2 = this->m_scene.allocateRenderGroup();
        this->m_scene.addRenderGroupToRenderGroup(renderGroupParent, renderGroupChild1, 3u);
        this->m_scene.addRenderGroupToRenderGroup(renderGroupParent, renderGroupChild2, 4u);

        const RenderGroup& rg = this->m_scene.getRenderGroup(renderGroupParent);
        EXPECT_TRUE(RenderGroupUtils::ContainsRenderGroup(renderGroupChild1, rg));
        EXPECT_TRUE(RenderGroupUtils::ContainsRenderGroup(renderGroupChild2, rg));

        this->m_scene.removeRenderGroupFromRenderGroup(renderGroupParent, renderGroupChild1);
        EXPECT_FALSE(RenderGroupUtils::ContainsRenderGroup(renderGroupChild1, rg));
        EXPECT_TRUE(RenderGroupUtils::ContainsRenderGroup(renderGroupChild2, rg));
    }

    TYPED_TEST(AScene, doesNotContainRenderGroupWhichWasNotCreated)
    {
        EXPECT_FALSE(this->m_scene.isRenderGroupAllocated(RenderGroupHandle(1)));
    }

    TYPED_TEST(AScene, canReaddRenderableWithDifferentOrder)
    {
        const RenderGroupHandle renderGroup = this->m_scene.allocateRenderGroup();
        const RenderableHandle renderable = this->m_scene.allocateRenderable(this->m_scene.allocateNode());

        this->m_scene.addRenderableToRenderGroup(renderGroup, renderable, 3);
        this->m_scene.removeRenderableFromRenderGroup(renderGroup, renderable);

        this->m_scene.addRenderableToRenderGroup(renderGroup, renderable, 15);
        const RenderGroup& rg = this->m_scene.getRenderGroup(renderGroup);
        EXPECT_TRUE(RenderGroupUtils::ContainsRenderable(renderable, rg));
        ASSERT_EQ(1u, rg.renderables.size());
        EXPECT_EQ(renderable, rg.renderables[0].renderable);
        EXPECT_EQ(15, rg.renderables[0].order);
    }

    TYPED_TEST(AScene, canReaddRenderGroupToRenderGroupWithDifferentOrder)
    {
        const RenderGroupHandle renderGroupParent = this->m_scene.allocateRenderGroup();
        const RenderGroupHandle renderGroupChild = this->m_scene.allocateRenderGroup();

        this->m_scene.addRenderGroupToRenderGroup(renderGroupParent, renderGroupChild, 3);
        this->m_scene.removeRenderGroupFromRenderGroup(renderGroupParent, renderGroupChild);

        this->m_scene.addRenderGroupToRenderGroup(renderGroupParent, renderGroupChild, 15);
        const RenderGroup& rg = this->m_scene.getRenderGroup(renderGroupParent);
        EXPECT_TRUE(RenderGroupUtils::ContainsRenderGroup(renderGroupChild, rg));
        ASSERT_EQ(1u, rg.renderGroups.size());
        EXPECT_EQ(renderGroupChild, rg.renderGroups[0].renderGroup);
        EXPECT_EQ(15, rg.renderGroups[0].order);
    }
}
