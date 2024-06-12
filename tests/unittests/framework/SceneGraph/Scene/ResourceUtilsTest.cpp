//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#include "gtest/gtest.h"
#include "internal/SceneGraph/SceneUtils/ResourceUtils.h"
#include "internal/SceneGraph/Scene/ClientScene.h"

namespace ramses::internal
{
    using SceneTypes = ::testing::Types<
        ClientScene,
        TransformationCachedSceneWithExplicitMemory
    >;

    template <typename SCENE>
    class AResourceUtils : public testing::Test
    {
    public:
        AResourceUtils()
        {
            // preallocate mempools for scene with explicit mempools
            SceneSizeInformation sizeInfo{ MaxHandlesUsed, MaxHandlesUsed, MaxHandlesUsed, MaxHandlesUsed, MaxHandlesUsed, MaxHandlesUsed,
                MaxHandlesUsed, MaxHandlesUsed, MaxHandlesUsed, MaxHandlesUsed, MaxHandlesUsed, MaxHandlesUsed, MaxHandlesUsed,
                MaxHandlesUsed, MaxHandlesUsed, MaxHandlesUsed, MaxHandlesUsed, MaxHandlesUsed, MaxHandlesUsed };
            scene.preallocateSceneSize(sizeInfo);
        }

    protected:
        template <typename HANDLETYPE>
        HANDLETYPE getNextHandle()
        {
            assert(handleCounter < MaxHandlesUsed);
            EXPECT_TRUE(handleCounter < MaxHandlesUsed) << "fix test max handles";
            return HANDLETYPE{ handleCounter++ };
        }

        RenderableHandle createRenderable()
        {
            return this->scene.allocateRenderable(this->scene.allocateNode(0u, getNextHandle<NodeHandle>()), getNextHandle<RenderableHandle>());
        }

        DataInstanceHandle createUniformDataInstanceWithSampler(RenderableHandle renderable, ResourceContentHash texHash = ResourceContentHash::Invalid(), ResourceContentHash const& effectHash = ResourceContentHash::Invalid())
        {
            DataFieldInfoVector uniformDataFields(2u);
            uniformDataFields[this->dataField.asMemoryHandle()] = DataFieldInfo(EDataType::Float);
            uniformDataFields[this->samplerField.asMemoryHandle()] = DataFieldInfo(EDataType::TextureSampler2D);
            const auto testUniformLayout = this->scene.allocateDataLayout(uniformDataFields, effectHash, getNextHandle<DataLayoutHandle>());

            const DataInstanceHandle uniformData = this->scene.allocateDataInstance(testUniformLayout, getNextHandle<DataInstanceHandle>());
            this->scene.setRenderableDataInstance(renderable, ERenderableDataSlotType_Uniforms, uniformData);

            if (texHash.isValid())
            {
                const auto sampler = this->scene.allocateTextureSampler({ {}, texHash }, getNextHandle<TextureSamplerHandle>());
                this->scene.setDataTextureSamplerHandle(uniformData, this->samplerField, sampler);
            }
            return uniformData;
        }

        DataInstanceHandle createVertexDataInstance(RenderableHandle renderable, ResourceContentHash const& effectHash = ResourceContentHash::Invalid())
        {
            DataFieldInfoVector geometryDataFields(2u);
            geometryDataFields[this->indicesField.asMemoryHandle()] = DataFieldInfo(EDataType::Indices, 1u, EFixedSemantics::Indices);
            geometryDataFields[this->vertAttribField.asMemoryHandle()] = DataFieldInfo(EDataType::Vector3Buffer, 1u, EFixedSemantics::Invalid);
            const auto testGeometryLayout = this->scene.allocateDataLayout(geometryDataFields, effectHash, getNextHandle<DataLayoutHandle>());

            const DataInstanceHandle geometryData = this->scene.allocateDataInstance(testGeometryLayout, getNextHandle<DataInstanceHandle>());
            this->scene.setRenderableDataInstance(renderable, ERenderableDataSlotType_Geometry, geometryData);

            return geometryData;
        }

        void expectResources(ResourceContentHashVector const& expectation)
        {
            ResourceContentHashVector resources;
            ResourceUtils::GetAllResourcesFromScene(resources, scene);
            EXPECT_EQ(expectation, resources);
        }

        SCENE scene;

        const DataFieldHandle indicesField{ 0u };
        const DataFieldHandle vertAttribField{ 1u };
        const DataFieldHandle dataField{ 0u };
        const DataFieldHandle samplerField{ 1u };

        static constexpr uint32_t MaxHandlesUsed = 100u;
        uint32_t handleCounter = 5u;
    };

    static constexpr ResourceContentHash hash1{ 111, 0 };
    static constexpr ResourceContentHash hash2{ 222, 0 };
    static constexpr ResourceContentHash hash3{ 333, 0 };
    static constexpr ResourceContentHash hash4{ 444, 0 };

    TYPED_TEST_SUITE(AResourceUtils, SceneTypes);

    TYPED_TEST(AResourceUtils, hasEmptyResourceListsAtCreation)
    {
        this->expectResources({});
    }

    TYPED_TEST(AResourceUtils, addsAllResourceTypesOfVisibleRenderable)
    {
        const RenderableHandle renderable = this->createRenderable();
        this->scene.setRenderableVisibility(renderable, EVisibilityMode::Visible);
        const DataInstanceHandle vertexData = this->createVertexDataInstance(renderable, hash1);
        this->createUniformDataInstanceWithSampler(renderable, hash2, hash1);
        this->scene.setDataResource(vertexData, this->vertAttribField, hash3, DataBufferHandle::Invalid(), 0u, 0u, 0u);
        this->scene.setDataResource(vertexData, this->indicesField, hash4, DataBufferHandle::Invalid(), 0u, 0u, 0u);

        this->expectResources({ hash1, hash2, hash3, hash4 });
    }

    TYPED_TEST(AResourceUtils, addsAllResourceTypesOfInvisibleRenderable)
    {
        const RenderableHandle renderable = this->createRenderable();
        this->scene.setRenderableVisibility(renderable, EVisibilityMode::Visible);
        const DataInstanceHandle vertexData = this->createVertexDataInstance(renderable, hash1);
        this->createUniformDataInstanceWithSampler(renderable, hash2, hash1);
        this->scene.setDataResource(vertexData, this->vertAttribField, hash3, DataBufferHandle::Invalid(), 0u, 0u, 0u);
        this->scene.setDataResource(vertexData, this->indicesField, hash4, DataBufferHandle::Invalid(), 0u, 0u, 0u);

        this->expectResources({ hash1, hash2, hash3, hash4 });
    }

    TYPED_TEST(AResourceUtils, doesNotAddAllResourceTypesOfOffRenderable)
    {
        const RenderableHandle renderable = this->createRenderable();
        this->scene.setRenderableVisibility(renderable, EVisibilityMode::Off);
        const DataInstanceHandle vertexData = this->createVertexDataInstance(renderable, hash1);
        this->createUniformDataInstanceWithSampler(renderable, hash2, hash1);
        this->scene.setDataResource(vertexData, this->vertAttribField, hash3, DataBufferHandle::Invalid(), 0u, 0u, 0u);
        this->scene.setDataResource(vertexData, this->indicesField, hash4, DataBufferHandle::Invalid(), 0u, 0u, 0u);

        this->expectResources({});
    }

    TYPED_TEST(AResourceUtils, doesNotAddInvalidHashFromUnusedTextureSamplerField)
    {
        const RenderableHandle renderable = this->createRenderable();
        this->createVertexDataInstance(renderable);
        this->createUniformDataInstanceWithSampler(renderable, ResourceContentHash::Invalid(), hash1);

        this->expectResources({ hash1 });
    }

    TYPED_TEST(AResourceUtils, doesNotAddEffectHashOfDataLayoutIfNotConnectedToRenderable)
    {
        DataFieldInfoVector geometryDataFields(2u);
        geometryDataFields[this->indicesField.asMemoryHandle()] = DataFieldInfo(EDataType::Indices, 1u, EFixedSemantics::Indices);
        geometryDataFields[this->vertAttribField.asMemoryHandle()] = DataFieldInfo(EDataType::Vector3Buffer, 1u, EFixedSemantics::Invalid);
        this->scene.allocateDataLayout(geometryDataFields, hash1, this->template getNextHandle<DataLayoutHandle>());

        this->expectResources({});
    }

    TYPED_TEST(AResourceUtils, addsAllResourcesIfConnectedRenderableIsNotOff)
    {
        const RenderableHandle renderable = this->createRenderable();
        this->scene.setRenderableVisibility(renderable, EVisibilityMode::Visible);
        const DataInstanceHandle vertexData = this->createVertexDataInstance(renderable, hash1);
        this->createUniformDataInstanceWithSampler(renderable, hash2, hash1);
        this->scene.setDataResource(vertexData, this->vertAttribField, hash3, DataBufferHandle::Invalid(), 0u, 0u, 0u);
        this->scene.setDataResource(vertexData, this->indicesField, hash4, DataBufferHandle::Invalid(), 0u, 0u, 0u);

        this->expectResources({ hash1, hash2, hash3, hash4 });

        this->scene.setRenderableVisibility(renderable, EVisibilityMode::Invisible);
        this->expectResources({ hash1, hash2, hash3, hash4 });

        this->scene.setRenderableVisibility(renderable, EVisibilityMode::Visible);
        this->expectResources({ hash1, hash2, hash3, hash4 });

        this->scene.setRenderableVisibility(renderable, EVisibilityMode::Off);
        this->expectResources({});
    }

    TYPED_TEST(AResourceUtils, addsIdenticalEffectHashOnlyOnceWhenUsedByMultipleDataInstances)
    {
        const RenderableHandle renderable = this->createRenderable();
        this->createVertexDataInstance(renderable, hash1);
        this->createUniformDataInstanceWithSampler(renderable, hash2, hash1);

        this->expectResources({ hash1, hash2 });
    }

    TYPED_TEST(AResourceUtils, addsAllResourceOnlyOnceWhenUsedMultipleTimes)
    {
        {
            const RenderableHandle renderable = this->createRenderable();
            this->scene.setRenderableVisibility(renderable, EVisibilityMode::Visible);
            const DataInstanceHandle vertexData = this->createVertexDataInstance(renderable, hash1);
            this->createUniformDataInstanceWithSampler(renderable, hash2, hash1);
            this->scene.setDataResource(vertexData, this->vertAttribField, hash3, DataBufferHandle::Invalid(), 0u, 0u, 0u);
            this->scene.setDataResource(vertexData, this->indicesField, hash4, DataBufferHandle::Invalid(), 0u, 0u, 0u);
        }

        {
            const RenderableHandle renderable = this->createRenderable();
            this->scene.setRenderableVisibility(renderable, EVisibilityMode::Visible);
            const DataInstanceHandle vertexData = this->createVertexDataInstance(renderable, hash1);
            this->createUniformDataInstanceWithSampler(renderable, hash2, hash1);
            this->scene.setDataResource(vertexData, this->vertAttribField, hash3, DataBufferHandle::Invalid(), 0u, 0u, 0u);
            this->scene.setDataResource(vertexData, this->indicesField, hash4, DataBufferHandle::Invalid(), 0u, 0u, 0u);
        }
        this->expectResources({ hash1, hash2, hash3, hash4 });

        {
            const RenderableHandle renderable = this->createRenderable();
            this->scene.setRenderableVisibility(renderable, EVisibilityMode::Visible);
            const DataInstanceHandle vertexData = this->createVertexDataInstance(renderable, hash1);
            this->createUniformDataInstanceWithSampler(renderable, hash2, hash1);
            this->scene.setDataResource(vertexData, this->vertAttribField, hash3, DataBufferHandle::Invalid(), 1u, 0u, 0u);
            this->scene.setDataResource(vertexData, this->indicesField, hash4, DataBufferHandle::Invalid(), 1u, 0u, 0u);
        }
        this->expectResources({ hash1, hash2, hash3, hash4 });
    }

    TYPED_TEST(AResourceUtils, doesNotAddSamplersTextureToListIfNotConnectedWithAnyRenderable)
    {
        const auto textureSamplerHandle = this->scene.allocateTextureSampler({ {}, hash2 }, this->template getNextHandle<TextureSamplerHandle>());
        this->expectResources({});

        DataFieldInfoVector uniformDataFields(2u);
        uniformDataFields[this->dataField.asMemoryHandle()] = DataFieldInfo(EDataType::Float);
        uniformDataFields[this->samplerField.asMemoryHandle()] = DataFieldInfo(EDataType::TextureSampler2D);
        const auto testUniformLayout = this->scene.allocateDataLayout(uniformDataFields, ResourceContentHash::Invalid(), this->template getNextHandle<DataLayoutHandle>());

        const DataInstanceHandle uniformData = this->scene.allocateDataInstance(testUniformLayout, this->template getNextHandle<DataInstanceHandle>());
        this->scene.setDataTextureSamplerHandle(uniformData, this->samplerField, textureSamplerHandle);

        this->expectResources({});
    }

    TYPED_TEST(AResourceUtils, doesNotAddEffectResourceOfUnusedDataLayout)
    {
        this->createRenderable();

        const auto dl = this->scene.allocateDataLayout({}, hash1, this->template getNextHandle<DataLayoutHandle>());
        this->expectResources({});
        this->scene.allocateDataInstance(dl, this->template getNextHandle<DataInstanceHandle>());
        this->expectResources({});
    }

    TYPED_TEST(AResourceUtils, doesNotAddResourcesIfItWasAddedAndReleasedInOneCycle)
    {
        const RenderableHandle renderable = this->createRenderable();
        const DataInstanceHandle dataInstance1 = this->createVertexDataInstance(renderable);
        const DataInstanceHandle dataInstance2 = this->createVertexDataInstance(renderable);

        this->expectResources({});

        this->scene.setDataResource(dataInstance1, this->vertAttribField, hash1, DataBufferHandle::Invalid(), 0u, 0u, 0u);
        this->scene.setDataResource(dataInstance2, this->vertAttribField, hash1, DataBufferHandle::Invalid(), 0u, 0u, 0u);
        this->scene.releaseDataInstance(dataInstance1);
        this->scene.releaseDataInstance(dataInstance2);

        this->expectResources({});
    }

    TYPED_TEST(AResourceUtils, doesNotMarkResourcesAsObsoleteNorNewIfItWasReleasedAndAddedInOneCycle)
    {
        const RenderableHandle renderable = this->createRenderable();
        const DataInstanceHandle dataInstance1 = this->createVertexDataInstance(renderable);
        const DataInstanceHandle dataInstance2 = this->createVertexDataInstance(renderable);

        this->scene.setDataResource(dataInstance1, this->vertAttribField, hash1, DataBufferHandle::Invalid(), 0u, 0u, 0u);
        this->scene.setDataResource(dataInstance2, this->vertAttribField, hash1, DataBufferHandle::Invalid(), 0u, 0u, 0u);

        this->expectResources({ hash1 });

        this->scene.setDataResource(dataInstance1, this->vertAttribField, ResourceContentHash::Invalid(), DataBufferHandle(0u), 0u, 0u, 0u);
        this->scene.setDataResource(dataInstance2, this->vertAttribField, ResourceContentHash::Invalid(), DataBufferHandle(0u), 0u, 0u, 0u); // now marked as obsolete
        this->scene.setDataResource(dataInstance1, this->vertAttribField, hash1, DataBufferHandle::Invalid(), 0u, 0u, 0u);
        this->scene.setDataResource(dataInstance2, this->vertAttribField, hash1, DataBufferHandle::Invalid(), 0u, 0u, 0u);

        this->expectResources({ hash1 });
    }

    TYPED_TEST(AResourceUtils, addsTextureResourceWhenUsedByCreatedDataSlot)
    {
        this->scene.allocateDataSlot({ EDataSlotType::TextureProvider, DataSlotId(0u), NodeHandle(), DataInstanceHandle(), hash1, TextureSamplerHandle() }, this->template getNextHandle<DataSlotHandle>());
        this->expectResources({ hash1 });
    }

    TYPED_TEST(AResourceUtils, addsTextureResourceWhenUsedBySetDataSlotTexture)
    {
        const auto slotHandle = this->scene.allocateDataSlot({ EDataSlotType::TextureProvider, DataSlotId(0u), NodeHandle(), DataInstanceHandle(), hash1, TextureSamplerHandle() }, this->template getNextHandle<DataSlotHandle>());
        this->expectResources({ hash1 });
        this->scene.setDataSlotTexture(slotHandle, hash2);
        this->expectResources({ hash2 });
    }

    TYPED_TEST(AResourceUtils, removesTextureWhenUsedByReleasedDataSlot)
    {
        const DataSlotHandle dataSlot = this->scene.allocateDataSlot({ EDataSlotType::TextureProvider, DataSlotId(0u), NodeHandle(), DataInstanceHandle(), hash1, TextureSamplerHandle() }, this->template getNextHandle<DataSlotHandle>());
        this->expectResources({ hash1 });

        this->scene.releaseDataSlot(dataSlot);
        this->expectResources({});
    }

    TYPED_TEST(AResourceUtils, doesNotAddTextureWhenUsedByCreatedDataSlotButAlreadyUsedBefore)
    {
        const RenderableHandle renderable = this->createRenderable();
        this->createUniformDataInstanceWithSampler(renderable, hash1);
        this->expectResources({ hash1 });

        this->scene.allocateDataSlot({ EDataSlotType::TextureProvider, DataSlotId(0u), NodeHandle(), DataInstanceHandle(), hash1, TextureSamplerHandle() }, this->template getNextHandle<DataSlotHandle>());
        this->expectResources({ hash1 });
    }

    TYPED_TEST(AResourceUtils, doesNotRemoveTextureWhenUsedByDestroyedDataSlotButStillInUse)
    {
        const RenderableHandle renderable = this->createRenderable();
        this->createUniformDataInstanceWithSampler(renderable, hash1);
        this->expectResources({ hash1 });

        auto dataSlot = this->scene.allocateDataSlot({ EDataSlotType::TextureProvider, DataSlotId(0u), NodeHandle(), DataInstanceHandle(), hash1, TextureSamplerHandle() }, this->template getNextHandle<DataSlotHandle>());
        this->expectResources({ hash1 });

        this->scene.releaseDataSlot(dataSlot);
        this->expectResources({ hash1 });
    }

    //---------------------------------------------------------------------------------------------------------------------------------------

    TEST(ASceneResourceUtilsCreateDiffFunction, outputsNothingWithEmptyInputs)
    {
        const ResourceContentHashVector old;
        const ResourceContentHashVector current;
        ResourceChanges changes;

        ResourceUtils::DiffResources(old, current, changes);
        EXPECT_TRUE(changes.m_resourcesAdded.empty());
        EXPECT_TRUE(changes.m_resourcesRemoved.empty());
    }

    TEST(ASceneResourceUtilsCreateDiffFunction, outputsOldAsRemovedWithEmptyCurrent)
    {
        const ResourceContentHashVector old{ {0, 111}, {0, 222}, {0, 333} };
        const ResourceContentHashVector current;
        ResourceChanges changes;

        ResourceUtils::DiffResources(old, current, changes);
        EXPECT_TRUE(changes.m_resourcesAdded.empty());
        EXPECT_EQ(old, changes.m_resourcesRemoved);
    }

    TEST(ASceneResourceUtilsCreateDiffFunction, outputsNewAsAddedWithEmptyOld)
    {
        const ResourceContentHashVector old;
        const ResourceContentHashVector current{ {0, 111}, {0, 222}, {0, 333} };
        ResourceChanges changes;

        ResourceUtils::DiffResources(old, current, changes);
        EXPECT_TRUE(changes.m_resourcesRemoved.empty());
        EXPECT_EQ(current, changes.m_resourcesAdded);
    }

    TEST(ASceneResourceUtilsCreateDiffFunction, outputsEmptyWhenOldAndCurrentEqual)
    {
        const ResourceContentHashVector old{ {0, 111}, {0, 222}, {0, 333}, {0, 444} };
        const ResourceContentHashVector current = old;
        (void)current;
        ResourceChanges changes;

        ResourceUtils::DiffResources(old, current, changes);
        EXPECT_TRUE(changes.m_resourcesAdded.empty());
        EXPECT_TRUE(changes.m_resourcesRemoved.empty());
    }

    TEST(ASceneResourceUtilsCreateDiffFunction, exclusivelyContainedResourcesMarkedAsAddedAndRemovedAccordingly)
    {
        const ResourceContentHashVector old{ {0, 111}, {0, 222}, {0, 333}, {0, 444} };
        const ResourceContentHashVector current{ {0, 555}, {0, 666}, {0, 777}, {0, 888} };
        ResourceChanges changes;

        ResourceUtils::DiffResources(old, current, changes);
        EXPECT_EQ(current, changes.m_resourcesAdded);
        EXPECT_EQ(old, changes.m_resourcesRemoved);
    }

    TEST(ASceneResourceUtilsCreateDiffFunction, outputsOldAsRemovedIfNotInCurrent)
    {
        const ResourceContentHashVector old{ {0, 111}, {0, 222}, {0, 333}, {0, 444} };
        const ResourceContentHashVector current{ {0, 111}, {0, 333} };
        ResourceChanges changes;

        ResourceUtils::DiffResources(old, current, changes);
        EXPECT_TRUE(changes.m_resourcesAdded.empty());
        ResourceContentHashVector result{ {0, 222}, {0, 444} };
        EXPECT_EQ(result, changes.m_resourcesRemoved);
    }

    TEST(ASceneResourceUtilsCreateDiffFunction, outputsNewAsAddedWhenNotInOld)
    {
        const ResourceContentHashVector old{ {0, 111}, {0, 333} };
        const ResourceContentHashVector current{ {0, 111}, {0, 222}, {0, 333}, {0, 444} };
        ResourceChanges changes;

        ResourceUtils::DiffResources(old, current, changes);
        EXPECT_TRUE(changes.m_resourcesRemoved.empty());
        ResourceContentHashVector result{ {0, 222}, {0, 444} };
        EXPECT_EQ(result, changes.m_resourcesAdded);
    }

    TEST(ASceneResourceUtilsCreateDiffFunction, removesAndAddsAtTheSameTimeWhileIgnoringShared)
    {
        const ResourceContentHashVector old{ {0, 111}, {0, 222}, {0, 333}, {0, 444}, {0, 555}, {0, 999} };
        const ResourceContentHashVector current{ {0, 333}, {0, 555}, {0, 666}, {0, 777}, {0, 888}, {0, 999} };
        ResourceChanges changes;

        ResourceUtils::DiffResources(old, current, changes);
        ResourceContentHashVector resultAdded{ {0, 666}, {0, 777}, {0, 888} };
        ResourceContentHashVector resultRemoved{ {0, 111}, {0, 222}, {0, 444} };
        EXPECT_EQ(resultAdded, changes.m_resourcesAdded);
        EXPECT_EQ(resultRemoved, changes.m_resourcesRemoved);
    }
}
