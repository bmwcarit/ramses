//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "renderer_common_gmock_header.h"
#include "TestSceneHelper.h"
#include "RendererLib/RendererCachedScene.h"
#include "RendererLib/RendererScenes.h"
#include "RendererEventCollector.h"
#include "FrameBufferInfo.h"

namespace ramses_internal
{
    class ARendererCachedScene : public testing::Test
    {
    public:
        ARendererCachedScene()
            : rendererScenes(rendererEventCollector)
            , scene(rendererScenes.createScene(SceneInfo()))
            , sceneAllocator(scene)
            , sceneHelper(scene)
        {
        }

    protected:
        void expectOrderedRenderablesInPass(RenderPassHandle pass, std::initializer_list<RenderableHandle> renderables)
        {
            const auto& orderedRenderables = scene.getOrderedRenderablesForPass(pass);
            ASSERT_EQ(renderables.size(), orderedRenderables.size());
            size_t i = 0u;
            for (const auto r : renderables)
                EXPECT_EQ(r, orderedRenderables[i++]);
        }

        RendererEventCollector rendererEventCollector;
        RendererScenes rendererScenes;
        RendererCachedScene& scene;
        SceneAllocateHelper sceneAllocator;
        TestSceneHelper sceneHelper;
    };

    TEST_F(ARendererCachedScene, HasEmptyRenderPassCacheForEmptyScene)
    {
        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);

        const RenderingPassInfoVector& orderedPasses = scene.getSortedRenderingPasses();
        EXPECT_EQ(0u, orderedPasses.size());
    }

    TEST_F(ARendererCachedScene, CachesRenderPassAfterUpdate)
    {
        const RenderPassHandle pass = sceneHelper.createRenderPassWithCamera();

        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);

        const RenderingPassInfoVector& orderedPasses = scene.getSortedRenderingPasses();
        EXPECT_EQ(1u, orderedPasses.size());
        EXPECT_EQ(pass, orderedPasses[0].getRenderPassHandle());
    }

    TEST_F(ARendererCachedScene, HasEmptyRenderableCacheForEmptyPass)
    {
        const RenderPassHandle pass = sceneHelper.createRenderPassWithCamera();

        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);

        expectOrderedRenderablesInPass(pass, {});
    }

    TEST_F(ARendererCachedScene, HasEmptyRenderableCacheForPassWithEmptyRenderGroup)
    {
        const RenderPassHandle pass = sceneHelper.createRenderPassWithCamera();
        scene.addRenderGroupToRenderPass(pass, sceneAllocator.allocateRenderGroup(), 1);

        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);

        expectOrderedRenderablesInPass(pass, {});
    }

    TEST_F(ARendererCachedScene, CachesRenderablesPerPassAfterUpdate)
    {
        const RenderPassHandle pass = sceneHelper.createRenderPassWithCamera();
        const RenderGroupHandle group = sceneHelper.createRenderGroup(pass);
        const RenderableHandle rend1 = sceneHelper.createRenderable(group);
        const RenderableHandle rend2 = sceneHelper.createRenderable(group);

        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);

        const RenderableVector& renderables = scene.getOrderedRenderablesForPass(pass);
        EXPECT_EQ(2u, renderables.size());
        EXPECT_TRUE(contains_c(renderables, rend1));
        EXPECT_TRUE(contains_c(renderables, rend2));
    }

    TEST_F(ARendererCachedScene, CachesRenderableOfNestedRenderGroupAfterUpdate)
    {
        const RenderPassHandle pass = sceneHelper.createRenderPassWithCamera();
        const RenderGroupHandle group = sceneHelper.createRenderGroup(pass);
        const RenderGroupHandle nestedGroup = sceneAllocator.allocateRenderGroup();
        scene.addRenderGroupToRenderGroup(group, nestedGroup, 0);
        const RenderableHandle rend1 = sceneHelper.createRenderable(group);
        const RenderableHandle rend2 = sceneHelper.createRenderable(nestedGroup);

        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);

        const RenderableVector& renderables = scene.getOrderedRenderablesForPass(pass);
        EXPECT_EQ(2u, renderables.size());
        EXPECT_TRUE(contains_c(renderables, rend1));
        EXPECT_TRUE(contains_c(renderables, rend2));
    }

    TEST_F(ARendererCachedScene, CachesRenderableAddedBetweenTwoUpdates)
    {
        const RenderPassHandle pass = sceneHelper.createRenderPassWithCamera();
        const RenderGroupHandle group = sceneHelper.createRenderGroup(pass);
        sceneHelper.createRenderable(group);

        const RenderableHandle rend2 = sceneHelper.createRenderable();

        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);

        const RenderableVector& renderables = scene.getOrderedRenderablesForPass(pass);
        ASSERT_FALSE(contains_c(renderables, rend2));

        scene.addRenderableToRenderGroup(group, rend2, 0);

        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);

        EXPECT_EQ(2u, renderables.size());
        EXPECT_TRUE(contains_c(renderables, rend2));
    }

    TEST_F(ARendererCachedScene, CachesRenderableOfNestedRenderGroupAddedBetweenTwoUpdates)
    {
        const RenderPassHandle pass = sceneHelper.createRenderPassWithCamera();
        const RenderGroupHandle group = sceneHelper.createRenderGroup(pass);
        sceneHelper.createRenderable(group);

        const RenderGroupHandle nestedGroup = sceneAllocator.allocateRenderGroup();
        const RenderableHandle rend2 = sceneHelper.createRenderable(nestedGroup);

        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);

        const RenderableVector& renderables = scene.getOrderedRenderablesForPass(pass);
        ASSERT_FALSE(contains_c(renderables, rend2));

        scene.addRenderGroupToRenderGroup(group, nestedGroup, 0);

        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);

        EXPECT_EQ(2u, renderables.size());
        EXPECT_TRUE(contains_c(renderables, rend2));
    }

    TEST_F(ARendererCachedScene, CachesRenderablesPerPassPerGroupAfterUpdate)
    {
        const RenderPassHandle pass = sceneHelper.createRenderPassWithCamera();
        const RenderableHandle rend1 = sceneHelper.createRenderable();
        const RenderableHandle rend2 = sceneHelper.createRenderable();
        const RenderGroupHandle group1 = sceneAllocator.allocateRenderGroup();
        const RenderGroupHandle group2 = sceneAllocator.allocateRenderGroup();
        scene.addRenderableToRenderGroup(group1, rend1, 0);
        scene.addRenderableToRenderGroup(group2, rend2, 0);
        scene.addRenderGroupToRenderPass(pass, group1, 0);
        scene.addRenderGroupToRenderPass(pass, group2, 0);

        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);

        const RenderableVector& renderables = scene.getOrderedRenderablesForPass(pass);
        EXPECT_EQ(2u, renderables.size());
        EXPECT_TRUE(contains_c(renderables, rend1));
        EXPECT_TRUE(contains_c(renderables, rend2));
    }

    TEST_F(ARendererCachedScene, RemovesRenderableFromCacheWhenDeleted)
    {
        const RenderPassHandle pass = sceneHelper.createRenderPassWithCamera();
        const RenderGroupHandle group = sceneHelper.createRenderGroup(pass);
        const RenderableHandle rend1 = sceneHelper.createRenderable(group);
        const RenderableHandle rend2 = sceneHelper.createRenderable(group);
        const RenderableHandle rend3 = sceneHelper.createRenderable(group);

        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);

        sceneHelper.removeRenderable(rend2, group);

        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);

        const RenderableVector& renderables = scene.getOrderedRenderablesForPass(pass);
        EXPECT_EQ(2u, renderables.size());
        EXPECT_TRUE(contains_c(renderables, rend1));
        EXPECT_FALSE(contains_c(renderables, rend2));
        EXPECT_TRUE(contains_c(renderables, rend3));
    }

    TEST_F(ARendererCachedScene, RemovesRenderableFromCacheWhenDeletedAndRemovedFromRenderGroup)
    {
        const RenderPassHandle pass = sceneHelper.createRenderPassWithCamera();
        const RenderableHandle rend1 = sceneHelper.createRenderable();
        const RenderableHandle rend2 = sceneHelper.createRenderable();
        const RenderableHandle rend3 = sceneHelper.createRenderable();
        const RenderGroupHandle group = sceneAllocator.allocateRenderGroup();
        scene.addRenderableToRenderGroup(group, rend1, 0);
        scene.addRenderableToRenderGroup(group, rend2, 0);
        scene.addRenderableToRenderGroup(group, rend3, 0);
        scene.addRenderGroupToRenderPass(pass, group, 0);

        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);

        scene.removeRenderableFromRenderGroup(group, rend2);

        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);

        const RenderableVector& renderables = scene.getOrderedRenderablesForPass(pass);
        EXPECT_EQ(2u, renderables.size());
        EXPECT_TRUE(contains_c(renderables, rend1));
        EXPECT_FALSE(contains_c(renderables, rend2));
        EXPECT_TRUE(contains_c(renderables, rend3));
    }

    TEST_F(ARendererCachedScene, RemovesRenderableFromCacheWhenItsNestedRenderGroupIsRemovedFromRenderGroup)
    {
        const RenderPassHandle pass = sceneHelper.createRenderPassWithCamera();
        const RenderableHandle rend1 = sceneHelper.createRenderable();
        const RenderableHandle rend2 = sceneHelper.createRenderable();
        const RenderableHandle rend3 = sceneHelper.createRenderable();
        const RenderGroupHandle group = sceneAllocator.allocateRenderGroup();
        const RenderGroupHandle nestedGroup = sceneAllocator.allocateRenderGroup();
        scene.addRenderableToRenderGroup(group, rend1, 0);
        scene.addRenderableToRenderGroup(nestedGroup, rend2, 0);
        scene.addRenderableToRenderGroup(group, rend3, 0);
        scene.addRenderGroupToRenderGroup(group, nestedGroup, 0);
        scene.addRenderGroupToRenderPass(pass, group, 0);

        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);

        scene.removeRenderGroupFromRenderGroup(group, nestedGroup);

        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);

        const RenderableVector& renderables = scene.getOrderedRenderablesForPass(pass);
        EXPECT_EQ(2u, renderables.size());
        EXPECT_TRUE(contains_c(renderables, rend1));
        EXPECT_FALSE(contains_c(renderables, rend2));
        EXPECT_TRUE(contains_c(renderables, rend3));
    }

    TEST_F(ARendererCachedScene, RemovesRenderPassFromCacheWhenDeleted)
    {
        const RenderPassHandle pass = sceneHelper.createRenderPassWithCamera();

        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);

        scene.releaseRenderPass(pass);

        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);

        const RenderingPassInfoVector& orderedPasses = scene.getSortedRenderingPasses();
        EXPECT_EQ(0u, orderedPasses.size());
    }

    TEST_F(ARendererCachedScene, RemovesRenderablesFromCacheWhenTheirPassDeleted)
    {
        const RenderPassHandle pass = sceneHelper.createRenderPassWithCamera();
        const RenderGroupHandle group = sceneHelper.createRenderGroup(pass);
        const RenderGroupHandle nestedGroup = sceneHelper.createRenderGroup(pass);
        scene.addRenderGroupToRenderGroup(group, nestedGroup, 0);
        sceneHelper.createRenderable(group);
        sceneHelper.createRenderable(group);
        sceneHelper.createRenderable(nestedGroup);

        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);

        scene.releaseRenderPass(pass);

        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);

        expectOrderedRenderablesInPass(pass, {});
    }

    TEST_F(ARendererCachedScene, RemovesRenderableFromCacheWhenItsRenderGroupDeleted)
    {
        const RenderPassHandle pass = sceneHelper.createRenderPassWithCamera();
        const RenderableHandle rend1 = sceneHelper.createRenderable();
        const RenderableHandle rend2 = sceneHelper.createRenderable();
        const RenderableHandle rend3 = sceneHelper.createRenderable();
        const RenderGroupHandle group = sceneAllocator.allocateRenderGroup();
        scene.addRenderableToRenderGroup(group, rend1, 0);
        scene.addRenderableToRenderGroup(group, rend2, 0);
        scene.addRenderableToRenderGroup(group, rend3, 0);
        scene.addRenderGroupToRenderPass(pass, group, 0);

        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);

        scene.removeRenderGroupFromRenderPass(pass, group);
        scene.releaseRenderGroup(group);

        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);

        expectOrderedRenderablesInPass(pass, {});
    }

    TEST_F(ARendererCachedScene, CachesMultiplePassesAfterUpdate)
    {
        const RenderPassHandle pass1 = sceneHelper.createRenderPassWithCamera();
        const RenderPassHandle pass2 = sceneHelper.createRenderPassWithCamera();
        scene.setRenderPassRenderOrder(pass1, 0);
        scene.setRenderPassRenderOrder(pass2, 1);

        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);

        const RenderingPassInfoVector& orderedPasses = scene.getSortedRenderingPasses();
        EXPECT_EQ(2u, orderedPasses.size());
        EXPECT_EQ(pass1, orderedPasses[0].getRenderPassHandle());
        EXPECT_EQ(pass2, orderedPasses[1].getRenderPassHandle());
    }

    TEST_F(ARendererCachedScene, CachesRenderablesContainedInTwoPassesAfterUpdate)
    {
        const RenderPassHandle pass1 = sceneHelper.createRenderPassWithCamera();
        const RenderPassHandle pass2 = sceneHelper.createRenderPassWithCamera();
        const RenderGroupHandle group1 = sceneHelper.createRenderGroup(pass1);
        const RenderGroupHandle group2 = sceneHelper.createRenderGroup(pass2);

        const RenderableHandle rend1 = sceneHelper.createRenderable(group1, group2);
        const RenderableHandle rend2 = sceneHelper.createRenderable(group1, group2);
        const RenderableHandle rend3 = sceneHelper.createRenderable(group1);

        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);

        const RenderableVector& renderables1 = scene.getOrderedRenderablesForPass(pass1);
        EXPECT_EQ(3u, renderables1.size());
        EXPECT_TRUE(contains_c(renderables1, rend1));
        EXPECT_TRUE(contains_c(renderables1, rend2));
        EXPECT_TRUE(contains_c(renderables1, rend3));
        const RenderableVector& renderables2 = scene.getOrderedRenderablesForPass(pass2);
        EXPECT_EQ(2u, renderables2.size());
        EXPECT_TRUE(contains_c(renderables2, rend1));
        EXPECT_TRUE(contains_c(renderables2, rend2));
        EXPECT_FALSE(contains_c(renderables2, rend3));
    }

    TEST_F(ARendererCachedScene, CachesRenderablesContainedInTwoPassesAndDifferentRenderGroups)
    {
        const RenderPassHandle pass1 = sceneHelper.createRenderPassWithCamera();
        const RenderPassHandle pass2 = sceneHelper.createRenderPassWithCamera();
        const RenderableHandle rend1 = sceneHelper.createRenderable();
        const RenderableHandle rend2 = sceneHelper.createRenderable();
        const RenderableHandle rend3 = sceneHelper.createRenderable();
        const RenderGroupHandle group1 = sceneAllocator.allocateRenderGroup();
        const RenderGroupHandle group2 = sceneAllocator.allocateRenderGroup();
        scene.addRenderableToRenderGroup(group1, rend1, 0);
        scene.addRenderableToRenderGroup(group1, rend2, 0);
        scene.addRenderableToRenderGroup(group1, rend3, 0);
        scene.addRenderableToRenderGroup(group2, rend1, 0);
        scene.addRenderableToRenderGroup(group2, rend2, 0);
        scene.addRenderGroupToRenderPass(pass1, group1, 0);
        scene.addRenderGroupToRenderPass(pass2, group2, 0);

        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);

        const RenderableVector& renderables1 = scene.getOrderedRenderablesForPass(pass1);
        EXPECT_EQ(3u, renderables1.size());
        EXPECT_TRUE(contains_c(renderables1, rend1));
        EXPECT_TRUE(contains_c(renderables1, rend2));
        EXPECT_TRUE(contains_c(renderables1, rend3));
        const RenderableVector& renderables2 = scene.getOrderedRenderablesForPass(pass2);
        EXPECT_EQ(2u, renderables2.size());
        EXPECT_TRUE(contains_c(renderables2, rend1));
        EXPECT_TRUE(contains_c(renderables2, rend2));
        EXPECT_FALSE(contains_c(renderables2, rend3));
    }

    TEST_F(ARendererCachedScene, CachesRenderablesContainedInTwoPassesAndSharedRenderGroup)
    {
        const RenderPassHandle pass1 = sceneHelper.createRenderPassWithCamera();
        const RenderPassHandle pass2 = sceneHelper.createRenderPassWithCamera();
        const RenderableHandle rend1 = sceneHelper.createRenderable();
        const RenderableHandle rend2 = sceneHelper.createRenderable();
        const RenderableHandle rend3 = sceneHelper.createRenderable();
        const RenderGroupHandle group = sceneAllocator.allocateRenderGroup();
        scene.addRenderableToRenderGroup(group, rend1, 0);
        scene.addRenderableToRenderGroup(group, rend2, 0);
        scene.addRenderableToRenderGroup(group, rend3, 0);
        scene.addRenderGroupToRenderPass(pass1, group, 0);
        scene.addRenderGroupToRenderPass(pass2, group, 0);

        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);

        const RenderableVector& renderables1 = scene.getOrderedRenderablesForPass(pass1);
        EXPECT_EQ(3u, renderables1.size());
        EXPECT_TRUE(contains_c(renderables1, rend1));
        EXPECT_TRUE(contains_c(renderables1, rend2));
        EXPECT_TRUE(contains_c(renderables1, rend3));
        const RenderableVector& renderables2 = scene.getOrderedRenderablesForPass(pass2);
        EXPECT_EQ(3u, renderables2.size());
        EXPECT_TRUE(contains_c(renderables2, rend1));
        EXPECT_TRUE(contains_c(renderables2, rend2));
        EXPECT_TRUE(contains_c(renderables2, rend3));
    }

    TEST_F(ARendererCachedScene, RemovesRenderableFromCacheWhenDeletedAndContainedInTwoPasses)
    {
        const RenderPassHandle pass1 = sceneHelper.createRenderPassWithCamera();
        const RenderPassHandle pass2 = sceneHelper.createRenderPassWithCamera();
        const RenderGroupHandle group1 = sceneHelper.createRenderGroup(pass1);
        const RenderGroupHandle group2 = sceneHelper.createRenderGroup(pass2);
        const RenderableHandle rend1 = sceneHelper.createRenderable(group1, group2);
        const RenderableHandle rend2 = sceneHelper.createRenderable(group1, group2);
        const RenderableHandle rend3 = sceneHelper.createRenderable(group1);

        sceneHelper.removeRenderable(rend2, group1, group2);

        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);

        const RenderableVector& renderables1 = scene.getOrderedRenderablesForPass(pass1);
        EXPECT_EQ(2u, renderables1.size());
        EXPECT_TRUE(contains_c(renderables1, rend1));
        EXPECT_FALSE(contains_c(renderables1, rend2));
        EXPECT_TRUE(contains_c(renderables1, rend3));
        const RenderableVector& renderables2 = scene.getOrderedRenderablesForPass(pass2);
        EXPECT_EQ(1u, renderables2.size());
        EXPECT_TRUE(contains_c(renderables2, rend1));
        EXPECT_FALSE(contains_c(renderables2, rend2));
        EXPECT_FALSE(contains_c(renderables2, rend3));
    }

    TEST_F(ARendererCachedScene, RemovesRenderableFromCacheWhenDeletedAndContainedInTwoRenderGroups)
    {
        const RenderPassHandle pass1 = sceneHelper.createRenderPassWithCamera();
        const RenderPassHandle pass2 = sceneHelper.createRenderPassWithCamera();
        const RenderableHandle rend1 = sceneHelper.createRenderable();
        const RenderableHandle rend2 = sceneHelper.createRenderable();
        const RenderableHandle rend3 = sceneHelper.createRenderable();
        const RenderGroupHandle group1 = sceneAllocator.allocateRenderGroup();
        const RenderGroupHandle group2 = sceneAllocator.allocateRenderGroup();

        scene.addRenderableToRenderGroup(group1, rend1, 0);
        scene.addRenderableToRenderGroup(group1, rend2, 0);
        scene.addRenderableToRenderGroup(group1, rend3, 0);
        scene.addRenderableToRenderGroup(group2, rend1, 0);
        scene.addRenderableToRenderGroup(group2, rend2, 0);
        scene.addRenderGroupToRenderPass(pass1, group1, 0);
        scene.addRenderGroupToRenderPass(pass2, group2, 0);

        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);

        scene.removeRenderableFromRenderGroup(group1, rend2);
        scene.removeRenderableFromRenderGroup(group2, rend2);
        scene.releaseRenderable(rend2);

        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);

        const RenderableVector& renderables1 = scene.getOrderedRenderablesForPass(pass1);
        EXPECT_EQ(2u, renderables1.size());
        EXPECT_TRUE(contains_c(renderables1, rend1));
        EXPECT_FALSE(contains_c(renderables1, rend2));
        EXPECT_TRUE(contains_c(renderables1, rend3));
        const RenderableVector& renderables2 = scene.getOrderedRenderablesForPass(pass2);
        EXPECT_EQ(1u, renderables2.size());
        EXPECT_TRUE(contains_c(renderables2, rend1));
        EXPECT_FALSE(contains_c(renderables2, rend2));
        EXPECT_FALSE(contains_c(renderables2, rend3));
    }

    TEST_F(ARendererCachedScene, RemovesRenderableFromCacheWhenDeletedAndContainedInRenderGroupSharedByTwoPasses)
    {
        const RenderPassHandle pass1 = sceneHelper.createRenderPassWithCamera();
        const RenderPassHandle pass2 = sceneHelper.createRenderPassWithCamera();
        const RenderableHandle rend1 = sceneHelper.createRenderable();
        const RenderableHandle rend2 = sceneHelper.createRenderable();
        const RenderableHandle rend3 = sceneHelper.createRenderable();
        const RenderGroupHandle group = sceneAllocator.allocateRenderGroup();

        scene.addRenderableToRenderGroup(group, rend1, 0);
        scene.addRenderableToRenderGroup(group, rend2, 0);
        scene.addRenderableToRenderGroup(group, rend3, 0);
        scene.addRenderGroupToRenderPass(pass1, group, 0);
        scene.addRenderGroupToRenderPass(pass2, group, 0);

        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);

        scene.removeRenderableFromRenderGroup(group, rend2);
        scene.releaseRenderable(rend2);

        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);

        const RenderableVector& renderables1 = scene.getOrderedRenderablesForPass(pass1);
        EXPECT_EQ(2u, renderables1.size());
        EXPECT_TRUE(contains_c(renderables1, rend1));
        EXPECT_FALSE(contains_c(renderables1, rend2));
        EXPECT_TRUE(contains_c(renderables1, rend3));
        const RenderableVector& renderables2 = scene.getOrderedRenderablesForPass(pass2);
        EXPECT_EQ(2u, renderables2.size());
        EXPECT_TRUE(contains_c(renderables2, rend1));
        EXPECT_FALSE(contains_c(renderables2, rend2));
        EXPECT_TRUE(contains_c(renderables2, rend3));
    }

    TEST_F(ARendererCachedScene, RemovesRenderableFromCacheWhenSetInvisibleAndContainedInTwoPasses)
    {
        const RenderPassHandle pass1 = sceneHelper.createRenderPassWithCamera();
        const RenderPassHandle pass2 = sceneHelper.createRenderPassWithCamera();
        const RenderGroupHandle group1 = sceneHelper.createRenderGroup(pass1);
        const RenderGroupHandle group2 = sceneHelper.createRenderGroup(pass2);
        const RenderableHandle rend1 = sceneHelper.createRenderable(group1, group2);
        const RenderableHandle rend2 = sceneHelper.createRenderable(group1, group2);
        const RenderableHandle rend3 = sceneHelper.createRenderable(group1);

        scene.setRenderableVisibility(rend2, false);

        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);

        const RenderableVector& renderables1 = scene.getOrderedRenderablesForPass(pass1);
        EXPECT_EQ(2u, renderables1.size());
        EXPECT_TRUE(contains_c(renderables1, rend1));
        EXPECT_FALSE(contains_c(renderables1, rend2));
        EXPECT_TRUE(contains_c(renderables1, rend3));
        const RenderableVector& renderables2 = scene.getOrderedRenderablesForPass(pass2);
        EXPECT_EQ(1u, renderables2.size());
        EXPECT_TRUE(contains_c(renderables2, rend1));
        EXPECT_FALSE(contains_c(renderables2, rend2));
        EXPECT_FALSE(contains_c(renderables2, rend3));
    }

    TEST_F(ARendererCachedScene, RemovesRenderableFromCacheWhenSetInvisibleAndContainedInTwoRenderGroups)
    {
        const RenderPassHandle pass1 = sceneHelper.createRenderPassWithCamera();
        const RenderPassHandle pass2 = sceneHelper.createRenderPassWithCamera();
        const RenderableHandle rend1 = sceneHelper.createRenderable();
        const RenderableHandle rend2 = sceneHelper.createRenderable();
        const RenderableHandle rend3 = sceneHelper.createRenderable();
        const RenderGroupHandle group1 = sceneAllocator.allocateRenderGroup();
        const RenderGroupHandle group2 = sceneAllocator.allocateRenderGroup();

        scene.addRenderableToRenderGroup(group1, rend1, 0);
        scene.addRenderableToRenderGroup(group1, rend2, 0);
        scene.addRenderableToRenderGroup(group1, rend3, 0);
        scene.addRenderableToRenderGroup(group2, rend1, 0);
        scene.addRenderableToRenderGroup(group2, rend2, 0);
        scene.addRenderGroupToRenderPass(pass1, group1, 0);
        scene.addRenderGroupToRenderPass(pass2, group2, 0);

        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);

        scene.setRenderableVisibility(rend2, false);

        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);

        const RenderableVector& renderables1 = scene.getOrderedRenderablesForPass(pass1);
        EXPECT_EQ(2u, renderables1.size());
        EXPECT_TRUE(contains_c(renderables1, rend1));
        EXPECT_FALSE(contains_c(renderables1, rend2));
        EXPECT_TRUE(contains_c(renderables1, rend3));
        const RenderableVector& renderables2 = scene.getOrderedRenderablesForPass(pass2);
        EXPECT_EQ(1u, renderables2.size());
        EXPECT_TRUE(contains_c(renderables2, rend1));
        EXPECT_FALSE(contains_c(renderables2, rend2));
        EXPECT_FALSE(contains_c(renderables2, rend3));
    }

    TEST_F(ARendererCachedScene, RemovesRenderableFromCacheWhenSetInvisibleAndContainedInRenderGroupSharedByTwoPasses)
    {
        const RenderPassHandle pass1 = sceneHelper.createRenderPassWithCamera();
        const RenderPassHandle pass2 = sceneHelper.createRenderPassWithCamera();
        const RenderableHandle rend1 = sceneHelper.createRenderable();
        const RenderableHandle rend2 = sceneHelper.createRenderable();
        const RenderableHandle rend3 = sceneHelper.createRenderable();
        const RenderGroupHandle group = sceneAllocator.allocateRenderGroup();

        scene.addRenderableToRenderGroup(group, rend1, 0);
        scene.addRenderableToRenderGroup(group, rend2, 0);
        scene.addRenderableToRenderGroup(group, rend3, 0);
        scene.addRenderGroupToRenderPass(pass1, group, 0);
        scene.addRenderGroupToRenderPass(pass2, group, 0);

        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);

        scene.setRenderableVisibility(rend2, false);

        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);

        const RenderableVector& renderables1 = scene.getOrderedRenderablesForPass(pass1);
        EXPECT_EQ(2u, renderables1.size());
        EXPECT_TRUE(contains_c(renderables1, rend1));
        EXPECT_FALSE(contains_c(renderables1, rend2));
        EXPECT_TRUE(contains_c(renderables1, rend3));
        const RenderableVector& renderables2 = scene.getOrderedRenderablesForPass(pass2);
        EXPECT_EQ(2u, renderables2.size());
        EXPECT_TRUE(contains_c(renderables2, rend1));
        EXPECT_FALSE(contains_c(renderables2, rend2));
        EXPECT_TRUE(contains_c(renderables2, rend3));
    }

    TEST_F(ARendererCachedScene, RetrievesOrderedRenderPassesBasedOnSetRenderOrder)
    {
        const RenderPassHandle pass1 = sceneHelper.createRenderPassWithCamera();
        const RenderPassHandle pass2 = sceneHelper.createRenderPassWithCamera();
        const RenderPassHandle pass3 = sceneHelper.createRenderPassWithCamera();

        scene.setRenderPassRenderOrder(pass1, 99);
        scene.setRenderPassRenderOrder(pass2, 1);
        scene.setRenderPassRenderOrder(pass3, -1);

        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);

        const RenderingPassInfoVector& orderedPasses = scene.getSortedRenderingPasses();
        EXPECT_EQ(pass3, orderedPasses[0].getRenderPassHandle());
        EXPECT_EQ(pass2, orderedPasses[1].getRenderPassHandle());
        EXPECT_EQ(pass1, orderedPasses[2].getRenderPassHandle());
    }

    TEST_F(ARendererCachedScene, SkipDisabledRenderPass)
    {
        const RenderPassHandle pass1 = sceneHelper.createRenderPassWithCamera();
        const RenderPassHandle pass2 = sceneHelper.createRenderPassWithCamera();

        scene.setRenderPassEnabled(pass2, false);

        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);

        const RenderingPassInfoVector& orderedPasses = scene.getSortedRenderingPasses();
        ASSERT_TRUE(1u == orderedPasses.size());
        EXPECT_EQ(pass1, orderedPasses[0].getRenderPassHandle());
    }

    TEST_F(ARendererCachedScene, cachesReenabledRenderPass)
    {
        const RenderPassHandle pass1 = sceneHelper.createRenderPassWithCamera();
        const RenderPassHandle pass2 = sceneHelper.createRenderPassWithCamera();

        scene.setRenderPassEnabled(pass2, false);
        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);

        scene.setRenderPassEnabled(pass2, true);
        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);

        const RenderingPassInfoVector& orderedPasses = scene.getSortedRenderingPasses();
        ASSERT_TRUE(2u == orderedPasses.size());
        EXPECT_EQ(pass1, orderedPasses[0].getRenderPassHandle());
        EXPECT_EQ(pass2, orderedPasses[1].getRenderPassHandle());
    }

    TEST_F(ARendererCachedScene, RenderGroupsAreOrdererdWithinRenderPass)
    {
        const RenderableHandle rend1 = sceneHelper.createRenderable();
        const RenderableHandle rend2 = sceneHelper.createRenderable();
        const RenderableHandle rend3 = sceneHelper.createRenderable();
        const RenderGroupHandle group1 = sceneAllocator.allocateRenderGroup();
        const RenderGroupHandle group2 = sceneAllocator.allocateRenderGroup();
        const RenderGroupHandle group3 = sceneAllocator.allocateRenderGroup();
        const RenderPassHandle pass = sceneHelper.createRenderPassWithCamera();

        scene.addRenderableToRenderGroup(group1, rend1, 0);
        scene.addRenderableToRenderGroup(group2, rend2, 0);
        scene.addRenderableToRenderGroup(group3, rend3, 0);
        scene.addRenderGroupToRenderPass(pass, group1, 1);
        scene.addRenderGroupToRenderPass(pass, group2, 5);
        scene.addRenderGroupToRenderPass(pass, group3, -3);

        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);

        expectOrderedRenderablesInPass(pass, { rend3, rend1, rend2 });
    }

    TEST_F(ARendererCachedScene, RenderablesInRenderGroupAreOrdered)
    {
        const RenderableHandle rend1 = sceneHelper.createRenderable();
        const RenderableHandle rend2 = sceneHelper.createRenderable();
        const RenderableHandle rend3 = sceneHelper.createRenderable();
        const RenderGroupHandle group = sceneAllocator.allocateRenderGroup();
        const RenderPassHandle pass = sceneHelper.createRenderPassWithCamera();

        scene.addRenderableToRenderGroup(group, rend1, 1);
        scene.addRenderableToRenderGroup(group, rend2, 5);
        scene.addRenderableToRenderGroup(group, rend3, -3);
        scene.addRenderGroupToRenderPass(pass, group, 0);

        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);

        expectOrderedRenderablesInPass(pass, { rend3, rend1, rend2 });
    }

    TEST_F(ARendererCachedScene, SameRenderableHasDifferentOrderInDifferentGroupAndPass)
    {
        const RenderableHandle rend1 = sceneHelper.createRenderable();
        const RenderableHandle rend2 = sceneHelper.createRenderable();
        const RenderableHandle rend3 = sceneHelper.createRenderable();
        const RenderGroupHandle group1 = sceneAllocator.allocateRenderGroup();
        const RenderGroupHandle group2 = sceneAllocator.allocateRenderGroup();
        const RenderPassHandle pass1 = sceneHelper.createRenderPassWithCamera();
        const RenderPassHandle pass2 = sceneHelper.createRenderPassWithCamera();

        scene.addRenderableToRenderGroup(group1, rend1, 1);
        scene.addRenderableToRenderGroup(group1, rend2, 5);
        scene.addRenderableToRenderGroup(group1, rend3, -3);
        scene.addRenderableToRenderGroup(group2, rend1, -99);
        scene.addRenderableToRenderGroup(group2, rend2, 500);
        scene.addRenderableToRenderGroup(group2, rend3, 15);
        scene.addRenderGroupToRenderPass(pass1, group1, 0);
        scene.addRenderGroupToRenderPass(pass2, group2, 0);

        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);

        expectOrderedRenderablesInPass(pass1, { rend3, rend1, rend2 });
        expectOrderedRenderablesInPass(pass2, { rend1, rend3, rend2 });
    }

    TEST_F(ARendererCachedScene, RenderableOrderCanBeChangedWhenReaddingToGroupAfterGroupAlreadyInPass)
    {
        const RenderableHandle rend1 = sceneHelper.createRenderable();
        const RenderableHandle rend2 = sceneHelper.createRenderable();
        const RenderableHandle rend3 = sceneHelper.createRenderable();
        const RenderGroupHandle group1 = sceneAllocator.allocateRenderGroup();
        const RenderGroupHandle group2 = sceneAllocator.allocateRenderGroup();
        const RenderPassHandle pass1 = sceneHelper.createRenderPassWithCamera();
        const RenderPassHandle pass2 = sceneHelper.createRenderPassWithCamera();

        scene.addRenderableToRenderGroup(group1, rend1, 1);
        scene.addRenderableToRenderGroup(group1, rend2, 5);
        scene.addRenderableToRenderGroup(group1, rend3, -3);
        scene.addRenderableToRenderGroup(group2, rend1, -99);
        scene.addRenderableToRenderGroup(group2, rend2, 500);
        scene.addRenderableToRenderGroup(group2, rend3, 15);
        scene.addRenderGroupToRenderPass(pass1, group1, 0);
        scene.addRenderGroupToRenderPass(pass2, group2, 0);

        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);

        scene.removeRenderableFromRenderGroup(group2, rend2);
        scene.addRenderableToRenderGroup(group2, rend2, -500);

        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);

        expectOrderedRenderablesInPass(pass1, { rend3, rend1, rend2 });
        expectOrderedRenderablesInPass(pass2, { rend2, rend1, rend3 });
    }

    TEST_F(ARendererCachedScene, RenderGroupOrderCanBeChangedWhenReaddingToPass)
    {
        const RenderableHandle rend1 = sceneHelper.createRenderable();
        const RenderableHandle rend2 = sceneHelper.createRenderable();
        const RenderableHandle rend3 = sceneHelper.createRenderable();
        const RenderGroupHandle group1 = sceneAllocator.allocateRenderGroup();
        const RenderGroupHandle group2 = sceneAllocator.allocateRenderGroup();
        const RenderPassHandle pass = sceneHelper.createRenderPassWithCamera();

        scene.addRenderableToRenderGroup(group1, rend1, 0);
        scene.addRenderableToRenderGroup(group2, rend2, 0);
        scene.addRenderableToRenderGroup(group2, rend3, 0);
        scene.addRenderGroupToRenderPass(pass, group1, 1);
        scene.addRenderGroupToRenderPass(pass, group2, 2);

        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);

        scene.removeRenderGroupFromRenderPass(pass, group2);
        scene.addRenderGroupToRenderPass(pass, group2, -2);

        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);

        expectOrderedRenderablesInPass(pass, { rend2, rend3, rend1 });
    }

    TEST_F(ARendererCachedScene, confidenceTest_RenderGroupOrderIsMovedFromOnePassToAnotherWithDifferentOrder)
    {
        const RenderableHandle rend1 = sceneHelper.createRenderable();
        const RenderableHandle rend2 = sceneHelper.createRenderable();
        const RenderableHandle rend3 = sceneHelper.createRenderable();
        const RenderGroupHandle group1 = sceneAllocator.allocateRenderGroup();
        const RenderGroupHandle group2 = sceneAllocator.allocateRenderGroup();
        const RenderPassHandle pass1 = sceneHelper.createRenderPassWithCamera();
        const RenderPassHandle pass2 = sceneHelper.createRenderPassWithCamera();

        scene.addRenderableToRenderGroup(group1, rend1, 1);
        scene.addRenderableToRenderGroup(group2, rend2, 2);
        scene.addRenderableToRenderGroup(group2, rend3, 3);
        scene.addRenderGroupToRenderPass(pass1, group1, 1);
        scene.addRenderGroupToRenderPass(pass2, group2, 2);

        scene.setRenderPassRenderOrder(pass1, 2);
        scene.setRenderPassRenderOrder(pass2, 1);

        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);

        scene.removeRenderGroupFromRenderPass(pass2, group2);
        scene.addRenderGroupToRenderPass(pass1, group2, -2);

        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);

        expectOrderedRenderablesInPass(pass1, { rend2, rend3, rend1 });
        expectOrderedRenderablesInPass(pass2, {});
    }

    TEST_F(ARendererCachedScene, ordersRenderablesWithSameEffectTogether)
    {
        const RenderPassHandle pass = sceneHelper.createRenderPassWithCamera();
        const RenderGroupHandle group = sceneHelper.createRenderGroup(pass);

        const RenderableHandle rend1 = sceneHelper.createRenderable(group);
        const RenderableHandle rend2 = sceneHelper.createRenderable(group);
        const RenderableHandle rend3 = sceneHelper.createRenderable(group);
        const RenderableHandle rend4 = sceneHelper.createRenderable(group);
        const RenderableHandle rend5 = sceneHelper.createRenderable(group);
        const RenderableHandle rend6 = sceneHelper.createRenderable(group);

        const ResourceContentHash effect1{ 1, 0 };
        const ResourceContentHash effect2{ 2, 0 };
        const ResourceContentHash effect3{ 3, 0 };
        ON_CALL(sceneHelper.resourceManager, getClientResourceDeviceHandle(effect1)).WillByDefault(Return(DeviceResourceHandle(1)));
        ON_CALL(sceneHelper.resourceManager, getClientResourceDeviceHandle(effect2)).WillByDefault(Return(DeviceResourceHandle(2)));
        ON_CALL(sceneHelper.resourceManager, getClientResourceDeviceHandle(effect3)).WillByDefault(Return(DeviceResourceHandle(3)));

        scene.setRenderableEffect(rend1, effect3);
        scene.setRenderableEffect(rend2, effect1);
        scene.setRenderableEffect(rend3, effect2);
        scene.setRenderableEffect(rend4, effect3);
        scene.setRenderableEffect(rend5, effect1);
        scene.setRenderableEffect(rend6, effect2);

        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);

        expectOrderedRenderablesInPass(pass, { rend2, rend5, rend3, rend6, rend1, rend4 });
    }

    TEST_F(ARendererCachedScene, explicitOrderingOverridesSameEffectGrouping)
    {
        const RenderPassHandle pass = sceneHelper.createRenderPassWithCamera();
        const RenderGroupHandle group = sceneHelper.createRenderGroup(pass);

        const RenderableHandle rend1 = sceneHelper.createRenderable();
        const RenderableHandle rend2 = sceneHelper.createRenderable(group);
        const RenderableHandle rend3 = sceneHelper.createRenderable();
        const RenderableHandle rend4 = sceneHelper.createRenderable(group);
        const RenderableHandle rend5 = sceneHelper.createRenderable(group);
        const RenderableHandle rend6 = sceneHelper.createRenderable(group);
        scene.addRenderableToRenderGroup(group, rend1, -1);
        scene.addRenderableToRenderGroup(group, rend3, -1);

        const ResourceContentHash effect1{ 1, 0 };
        const ResourceContentHash effect2{ 2, 0 };
        const ResourceContentHash effect3{ 3, 0 };
        ON_CALL(sceneHelper.resourceManager, getClientResourceDeviceHandle(effect1)).WillByDefault(Return(DeviceResourceHandle(1)));
        ON_CALL(sceneHelper.resourceManager, getClientResourceDeviceHandle(effect2)).WillByDefault(Return(DeviceResourceHandle(2)));
        ON_CALL(sceneHelper.resourceManager, getClientResourceDeviceHandle(effect3)).WillByDefault(Return(DeviceResourceHandle(3)));

        scene.setRenderableEffect(rend1, effect3);
        scene.setRenderableEffect(rend2, effect1);
        scene.setRenderableEffect(rend3, effect2);
        scene.setRenderableEffect(rend4, effect3);
        scene.setRenderableEffect(rend5, effect1);
        scene.setRenderableEffect(rend6, effect2);

        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);

        expectOrderedRenderablesInPass(pass, { rend3, rend1, rend2, rend5, rend6, rend4 });
    }

    TEST_F(ARendererCachedScene, renderablesFromDifferentGroupsWillNotBeOrderedTogetherEvenIfHaveSameEffect)
    {
        const RenderPassHandle pass = sceneHelper.createRenderPassWithCamera();
        const RenderGroupHandle group1= sceneHelper.createRenderGroup(pass);
        const RenderGroupHandle group2 = sceneHelper.createRenderGroup(pass);

        const RenderableHandle rend1 = sceneHelper.createRenderable(group1);
        const RenderableHandle rend2 = sceneHelper.createRenderable(group1);
        const RenderableHandle rend3 = sceneHelper.createRenderable(group2);

        const ResourceContentHash effect1{ 1, 0 };
        const ResourceContentHash effect2{ 2, 0 };
        ON_CALL(sceneHelper.resourceManager, getClientResourceDeviceHandle(effect1)).WillByDefault(Return(DeviceResourceHandle(1)));
        ON_CALL(sceneHelper.resourceManager, getClientResourceDeviceHandle(effect2)).WillByDefault(Return(DeviceResourceHandle(2)));

        // rend1 and rend3 have same effect
        scene.setRenderableEffect(rend1, effect1);
        scene.setRenderableEffect(rend2, effect2);
        scene.setRenderableEffect(rend3, effect1);

        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);

        // but are not grouped together because they are from different groups
        expectOrderedRenderablesInPass(pass, { rend1, rend2, rend3 });
    }

    TEST_F(ARendererCachedScene, ordersRenderablesByEffectAndThenByGeometryIntance)
    {
        const RenderPassHandle pass = sceneHelper.createRenderPassWithCamera();
        const RenderGroupHandle group = sceneHelper.createRenderGroup(pass);

        const RenderableHandle rend1 = sceneHelper.createRenderable(group);
        const RenderableHandle rend2 = sceneHelper.createRenderable(group);
        const RenderableHandle rend3 = sceneHelper.createRenderable(group);
        const RenderableHandle rend4 = sceneHelper.createRenderable(group);
        const RenderableHandle rend5 = sceneHelper.createRenderable(group);
        const RenderableHandle rend6 = sceneHelper.createRenderable(group);

        const ResourceContentHash effect1{ 1, 0 };
        const ResourceContentHash effect2{ 2, 0 };
        ON_CALL(sceneHelper.resourceManager, getClientResourceDeviceHandle(effect1)).WillByDefault(Return(DeviceResourceHandle(1)));
        ON_CALL(sceneHelper.resourceManager, getClientResourceDeviceHandle(effect2)).WillByDefault(Return(DeviceResourceHandle(2)));

        scene.setRenderableEffect(rend1, effect2);
        scene.setRenderableEffect(rend2, effect1);
        scene.setRenderableEffect(rend3, effect2);
        scene.setRenderableEffect(rend4, effect1);
        scene.setRenderableEffect(rend5, effect2);
        scene.setRenderableEffect(rend6, effect2);

        // assign different geometries
        const DataInstanceHandle geometry1 = sceneAllocator.allocateDataInstance(sceneHelper.testGeometryLayout);
        const DataInstanceHandle geometry2 = sceneAllocator.allocateDataInstance(sceneHelper.testGeometryLayout);
        // within effect1
        scene.setRenderableDataInstance(rend4, ERenderableDataSlotType_Geometry, geometry1);
        scene.setRenderableDataInstance(rend2, ERenderableDataSlotType_Geometry, geometry2);
        // within effect2
        scene.setRenderableDataInstance(rend5, ERenderableDataSlotType_Geometry, geometry1);
        scene.setRenderableDataInstance(rend6, ERenderableDataSlotType_Geometry, geometry1);
        scene.setRenderableDataInstance(rend1, ERenderableDataSlotType_Geometry, geometry2);
        scene.setRenderableDataInstance(rend3, ERenderableDataSlotType_Geometry, geometry2);

        // group by effect and geometry within effect groups
        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);
        expectOrderedRenderablesInPass(pass, { rend4, rend2, rend5, rend6, rend1, rend3 });
    }

    TEST_F(ARendererCachedScene, updatesWorldMatrixCacheForRenderable)
    {
        const RenderPassHandle pass = sceneHelper.createRenderPassWithCamera();
        const RenderGroupHandle group = sceneHelper.createRenderGroup(pass);
        const RenderableHandle rend = sceneHelper.createRenderable(group);
        const NodeHandle rendNode = scene.getRenderable(rend).node;

        const NodeHandle transformNode = sceneAllocator.allocateNode();
        const TransformHandle transform = sceneAllocator.allocateTransform(transformNode);

        scene.addChildToNode(transformNode, rendNode);
        scene.setTranslation(transform, Vector3(1, 2, 3));

        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);
        scene.updateRenderableWorldMatrices();

        const Matrix44f expectedWorldMatrix = scene.updateMatrixCache(ETransformationMatrixType_World, rendNode);
        const Matrix44f cachedWorldMatrix = scene.getRenderableWorldMatrix(rend);
        EXPECT_EQ(expectedWorldMatrix, cachedWorldMatrix);
    }

    TEST_F(ARendererCachedScene, updatesWorldMatrixCacheForRenderable_LinksVersionEquivalentToRegularVersionIfNoTransformationLinksInvolved)
    {
        const RenderPassHandle pass = sceneHelper.createRenderPassWithCamera();
        const RenderGroupHandle group = sceneHelper.createRenderGroup(pass);
        const RenderableHandle rend = sceneHelper.createRenderable(group);
        const NodeHandle rendNode = scene.getRenderable(rend).node;

        const NodeHandle transformNode = sceneAllocator.allocateNode();
        const TransformHandle transform = sceneAllocator.allocateTransform(transformNode);

        scene.addChildToNode(transformNode, rendNode);
        scene.setTranslation(transform, Vector3(1, 2, 3));

        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);
        scene.updateRenderableWorldMatricesWithLinks();

        const Matrix44f expectedWorldMatrix = scene.updateMatrixCache(ETransformationMatrixType_World, rendNode);
        const Matrix44f cachedWorldMatrix = scene.getRenderableWorldMatrix(rend);
        EXPECT_EQ(expectedWorldMatrix, cachedWorldMatrix);
    }

    TEST_F(ARendererCachedScene, CanSortPassesWithRenderOrder_RenderPasses)
    {
        const RenderPassHandle pass1 = sceneHelper.createRenderPassWithCamera();
        const RenderPassHandle pass2 = sceneHelper.createRenderPassWithCamera();
        const RenderPassHandle pass3 = sceneHelper.createRenderPassWithCamera();

        scene.setRenderPassRenderOrder(pass1, 0);
        scene.setRenderPassRenderOrder(pass2, 1);
        scene.setRenderPassRenderOrder(pass3, 2);

        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);

        const RenderingPassInfoVector& sortedPasses = scene.getSortedRenderingPasses();
        ASSERT_EQ(3u, sortedPasses.size());
        EXPECT_EQ(ERenderingPassType::RenderPass, sortedPasses[0].getType());
        EXPECT_EQ(ERenderingPassType::RenderPass, sortedPasses[1].getType());
        EXPECT_EQ(ERenderingPassType::RenderPass, sortedPasses[2].getType());
        EXPECT_EQ(pass1, sortedPasses[0].getRenderPassHandle());
        EXPECT_EQ(pass2, sortedPasses[1].getRenderPassHandle());
        EXPECT_EQ(pass3, sortedPasses[2].getRenderPassHandle());
    }

    TEST_F(ARendererCachedScene, CanSortPassesWithRenderOrder_BlitPasses)
    {
        const BlitPassHandle pass1 = sceneHelper.createBlitPassWithDummyRenderBuffers();
        const BlitPassHandle pass2 = sceneHelper.createBlitPassWithDummyRenderBuffers();
        const BlitPassHandle pass3 = sceneHelper.createBlitPassWithDummyRenderBuffers();

        scene.setBlitPassRenderOrder(pass1, 0);
        scene.setBlitPassRenderOrder(pass2, 1);
        scene.setBlitPassRenderOrder(pass3, 2);

        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);

        const RenderingPassInfoVector& sortedPasses = scene.getSortedRenderingPasses();
        ASSERT_EQ(3u, sortedPasses.size());
        EXPECT_EQ(ERenderingPassType::BlitPass, sortedPasses[0].getType());
        EXPECT_EQ(ERenderingPassType::BlitPass, sortedPasses[1].getType());
        EXPECT_EQ(ERenderingPassType::BlitPass, sortedPasses[2].getType());
        EXPECT_EQ(pass1, sortedPasses[0].getBlitPassHandle());
        EXPECT_EQ(pass2, sortedPasses[1].getBlitPassHandle());
        EXPECT_EQ(pass3, sortedPasses[2].getBlitPassHandle());
    }

    TEST_F(ARendererCachedScene, CanSortPassesWithRenderOrder_RenderAndBlitPassesTogether)
    {
        const BlitPassHandle blitPass1 = sceneHelper.createBlitPassWithDummyRenderBuffers();
        const RenderPassHandle renderPass = sceneHelper.createRenderPassWithCamera();
        const BlitPassHandle blitPass2 = sceneHelper.createBlitPassWithDummyRenderBuffers();

        scene.setBlitPassRenderOrder(blitPass1, 0);
        scene.setRenderPassRenderOrder(renderPass, 1);
        scene.setBlitPassRenderOrder(blitPass2, 2);

        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);

        const RenderingPassInfoVector& sortedPasses = scene.getSortedRenderingPasses();
        ASSERT_EQ(3u, sortedPasses.size());
        EXPECT_EQ(ERenderingPassType::BlitPass, sortedPasses[0].getType());
        EXPECT_EQ(ERenderingPassType::RenderPass, sortedPasses[1].getType());
        EXPECT_EQ(ERenderingPassType::BlitPass, sortedPasses[2].getType());
        EXPECT_EQ(blitPass1, sortedPasses[0].getBlitPassHandle());
        EXPECT_EQ(renderPass, sortedPasses[1].getRenderPassHandle());
        EXPECT_EQ(blitPass2, sortedPasses[2].getBlitPassHandle());
    }

    TEST_F(ARendererCachedScene, UpdatesRenderOrderIfNewBlitPassesAreAllocated)
    {
        const RenderPassHandle renderPass = sceneHelper.createRenderPassWithCamera();
        scene.setRenderPassRenderOrder(renderPass, 1);

        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);

        //allocate blit passes and update cache
        const BlitPassHandle blitPass1 = sceneHelper.createBlitPassWithDummyRenderBuffers();
        const BlitPassHandle blitPass2 = sceneHelper.createBlitPassWithDummyRenderBuffers();
        scene.setBlitPassRenderOrder(blitPass1, 0);
        scene.setBlitPassRenderOrder(blitPass2, 2);

        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);

        const RenderingPassInfoVector& sortedPasses = scene.getSortedRenderingPasses();
        ASSERT_EQ(3u, sortedPasses.size());
        EXPECT_EQ(ERenderingPassType::BlitPass, sortedPasses[0].getType());
        EXPECT_EQ(ERenderingPassType::RenderPass, sortedPasses[1].getType());
        EXPECT_EQ(ERenderingPassType::BlitPass, sortedPasses[2].getType());
        EXPECT_EQ(blitPass1, sortedPasses[0].getBlitPassHandle());
        EXPECT_EQ(renderPass, sortedPasses[1].getRenderPassHandle());
        EXPECT_EQ(blitPass2, sortedPasses[2].getBlitPassHandle());
    }

    TEST_F(ARendererCachedScene, UpdatesRenderOrderIfBlitPassesAreReleased)
    {
        const BlitPassHandle blitPass1 = sceneHelper.createBlitPassWithDummyRenderBuffers();
        const RenderPassHandle renderPass = sceneHelper.createRenderPassWithCamera();
        const BlitPassHandle blitPass2 = sceneHelper.createBlitPassWithDummyRenderBuffers();

        scene.setBlitPassRenderOrder(blitPass1, 0);
        scene.setRenderPassRenderOrder(renderPass, 1);
        scene.setBlitPassRenderOrder(blitPass2, 2);
        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);

        scene.releaseBlitPass(blitPass2);
        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);

        const RenderingPassInfoVector& sortedPasses = scene.getSortedRenderingPasses();
        ASSERT_EQ(2u, sortedPasses.size());
        EXPECT_EQ(ERenderingPassType::BlitPass, sortedPasses[0].getType());
        EXPECT_EQ(ERenderingPassType::RenderPass, sortedPasses[1].getType());
        EXPECT_EQ(blitPass1, sortedPasses[0].getBlitPassHandle());
        EXPECT_EQ(renderPass, sortedPasses[1].getRenderPassHandle());
    }

    TEST_F(ARendererCachedScene, UpdatesRenderOrderIfBlitPassesRenderOrderIsUpdated)
    {
        const BlitPassHandle blitPass1 = sceneHelper.createBlitPassWithDummyRenderBuffers();
        const RenderPassHandle renderPass = sceneHelper.createRenderPassWithCamera();
        const BlitPassHandle blitPass2 = sceneHelper.createBlitPassWithDummyRenderBuffers();

        scene.setBlitPassRenderOrder(blitPass1, 0);
        scene.setRenderPassRenderOrder(renderPass, 1);
        scene.setBlitPassRenderOrder(blitPass2, 2);
        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);

        scene.setBlitPassRenderOrder(blitPass2, -200);
        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);

        const RenderingPassInfoVector& sortedPasses = scene.getSortedRenderingPasses();
        ASSERT_EQ(3u, sortedPasses.size());
        EXPECT_EQ(ERenderingPassType::BlitPass, sortedPasses[0].getType());
        EXPECT_EQ(ERenderingPassType::BlitPass, sortedPasses[1].getType());
        EXPECT_EQ(ERenderingPassType::RenderPass, sortedPasses[2].getType());
        EXPECT_EQ(blitPass2, sortedPasses[0].getBlitPassHandle());
        EXPECT_EQ(blitPass1, sortedPasses[1].getBlitPassHandle());
        EXPECT_EQ(renderPass, sortedPasses[2].getRenderPassHandle());
    }

    TEST_F(ARendererCachedScene, UpdatesRenderOrderIfBlitPassIsEnabledOrDisabled)
    {
        const BlitPassHandle blitPass1 = sceneHelper.createBlitPassWithDummyRenderBuffers();
        const RenderPassHandle renderPass = sceneHelper.createRenderPassWithCamera();
        const BlitPassHandle blitPass2 = sceneHelper.createBlitPassWithDummyRenderBuffers();

        scene.setBlitPassRenderOrder(blitPass1, 0);
        scene.setRenderPassRenderOrder(renderPass, 1);
        scene.setBlitPassRenderOrder(blitPass2, 2);
        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);

        //disable blit pass
        scene.setBlitPassEnabled(blitPass2, false);
        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);

        const RenderingPassInfoVector& sortedPasses = scene.getSortedRenderingPasses();
        ASSERT_EQ(2u, sortedPasses.size());
        EXPECT_EQ(ERenderingPassType::BlitPass, sortedPasses[0].getType());
        EXPECT_EQ(ERenderingPassType::RenderPass, sortedPasses[1].getType());
        EXPECT_EQ(blitPass1, sortedPasses[0].getBlitPassHandle());
        EXPECT_EQ(renderPass, sortedPasses[1].getRenderPassHandle());

        //re-enable blit pass
        scene.setBlitPassEnabled(blitPass2, true);
        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);

        const RenderingPassInfoVector& resortedPasses = scene.getSortedRenderingPasses();
        ASSERT_EQ(3u, resortedPasses.size());
        EXPECT_EQ(ERenderingPassType::BlitPass, sortedPasses[2].getType());
        EXPECT_EQ(blitPass2, sortedPasses[2].getBlitPassHandle());
    }

    TEST_F(ARendererCachedScene, doesNotCacheRenderPassWithoutCamera)
    {
        sceneAllocator.allocateRenderPass();

        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);

        const RenderingPassInfoVector& orderedPasses = scene.getSortedRenderingPasses();
        EXPECT_TRUE(orderedPasses.empty());
    }

    TEST_F(ARendererCachedScene, cachesRenderOncePassTillMarkedAsRendered)
    {
        const RenderPassHandle pass = sceneHelper.createRenderPassWithCamera();
        scene.setRenderPassRenderOnce(pass, true);

        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);
        const RenderingPassInfoVector& orderedPasses = scene.getSortedRenderingPasses();
        EXPECT_EQ(1u, orderedPasses.size());
        EXPECT_EQ(pass, orderedPasses[0].getRenderPassHandle());
        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);
        EXPECT_EQ(1u, orderedPasses.size());
        EXPECT_EQ(pass, orderedPasses[0].getRenderPassHandle());

        scene.markAllRenderOncePassesAsRendered();
        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);
        EXPECT_TRUE(orderedPasses.empty());
    }

    TEST_F(ARendererCachedScene, doesNotCacheDisabledRenderOncePass)
    {
        const RenderPassHandle pass = sceneHelper.createRenderPassWithCamera();

        scene.setRenderPassRenderOnce(pass, true);
        scene.setRenderPassEnabled(pass, false);

        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);
        const RenderingPassInfoVector& orderedPasses = scene.getSortedRenderingPasses();
        EXPECT_TRUE(orderedPasses.empty());

        scene.setRenderPassEnabled(pass, false);
        scene.setRenderPassRenderOnce(pass, true);

        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);
        EXPECT_TRUE(orderedPasses.empty());
    }

    TEST_F(ARendererCachedScene, cachesReenabledRenderOncePassTillMarkedAsRendered)
    {
        const RenderPassHandle pass = sceneHelper.createRenderPassWithCamera();
        scene.setRenderPassEnabled(pass, false);
        scene.setRenderPassRenderOnce(pass, true);

        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);
        const RenderingPassInfoVector& orderedPasses = scene.getSortedRenderingPasses();
        EXPECT_TRUE(orderedPasses.empty());

        scene.setRenderPassEnabled(pass, true);

        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);
        EXPECT_EQ(1u, orderedPasses.size());
        EXPECT_EQ(pass, orderedPasses[0].getRenderPassHandle());
        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);
        EXPECT_EQ(1u, orderedPasses.size());
        EXPECT_EQ(pass, orderedPasses[0].getRenderPassHandle());

        scene.markAllRenderOncePassesAsRendered();
        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);
        EXPECT_TRUE(orderedPasses.empty());
    }

    TEST_F(ARendererCachedScene, cachesRetriggeredRenderOncePass)
    {
        const RenderPassHandle pass = sceneHelper.createRenderPassWithCamera();
        scene.setRenderPassRenderOnce(pass, true);

        // render and mark as rendered
        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);
        const RenderingPassInfoVector& orderedPasses = scene.getSortedRenderingPasses();
        EXPECT_EQ(1u, orderedPasses.size());
        EXPECT_EQ(pass, orderedPasses[0].getRenderPassHandle());
        scene.markAllRenderOncePassesAsRendered();

        // expect not in cached list
        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);
        EXPECT_TRUE(orderedPasses.empty());

        // retrigger
        scene.retriggerRenderPassRenderOnce(pass);

        // expect in cached list
        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);
        EXPECT_EQ(1u, orderedPasses.size());
        EXPECT_EQ(pass, orderedPasses[0].getRenderPassHandle());
        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);
        EXPECT_EQ(1u, orderedPasses.size());
        EXPECT_EQ(pass, orderedPasses[0].getRenderPassHandle());

        // till marked as rendered
        scene.markAllRenderOncePassesAsRendered();
        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);
        EXPECT_TRUE(orderedPasses.empty());
    }

    TEST_F(ARendererCachedScene, cachesAllRetriggeredRenderOncePasses)
    {
        const RenderPassHandle pass1 = sceneHelper.createRenderPassWithCamera();
        const RenderPassHandle pass2 = sceneHelper.createRenderPassWithCamera();
        scene.setRenderPassRenderOnce(pass1, true);
        scene.setRenderPassRenderOnce(pass2, true);

        // render and mark as rendered
        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);
        const RenderingPassInfoVector& orderedPasses = scene.getSortedRenderingPasses();
        EXPECT_EQ(2u, orderedPasses.size());
        EXPECT_EQ(pass1, orderedPasses[0].getRenderPassHandle());
        EXPECT_EQ(pass2, orderedPasses[1].getRenderPassHandle());
        scene.markAllRenderOncePassesAsRendered();

        // expect not in cached list
        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);
        EXPECT_TRUE(orderedPasses.empty());

        // retrigger all
        scene.retriggerAllRenderOncePasses();

        // expect in cached list
        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);
        EXPECT_EQ(2u, orderedPasses.size());
        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);
        EXPECT_EQ(2u, orderedPasses.size());

        // till marked as rendered
        scene.markAllRenderOncePassesAsRendered();
        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);
        EXPECT_TRUE(orderedPasses.empty());
    }

    TEST_F(ARendererCachedScene, confidenceTest_enabledRenderPassOnceIsCachedTillMarkedAsRendered_regardlessOfPreviousStates)
    {
        const RenderPassHandle pass = sceneHelper.createRenderPassWithCamera();

        // simulate many state changes with no render in between
        scene.setRenderPassRenderOnce(pass, true);
        scene.setRenderPassEnabled(pass, false);
        scene.retriggerRenderPassRenderOnce(pass);

        scene.setRenderPassRenderOnce(pass, false);
        scene.setRenderPassEnabled(pass, true);

        scene.setRenderPassRenderOnce(pass, true);
        scene.setRenderPassEnabled(pass, false);
        scene.retriggerRenderPassRenderOnce(pass);

        scene.setRenderPassEnabled(pass, true);
        scene.retriggerRenderPassRenderOnce(pass);

        // now render and expect render only till marked as really rendered - ie. render 'once'
        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);
        const RenderingPassInfoVector& orderedPasses = scene.getSortedRenderingPasses();
        EXPECT_EQ(1u, orderedPasses.size());
        EXPECT_EQ(pass, orderedPasses[0].getRenderPassHandle());
        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);
        EXPECT_EQ(1u, orderedPasses.size());
        EXPECT_EQ(pass, orderedPasses[0].getRenderPassHandle());

        scene.markAllRenderOncePassesAsRendered();
        scene.updateRenderablesAndResourceCache(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);
        EXPECT_TRUE(orderedPasses.empty());
    }
}
