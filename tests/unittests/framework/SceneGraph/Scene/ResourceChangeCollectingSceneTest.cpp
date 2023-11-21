//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "internal/SceneGraph/Scene/ResourceChangeCollectingScene.h"
#include "internal/SceneGraph/SceneUtils/ResourceUtils.h"

namespace ramses::internal
{
    class AResourceChangeCollectingScene : public testing::Test
    {
    public:
        AResourceChangeCollectingScene()
            : sceneResourceActions(scene.getSceneResourceActions())
            , testUniformLayout(0u)
            , testGeometryLayout(2u)
            , indicesField(0u)
            , vertAttribField(1u)
            , dataField(0u)
            , samplerField(1u)
        {
            // DataLayout triggers marking its effect hash as new resource, in order not to affect all the test cases here
            // the effect hash used by these default layouts is 'invalid'
            DataFieldInfoVector geometryDataFields(2u);
            geometryDataFields[indicesField.asMemoryHandle()] = DataFieldInfo(EDataType::Indices, 1u, EFixedSemantics::Indices);
            geometryDataFields[vertAttribField.asMemoryHandle()] = DataFieldInfo(EDataType::Vector3Buffer, 1u, EFixedSemantics::Invalid);
            scene.allocateDataLayout(geometryDataFields, ResourceContentHash::Invalid(), testGeometryLayout);

            DataFieldInfoVector uniformDataFields(2u);
            uniformDataFields[dataField.asMemoryHandle()] = DataFieldInfo(EDataType::Float);
            uniformDataFields[samplerField.asMemoryHandle()] = DataFieldInfo(EDataType::TextureSampler2D);
            scene.allocateDataLayout(uniformDataFields, ResourceContentHash::Invalid(), testUniformLayout);

            scene.resetResourceChanges();
        }

    protected:
        RenderableHandle createRenderable()
        {
            return scene.allocateRenderable(scene.allocateNode(0, {}), {});
        }

        DataInstanceHandle createUniformDataInstanceWithSampler(RenderableHandle renderable, ResourceContentHash texHash)
        {
            const DataInstanceHandle uniformData = scene.allocateDataInstance(testUniformLayout, {});
            scene.setRenderableDataInstance(renderable, ERenderableDataSlotType_Uniforms, uniformData);
            const TextureSamplerHandle sampler = scene.allocateTextureSampler({{}, texHash}, {});
            scene.setDataTextureSamplerHandle(uniformData, samplerField, sampler);
            return uniformData;
        }

        DataInstanceHandle createVertexDataInstance(RenderableHandle renderable)
        {
            const DataInstanceHandle geometryData = scene.allocateDataInstance(testGeometryLayout, {});
            scene.setRenderableDataInstance(renderable, ERenderableDataSlotType_Geometry, geometryData);

            return geometryData;
        }

        void expectSameSceneResourceChangesWhenExtractedFromScene(size_t expectedSceneResourcesByteSize = 0u)
        {
            SceneResourceActionVector fromScene;
            size_t fromSceneSceneResourcesByteSize = 0u;
            ResourceUtils::GetAllSceneResourcesFromScene(fromScene, scene, fromSceneSceneResourcesByteSize);

            EXPECT_EQ(sceneResourceActions.size(), fromScene.size());
            EXPECT_EQ(sceneResourceActions, fromScene);

            EXPECT_EQ(expectedSceneResourcesByteSize, fromSceneSceneResourcesByteSize);
        }

        ResourceChangeCollectingScene scene;
        const SceneResourceActionVector& sceneResourceActions;

        const DataLayoutHandle testUniformLayout;
        const DataLayoutHandle testGeometryLayout;
        const DataFieldHandle indicesField;
        const DataFieldHandle vertAttribField;
        const DataFieldHandle dataField;
        const DataFieldHandle samplerField;
    };

    TEST_F(AResourceChangeCollectingScene, createdRenderTargetIsTracked)
    {
        const RenderTargetHandle handle = scene.allocateRenderTarget({});
        ASSERT_EQ(1u, sceneResourceActions.size());
        EXPECT_EQ(handle, sceneResourceActions[0].handle);
        EXPECT_EQ(ESceneResourceAction_CreateRenderTarget, sceneResourceActions[0].action);

        scene.resetResourceChanges();
        EXPECT_EQ(0u, sceneResourceActions.size());
    }

    TEST_F(AResourceChangeCollectingScene, destroyedRenderTargetIsTracked)
    {
        const RenderTargetHandle targetHandle = scene.allocateRenderTarget({});
        scene.resetResourceChanges();

        scene.releaseRenderTarget(targetHandle);
        ASSERT_EQ(1u, sceneResourceActions.size());
        EXPECT_EQ(targetHandle, sceneResourceActions[0].handle);
        EXPECT_EQ(ESceneResourceAction_DestroyRenderTarget, sceneResourceActions[0].action);

        scene.resetResourceChanges();
        EXPECT_EQ(0u, sceneResourceActions.size());
    }

    TEST_F(AResourceChangeCollectingScene, createdRenderBufferIsTracked)
    {
        const RenderBufferHandle handle = scene.allocateRenderBuffer({ 1u, 1u, EPixelStorageFormat::R8, ERenderBufferAccessMode::ReadWrite, 0u }, {});
        ASSERT_EQ(1u, sceneResourceActions.size());
        EXPECT_EQ(handle, sceneResourceActions[0].handle);
        EXPECT_EQ(ESceneResourceAction_CreateRenderBuffer, sceneResourceActions[0].action);

        scene.resetResourceChanges();
        EXPECT_EQ(0u, sceneResourceActions.size());
    }

    TEST_F(AResourceChangeCollectingScene, destroyedRenderBufferIsTracked)
    {
        const RenderBufferHandle bufferHandle = scene.allocateRenderBuffer({ 1u, 1u, EPixelStorageFormat::R8, ERenderBufferAccessMode::ReadWrite, 0u }, {});
        scene.resetResourceChanges();

        scene.releaseRenderBuffer(bufferHandle);
        ASSERT_EQ(1u, sceneResourceActions.size());
        EXPECT_EQ(bufferHandle, sceneResourceActions[0].handle);
        EXPECT_EQ(ESceneResourceAction_DestroyRenderBuffer, sceneResourceActions[0].action);

        scene.resetResourceChanges();
        EXPECT_EQ(0u, sceneResourceActions.size());
    }

    TEST_F(AResourceChangeCollectingScene, setRenderBufferPropertiesIsTracked)
    {
        const RenderBufferHandle bufferHandle = scene.allocateRenderBuffer({ 1u, 1u, EPixelStorageFormat::R8, ERenderBufferAccessMode::ReadWrite, 0u }, {});
        scene.resetResourceChanges();

        scene.setRenderBufferProperties(bufferHandle, 1u, 2u, 3u);
        ASSERT_EQ(1u, sceneResourceActions.size());
        EXPECT_EQ(bufferHandle, sceneResourceActions[0].handle);
        EXPECT_EQ(ESceneResourceAction_UpdateRenderBufferProperties, sceneResourceActions[0].action);
        EXPECT_TRUE(scene.haveResourcesChanged());

        scene.resetResourceChanges();
        EXPECT_EQ(0u, sceneResourceActions.size());
    }

    TEST_F(AResourceChangeCollectingScene, createdBlitPassIsTracked)
    {
        const BlitPassHandle handle = scene.allocateBlitPass(RenderBufferHandle(0u), RenderBufferHandle(1u), {});
        ASSERT_EQ(1u, sceneResourceActions.size());
        EXPECT_EQ(handle, sceneResourceActions[0].handle);
        EXPECT_EQ(ESceneResourceAction_CreateBlitPass, sceneResourceActions[0].action);
        expectSameSceneResourceChangesWhenExtractedFromScene();

        scene.resetResourceChanges();
        EXPECT_EQ(0u, sceneResourceActions.size());
    }

    TEST_F(AResourceChangeCollectingScene, destroyedBlitPassIsTracked)
    {
        const BlitPassHandle handle = scene.allocateBlitPass(RenderBufferHandle(0u), RenderBufferHandle(1u), {});
        scene.resetResourceChanges();

        scene.releaseBlitPass(handle);
        ASSERT_EQ(1u, sceneResourceActions.size());
        EXPECT_EQ(handle, sceneResourceActions[0].handle);
        EXPECT_EQ(ESceneResourceAction_DestroyBlitPass, sceneResourceActions[0].action);

        scene.resetResourceChanges();
        EXPECT_EQ(0u, sceneResourceActions.size());
    }

    TEST_F(AResourceChangeCollectingScene, createdDataBufferIsTracked)
    {
        const DataBufferHandle handle = scene.allocateDataBuffer(EDataBufferType::IndexBuffer, EDataType::UInt32, 10u, {});
        ASSERT_EQ(1u, sceneResourceActions.size());
        EXPECT_EQ(handle, sceneResourceActions[0].handle);
        EXPECT_EQ(ESceneResourceAction_CreateDataBuffer, sceneResourceActions[0].action);

        scene.resetResourceChanges();
        EXPECT_EQ(0u, sceneResourceActions.size());
    }

    TEST_F(AResourceChangeCollectingScene, createdAndUpdatedDataBufferIsTrackedAndSameAsExtractedFromScene)
    {
        const DataBufferHandle handle = scene.allocateDataBuffer(EDataBufferType::IndexBuffer, EDataType::UInt32, 10u, {});
        scene.updateDataBuffer(handle, 0, 0, nullptr);
        ASSERT_EQ(2u, sceneResourceActions.size());
        EXPECT_EQ(handle, sceneResourceActions[0].handle);
        EXPECT_EQ(ESceneResourceAction_CreateDataBuffer, sceneResourceActions[0].action);
        EXPECT_EQ(handle, sceneResourceActions[1].handle);
        EXPECT_EQ(ESceneResourceAction_UpdateDataBuffer, sceneResourceActions[1].action);
        expectSameSceneResourceChangesWhenExtractedFromScene();

        scene.resetResourceChanges();
        EXPECT_EQ(0u, sceneResourceActions.size());
    }

    TEST_F(AResourceChangeCollectingScene, destroyedDataBufferIsTracked)
    {
        const DataBufferHandle handle = scene.allocateDataBuffer(EDataBufferType::IndexBuffer, EDataType::UInt32, 10u, {});
        scene.resetResourceChanges();

        scene.releaseDataBuffer(handle);
        ASSERT_EQ(1u, sceneResourceActions.size());
        EXPECT_EQ(handle, sceneResourceActions[0].handle);
        EXPECT_EQ(ESceneResourceAction_DestroyDataBuffer, sceneResourceActions[0].action);

        scene.resetResourceChanges();
        EXPECT_EQ(0u, sceneResourceActions.size());
    }

    TEST_F(AResourceChangeCollectingScene, updatedDataBufferIsTracked)
    {
        const DataBufferHandle handle = scene.allocateDataBuffer(EDataBufferType::IndexBuffer, EDataType::UInt32, 10u, {});
        scene.resetResourceChanges();

        scene.updateDataBuffer(handle, 0u, 0u, nullptr);
        ASSERT_EQ(1u, sceneResourceActions.size());
        EXPECT_EQ(handle, sceneResourceActions[0].handle);
        EXPECT_EQ(ESceneResourceAction_UpdateDataBuffer, sceneResourceActions[0].action);

        scene.resetResourceChanges();
        EXPECT_EQ(0u, sceneResourceActions.size());
    }

    TEST_F(AResourceChangeCollectingScene, createdTextureBufferIsTracked)
    {
        const TextureBufferHandle handle = scene.allocateTextureBuffer(EPixelStorageFormat::R16F, { { 1, 1 } }, {});
        ASSERT_EQ(1u, sceneResourceActions.size());
        EXPECT_EQ(handle, sceneResourceActions[0].handle);
        EXPECT_EQ(ESceneResourceAction_CreateTextureBuffer, sceneResourceActions[0].action);

        scene.resetResourceChanges();
        EXPECT_EQ(0u, sceneResourceActions.size());
    }

    TEST_F(AResourceChangeCollectingScene, createdAndUpdatedTextureBufferIsTrackedAndSameAsExtractedFromScene)
    {
        const TextureBufferHandle handle = scene.allocateTextureBuffer(EPixelStorageFormat::R16F, { { 1, 1 } }, {});
        scene.updateTextureBuffer(handle, 0, 0, 0, 0, 0, nullptr);
        ASSERT_EQ(2u, sceneResourceActions.size());
        EXPECT_EQ(handle, sceneResourceActions[0].handle);
        EXPECT_EQ(ESceneResourceAction_CreateTextureBuffer, sceneResourceActions[0].action);
        EXPECT_EQ(handle, sceneResourceActions[1].handle);
        EXPECT_EQ(ESceneResourceAction_UpdateTextureBuffer, sceneResourceActions[1].action);
        expectSameSceneResourceChangesWhenExtractedFromScene(2u);

        scene.resetResourceChanges();
        EXPECT_EQ(0u, sceneResourceActions.size());
    }

    TEST_F(AResourceChangeCollectingScene, destroyedTextureBufferIsTracked)
    {
        const TextureBufferHandle handle = scene.allocateTextureBuffer(EPixelStorageFormat::R16F, { {1, 1} }, {});
        scene.resetResourceChanges();

        scene.releaseTextureBuffer(handle);
        ASSERT_EQ(1u, sceneResourceActions.size());
        EXPECT_EQ(handle, sceneResourceActions[0].handle);
        EXPECT_EQ(ESceneResourceAction_DestroyTextureBuffer, sceneResourceActions[0].action);

        scene.resetResourceChanges();
        EXPECT_EQ(0u, sceneResourceActions.size());
    }

    TEST_F(AResourceChangeCollectingScene, updatedTextureBufferIsTracked)
    {
        const TextureBufferHandle handle = scene.allocateTextureBuffer(EPixelStorageFormat::R16F, { { 1, 1 } }, {});
        scene.resetResourceChanges();

        const std::byte dummyData[2] = { std::byte{0} }; // width*height*sizeof(float16) bytes needed
        scene.updateTextureBuffer(handle, 0u, 0u, 0u, 1u, 1u, dummyData);
        ASSERT_EQ(1u, sceneResourceActions.size());
        EXPECT_EQ(handle, sceneResourceActions[0].handle);
        EXPECT_EQ(ESceneResourceAction_UpdateTextureBuffer, sceneResourceActions[0].action);

        scene.resetResourceChanges();
        EXPECT_EQ(0u, sceneResourceActions.size());
    }

    TEST_F(AResourceChangeCollectingScene, hasClientResourcesNotDirtyOnCreation)
    {
        EXPECT_FALSE(scene.haveResourcesChanged());
    }

    TEST_F(AResourceChangeCollectingScene, doesNotMarkClientResourcesDirtyOnNewRenderable)
    {
        createRenderable();
        EXPECT_FALSE(scene.haveResourcesChanged());
    }

    TEST_F(AResourceChangeCollectingScene, doesNotMarkClientResourcesDirtyOnNewDataInstance)
    {
        scene.allocateDataInstance(testUniformLayout, {});
        EXPECT_FALSE(scene.haveResourcesChanged());
    }

    TEST_F(AResourceChangeCollectingScene, marksClientResourcesDirtyWhenAddingADataInstanceToARenderable)
    {
        const RenderableHandle renderable = createRenderable();
        createVertexDataInstance(renderable);
        EXPECT_TRUE(scene.haveResourcesChanged());
    }

    TEST_F(AResourceChangeCollectingScene, marksClientResourcesNotDirtyAfterResettingResourceCollection)
    {
        const RenderableHandle renderable = createRenderable();
        createVertexDataInstance(renderable);
        EXPECT_TRUE(scene.haveResourcesChanged());
        scene.resetResourceChanges();
        EXPECT_FALSE(scene.haveResourcesChanged());
    }

    TEST_F(AResourceChangeCollectingScene, marksClientResourcesDirtyWhenReleasingARenderable)
    {
        const RenderableHandle renderable = createRenderable();
        scene.releaseRenderable(renderable);
        EXPECT_TRUE(scene.haveResourcesChanged());
    }

    TEST_F(AResourceChangeCollectingScene, marksClientResourcesDirtyWhenChangingRenderableVisibilityOnlyFromOrToOff)
    {
        const RenderableHandle renderable = createRenderable();
        scene.setRenderableVisibility(renderable, EVisibilityMode::Visible);
        EXPECT_FALSE(scene.haveResourcesChanged());
        scene.setRenderableVisibility(renderable, EVisibilityMode::Invisible);
        EXPECT_FALSE(scene.haveResourcesChanged());
        scene.setRenderableVisibility(renderable, EVisibilityMode::Visible);
        EXPECT_FALSE(scene.haveResourcesChanged());
        scene.setRenderableVisibility(renderable, EVisibilityMode::Off);
        EXPECT_TRUE(scene.haveResourcesChanged());
        scene.resetResourceChanges();
        scene.setRenderableVisibility(renderable, EVisibilityMode::Visible);
        EXPECT_TRUE(scene.haveResourcesChanged());
        scene.resetResourceChanges();
        scene.setRenderableVisibility(renderable, EVisibilityMode::Off);
        EXPECT_TRUE(scene.haveResourcesChanged());
        scene.resetResourceChanges();
        scene.setRenderableVisibility(renderable, EVisibilityMode::Invisible);
        EXPECT_TRUE(scene.haveResourcesChanged());
        scene.resetResourceChanges();
        scene.setRenderableVisibility(renderable, EVisibilityMode::Off);
        EXPECT_TRUE(scene.haveResourcesChanged());
    }

    TEST_F(AResourceChangeCollectingScene, marksClientResourcesDirtyWhenSettingADataResource)
    {
        const RenderableHandle renderable = createRenderable();
        const DataInstanceHandle dataInstance = createVertexDataInstance(renderable);
        scene.resetResourceChanges();

        scene.setDataResource(dataInstance, vertAttribField, { 123, 0 }, DataBufferHandle::Invalid(), 0u, 0u, 0u);
        EXPECT_TRUE(scene.haveResourcesChanged());

        scene.resetResourceChanges();
        scene.setDataResource(dataInstance, indicesField, { 456, 0 }, DataBufferHandle::Invalid(), 0u, 0u, 0u);
        EXPECT_TRUE(scene.haveResourcesChanged());
    }

    TEST_F(AResourceChangeCollectingScene, marksClientResourcesDirtyWhenAllocatingATextureSampler)
    {
        scene.allocateTextureSampler({ {}, { 123, 0 } }, {});
        EXPECT_TRUE(scene.haveResourcesChanged());
    }

    TEST_F(AResourceChangeCollectingScene, marksClientResourcesNotDirtyWhenAllocatingATextureSamplerWithInvalidTexture)
    {
        TextureSampler sampler;
        sampler.contentType = TextureSampler::ContentType::RenderBuffer;
        sampler.textureResource = ResourceContentHash::Invalid();
        sampler.contentHandle   = scene.allocateRenderBuffer({}, {}).asMemoryHandle();
        scene.allocateTextureSampler(sampler, {});
        EXPECT_FALSE(scene.haveResourcesChanged());
    }

    TEST_F(AResourceChangeCollectingScene, marksClientResourcesDirtyWhenSettingADataTextureSampler)
    {
        const RenderableHandle renderable = createRenderable();
        const DataInstanceHandle dataInstance = createUniformDataInstanceWithSampler(renderable, { 123, 0 });
        scene.resetResourceChanges();

        const TextureSamplerHandle sampler = scene.allocateTextureSampler({ {}, { 456, 0 } }, {});
        scene.setDataTextureSamplerHandle(dataInstance, samplerField, sampler);
        EXPECT_TRUE(scene.haveResourcesChanged());
    }

    TEST_F(AResourceChangeCollectingScene, marksClientResourcesDirtyWhenReleasingATextureSampler)
    {
        auto handle = scene.allocateTextureSampler({ {}, { 123, 0 } }, {});
        scene.resetResourceChanges();

        scene.releaseTextureSampler(handle);
        EXPECT_TRUE(scene.haveResourcesChanged());
    }

    TEST_F(AResourceChangeCollectingScene, marksClientResourcesNotDirtyWhenReleasingATextureSamplerWithInvalidTexture)
    {
        TextureSampler sampler;
        sampler.contentType = TextureSampler::ContentType::RenderBuffer;
        sampler.textureResource = ResourceContentHash::Invalid();
        sampler.contentHandle   = scene.allocateRenderBuffer({}, {}).asMemoryHandle();
        auto handle = scene.allocateTextureSampler(sampler, {});
        scene.resetResourceChanges();

        scene.releaseTextureSampler(handle);
        EXPECT_FALSE(scene.haveResourcesChanged());
    }

    TEST_F(AResourceChangeCollectingScene, marksClientResourcesDirtyWhenAllocatingADataSlot)
    {
        scene.allocateDataSlot({ EDataSlotType::TextureProvider, DataSlotId(0u), NodeHandle(), DataInstanceHandle(), { 123, 0 }, TextureSamplerHandle() }, {});
        EXPECT_TRUE(scene.haveResourcesChanged());
    }

    TEST_F(AResourceChangeCollectingScene, marksClientResourcesNotDirtyWhenAllocatingADataSlotWithInvalidTexture)
    {
        scene.allocateDataSlot({ EDataSlotType::TextureProvider, DataSlotId(0u), NodeHandle(), DataInstanceHandle(), ResourceContentHash::Invalid(), TextureSamplerHandle() }, {});
        EXPECT_FALSE(scene.haveResourcesChanged());
    }

    TEST_F(AResourceChangeCollectingScene, marksClientResourcesDirtyWhenSettingTextureToDataSlot)
    {
        const DataSlotHandle dataSlot = scene.allocateDataSlot({ EDataSlotType::TextureProvider, DataSlotId(0u), NodeHandle(), DataInstanceHandle(), { 123, 0 }, TextureSamplerHandle() }, {});
        scene.resetResourceChanges();

        scene.setDataSlotTexture(dataSlot, { 456, 0 });
        EXPECT_TRUE(scene.haveResourcesChanged());
    }

    TEST_F(AResourceChangeCollectingScene, marksClientResourcesDirtyWhenReleasingADataSlot)
    {
        const DataSlotHandle dataSlot = scene.allocateDataSlot({ EDataSlotType::TextureProvider, DataSlotId(0u), NodeHandle(), DataInstanceHandle(), { 123, 0 }, TextureSamplerHandle() }, {});
        scene.resetResourceChanges();

        scene.releaseDataSlot(dataSlot);
        EXPECT_TRUE(scene.haveResourcesChanged());
    }

    TEST_F(AResourceChangeCollectingScene, marksClientResourcesNotDirtyWhenReleasingADataSlotWithInvalidTexture)
    {
        const DataSlotHandle dataSlot = scene.allocateDataSlot({ EDataSlotType::TextureProvider, DataSlotId(0u), NodeHandle(), DataInstanceHandle(), ResourceContentHash::Invalid(), TextureSamplerHandle() }, {});
        scene.resetResourceChanges();

        scene.releaseDataSlot(dataSlot);
        EXPECT_FALSE(scene.haveResourcesChanged());
    }
}
