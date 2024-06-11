//  -------------------------------------------------------------------------
//  Copyright (C) 2024 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "AllocationHelper.h"
#include "internal/SceneGraph/Scene/DataLayoutCachedScene.h"
#include "internal/SceneGraph/Scene/MergeScene.h"
#include "internal/SceneGraph/Scene/SceneMergeHandleMapping.h"
#include "internal/SceneGraph/SceneAPI/DataFieldInfo.h"
#include "internal/SceneGraph/SceneAPI/EDataType.h"
#include "internal/SceneGraph/SceneAPI/Handles.h"
#include "TestingScene.h"
#include "internal/SceneGraph/SceneAPI/ResourceContentHash.h"
#include "ramses/framework/EFeatureLevel.h"

using namespace testing;

namespace ramses::internal
{
    template <typename T>
    class AMergeSceneTest : public ::testing::Test
    {
    public:
        void setupPrerequisits(MergeScene& scene)
        {
            AllocationHelper::setupPrerequisits<T>(scene);
        }

        ActionCollectingScene createOriginalScene()
        {
            return ActionCollectingScene();
        }
    };

    using HandleTypes = ::testing::Types<
        RenderableHandle,
        RenderStateHandle,
        CameraHandle,
        NodeHandle,
        TransformHandle,
        DataLayoutHandle,
        DataInstanceHandle,
        UniformBufferHandle,
        TextureSamplerHandle,
        RenderGroupHandle,
        RenderPassHandle,
        BlitPassHandle,
        PickableObjectHandle,
        RenderTargetHandle,
        RenderBufferHandle,
        DataBufferHandle,
        TextureBufferHandle,
        DataSlotHandle,
        SceneReferenceHandle>;

    TYPED_TEST_SUITE(AMergeSceneTest, HandleTypes);


    TYPED_TEST(AMergeSceneTest, canAllocateAndBuildMapping)
    {
        auto originalScene = this->createOriginalScene();
        originalScene.preallocateSceneSize(AllocationHelper::SCENE_SIZE_INFO);
        SceneMergeHandleMapping mapping;
        MergeScene scene(originalScene, mapping);
        this->setupPrerequisits(scene);

        const TypeParam handle{3u};
        const TypeParam expectedHandle = handle + 42u;
        const auto handleAllocated = AllocationHelper::allocate(scene, handle);
        EXPECT_EQ(expectedHandle, handleAllocated);

        EXPECT_TRUE(mapping.hasMapping(handle));
        const auto handleMapped = mapping.getMapping(handle);
        EXPECT_EQ(expectedHandle, handleMapped);
    }

    class MergeSceneTest : public ::testing::Test
    {
    };

    TEST_F(MergeSceneTest, canMergeScenes)
    {
        DataLayoutCachedScene originalScene;
        SceneMergeHandleMapping mapping;

        TestingScene testingScene(originalScene, EFeatureLevel_Latest);
        testingScene.VerifyContent(originalScene);

        MergeScene scene(originalScene, mapping);
        TestingScene testingSceneMerged(scene, EFeatureLevel_Latest);

        // verify original content
        testingSceneMerged.VerifyContent(originalScene);

        // verify merged content using mapping
        testingSceneMerged.setMapping(&mapping);
        testingSceneMerged.VerifyContent(scene);
    }

    TEST_F(MergeSceneTest, canAllocateSameDataLayoutMultipleTimes)
    {
        DataLayoutCachedScene originalScene;
        SceneMergeHandleMapping mapping;
        ResourceContentHash effectHash {7, 12};

        DataFieldInfoVector dataFields;
        dataFields.emplace_back(DataFieldInfo(EDataType::Vector3F));

        const DataLayoutHandle originalHandle {0u};
        auto handle1 = originalScene.allocateDataLayout(dataFields, effectHash, originalHandle);
        ASSERT_EQ(originalHandle, handle1);

        MergeScene scene(originalScene, mapping);

        auto handle2 = scene.allocateDataLayout(dataFields, effectHash, originalHandle);

        EXPECT_EQ(originalHandle, handle2);

        // SceneDescriber will create allocateDataLayout actions for each reference to the same data layout
        auto handle3 = scene.allocateDataLayout(dataFields, effectHash, originalHandle);

        EXPECT_EQ(originalHandle, handle3);

        // now also test for previously unused handle
        const DataLayoutHandle newHandle {1u};
        auto handle4 = scene.allocateDataLayout(dataFields, effectHash, newHandle);

        EXPECT_EQ(originalHandle, handle4);

        auto handle5 = scene.allocateDataLayout(dataFields, effectHash, newHandle);

        EXPECT_EQ(originalHandle, handle5);
    }
}
