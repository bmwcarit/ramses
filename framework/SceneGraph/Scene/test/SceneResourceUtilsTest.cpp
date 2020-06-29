//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#include "framework_common_gmock_header.h"
#include "gtest/gtest.h"
#include "Scene/SceneResourceUtils.h"
#include "Scene/ClientScene.h"

namespace ramses_internal
{
    const ResourceContentHash hash1{ 111, 0 };
    const ResourceContentHash hash2{ 222, 0 };
    const ResourceContentHash hash3{ 333, 0 };
    const ResourceContentHash hash4{ 444, 0 };

    class ASceneResourceUtils : public testing::Test
    {
    public:
        ASceneResourceUtils()
            : testUniformLayout(0u)
            , testGeometryLayout(2u)
            , indicesField(0u)
            , vertAttribField(1u)
            , dataField(0u)
            , samplerField(1u)
        {
        }

    protected:
        RenderableHandle createRenderable()
        {
            return scene.allocateRenderable(scene.allocateNode());
        }

        DataInstanceHandle createUniformDataInstanceWithSampler(RenderableHandle renderable, ResourceContentHash texHash = ResourceContentHash::Invalid(), ResourceContentHash const& effectHash = ResourceContentHash::Invalid())
        {
            DataFieldInfoVector uniformDataFields(2u);
            uniformDataFields[dataField.asMemoryHandle()] = DataFieldInfo(EDataType_Float);
            uniformDataFields[samplerField.asMemoryHandle()] = DataFieldInfo(EDataType_TextureSampler);
            scene.allocateDataLayout(uniformDataFields, effectHash, testUniformLayout);

            const DataInstanceHandle uniformData = scene.allocateDataInstance(testUniformLayout);
            scene.setRenderableDataInstance(renderable, ERenderableDataSlotType_Uniforms, uniformData);

            if (texHash.isValid())
            {
                sampler = scene.allocateTextureSampler({ {}, texHash });
                scene.setDataTextureSamplerHandle(uniformData, samplerField, sampler);
            }
            return uniformData;
        }

        DataInstanceHandle createVertexDataInstance(RenderableHandle renderable, ResourceContentHash const& effectHash = ResourceContentHash::Invalid())
        {
            DataFieldInfoVector geometryDataFields(2u);
            geometryDataFields[indicesField.asMemoryHandle()] = DataFieldInfo(EDataType_Indices, 1u, EFixedSemantics_Indices);
            geometryDataFields[vertAttribField.asMemoryHandle()] = DataFieldInfo(EDataType_Vector3Buffer, 1u, EFixedSemantics_VertexPositionAttribute);
            scene.allocateDataLayout(geometryDataFields, effectHash, testGeometryLayout);

            const DataInstanceHandle geometryData = scene.allocateDataInstance(testGeometryLayout);
            scene.setRenderableDataInstance(renderable, ERenderableDataSlotType_Geometry, geometryData);

            return geometryData;
        }

        void expectResources(ResourceContentHashVector const& expectation)
        {
            ResourceContentHashVector clientResources;
            SceneResourceUtils::GetAllClientResourcesFromScene(clientResources, scene);
            EXPECT_EQ(expectation, clientResources);
        }

        ClientScene scene;

        const DataLayoutHandle testUniformLayout;
        const DataLayoutHandle testGeometryLayout;
        const DataFieldHandle indicesField;
        const DataFieldHandle vertAttribField;
        const DataFieldHandle dataField;
        const DataFieldHandle samplerField;
        TextureSamplerHandle sampler;
    };

    TEST_F(ASceneResourceUtils, hasEmptyResourceListsAtCreation)
    {
        expectResources({});
    }

    TEST_F(ASceneResourceUtils, addsAllResourceTypesOfVisibleRenderable)
    {
        const RenderableHandle renderable = createRenderable();
        scene.setRenderableVisibility(renderable, EVisibilityMode::Visible);
        const DataInstanceHandle vertexData = createVertexDataInstance(renderable, hash1);
        createUniformDataInstanceWithSampler(renderable, hash2, hash1);
        scene.setDataResource(vertexData, vertAttribField, hash3, DataBufferHandle::Invalid(), 0u);
        scene.setDataResource(vertexData, indicesField, hash4, DataBufferHandle::Invalid(), 0u);

        expectResources({ hash1, hash2, hash3, hash4 });
    }

    TEST_F(ASceneResourceUtils, addsAllResourceTypesOfInvisibleRenderable)
    {
        const RenderableHandle renderable = createRenderable();
        scene.setRenderableVisibility(renderable, EVisibilityMode::Visible);
        const DataInstanceHandle vertexData = createVertexDataInstance(renderable, hash1);
        createUniformDataInstanceWithSampler(renderable, hash2, hash1);
        scene.setDataResource(vertexData, vertAttribField, hash3, DataBufferHandle::Invalid(), 0u);
        scene.setDataResource(vertexData, indicesField, hash4, DataBufferHandle::Invalid(), 0u);

        expectResources({ hash1, hash2, hash3, hash4 });
    }

    TEST_F(ASceneResourceUtils, doesNotAddAllResourceTypesOfOffRenderable)
    {
        const RenderableHandle renderable = createRenderable();
        scene.setRenderableVisibility(renderable, EVisibilityMode::Off);
        const DataInstanceHandle vertexData = createVertexDataInstance(renderable, hash1);
        createUniformDataInstanceWithSampler(renderable, hash2, hash1);
        scene.setDataResource(vertexData, vertAttribField, hash3, DataBufferHandle::Invalid(), 0u);
        scene.setDataResource(vertexData, indicesField, hash4, DataBufferHandle::Invalid(), 0u);

        expectResources({});
    }

    TEST_F(ASceneResourceUtils, doesNotAddInvalidHashes)
    {
        const RenderableHandle renderable = createRenderable();
        createVertexDataInstance(renderable);
        createUniformDataInstanceWithSampler(renderable, hash1); //textureResource can't be invalid
        // data resources can't be invalid either

        expectResources({ hash1 });
    }

    TEST_F(ASceneResourceUtils, doesNotAddEffectHashOfDataLayoutIfNotConnectedToRenderable)
    {
        DataFieldInfoVector geometryDataFields(2u);
        geometryDataFields[indicesField.asMemoryHandle()] = DataFieldInfo(EDataType_Indices, 1u, EFixedSemantics_Indices);
        geometryDataFields[vertAttribField.asMemoryHandle()] = DataFieldInfo(EDataType_Vector3Buffer, 1u, EFixedSemantics_VertexPositionAttribute);
        scene.allocateDataLayout(geometryDataFields, hash1, testGeometryLayout);

        expectResources({});
    }

    TEST_F(ASceneResourceUtils, addsAllResourcesIfConnectedRenderableIsNotOff)
    {
        const RenderableHandle renderable = createRenderable();
        scene.setRenderableVisibility(renderable, EVisibilityMode::Visible);
        const DataInstanceHandle vertexData = createVertexDataInstance(renderable, hash1);
        createUniformDataInstanceWithSampler(renderable, hash2, hash1);
        scene.setDataResource(vertexData, vertAttribField, hash3, DataBufferHandle::Invalid(), 0u);
        scene.setDataResource(vertexData, indicesField, hash4, DataBufferHandle::Invalid(), 0u);

        expectResources({ hash1, hash2, hash3, hash4 });

        scene.setRenderableVisibility(renderable, EVisibilityMode::Invisible);
        expectResources({ hash1, hash2, hash3, hash4 });

        scene.setRenderableVisibility(renderable, EVisibilityMode::Visible);
        expectResources({ hash1, hash2, hash3, hash4 });

        scene.setRenderableVisibility(renderable, EVisibilityMode::Off);
        expectResources({});
    }

    TEST_F(ASceneResourceUtils, addsIdenticalEffectHashOnlyOnceWhenUsedByMultipleDataInstances)
    {
        const RenderableHandle renderable = createRenderable();
        createVertexDataInstance(renderable, hash1);
        createUniformDataInstanceWithSampler(renderable, hash2, hash1);

        expectResources({ hash1, hash2 });
    }

    TEST_F(ASceneResourceUtils, addsAllResourceOnlyOnceWhenUsedMultipleTimes)
    {
        {
            const RenderableHandle renderable = createRenderable();
            scene.setRenderableVisibility(renderable, EVisibilityMode::Visible);
            const DataInstanceHandle vertexData = createVertexDataInstance(renderable, hash1);
            createUniformDataInstanceWithSampler(renderable, hash2, hash1);
            scene.setDataResource(vertexData, vertAttribField, hash3, DataBufferHandle::Invalid(), 0u);
            scene.setDataResource(vertexData, indicesField, hash4, DataBufferHandle::Invalid(), 0u);
        }

        {
            const RenderableHandle renderable = createRenderable();
            scene.setRenderableVisibility(renderable, EVisibilityMode::Visible);
            const DataInstanceHandle vertexData = createVertexDataInstance(renderable, hash1);
            createUniformDataInstanceWithSampler(renderable, hash2, hash1);
            scene.setDataResource(vertexData, vertAttribField, hash3, DataBufferHandle::Invalid(), 0u);
            scene.setDataResource(vertexData, indicesField, hash4, DataBufferHandle::Invalid(), 0u);
        }
        expectResources({ hash1, hash2, hash3, hash4 });

        {
            const RenderableHandle renderable = createRenderable();
            scene.setRenderableVisibility(renderable, EVisibilityMode::Visible);
            const DataInstanceHandle vertexData = createVertexDataInstance(renderable, hash1);
            createUniformDataInstanceWithSampler(renderable, hash2, hash1);
            scene.setDataResource(vertexData, vertAttribField, hash3, DataBufferHandle::Invalid(), 1u);
            scene.setDataResource(vertexData, indicesField, hash4, DataBufferHandle::Invalid(), 1u);
        }
        expectResources({ hash1, hash2, hash3, hash4 });
    }

    TEST_F(ASceneResourceUtils, doesNotAddSamplersTextureToListIfNotConnectedWithAnyRenderable)
    {
        const auto textureSamplerHandle = scene.allocateTextureSampler({ {}, hash2 });
        expectResources({});

        DataFieldInfoVector uniformDataFields(2u);
        uniformDataFields[dataField.asMemoryHandle()] = DataFieldInfo(EDataType_Float);
        uniformDataFields[samplerField.asMemoryHandle()] = DataFieldInfo(EDataType_TextureSampler);
        scene.allocateDataLayout(uniformDataFields, ResourceContentHash::Invalid(), testUniformLayout);

        const DataInstanceHandle uniformData = scene.allocateDataInstance(testUniformLayout);
        scene.setDataTextureSamplerHandle(uniformData, samplerField, textureSamplerHandle);

        expectResources({});
    }

    TEST_F(ASceneResourceUtils, doesNotAddEffectResourceOfUnusedDataLayout)
    {
        createRenderable();

        const auto dl = scene.allocateDataLayout({}, hash1);
        expectResources({});
        scene.allocateDataInstance(dl);
        expectResources({});
    }

    TEST_F(ASceneResourceUtils, doesNotAddResourcesIfItWasAddedAndReleasedInOneCycle)
    {
        const RenderableHandle renderable = createRenderable();
        const DataInstanceHandle dataInstance1 = createVertexDataInstance(renderable);
        const DataInstanceHandle dataInstance2 = createVertexDataInstance(renderable);

        expectResources({});

        scene.setDataResource(dataInstance1, vertAttribField, hash1, DataBufferHandle::Invalid(), 0u);
        scene.setDataResource(dataInstance2, vertAttribField, hash1, DataBufferHandle::Invalid(), 0u);
        scene.releaseDataInstance(dataInstance1);
        scene.releaseDataInstance(dataInstance2);

        expectResources({});
    }

    TEST_F(ASceneResourceUtils, doesNotMarkResourcesAsObsoleteNorNewIfItWasReleasedAndAddedInOneCycle)
    {
        const RenderableHandle renderable = createRenderable();
        const DataInstanceHandle dataInstance1 = createVertexDataInstance(renderable);
        const DataInstanceHandle dataInstance2 = createVertexDataInstance(renderable);

        scene.setDataResource(dataInstance1, vertAttribField, hash1, DataBufferHandle::Invalid(), 0u);
        scene.setDataResource(dataInstance2, vertAttribField, hash1, DataBufferHandle::Invalid(), 0u);

        expectResources({ hash1 });

        scene.setDataResource(dataInstance1, vertAttribField, ResourceContentHash::Invalid(), DataBufferHandle(0u), 0u);
        scene.setDataResource(dataInstance2, vertAttribField, ResourceContentHash::Invalid(), DataBufferHandle(0u), 0u); // now marked as obsolete
        scene.setDataResource(dataInstance1, vertAttribField, hash1, DataBufferHandle::Invalid(), 0u);
        scene.setDataResource(dataInstance2, vertAttribField, hash1, DataBufferHandle::Invalid(), 0u);

        expectResources({ hash1 });
    }

    TEST_F(ASceneResourceUtils, createdStreamTextureAddsFallbackTexture)
    {
        scene.allocateStreamTexture(1u, hash1);
        expectResources({ hash1 });
    }

    TEST_F(ASceneResourceUtils, createdStreamTextureDoesNotMarkFallbackTextureAsNewIfItWasAlreadyUsedInScene)
    {
        const RenderableHandle renderable = createRenderable();
        createUniformDataInstanceWithSampler(renderable, hash1);
        expectResources({ hash1 });

        scene.allocateStreamTexture(1u, hash1);
        expectResources({ hash1 });
    }

    TEST_F(ASceneResourceUtils, destroyedStreamTextureRemovesFallbackTexture)
    {
        const StreamTextureHandle handle = scene.allocateStreamTexture(1u, hash1);
        expectResources({ hash1 });

        scene.releaseStreamTexture(handle);
        expectResources({});
    }

    TEST_F(ASceneResourceUtils, destroyedStreamTextureDoesNotRemoveFallbackTextureIfItIsStillUsedInScene)
    {
        const RenderableHandle renderable = createRenderable();
        createUniformDataInstanceWithSampler(renderable, hash1);
        expectResources({ hash1 });

        const StreamTextureHandle handle = scene.allocateStreamTexture(1u, hash1);
        expectResources({ hash1 });

        scene.releaseStreamTexture(handle);
        expectResources({ hash1 });
    }

    TEST_F(ASceneResourceUtils, addsTextureResourceWhenUsedByCreatedDataSlot)
    {
        scene.allocateDataSlot({ EDataSlotType_TextureProvider, DataSlotId(0u), NodeHandle(), DataInstanceHandle(), hash1, TextureSamplerHandle() });
        expectResources({ hash1 });
    }

    TEST_F(ASceneResourceUtils, addsTextureResourceWhenUsedBySetDataSlotTexture)
    {
        const auto slotHandle = scene.allocateDataSlot({ EDataSlotType_TextureProvider, DataSlotId(0u), NodeHandle(), DataInstanceHandle(), hash1, TextureSamplerHandle() });
        expectResources({ hash1 });
        scene.setDataSlotTexture(slotHandle, hash2);
        expectResources({ hash2 });
    }

    TEST_F(ASceneResourceUtils, removesTextureWhenUsedByReleasedDataSlot)
    {
        const DataSlotHandle dataSlot = scene.allocateDataSlot({ EDataSlotType_TextureProvider, DataSlotId(0u), NodeHandle(), DataInstanceHandle(), hash1, TextureSamplerHandle() });
        expectResources({ hash1 });

        scene.releaseDataSlot(dataSlot);
        expectResources({});
    }

    TEST_F(ASceneResourceUtils, doesNotAddTextureWhenUsedByCreatedDataSlotButAlreadyUsedBefore)
    {
        const RenderableHandle renderable = createRenderable();
        createUniformDataInstanceWithSampler(renderable, hash1);
        expectResources({ hash1 });

        scene.allocateDataSlot({ EDataSlotType_TextureProvider, DataSlotId(0u), NodeHandle(), DataInstanceHandle(), hash1, TextureSamplerHandle() });
        expectResources({ hash1 });
    }

    TEST_F(ASceneResourceUtils, doesNotRemoveTextureWhenUsedByDestroyedDataSlotButStillInUse)
    {
        const RenderableHandle renderable = createRenderable();
        createUniformDataInstanceWithSampler(renderable, hash1);
        expectResources({ hash1 });

        auto dataSlot = scene.allocateDataSlot({ EDataSlotType_TextureProvider, DataSlotId(0u), NodeHandle(), DataInstanceHandle(), hash1, TextureSamplerHandle() });
        expectResources({ hash1 });

        scene.releaseDataSlot(dataSlot);
        expectResources({ hash1 });
    }

    //---------------------------------------------------------------------------------------------------------------------------------------

    TEST(ASceneResourceUtilsCreateDiffFunction, outputsNothingWithEmptyInputs)
    {
        const ResourceContentHashVector old;
        const ResourceContentHashVector current;
        SceneResourceChanges changes;

        SceneResourceUtils::DiffClientResources(old, current, changes);
        EXPECT_TRUE(changes.m_addedClientResourceRefs.empty());
        EXPECT_TRUE(changes.m_removedClientResourceRefs.empty());
    }

    TEST(ASceneResourceUtilsCreateDiffFunction, outputsOldAsRemovedWithEmptyCurrent)
    {
        const ResourceContentHashVector old{ {0, 111}, {0, 222}, {0, 333} };
        const ResourceContentHashVector current;
        SceneResourceChanges changes;

        SceneResourceUtils::DiffClientResources(old, current, changes);
        EXPECT_TRUE(changes.m_addedClientResourceRefs.empty());
        EXPECT_EQ(old, changes.m_removedClientResourceRefs);
    }

    TEST(ASceneResourceUtilsCreateDiffFunction, outputsNewAsAddedWithEmptyOld)
    {
        const ResourceContentHashVector old;
        const ResourceContentHashVector current{ {0, 111}, {0, 222}, {0, 333} };
        SceneResourceChanges changes;

        SceneResourceUtils::DiffClientResources(old, current, changes);
        EXPECT_TRUE(changes.m_removedClientResourceRefs.empty());
        EXPECT_EQ(current, changes.m_addedClientResourceRefs);
    }

    TEST(ASceneResourceUtilsCreateDiffFunction, outputsEmptyWhenOldAndCurrentEqual)
    {
        const ResourceContentHashVector old{ {0, 111}, {0, 222}, {0, 333}, {0, 444} };
        const ResourceContentHashVector current = old;
        (void)current;
        SceneResourceChanges changes;

        SceneResourceUtils::DiffClientResources(old, current, changes);
        EXPECT_TRUE(changes.m_addedClientResourceRefs.empty());
        EXPECT_TRUE(changes.m_removedClientResourceRefs.empty());
    }

    TEST(ASceneResourceUtilsCreateDiffFunction, exclusivelyContainedResourcesMarkedAsAddedAndRemovedAccordingly)
    {
        const ResourceContentHashVector old{ {0, 111}, {0, 222}, {0, 333}, {0, 444} };
        const ResourceContentHashVector current{ {0, 555}, {0, 666}, {0, 777}, {0, 888} };
        SceneResourceChanges changes;

        SceneResourceUtils::DiffClientResources(old, current, changes);
        EXPECT_EQ(current, changes.m_addedClientResourceRefs);
        EXPECT_EQ(old, changes.m_removedClientResourceRefs);
    }

    TEST(ASceneResourceUtilsCreateDiffFunction, outputsOldAsRemovedIfNotInCurrent)
    {
        const ResourceContentHashVector old{ {0, 111}, {0, 222}, {0, 333}, {0, 444} };
        const ResourceContentHashVector current{ {0, 111}, {0, 333} };
        SceneResourceChanges changes;

        SceneResourceUtils::DiffClientResources(old, current, changes);
        EXPECT_TRUE(changes.m_addedClientResourceRefs.empty());
        ResourceContentHashVector result{ {0, 222}, {0, 444} };
        EXPECT_EQ(result, changes.m_removedClientResourceRefs);
    }

    TEST(ASceneResourceUtilsCreateDiffFunction, outputsNewAsAddedWhenNotInOld)
    {
        const ResourceContentHashVector old{ {0, 111}, {0, 333} };
        const ResourceContentHashVector current{ {0, 111}, {0, 222}, {0, 333}, {0, 444} };
        SceneResourceChanges changes;

        SceneResourceUtils::DiffClientResources(old, current, changes);
        EXPECT_TRUE(changes.m_removedClientResourceRefs.empty());
        ResourceContentHashVector result{ {0, 222}, {0, 444} };
        EXPECT_EQ(result, changes.m_addedClientResourceRefs);
    }

    TEST(ASceneResourceUtilsCreateDiffFunction, removesAndAddsAtTheSameTimeWhileIgnoringShared)
    {
        const ResourceContentHashVector old{ {0, 111}, {0, 222}, {0, 333}, {0, 444}, {0, 555}, {0, 999} };
        const ResourceContentHashVector current{ {0, 333}, {0, 555}, {0, 666}, {0, 777}, {0, 888}, {0, 999} };
        SceneResourceChanges changes;

        SceneResourceUtils::DiffClientResources(old, current, changes);
        ResourceContentHashVector resultAdded{ {0, 666}, {0, 777}, {0, 888} };
        ResourceContentHashVector resultRemoved{ {0, 111}, {0, 222}, {0, 444} };
        EXPECT_EQ(resultAdded, changes.m_addedClientResourceRefs);
        EXPECT_EQ(resultRemoved, changes.m_removedClientResourceRefs);
    }
}
