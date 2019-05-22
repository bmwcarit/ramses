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
    TYPED_TEST_CASE(AScene, SceneTypes);

    TYPED_TEST(AScene, PreallocatesMemoryPoolsBasedOnSizeInformation)
    {
        const SceneSizeInformation sizeInfo(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18);
        const SceneInfo sceneInfo;
        TypeParam preallocatedScene(sceneInfo);

        preallocatedScene.preallocateSceneSize(sizeInfo);

        EXPECT_EQ(sizeInfo, preallocatedScene.getSceneSizeInformation());
        EXPECT_EQ(sizeInfo.nodeCount, preallocatedScene.getNodeCount());
        EXPECT_EQ(sizeInfo.cameraCount, preallocatedScene.getCameraCount());
        EXPECT_EQ(sizeInfo.transformCount, preallocatedScene.getTransformCount());
        EXPECT_EQ(sizeInfo.renderableCount, preallocatedScene.getRenderableCount());
        EXPECT_EQ(sizeInfo.renderStateCount, preallocatedScene.getRenderStateCount());
        EXPECT_EQ(sizeInfo.datalayoutCount, preallocatedScene.getDataLayoutCount());
        EXPECT_EQ(sizeInfo.datainstanceCount, preallocatedScene.getDataInstanceCount());
        EXPECT_EQ(sizeInfo.renderGroupCount, preallocatedScene.getRenderGroupCount());
        EXPECT_EQ(sizeInfo.renderPassCount, preallocatedScene.getRenderPassCount());
        EXPECT_EQ(sizeInfo.blitPassCount, preallocatedScene.getBlitPassCount());
        EXPECT_EQ(sizeInfo.renderTargetCount, preallocatedScene.getRenderTargetCount());
        EXPECT_EQ(sizeInfo.renderBufferCount, preallocatedScene.getRenderBufferCount());
        EXPECT_EQ(sizeInfo.textureSamplerCount, preallocatedScene.getTextureSamplerCount());
        EXPECT_EQ(sizeInfo.streamTextureCount, preallocatedScene.getStreamTextureCount());
        EXPECT_EQ(sizeInfo.dataSlotCount, preallocatedScene.getDataSlotCount());
        EXPECT_EQ(sizeInfo.dataBufferCount, preallocatedScene.getDataBufferCount());
        EXPECT_EQ(sizeInfo.animationSystemCount, preallocatedScene.getAnimationSystemCount());
    }

    TYPED_TEST(AScene, MemoryPoolSizesInUseStayZeroUponCreation)
    {
        const SceneInfo sceneInfo;
        const SceneSizeInformation zeroSizeInfo;
        TypeParam preallocatedScene(sceneInfo);
        EXPECT_EQ(zeroSizeInfo, preallocatedScene.getSceneSizeInformation());
    }

    TYPED_TEST(AScene, PreallocatesMemoryPoolsBasedOnSizeInformationNeverShrink)
    {
        const SceneSizeInformation sizeInfo(21, 22, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18);
        const SceneInfo sceneInfo;
        TypeParam preallocatedScene(sceneInfo);
        preallocatedScene.preallocateSceneSize(sizeInfo);
        EXPECT_EQ(sizeInfo, preallocatedScene.getSceneSizeInformation());

        const SceneSizeInformation smallerSizeInfo(1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1);
        preallocatedScene.preallocateSceneSize(smallerSizeInfo);

        EXPECT_EQ(sizeInfo, preallocatedScene.getSceneSizeInformation());
        EXPECT_EQ(sizeInfo.nodeCount, preallocatedScene.getNodeCount());
        EXPECT_EQ(sizeInfo.cameraCount, preallocatedScene.getCameraCount());
        EXPECT_EQ(sizeInfo.transformCount, preallocatedScene.getTransformCount());
        EXPECT_EQ(sizeInfo.renderableCount, preallocatedScene.getRenderableCount());
        EXPECT_EQ(sizeInfo.renderStateCount, preallocatedScene.getRenderStateCount());
        EXPECT_EQ(sizeInfo.datalayoutCount, preallocatedScene.getDataLayoutCount());
        EXPECT_EQ(sizeInfo.datainstanceCount, preallocatedScene.getDataInstanceCount());
        EXPECT_EQ(sizeInfo.renderGroupCount, preallocatedScene.getRenderGroupCount());
        EXPECT_EQ(sizeInfo.renderPassCount, preallocatedScene.getRenderPassCount());
        EXPECT_EQ(sizeInfo.blitPassCount, preallocatedScene.getBlitPassCount());
        EXPECT_EQ(sizeInfo.renderTargetCount, preallocatedScene.getRenderTargetCount());
        EXPECT_EQ(sizeInfo.renderBufferCount, preallocatedScene.getRenderBufferCount());
        EXPECT_EQ(sizeInfo.textureSamplerCount, preallocatedScene.getTextureSamplerCount());
        EXPECT_EQ(sizeInfo.streamTextureCount, preallocatedScene.getStreamTextureCount());
        EXPECT_EQ(sizeInfo.dataSlotCount, preallocatedScene.getDataSlotCount());
        EXPECT_EQ(sizeInfo.dataBufferCount, preallocatedScene.getDataBufferCount());
    }

    TYPED_TEST(AScene, InitializesCorrectly)
    {
        const SceneInfo sceneInfo(SceneId(537u), "TestScene");
        TypeParam scene(sceneInfo);

        EXPECT_EQ(sceneInfo.sceneID, scene.getSceneId());
        EXPECT_EQ(sceneInfo.friendlyName, scene.getName());
    }
}
