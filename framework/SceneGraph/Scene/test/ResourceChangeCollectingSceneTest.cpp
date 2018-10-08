//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "framework_common_gmock_header.h"
#include "gtest/gtest.h"
#include "Scene/ResourceChangeCollectingScene.h"
#include "Scene/SceneResourceUtils.h"

namespace ramses_internal
{
    class AResourceChangeCollectingScene : public testing::Test
    {
    public:
        AResourceChangeCollectingScene()
            : resourceChanges(scene.getResourceChanges())
            , testUniformLayout(0u)
            , testGeometryLayout(2u)
            , indicesField(0u)
            , vertAttribField(1u)
            , dataField(0u)
            , samplerField(1u)
        {
            DataFieldInfoVector geometryDataFields(2u);
            geometryDataFields[indicesField.asMemoryHandle()] = DataFieldInfo(EDataType_Indices, 1u, EFixedSemantics_Indices);
            geometryDataFields[vertAttribField.asMemoryHandle()] = DataFieldInfo(EDataType_Vector3Buffer, 1u, EFixedSemantics_VertexPositionAttribute);
            scene.allocateDataLayout(geometryDataFields, testGeometryLayout);

            DataFieldInfoVector uniformDataFields(2u);
            uniformDataFields[dataField.asMemoryHandle()] = DataFieldInfo(EDataType_Float);
            uniformDataFields[samplerField.asMemoryHandle()] = DataFieldInfo(EDataType_TextureSampler);
            scene.allocateDataLayout(uniformDataFields, testUniformLayout);
        }

    protected:
        RenderableHandle createRenderable()
        {
            return scene.allocateRenderable(scene.allocateNode());
        }

        DataInstanceHandle createUniformDataInstanceWithSampler(RenderableHandle renderable, ResourceContentHash texHash)
        {
            const DataInstanceHandle uniformData = scene.allocateDataInstance(testUniformLayout);
            scene.setRenderableDataInstance(renderable, ERenderableDataSlotType_Uniforms, uniformData);
            const TextureSamplerHandle sampler = scene.allocateTextureSampler({ {}, texHash });
            scene.setDataTextureSamplerHandle(uniformData, samplerField, sampler);
            return uniformData;
        }

        DataInstanceHandle createVertexDataInstance(RenderableHandle renderable)
        {
            const DataInstanceHandle geometryData = scene.allocateDataInstance(testGeometryLayout);
            scene.setRenderableDataInstance(renderable, ERenderableDataSlotType_Geometry, geometryData);

            return geometryData;
        }

        void expectSameSceneResourceChangesWhenExtractedFromScene()
        {
            SceneResourceChanges fromScene;
            SceneResourceUtils::GetSceneResourceChangesFromScene(fromScene, scene);

            SceneResourceChanges fromResourceChanges = resourceChanges;

            std::sort(fromScene.m_addedClientResourceRefs.begin(), fromScene.m_addedClientResourceRefs.end());
            std::sort(fromResourceChanges.m_addedClientResourceRefs.begin(), fromResourceChanges.m_addedClientResourceRefs.end());

            EXPECT_EQ(fromResourceChanges.m_addedClientResourceRefs, fromScene.m_addedClientResourceRefs);
            EXPECT_EQ(0u, fromScene.m_removedClientResourceRefs.size());

            EXPECT_EQ(fromResourceChanges.m_sceneResourceActions.size(), fromScene.m_sceneResourceActions.size());
            EXPECT_EQ(fromResourceChanges.m_sceneResourceActions, fromScene.m_sceneResourceActions);
        }

        ResourceChangeCollectingScene scene;
        const SceneResourceChanges& resourceChanges;

        const DataLayoutHandle testUniformLayout;
        const DataLayoutHandle testGeometryLayout;
        const DataFieldHandle indicesField;
        const DataFieldHandle vertAttribField;
        const DataFieldHandle dataField;
        const DataFieldHandle samplerField;
    };

    TEST_F(AResourceChangeCollectingScene, hasEmptyResourceListsAtCreation)
    {
        EXPECT_TRUE(resourceChanges.m_addedClientResourceRefs.empty());
        EXPECT_TRUE(resourceChanges.m_removedClientResourceRefs.empty());
        EXPECT_TRUE(resourceChanges.m_sceneResourceActions.empty());

        expectSameSceneResourceChangesWhenExtractedFromScene();
    }

    TEST_F(AResourceChangeCollectingScene, addsResourceToListWithNewResourcesAfterSetting)
    {
        const RenderableHandle renderable = createRenderable();
        const DataInstanceHandle vertexData = createVertexDataInstance(renderable);

        EXPECT_EQ(0u, resourceChanges.m_addedClientResourceRefs.size());
        EXPECT_EQ(0u, resourceChanges.m_removedClientResourceRefs.size());

        scene.setDataResource(vertexData, vertAttribField, ResourceContentHash(111u, 0), DataBufferHandle::Invalid(), 0u);

        scene.allocateTextureSampler({ {}, ResourceContentHash(222u, 0) });
        EXPECT_EQ(2u, resourceChanges.m_addedClientResourceRefs.size());
        EXPECT_EQ(ResourceContentHash(111u, 0), resourceChanges.m_addedClientResourceRefs[0]);
        EXPECT_EQ(ResourceContentHash(222u, 0), resourceChanges.m_addedClientResourceRefs[1]);
        EXPECT_EQ(0u, resourceChanges.m_removedClientResourceRefs.size()); // old value was invalid!
        expectSameSceneResourceChangesWhenExtractedFromScene();

        scene.clearResourceChanges();
    }

    TEST_F(AResourceChangeCollectingScene, doesNotAddResourceToListsWhenNewHashIsTheSameAsOldHash)
    {
        const RenderableHandle renderable = createRenderable();
        const DataInstanceHandle dataInstance = createVertexDataInstance(renderable);
        const ResourceContentHash hash(123u, 0);
        scene.setDataResource(dataInstance, vertAttribField, hash, DataBufferHandle::Invalid(), 0u);

        scene.clearResourceChanges();

        scene.setDataResource(dataInstance, vertAttribField, hash, DataBufferHandle::Invalid(), 0u);
        EXPECT_EQ(0u, resourceChanges.m_addedClientResourceRefs.size());
        EXPECT_EQ(0u, resourceChanges.m_removedClientResourceRefs.size());
    }

    TEST_F(AResourceChangeCollectingScene, doesNotAddResourceToListsWhenNewHashIsTheSameAsOldHashButDivisorIsDifferent)
    {
        const RenderableHandle renderable = createRenderable();
        const DataInstanceHandle dataInstance = createVertexDataInstance(renderable);
        const ResourceContentHash hash(123u, 0);

        scene.setDataResource(dataInstance, vertAttribField, hash, DataBufferHandle::Invalid(), 0u);

        scene.clearResourceChanges();

        scene.setDataResource(dataInstance, vertAttribField, hash, DataBufferHandle::Invalid(), 1u);
        EXPECT_EQ(0u, resourceChanges.m_addedClientResourceRefs.size());
        EXPECT_EQ(0u, resourceChanges.m_removedClientResourceRefs.size());
    }

    TEST_F(AResourceChangeCollectingScene, sameResourceUsedSecondTimeNotAddedToNewList)
    {
        const RenderableHandle renderable = createRenderable();
        const ResourceContentHash hash(123u, 0);
        createUniformDataInstanceWithSampler(renderable, hash);
        const DataInstanceHandle vertexData = createVertexDataInstance(renderable);
        scene.setDataResource(vertexData, vertAttribField, hash, DataBufferHandle::Invalid(), 0u);

        scene.clearResourceChanges();

        scene.allocateTextureSampler({ {}, hash });
        EXPECT_EQ(0u, resourceChanges.m_addedClientResourceRefs.size());
        EXPECT_EQ(0u, resourceChanges.m_removedClientResourceRefs.size());
    }

    TEST_F(AResourceChangeCollectingScene, canSwitchFromClientResourceHashToDataBufferAndMarkResourceObsolete)
    {
        const RenderableHandle renderable = createRenderable();
        const DataInstanceHandle dataInstance = createVertexDataInstance(renderable);
        const ResourceContentHash hash(123u, 0);
        const DataBufferHandle dataBuffer(0u);

        scene.setDataResource(dataInstance, vertAttribField, hash, DataBufferHandle::Invalid(), 0u);

        scene.clearResourceChanges();

        scene.setDataResource(dataInstance, vertAttribField, ResourceContentHash::Invalid(), dataBuffer, 0u);
        {
            const ResourceField& dataResourceOut =  scene.getDataResource(dataInstance, vertAttribField);
            EXPECT_EQ(ResourceContentHash::Invalid(), dataResourceOut.hash);
            EXPECT_EQ(dataBuffer, dataResourceOut.dataBuffer);
        }

        EXPECT_EQ(0u, resourceChanges.m_addedClientResourceRefs.size());
        EXPECT_EQ(1u, resourceChanges.m_removedClientResourceRefs.size());
        EXPECT_EQ(hash, resourceChanges.m_removedClientResourceRefs[0]);
    }

    TEST_F(AResourceChangeCollectingScene, canSwitchFromClientResourceHashToDataBufferAndBackToResourceHashWithoutMarkingResourceObsolete)
    {
        const RenderableHandle renderable = createRenderable();
        const DataInstanceHandle dataInstance = createVertexDataInstance(renderable);
        const ResourceContentHash hash(123u, 0);
        const DataBufferHandle dataBuffer(0u);

        scene.setDataResource(dataInstance, vertAttribField, hash, DataBufferHandle::Invalid(), 0u);

        scene.clearResourceChanges();

        scene.setDataResource(dataInstance, vertAttribField, ResourceContentHash::Invalid(), dataBuffer, 0u);
        scene.setDataResource(dataInstance, vertAttribField, hash, DataBufferHandle::Invalid(), 0u);
        {
            const ResourceField& dataResourceOut = scene.getDataResource(dataInstance, vertAttribField);
            EXPECT_EQ(hash, dataResourceOut.hash);
            EXPECT_FALSE(dataResourceOut.dataBuffer.isValid());
        }

        EXPECT_EQ(0u, resourceChanges.m_addedClientResourceRefs.size());
        EXPECT_EQ(0u, resourceChanges.m_removedClientResourceRefs.size());
    }

    TEST_F(AResourceChangeCollectingScene, canSwitchFromDataBufferToClientResourceHashAndMarkResourceAdded)
    {
        const RenderableHandle renderable = createRenderable();
        const DataInstanceHandle dataInstance = createVertexDataInstance(renderable);
        const ResourceContentHash hash(123u, 0);
        const DataBufferHandle dataBuffer(0u);

        scene.setDataResource(dataInstance, vertAttribField, ResourceContentHash::Invalid(), dataBuffer, 0u);

        scene.clearResourceChanges();

        scene.setDataResource(dataInstance, vertAttribField, hash, DataBufferHandle::Invalid(), 0u);
        {
            const ResourceField& dataResourceOut = scene.getDataResource(dataInstance, vertAttribField);
            EXPECT_EQ(hash, dataResourceOut.hash);
            EXPECT_FALSE(dataResourceOut.dataBuffer.isValid());
        }

        EXPECT_EQ(1u, resourceChanges.m_addedClientResourceRefs.size());
        EXPECT_EQ(0u, resourceChanges.m_removedClientResourceRefs.size());
        EXPECT_EQ(hash, resourceChanges.m_addedClientResourceRefs[0]);
    }

    TEST_F(AResourceChangeCollectingScene, marksResourcesObsoleteAfterDeletionOfDataInstance)
    {
        const RenderableHandle renderable = createRenderable();
        const DataInstanceHandle dataInstance = createVertexDataInstance(renderable);
        const ResourceContentHash hash(123u, 0);
        scene.setDataResource(dataInstance, vertAttribField, hash, DataBufferHandle::Invalid(), 0u);

        scene.clearResourceChanges();

        scene.releaseDataInstance(dataInstance);
        EXPECT_EQ(1u, resourceChanges.m_removedClientResourceRefs.size());
        EXPECT_EQ(hash, resourceChanges.m_removedClientResourceRefs[0]);
    }

    TEST_F(AResourceChangeCollectingScene, marksEffectNewIfSetOnRenderable)
    {
        const RenderableHandle renderable = createRenderable();
        const ResourceContentHash hash(123u, 0);
        scene.setRenderableEffect(renderable, hash);

        EXPECT_EQ(1u, resourceChanges.m_addedClientResourceRefs.size());
        EXPECT_EQ(hash, resourceChanges.m_addedClientResourceRefs[0]);
    }

    TEST_F(AResourceChangeCollectingScene, marksEffectObsoleteIfRenderableUsingItDeleted)
    {
        const RenderableHandle renderable = createRenderable();
        const ResourceContentHash hash(123u, 0);
        scene.setRenderableEffect(renderable, hash);

        scene.clearResourceChanges();

        scene.releaseRenderable(renderable);
        EXPECT_EQ(1u, resourceChanges.m_removedClientResourceRefs.size());
        EXPECT_EQ(hash, resourceChanges.m_removedClientResourceRefs[0]);
    }

    TEST_F(AResourceChangeCollectingScene, marksResourcesObsoleteOnlyWhenUsageDropsToZero)
    {
        const RenderableHandle renderable = createRenderable();
        const DataInstanceHandle dataInstance1 = createVertexDataInstance(renderable);
        const DataInstanceHandle dataInstance2 = createVertexDataInstance(renderable);
        const ResourceContentHash hash(123u, 0);
        scene.setDataResource(dataInstance1, vertAttribField, hash, DataBufferHandle::Invalid(), 0u);
        scene.setDataResource(dataInstance2, vertAttribField, hash, DataBufferHandle::Invalid(), 0u);
        expectSameSceneResourceChangesWhenExtractedFromScene();

        scene.clearResourceChanges();

        scene.releaseDataInstance(dataInstance1);
        EXPECT_EQ(0u, resourceChanges.m_removedClientResourceRefs.size());

        scene.releaseDataInstance(dataInstance2);
        EXPECT_EQ(1u, resourceChanges.m_removedClientResourceRefs.size());
        EXPECT_EQ(hash, resourceChanges.m_removedClientResourceRefs[0]);
    }

    TEST_F(AResourceChangeCollectingScene, doesNotMarkResourcesAsObsoleteNorNewIfItWasAddedAndReleasedInOneCycle)
    {
        const RenderableHandle renderable = createRenderable();
        const DataInstanceHandle dataInstance1 = createVertexDataInstance(renderable);
        const DataInstanceHandle dataInstance2 = createVertexDataInstance(renderable);
        const ResourceContentHash hash(123u, 0);

        scene.clearResourceChanges();

        scene.setDataResource(dataInstance1, vertAttribField, hash, DataBufferHandle::Invalid(), 0u);
        scene.setDataResource(dataInstance2, vertAttribField, hash, DataBufferHandle::Invalid(), 0u);
        scene.releaseDataInstance(dataInstance1);
        scene.releaseDataInstance(dataInstance2);

        EXPECT_EQ(0u, resourceChanges.m_addedClientResourceRefs.size());
        EXPECT_EQ(0u, resourceChanges.m_removedClientResourceRefs.size());
        expectSameSceneResourceChangesWhenExtractedFromScene();
    }

    TEST_F(AResourceChangeCollectingScene, doesNotMarkResourcesAsObsoleteNorNewIfItWasReleasedAndAddedInOneCycle)
    {
        const RenderableHandle renderable = createRenderable();
        const DataInstanceHandle dataInstance1 = createVertexDataInstance(renderable);
        const DataInstanceHandle dataInstance2 = createVertexDataInstance(renderable);
        const ResourceContentHash hash(123u, 0);

        scene.setDataResource(dataInstance1, vertAttribField, hash, DataBufferHandle::Invalid(), 0u);
        scene.setDataResource(dataInstance2, vertAttribField, hash, DataBufferHandle::Invalid(), 0u);

        scene.clearResourceChanges();

        scene.setDataResource(dataInstance1, vertAttribField, ResourceContentHash::Invalid(), DataBufferHandle(0u), 0u);
        scene.setDataResource(dataInstance2, vertAttribField, ResourceContentHash::Invalid(), DataBufferHandle(0u), 0u); // now marked as obsolete
        scene.setDataResource(dataInstance1, vertAttribField, hash, DataBufferHandle::Invalid(), 0u);
        scene.setDataResource(dataInstance2, vertAttribField, hash, DataBufferHandle::Invalid(), 0u);

        EXPECT_EQ(0u, resourceChanges.m_addedClientResourceRefs.size());
        EXPECT_EQ(0u, resourceChanges.m_removedClientResourceRefs.size());
    }

    TEST_F(AResourceChangeCollectingScene, marksResourcesObsoleteOnlyWhenUsageDropsToZero_DifferentDivisor)
    {
        const RenderableHandle renderable = createRenderable();
        const DataInstanceHandle dataInstance1 = createVertexDataInstance(renderable);
        const DataInstanceHandle dataInstance2 = createVertexDataInstance(renderable);
        const ResourceContentHash hash(123u, 0);
        scene.setDataResource(dataInstance1, vertAttribField, hash, DataBufferHandle::Invalid(), 0u);
        scene.setDataResource(dataInstance2, vertAttribField, hash, DataBufferHandle::Invalid(), 1u);

        scene.clearResourceChanges();

        scene.releaseDataInstance(dataInstance1);
        EXPECT_EQ(0u, resourceChanges.m_removedClientResourceRefs.size());

        scene.releaseDataInstance(dataInstance2);
        EXPECT_EQ(1u, resourceChanges.m_removedClientResourceRefs.size());
        EXPECT_EQ(hash, resourceChanges.m_removedClientResourceRefs[0]);
    }

    TEST_F(AResourceChangeCollectingScene, doesNotMarkResourcesAsObsoleteNorNewIfItWasAddedAndReleasedInOneCycle_DifferentDivisor)
    {
        const RenderableHandle renderable = createRenderable();
        const DataInstanceHandle dataInstance1 = createVertexDataInstance(renderable);
        const DataInstanceHandle dataInstance2 = createVertexDataInstance(renderable);
        const ResourceContentHash hash(123u, 0);

        scene.clearResourceChanges();

        scene.setDataResource(dataInstance1, vertAttribField, hash, DataBufferHandle::Invalid(), 0u);
        scene.setDataResource(dataInstance2, vertAttribField, hash, DataBufferHandle::Invalid(), 1u);
        scene.releaseDataInstance(dataInstance1);
        scene.releaseDataInstance(dataInstance2);

        EXPECT_EQ(0u, resourceChanges.m_addedClientResourceRefs.size());
        EXPECT_EQ(0u, resourceChanges.m_removedClientResourceRefs.size());
    }

    TEST_F(AResourceChangeCollectingScene, doesNotMarkResourcesAsObsoleteNorNewIfItWasReleasedAndAddedInOneCycle_DifferentDivisor)
    {
        const RenderableHandle renderable = createRenderable();
        const DataInstanceHandle dataInstance1 = createVertexDataInstance(renderable);
        const DataInstanceHandle dataInstance2 = createVertexDataInstance(renderable);
        const ResourceContentHash hash(123u, 0);

        scene.setDataResource(dataInstance1, vertAttribField, hash, DataBufferHandle::Invalid(), 0u);
        scene.setDataResource(dataInstance2, vertAttribField, hash, DataBufferHandle::Invalid(), 1u);

        scene.clearResourceChanges();

        scene.setDataResource(dataInstance1, vertAttribField, ResourceContentHash::Invalid(), DataBufferHandle(0u), 0u);
        scene.setDataResource(dataInstance2, vertAttribField, ResourceContentHash::Invalid(), DataBufferHandle(0u), 0u); // now marked as obsolete
        scene.setDataResource(dataInstance1, vertAttribField, hash, DataBufferHandle::Invalid(), 0u);
        scene.setDataResource(dataInstance2, vertAttribField, hash, DataBufferHandle::Invalid(), 1u);

        EXPECT_EQ(0u, resourceChanges.m_addedClientResourceRefs.size());
        EXPECT_EQ(0u, resourceChanges.m_removedClientResourceRefs.size());
    }

    TEST_F(AResourceChangeCollectingScene, createdRenderTargetIsTracked)
    {
        const RenderTargetHandle handle = scene.allocateRenderTarget();
        ASSERT_EQ(1u, resourceChanges.m_sceneResourceActions.size());
        EXPECT_EQ(handle, resourceChanges.m_sceneResourceActions[0].handle);
        EXPECT_EQ(ESceneResourceAction_CreateRenderTarget, resourceChanges.m_sceneResourceActions[0].action);

        scene.clearResourceChanges();
        EXPECT_EQ(0u, resourceChanges.m_sceneResourceActions.size());
    }

    TEST_F(AResourceChangeCollectingScene, destroyedRenderTargetIsTracked)
    {
        const RenderTargetHandle targetHandle = scene.allocateRenderTarget();
        scene.clearResourceChanges();

        scene.releaseRenderTarget(targetHandle);
        ASSERT_EQ(1u, resourceChanges.m_sceneResourceActions.size());
        EXPECT_EQ(targetHandle, resourceChanges.m_sceneResourceActions[0].handle);
        EXPECT_EQ(ESceneResourceAction_DestroyRenderTarget, resourceChanges.m_sceneResourceActions[0].action);

        scene.clearResourceChanges();
        EXPECT_EQ(0u, resourceChanges.m_sceneResourceActions.size());
    }

    TEST_F(AResourceChangeCollectingScene, createdRenderBufferIsTracked)
    {
        const RenderBufferHandle handle = scene.allocateRenderBuffer({ 1u, 1u, ERenderBufferType_ColorBuffer, ETextureFormat_R8, ERenderBufferAccessMode_ReadWrite, 0u });
        ASSERT_EQ(1u, resourceChanges.m_sceneResourceActions.size());
        EXPECT_EQ(handle, resourceChanges.m_sceneResourceActions[0].handle);
        EXPECT_EQ(ESceneResourceAction_CreateRenderBuffer, resourceChanges.m_sceneResourceActions[0].action);

        scene.clearResourceChanges();
        EXPECT_EQ(0u, resourceChanges.m_sceneResourceActions.size());
    }

    TEST_F(AResourceChangeCollectingScene, destroyedRenderBufferIsTracked)
    {
        const RenderBufferHandle bufferHandle = scene.allocateRenderBuffer({ 1u, 1u, ERenderBufferType_ColorBuffer, ETextureFormat_R8, ERenderBufferAccessMode_ReadWrite, 0u });
        scene.clearResourceChanges();

        scene.releaseRenderBuffer(bufferHandle);
        ASSERT_EQ(1u, resourceChanges.m_sceneResourceActions.size());
        EXPECT_EQ(bufferHandle, resourceChanges.m_sceneResourceActions[0].handle);
        EXPECT_EQ(ESceneResourceAction_DestroyRenderBuffer, resourceChanges.m_sceneResourceActions[0].action);

        scene.clearResourceChanges();
        EXPECT_EQ(0u, resourceChanges.m_sceneResourceActions.size());
    }

    TEST_F(AResourceChangeCollectingScene, createdStreamTextureCollectsBothSceneResourceActionAndFallbackTexture)
    {
        const ResourceContentHash fallbackTex(1u, 2u);
        const StreamTextureHandle handle = scene.allocateStreamTexture(1u, fallbackTex);

        ASSERT_EQ(1u, resourceChanges.m_sceneResourceActions.size());
        EXPECT_EQ(handle, resourceChanges.m_sceneResourceActions[0].handle);
        EXPECT_EQ(ESceneResourceAction_CreateStreamTexture, resourceChanges.m_sceneResourceActions[0].action);
        expectSameSceneResourceChangesWhenExtractedFromScene();

        ASSERT_EQ(1u, resourceChanges.m_addedClientResourceRefs.size());
        EXPECT_TRUE(resourceChanges.m_removedClientResourceRefs.empty());
        EXPECT_EQ(fallbackTex, resourceChanges.m_addedClientResourceRefs.front());
    }

    TEST_F(AResourceChangeCollectingScene, createdStreamTextureDoesNotMarkFallbackTextureAsNewIfItWasAlreadyUsedInScene)
    {
        const ResourceContentHash fallbackTex(1u, 2u);
        scene.allocateTextureSampler({ {}, fallbackTex });
        expectSameSceneResourceChangesWhenExtractedFromScene();
        scene.clearResourceChanges();

        const StreamTextureHandle handle = scene.allocateStreamTexture(1u, fallbackTex);

        ASSERT_EQ(1u, resourceChanges.m_sceneResourceActions.size());
        EXPECT_EQ(handle, resourceChanges.m_sceneResourceActions[0].handle);
        EXPECT_EQ(ESceneResourceAction_CreateStreamTexture, resourceChanges.m_sceneResourceActions[0].action);

        EXPECT_TRUE(resourceChanges.m_removedClientResourceRefs.empty());
        EXPECT_TRUE(resourceChanges.m_addedClientResourceRefs.empty());
    }

    TEST_F(AResourceChangeCollectingScene, destroyedStreamTextureCollectsBothSceneResourceActionAndFallbackTexture)
    {
        const ResourceContentHash fallbackTex(1u, 2u);
        const StreamTextureHandle handle = scene.allocateStreamTexture(1u, fallbackTex);
        scene.clearResourceChanges();

        scene.releaseStreamTexture(handle);
        ASSERT_EQ(1u, resourceChanges.m_sceneResourceActions.size());
        EXPECT_EQ(handle, resourceChanges.m_sceneResourceActions[0].handle);
        EXPECT_EQ(ESceneResourceAction_DestroyStreamTexture, resourceChanges.m_sceneResourceActions[0].action);

        ASSERT_EQ(1u, resourceChanges.m_removedClientResourceRefs.size());
        EXPECT_TRUE(resourceChanges.m_addedClientResourceRefs.empty());
        EXPECT_EQ(fallbackTex, resourceChanges.m_removedClientResourceRefs.front());
    }

    TEST_F(AResourceChangeCollectingScene, destroyedStreamTextureDoesNotMarkFallbackTextureAsRemovedIfItIsStillUsedInScene)
    {
        const ResourceContentHash fallbackTex(1u, 2u);
        scene.allocateTextureSampler({ {}, fallbackTex });
        const StreamTextureHandle handle = scene.allocateStreamTexture(1u, fallbackTex);
        expectSameSceneResourceChangesWhenExtractedFromScene();
        scene.clearResourceChanges();

        scene.releaseStreamTexture(handle);
        ASSERT_EQ(1u, resourceChanges.m_sceneResourceActions.size());
        EXPECT_EQ(handle, resourceChanges.m_sceneResourceActions[0].handle);
        EXPECT_EQ(ESceneResourceAction_DestroyStreamTexture, resourceChanges.m_sceneResourceActions[0].action);

        EXPECT_TRUE(resourceChanges.m_removedClientResourceRefs.empty());
        EXPECT_TRUE(resourceChanges.m_addedClientResourceRefs.empty());
    }

    TEST_F(AResourceChangeCollectingScene, createdBlitPassIsTracked)
    {
        const BlitPassHandle handle = scene.allocateBlitPass(RenderBufferHandle(0u), RenderBufferHandle(1u));
        ASSERT_EQ(1u, resourceChanges.m_sceneResourceActions.size());
        EXPECT_EQ(handle, resourceChanges.m_sceneResourceActions[0].handle);
        EXPECT_EQ(ESceneResourceAction_CreateBlitPass, resourceChanges.m_sceneResourceActions[0].action);
        expectSameSceneResourceChangesWhenExtractedFromScene();

        scene.clearResourceChanges();
        EXPECT_EQ(0u, resourceChanges.m_sceneResourceActions.size());
    }

    TEST_F(AResourceChangeCollectingScene, destroyedBlitPassIsTracked)
    {
        const BlitPassHandle handle = scene.allocateBlitPass(RenderBufferHandle(0u), RenderBufferHandle(1u));
        scene.clearResourceChanges();

        scene.releaseBlitPass(handle);
        ASSERT_EQ(1u, resourceChanges.m_sceneResourceActions.size());
        EXPECT_EQ(handle, resourceChanges.m_sceneResourceActions[0].handle);
        EXPECT_EQ(ESceneResourceAction_DestroyBlitPass, resourceChanges.m_sceneResourceActions[0].action);

        scene.clearResourceChanges();
        EXPECT_EQ(0u, resourceChanges.m_sceneResourceActions.size());
    }

    TEST_F(AResourceChangeCollectingScene, createdDataBufferIsTracked)
    {
        const DataBufferHandle handle = scene.allocateDataBuffer(EDataBufferType::IndexBuffer, EDataType_UInt32, 10u);
        ASSERT_EQ(1u, resourceChanges.m_sceneResourceActions.size());
        EXPECT_EQ(handle, resourceChanges.m_sceneResourceActions[0].handle);
        EXPECT_EQ(ESceneResourceAction_CreateDataBuffer, resourceChanges.m_sceneResourceActions[0].action);

        scene.clearResourceChanges();
        EXPECT_EQ(0u, resourceChanges.m_sceneResourceActions.size());
    }

    TEST_F(AResourceChangeCollectingScene, createdAndUpdatedDataBufferIsTrackedAndSameAsExtractedFromScene)
    {
        const DataBufferHandle handle = scene.allocateDataBuffer(EDataBufferType::IndexBuffer, EDataType_UInt32, 10u);
        scene.updateDataBuffer(handle, 0, 0, nullptr);
        ASSERT_EQ(2u, resourceChanges.m_sceneResourceActions.size());
        EXPECT_EQ(handle, resourceChanges.m_sceneResourceActions[0].handle);
        EXPECT_EQ(ESceneResourceAction_CreateDataBuffer, resourceChanges.m_sceneResourceActions[0].action);
        EXPECT_EQ(handle, resourceChanges.m_sceneResourceActions[1].handle);
        EXPECT_EQ(ESceneResourceAction_UpdateDataBuffer, resourceChanges.m_sceneResourceActions[1].action);
        expectSameSceneResourceChangesWhenExtractedFromScene();

        scene.clearResourceChanges();
        EXPECT_EQ(0u, resourceChanges.m_sceneResourceActions.size());
    }

    TEST_F(AResourceChangeCollectingScene, destroyedDataBufferIsTracked)
    {
        const DataBufferHandle handle = scene.allocateDataBuffer(EDataBufferType::IndexBuffer, EDataType_UInt32, 10u);
        scene.clearResourceChanges();

        scene.releaseDataBuffer(handle);
        ASSERT_EQ(1u, resourceChanges.m_sceneResourceActions.size());
        EXPECT_EQ(handle, resourceChanges.m_sceneResourceActions[0].handle);
        EXPECT_EQ(ESceneResourceAction_DestroyDataBuffer, resourceChanges.m_sceneResourceActions[0].action);

        scene.clearResourceChanges();
        EXPECT_EQ(0u, resourceChanges.m_sceneResourceActions.size());
    }

    TEST_F(AResourceChangeCollectingScene, updatedDataBufferIsTracked)
    {
        const DataBufferHandle handle = scene.allocateDataBuffer(EDataBufferType::IndexBuffer, EDataType_UInt32, 10u);
        scene.clearResourceChanges();

        scene.updateDataBuffer(handle, 0u, 0u, nullptr);
        ASSERT_EQ(1u, resourceChanges.m_sceneResourceActions.size());
        EXPECT_EQ(handle, resourceChanges.m_sceneResourceActions[0].handle);
        EXPECT_EQ(ESceneResourceAction_UpdateDataBuffer, resourceChanges.m_sceneResourceActions[0].action);

        scene.clearResourceChanges();
        EXPECT_EQ(0u, resourceChanges.m_sceneResourceActions.size());
    }

    TEST_F(AResourceChangeCollectingScene, createdTextureBufferIsTracked)
    {
        const TextureBufferHandle handle = scene.allocateTextureBuffer(ETextureFormat_R16F, { { 1, 1 } });
        ASSERT_EQ(1u, resourceChanges.m_sceneResourceActions.size());
        EXPECT_EQ(handle, resourceChanges.m_sceneResourceActions[0].handle);
        EXPECT_EQ(ESceneResourceAction_CreateTextureBuffer, resourceChanges.m_sceneResourceActions[0].action);

        scene.clearResourceChanges();
        EXPECT_EQ(0u, resourceChanges.m_sceneResourceActions.size());
    }

    TEST_F(AResourceChangeCollectingScene, createdAndUpdatedTextureBufferIsTrackedAndSameAsExtractedFromScene)
    {
        const TextureBufferHandle handle = scene.allocateTextureBuffer(ETextureFormat_R16F, { { 1, 1 } });
        scene.updateTextureBuffer(handle, 0, 0, 0, 0, 0, nullptr);
        ASSERT_EQ(2u, resourceChanges.m_sceneResourceActions.size());
        EXPECT_EQ(handle, resourceChanges.m_sceneResourceActions[0].handle);
        EXPECT_EQ(ESceneResourceAction_CreateTextureBuffer, resourceChanges.m_sceneResourceActions[0].action);
        EXPECT_EQ(handle, resourceChanges.m_sceneResourceActions[1].handle);
        EXPECT_EQ(ESceneResourceAction_UpdateTextureBuffer, resourceChanges.m_sceneResourceActions[1].action);
        expectSameSceneResourceChangesWhenExtractedFromScene();

        scene.clearResourceChanges();
        EXPECT_EQ(0u, resourceChanges.m_sceneResourceActions.size());
    }

    TEST_F(AResourceChangeCollectingScene, destroyedTextureBufferIsTracked)
    {
        const TextureBufferHandle handle = scene.allocateTextureBuffer(ETextureFormat_R16F, { {1, 1} });
        scene.clearResourceChanges();

        scene.releaseTextureBuffer(handle);
        ASSERT_EQ(1u, resourceChanges.m_sceneResourceActions.size());
        EXPECT_EQ(handle, resourceChanges.m_sceneResourceActions[0].handle);
        EXPECT_EQ(ESceneResourceAction_DestroyTextureBuffer, resourceChanges.m_sceneResourceActions[0].action);

        scene.clearResourceChanges();
        EXPECT_EQ(0u, resourceChanges.m_sceneResourceActions.size());
    }

    TEST_F(AResourceChangeCollectingScene, updatedTextureBufferIsTracked)
    {
        const TextureBufferHandle handle = scene.allocateTextureBuffer(ETextureFormat_R16F, { { 1, 1 } });
        scene.clearResourceChanges();

        const Byte dummyData[2] = {0}; // width*height*sizeof(float16) bytes needed
        scene.updateTextureBuffer(handle, 0u, 0u, 0u, 1u, 1u, dummyData);
        ASSERT_EQ(1u, resourceChanges.m_sceneResourceActions.size());
        EXPECT_EQ(handle, resourceChanges.m_sceneResourceActions[0].handle);
        EXPECT_EQ(ESceneResourceAction_UpdateTextureBuffer, resourceChanges.m_sceneResourceActions[0].action);

        scene.clearResourceChanges();
        EXPECT_EQ(0u, resourceChanges.m_sceneResourceActions.size());
    }

    TEST_F(AResourceChangeCollectingScene, marksTextureResourceNewWhenUsedByCreatedDataSlot)
    {
        const ResourceContentHash hash(1234u, 0);
        scene.allocateDataSlot({ EDataSlotType_TextureProvider, DataSlotId(0u), NodeHandle(), DataInstanceHandle(), hash, TextureSamplerHandle() });

        ASSERT_EQ(1u, resourceChanges.m_addedClientResourceRefs.size());
        EXPECT_EQ(hash, resourceChanges.m_addedClientResourceRefs.front());
        EXPECT_EQ(0u, resourceChanges.m_removedClientResourceRefs.size());
        expectSameSceneResourceChangesWhenExtractedFromScene();
    }

    TEST_F(AResourceChangeCollectingScene, marksTextureResourceNewWhenUsedBySetDataSlotTexture)
    {
        const ResourceContentHash hash1(1234u, 0);
        const ResourceContentHash hash2(5678u, 0);
        const auto slotHandle = scene.allocateDataSlot({ EDataSlotType_TextureProvider, DataSlotId(0u), NodeHandle(), DataInstanceHandle(), hash1, TextureSamplerHandle() });
        scene.setDataSlotTexture(slotHandle, hash2);

        ASSERT_EQ(1u, resourceChanges.m_addedClientResourceRefs.size());
        EXPECT_EQ(hash2, resourceChanges.m_addedClientResourceRefs.front());
        EXPECT_EQ(0u, resourceChanges.m_removedClientResourceRefs.size());
        expectSameSceneResourceChangesWhenExtractedFromScene();
    }

    TEST_F(AResourceChangeCollectingScene, marksTextureResourceObsoleteTextureSamplerReleased)
    {
        const ResourceContentHash hash(1234u, 0);
        const TextureSamplerHandle sampler = scene.allocateTextureSampler({ {}, hash });
        expectSameSceneResourceChangesWhenExtractedFromScene();
        scene.clearResourceChanges();

        scene.releaseTextureSampler(sampler);

        ASSERT_EQ(0u, resourceChanges.m_addedClientResourceRefs.size());
        EXPECT_EQ(1u, resourceChanges.m_removedClientResourceRefs.size());
        EXPECT_EQ(hash, resourceChanges.m_removedClientResourceRefs.front());
    }

    TEST_F(AResourceChangeCollectingScene, marksTextureResourceNewWhenUsedByTextureSampler)
    {
        const ResourceContentHash hash(1234u, 0);
        scene.allocateTextureSampler({ {}, hash });

        ASSERT_EQ(1u, resourceChanges.m_addedClientResourceRefs.size());
        EXPECT_EQ(hash, resourceChanges.m_addedClientResourceRefs.front());
        EXPECT_EQ(0u, resourceChanges.m_removedClientResourceRefs.size());
        expectSameSceneResourceChangesWhenExtractedFromScene();
    }

    TEST_F(AResourceChangeCollectingScene, marksTextureObsoleteWhenUsedByReleasedDataSlot)
    {
        const ResourceContentHash hash(1234u, 0);
        const DataSlotHandle dataSlot = scene.allocateDataSlot({ EDataSlotType_TextureProvider, DataSlotId(0u), NodeHandle(), DataInstanceHandle(), hash, TextureSamplerHandle() });
        scene.clearResourceChanges();

        scene.releaseDataSlot(dataSlot);

        EXPECT_EQ(0u, resourceChanges.m_addedClientResourceRefs.size());
        ASSERT_EQ(1u, resourceChanges.m_removedClientResourceRefs.size());
        EXPECT_EQ(hash, resourceChanges.m_removedClientResourceRefs.front());
    }

    TEST_F(AResourceChangeCollectingScene, doesNotMarkTextureNewWhenUsedByCreatedDataSlotButAlreadyUsedBefore)
    {
        const ResourceContentHash hash(1234u, 0);
        scene.allocateTextureSampler({ {}, hash });
        scene.clearResourceChanges();

        scene.allocateDataSlot({ EDataSlotType_TextureProvider, DataSlotId(0u), NodeHandle(), DataInstanceHandle(), hash, TextureSamplerHandle() });

        EXPECT_EQ(0u, resourceChanges.m_addedClientResourceRefs.size());
        EXPECT_EQ(0u, resourceChanges.m_removedClientResourceRefs.size());
    }

    TEST_F(AResourceChangeCollectingScene, doesNotMarkTextureObsoleteWhenUsedByDestroyedDataSlotButStillInUse)
    {
        const ResourceContentHash hash(1234u, 0);
        scene.allocateTextureSampler({ {}, hash });

        const DataSlotHandle dataSlot = scene.allocateDataSlot({ EDataSlotType_TextureProvider, DataSlotId(0u), NodeHandle(), DataInstanceHandle(), hash, TextureSamplerHandle() });
        expectSameSceneResourceChangesWhenExtractedFromScene();
        scene.clearResourceChanges();

        scene.releaseDataSlot(dataSlot);
        EXPECT_EQ(0u, resourceChanges.m_addedClientResourceRefs.size());
        EXPECT_EQ(0u, resourceChanges.m_removedClientResourceRefs.size());
    }
}
