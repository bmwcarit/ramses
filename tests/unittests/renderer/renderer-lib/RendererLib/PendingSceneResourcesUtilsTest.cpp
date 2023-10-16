//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/RendererLib/Types.h"
#include "internal/RendererLib/PendingSceneResourcesUtils.h"
#include "RendererResourceManagerMock.h"
#include "internal/RendererLib/RendererCachedScene.h"
#include "internal/RendererLib/SceneLinksManager.h"
#include "internal/RendererLib/RendererScenes.h"
#include "internal/RendererLib/RendererEventCollector.h"
#include "internal/SceneGraph/Scene/Scene.h"
#include "internal/SceneGraph/SceneUtils/ResourceUtils.h"
#include "SceneAllocateHelper.h"

namespace ramses::internal {
    using namespace testing;

    class APendingSceneResourcesUtils : public ::testing::Test
    {
    public:
        APendingSceneResourcesUtils()
        : rendererScenes(rendererEventCollector)
        , sceneLinksManager(rendererScenes, rendererEventCollector)
        , scene(sceneLinksManager, SceneInfo(sceneID))
            , allocateHelper(scene)
        {
            allocateHelper.allocateRenderTarget(renderTargetHandle);
            allocateHelper.allocateRenderBuffer({ 16u, 16u, EPixelStorageFormat::RGBA8, ERenderBufferAccessMode::ReadWrite, 0u }, renderBufferHandle);
            allocateHelper.allocateBlitPass(RenderBufferHandle(81u), RenderBufferHandle(82u), blitPassHandle);
            allocateHelper.allocateDataBuffer(EDataBufferType::IndexBuffer, EDataType::UInt32, 10u, dataBufferHandle);
            allocateHelper.allocateTextureBuffer(EPixelStorageFormat::R8, { { 32, 32 },{ 16, 16 },{ 8, 8 } }, textureBufferHandle);
        }

    protected:
        const SceneId               sceneID = SceneId(13u);
        const RenderTargetHandle    renderTargetHandle = RenderTargetHandle(5u);
        const RenderBufferHandle    renderBufferHandle = RenderBufferHandle(6u);
        const BlitPassHandle        blitPassHandle = BlitPassHandle(8u);
        const DataBufferHandle      dataBufferHandle = DataBufferHandle(9u);
        const TextureBufferHandle   textureBufferHandle = TextureBufferHandle(10u);

        const MemoryHandle dummyHandle = MemoryHandle(123u);
        const MemoryHandle dummyHandle2 = MemoryHandle(124u);

        RendererEventCollector rendererEventCollector;
        RendererScenes         rendererScenes;
        SceneLinksManager      sceneLinksManager;

        RendererCachedScene scene;
        SceneAllocateHelper allocateHelper;
        StrictMock<RendererResourceManagerMock> resourceManager;
    };

    struct BasicActionSet
    {
        explicit BasicActionSet(ESceneResourceAction c = ESceneResourceAction_Invalid, ESceneResourceAction d = ESceneResourceAction_Invalid, ESceneResourceAction u = ESceneResourceAction_Invalid)
            : create(c)
            , destroy(d)
            , update(u)
        {
        }

        ESceneResourceAction create;
        ESceneResourceAction destroy;
        ESceneResourceAction update;
    };

    static const BasicActionSet ActionSet_RenderTarget{ ESceneResourceAction_CreateRenderTarget, ESceneResourceAction_DestroyRenderTarget };
    static const BasicActionSet ActionSet_RenderBuffer{ ESceneResourceAction_CreateRenderBuffer, ESceneResourceAction_DestroyRenderBuffer };
    static const BasicActionSet ActionSet_BlitPass{ ESceneResourceAction_CreateBlitPass, ESceneResourceAction_DestroyBlitPass };
    static const BasicActionSet ActionSet_DataBuffer{ ESceneResourceAction_CreateDataBuffer, ESceneResourceAction_DestroyDataBuffer, ESceneResourceAction_UpdateDataBuffer };
    static const BasicActionSet ActionSet_TextureBuffer{ ESceneResourceAction_CreateTextureBuffer, ESceneResourceAction_DestroyTextureBuffer, ESceneResourceAction_UpdateTextureBuffer };

    static const BasicActionSet TestSceneResourceActions[] =
    {
        ActionSet_RenderTarget,
        ActionSet_RenderBuffer,
        ActionSet_BlitPass,
        ActionSet_DataBuffer,
        ActionSet_TextureBuffer
    };

    static const BasicActionSet TestSceneResourceActions_Buffers[] =
    {
        ActionSet_DataBuffer,
        ActionSet_TextureBuffer
    };

    TEST_F(APendingSceneResourcesUtils, appliesSceneResourceActions)
    {
        SceneResourceActionVector actions;
        actions.push_back(SceneResourceAction(renderBufferHandle.asMemoryHandle(), ESceneResourceAction_CreateRenderBuffer));
        actions.push_back(SceneResourceAction(renderTargetHandle.asMemoryHandle(), ESceneResourceAction_CreateRenderTarget));
        actions.push_back(SceneResourceAction(blitPassHandle.asMemoryHandle(), ESceneResourceAction_CreateBlitPass));
        actions.push_back(SceneResourceAction(dataBufferHandle.asMemoryHandle(), ESceneResourceAction_CreateDataBuffer));
        actions.push_back(SceneResourceAction(dataBufferHandle.asMemoryHandle(), ESceneResourceAction_UpdateDataBuffer));
        actions.push_back(SceneResourceAction(textureBufferHandle.asMemoryHandle(), ESceneResourceAction_CreateTextureBuffer));
        actions.push_back(SceneResourceAction(textureBufferHandle.asMemoryHandle(), ESceneResourceAction_UpdateTextureBuffer));
        const std::array<std::byte, 4> data{};
        scene.updateTextureBuffer(textureBufferHandle, 0, 0, 0, 1, 1, data.data());
        scene.updateTextureBuffer(textureBufferHandle, 1, 0, 0, 1, 1, data.data());
        scene.updateTextureBuffer(textureBufferHandle, 2, 0, 0, 1, 1, data.data());

        InSequence seq;
        EXPECT_CALL(resourceManager, uploadRenderTargetBuffer(renderBufferHandle, sceneID, _));
        EXPECT_CALL(resourceManager, uploadRenderTarget(renderTargetHandle, _, sceneID));
        EXPECT_CALL(resourceManager, uploadBlitPassRenderTargets(blitPassHandle, _, _, sceneID));
        EXPECT_CALL(resourceManager, uploadDataBuffer(dataBufferHandle, _, _, _, sceneID));
        EXPECT_CALL(resourceManager, updateDataBuffer(dataBufferHandle, _, _, sceneID));
        EXPECT_CALL(resourceManager, uploadTextureBuffer(textureBufferHandle, _, _, _, _, sceneID));
        EXPECT_CALL(resourceManager, updateTextureBuffer(textureBufferHandle, _, _, _, _, sceneID)).Times(3u); // 3 mips
        PendingSceneResourcesUtils::ApplySceneResourceActions(actions, scene, resourceManager);
    }

    TEST_F(APendingSceneResourcesUtils, appliesSceneResourceActions_Unloads)
    {
        SceneResourceActionVector actions;
        actions.push_back(SceneResourceAction(renderTargetHandle.asMemoryHandle(), ESceneResourceAction_DestroyRenderTarget));
        actions.push_back(SceneResourceAction(renderBufferHandle.asMemoryHandle(), ESceneResourceAction_DestroyRenderBuffer));
        actions.push_back(SceneResourceAction(blitPassHandle.asMemoryHandle(), ESceneResourceAction_DestroyBlitPass));
        actions.push_back(SceneResourceAction(dataBufferHandle.asMemoryHandle(), ESceneResourceAction_DestroyDataBuffer));
        actions.push_back(SceneResourceAction(textureBufferHandle.asMemoryHandle(), ESceneResourceAction_DestroyTextureBuffer));

        InSequence seq;
        EXPECT_CALL(resourceManager, unloadRenderTarget(renderTargetHandle, sceneID));
        EXPECT_CALL(resourceManager, unloadRenderTargetBuffer(renderBufferHandle, sceneID));
        EXPECT_CALL(resourceManager, unloadBlitPassRenderTargets(blitPassHandle, sceneID));
        EXPECT_CALL(resourceManager, unloadDataBuffer(dataBufferHandle, sceneID));
        EXPECT_CALL(resourceManager, unloadTextureBuffer(textureBufferHandle, sceneID));
        PendingSceneResourcesUtils::ApplySceneResourceActions(actions, scene, resourceManager);
    }

    TEST_F(APendingSceneResourcesUtils, getsSceneResourcesFromSceneAndApliesThem)
    {
        SceneResourceActionVector actions;
        size_t resSize = 999u;
        ResourceUtils::GetAllSceneResourcesFromScene(actions, scene, resSize);
        EXPECT_FALSE(actions.empty());
        EXPECT_EQ(1344u, resSize);

        static const std::array<std::byte, 32*32*4> data{};
        scene.updateTextureBuffer(textureBufferHandle, 0u, 0u, 0u, 32, 32, data.data());
        scene.updateTextureBuffer(textureBufferHandle, 1u, 0u, 0u, 16, 16, data.data());
        scene.updateTextureBuffer(textureBufferHandle, 2u, 0u, 0u, 8, 8, data.data());

        InSequence seq;
        EXPECT_CALL(resourceManager, uploadRenderTargetBuffer(renderBufferHandle, sceneID, _));
        EXPECT_CALL(resourceManager, uploadRenderTarget(renderTargetHandle, _, sceneID));
        EXPECT_CALL(resourceManager, uploadBlitPassRenderTargets(blitPassHandle, RenderBufferHandle(81), RenderBufferHandle(82), sceneID));
        EXPECT_CALL(resourceManager, uploadDataBuffer(dataBufferHandle, _, _, _, sceneID));
        EXPECT_CALL(resourceManager, updateDataBuffer(dataBufferHandle, _, _, sceneID));

        EXPECT_CALL(resourceManager, uploadTextureBuffer(textureBufferHandle, _, _, _, _, sceneID));
        EXPECT_CALL(resourceManager, updateTextureBuffer(textureBufferHandle, 0u, Quad{0u, 0u, 32, 32}, 32, _, sceneID));
        EXPECT_CALL(resourceManager, updateTextureBuffer(textureBufferHandle, 1u, Quad{0u, 0u, 16, 16}, 16, _, sceneID));
        EXPECT_CALL(resourceManager, updateTextureBuffer(textureBufferHandle, 2u, Quad{0u, 0u, 8, 8}, 8, _, sceneID));
        PendingSceneResourcesUtils::ApplySceneResourceActions(actions, scene, resourceManager);
    }

    TEST_F(APendingSceneResourcesUtils, cancelsOutCreateAndDeleteDuringConsolidation)
    {
        for (const auto& crateDestroyPair : TestSceneResourceActions)
        {
            SceneResourceActionVector actions;
            SceneResourceActionVector actionsNew;

            // test create/destroy pair across old/new and also another pair within new only
            actions.push_back(SceneResourceAction(dummyHandle, crateDestroyPair.create));
            actionsNew.push_back(SceneResourceAction(dummyHandle, crateDestroyPair.destroy));
            actionsNew.push_back(SceneResourceAction(dummyHandle, crateDestroyPair.create));
            actionsNew.push_back(SceneResourceAction(dummyHandle, crateDestroyPair.destroy));

            PendingSceneResourcesUtils::ConsolidateSceneResourceActions(actionsNew, actions);
            EXPECT_TRUE(actions.empty());
        }
    }

    TEST_F(APendingSceneResourcesUtils, doesNotCancelOutDestroyAndCreateDuringConsolidation)
    {
        for (const auto& crateDestroyPair : TestSceneResourceActions)
        {
            SceneResourceActionVector actionsNew;
            SceneResourceActionVector actions;

            actionsNew.push_back(SceneResourceAction(dummyHandle, crateDestroyPair.destroy));
            actionsNew.push_back(SceneResourceAction(dummyHandle, crateDestroyPair.create));

            PendingSceneResourcesUtils::ConsolidateSceneResourceActions(actionsNew, actions);
            ASSERT_EQ(2u, actions.size());
            EXPECT_EQ(crateDestroyPair.destroy, actions.front().action);
            EXPECT_EQ(crateDestroyPair.create, actions.back().action);
        }
    }

    TEST_F(APendingSceneResourcesUtils, doesNotCancelOutCreateAndDeleteOfDifferentHandlesDuringConsolidation)
    {
        for (const auto& crateDestroyPair : TestSceneResourceActions)
        {
            SceneResourceActionVector actions;
            SceneResourceActionVector actionsNew;

            // test create/destroy pair across old/new and also another pair within new only
            actions.push_back(SceneResourceAction(dummyHandle, crateDestroyPair.create));
            actionsNew.push_back(SceneResourceAction(dummyHandle, crateDestroyPair.destroy));
            actionsNew.push_back(SceneResourceAction(dummyHandle, crateDestroyPair.create));
            actionsNew.push_back(SceneResourceAction(dummyHandle2, crateDestroyPair.destroy));

            PendingSceneResourcesUtils::ConsolidateSceneResourceActions(actionsNew, actions);
            ASSERT_EQ(2u, actions.size());
            EXPECT_EQ(SceneResourceAction(dummyHandle, crateDestroyPair.create), actions.front());
            EXPECT_EQ(SceneResourceAction(dummyHandle2, crateDestroyPair.destroy), actions.back());
        }
    }

    TEST_F(APendingSceneResourcesUtils, keepsAdditionalCreateActionAfterCanceledCreateAndDeleteDuringConsolidation)
    {
        for (const auto& crateDestroyPair : TestSceneResourceActions)
        {
            SceneResourceActionVector actions;
            SceneResourceActionVector actionsNew;

            // test create/destroy pair across old/new and also another pair within new only
            actions.push_back(SceneResourceAction(dummyHandle, crateDestroyPair.create));
            actionsNew.push_back(SceneResourceAction(dummyHandle, crateDestroyPair.destroy));
            actionsNew.push_back(SceneResourceAction(dummyHandle, crateDestroyPair.create));
            actionsNew.push_back(SceneResourceAction(dummyHandle, crateDestroyPair.destroy));
            // additional create
            actionsNew.push_back(SceneResourceAction(dummyHandle, crateDestroyPair.create));

            PendingSceneResourcesUtils::ConsolidateSceneResourceActions(actionsNew, actions);
            ASSERT_EQ(1u, actions.size());
            EXPECT_EQ(crateDestroyPair.create, actions.front().action);
        }
    }

    TEST_F(APendingSceneResourcesUtils, keepsAdditionalDestroyActionAfterCanceledCreateAndDeleteDuringConsolidation)
    {
        for (const auto& crateDestroyPair : TestSceneResourceActions)
        {
            SceneResourceActionVector actions;
            SceneResourceActionVector actionsNew;

            // test create/destroy pair across old/new and also another pair within new only
            actions.push_back(SceneResourceAction(dummyHandle, crateDestroyPair.create));
            actionsNew.push_back(SceneResourceAction(dummyHandle, crateDestroyPair.destroy));
            actionsNew.push_back(SceneResourceAction(dummyHandle, crateDestroyPair.create));
            actionsNew.push_back(SceneResourceAction(dummyHandle, crateDestroyPair.destroy));
            // additional create
            actionsNew.push_back(SceneResourceAction(dummyHandle, crateDestroyPair.destroy));

            PendingSceneResourcesUtils::ConsolidateSceneResourceActions(actionsNew, actions);
            ASSERT_EQ(1u, actions.size());
            EXPECT_EQ(crateDestroyPair.destroy, actions.front().action);
        }
    }

    TEST_F(APendingSceneResourcesUtils, squashesBufferUpdateActionsToSingleActionDuringConsolidation)
    {
        for (const auto& bufferAction : TestSceneResourceActions_Buffers)
        {
            SceneResourceActionVector actions;
            SceneResourceActionVector actionsNew;

            // updates both in old and new actions
            actions.push_back(SceneResourceAction(dummyHandle, bufferAction.update));
            actionsNew.push_back(SceneResourceAction(dummyHandle, bufferAction.update));
            actionsNew.push_back(SceneResourceAction(dummyHandle, bufferAction.update));

            PendingSceneResourcesUtils::ConsolidateSceneResourceActions(actionsNew, actions);
            ASSERT_EQ(1u, actions.size());
            EXPECT_EQ(bufferAction.update, actions.front().action);
        }
    }

    TEST_F(APendingSceneResourcesUtils, removesAnyDataBufferUpdateActionsBeforeDestroyDuringConsolidation)
    {
        for (const auto& bufferAction : TestSceneResourceActions_Buffers)
        {
            SceneResourceActionVector actions;
            SceneResourceActionVector actionsNew;

            // updates both in old and new actions
            actions.push_back(SceneResourceAction(dummyHandle, bufferAction.update));
            actionsNew.push_back(SceneResourceAction(dummyHandle, bufferAction.update));
            actionsNew.push_back(SceneResourceAction(dummyHandle, bufferAction.update));

            // destroy buffer
            actionsNew.push_back(SceneResourceAction(dummyHandle, bufferAction.destroy));

            PendingSceneResourcesUtils::ConsolidateSceneResourceActions(actionsNew, actions);
            ASSERT_EQ(1u, actions.size());
            EXPECT_EQ(bufferAction.destroy, actions.front().action);
        }
    }

    TEST_F(APendingSceneResourcesUtils, keepsDataBufferUpdateActionForNewlyCreatedAfterRemovedPreviousUpdatesDueToDestroy)
    {
        for (const auto& bufferAction : TestSceneResourceActions_Buffers)
        {
            SceneResourceActionVector actions;
            SceneResourceActionVector actionsNew;

            // create buffer
            actionsNew.push_back(SceneResourceAction(dummyHandle, bufferAction.create));
            // updates both in old and new actions
            actions.push_back(SceneResourceAction(dummyHandle, bufferAction.update));
            actionsNew.push_back(SceneResourceAction(dummyHandle, bufferAction.update));
            actionsNew.push_back(SceneResourceAction(dummyHandle, bufferAction.update));
            // destroy buffer
            actionsNew.push_back(SceneResourceAction(dummyHandle, bufferAction.destroy));
            // all previous ones should be canceled

            // create new buffer
            actionsNew.push_back(SceneResourceAction(dummyHandle, bufferAction.create));
            // update new buffer
            actionsNew.push_back(SceneResourceAction(dummyHandle, bufferAction.update));
            actionsNew.push_back(SceneResourceAction(dummyHandle, bufferAction.update));

            PendingSceneResourcesUtils::ConsolidateSceneResourceActions(actionsNew, actions);
            ASSERT_EQ(2u, actions.size());
            EXPECT_EQ(bufferAction.create, actions.front().action);
            EXPECT_EQ(bufferAction.update, actions.back().action);
        }
    }

    TEST_F(APendingSceneResourcesUtils, textureBufferPartialUpdate)
    {
        SceneResourceActionVector actions;
        actions.push_back(SceneResourceAction(textureBufferHandle.asMemoryHandle(), ESceneResourceAction_UpdateTextureBuffer));

        const std::array<std::byte, 3 * 4> data{};
        scene.updateTextureBuffer(textureBufferHandle, 0, 1, 2, 3, 4, data.data());
        EXPECT_CALL(resourceManager, updateTextureBuffer(textureBufferHandle, 0u, Quad{1, 2, 3, 4}, 32, _, sceneID));
        PendingSceneResourcesUtils::ApplySceneResourceActions(actions, scene, resourceManager);
    }

    TEST_F(APendingSceneResourcesUtils, textureBufferCombinePartialUpdates)
    {
        SceneResourceActionVector actions;
        SceneResourceActionVector actionsNew;
        actionsNew.push_back(SceneResourceAction(textureBufferHandle.asMemoryHandle(), ESceneResourceAction_UpdateTextureBuffer));
        actionsNew.push_back(SceneResourceAction(textureBufferHandle.asMemoryHandle(), ESceneResourceAction_UpdateTextureBuffer));
        actionsNew.push_back(SceneResourceAction(textureBufferHandle.asMemoryHandle(), ESceneResourceAction_UpdateTextureBuffer));
        actionsNew.push_back(SceneResourceAction(textureBufferHandle.asMemoryHandle(), ESceneResourceAction_UpdateTextureBuffer));
        actionsNew.push_back(SceneResourceAction(textureBufferHandle.asMemoryHandle(), ESceneResourceAction_UpdateTextureBuffer));

        const std::array<std::byte, 5 * 4> data{};
        scene.updateTextureBuffer(textureBufferHandle, 0, 2, 2, 5, 4, data.data());
        scene.updateTextureBuffer(textureBufferHandle, 1, 1, 1, 2, 2, data.data());
        scene.updateTextureBuffer(textureBufferHandle, 0, 1, 1, 2, 2, data.data());
        scene.updateTextureBuffer(textureBufferHandle, 0, 11, 12, 2, 2, data.data());
        scene.updateTextureBuffer(textureBufferHandle, 1, 0, 0, 3, 4, data.data());
        PendingSceneResourcesUtils::ConsolidateSceneResourceActions(actionsNew, actions);

        EXPECT_CALL(resourceManager, updateTextureBuffer(textureBufferHandle, 0u, Quad{1, 1, 12, 13}, 32, _, sceneID));
        EXPECT_CALL(resourceManager, updateTextureBuffer(textureBufferHandle, 1u, Quad{0, 0, 3, 4}, 16, _, sceneID));
        EXPECT_EQ(actions.size(), 1u);
        PendingSceneResourcesUtils::ApplySceneResourceActions(actions, scene, resourceManager);
    }
}
