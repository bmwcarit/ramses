//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "Scene/Scene.h"
#include "SceneAPI/RenderState.h"
#include "Scene/SceneActionCollectionCreator.h"
#include "Scene/SceneActionApplier.h"
#include "ResourceSerializationTestHelper.h"
#include "Resource/IResource.h"

using namespace testing;

namespace ramses_internal
{
    class SceneForSceneActionApplierTesting : public Scene
    {
    public:
        SceneForSceneActionApplierTesting()
        {}

        MOCK_METHOD2(allocateRenderable,         RenderableHandle(NodeHandle, RenderableHandle));
        MOCK_METHOD2(setRenderableStartIndex,    void (RenderableHandle, UInt32));
        MOCK_METHOD2(setRenderableIndexCount,    void (RenderableHandle, UInt32));
        MOCK_METHOD2(setRenderableEffect,        void (RenderableHandle, const ResourceContentHash&));
        MOCK_METHOD2(setRenderableRenderState,   void (RenderableHandle, RenderStateHandle));
        MOCK_METHOD2(setRenderableVisibility,    void (RenderableHandle, Bool));
        MOCK_METHOD2(setRenderableInstanceCount, void (RenderableHandle, UInt32));
        MOCK_METHOD3(setRenderableDataInstance,  void (RenderableHandle, ERenderableDataSlotType, DataInstanceHandle));

        MOCK_METHOD1(allocateRenderState,           RenderStateHandle(RenderStateHandle));
        MOCK_METHOD5(setRenderStateBlendFactors,    void (RenderStateHandle, EBlendFactor, EBlendFactor, EBlendFactor, EBlendFactor));
        MOCK_METHOD3(setRenderStateBlendOperations, void (RenderStateHandle, EBlendOperation, EBlendOperation));
        MOCK_METHOD2(setRenderStateCullMode,        void (RenderStateHandle, ECullMode));
        MOCK_METHOD2(setRenderStateDrawMode,        void (RenderStateHandle, EDrawMode));
        MOCK_METHOD2(setRenderStateDepthWrite,      void (RenderStateHandle, EDepthWrite));
        MOCK_METHOD2(setRenderStateDepthFunc,       void (RenderStateHandle, EDepthFunc));
        MOCK_METHOD3(setRenderStateScissorTest,     void(RenderStateHandle, EScissorTest, const RenderState::ScissorRegion&));
        MOCK_METHOD4(setRenderStateStencilFunc,     void (RenderStateHandle, EStencilFunc, UInt8, UInt8));
        MOCK_METHOD4(setRenderStateStencilOps,      void (RenderStateHandle, EStencilOp, EStencilOp, EStencilOp));
        MOCK_METHOD2(setRenderStateColorWriteMask,  void (RenderStateHandle, ColorWriteMask));

        MOCK_METHOD2(allocateTextureSampler,    TextureSamplerHandle(const TextureSampler& sampler, TextureSamplerHandle handle));

        MOCK_METHOD1(setAckFlushState, void(bool));
    };

    class ASceneActionCreatorAndApplier : public ::testing::Test
    {
    public:
        ASceneActionCreatorAndApplier()
            : creator(collection)
        {}

        StrictMock<SceneForSceneActionApplierTesting> scene;
        SceneActionCollection collection;
        SceneActionCollectionCreator creator;
    };


    TEST_F(ASceneActionCreatorAndApplier, CanSerializeCompoundRenderable)
    {
        const RenderableHandle renderableHandle(43u);
        Renderable renderable;
        renderable.node = NodeHandle{ 45u };
        renderable.startIndex= 37u;
        renderable.indexCount= 87u;
        renderable.effectResource = { 99u, 24u };
        renderable.renderState = RenderStateHandle{ 33u };
        renderable.isVisible = false;
        renderable.instanceCount = 7u;
        renderable.dataInstances[ERenderableDataSlotType_Geometry] = DataInstanceHandle{ 12u };
        renderable.dataInstances[ERenderableDataSlotType_Uniforms] = DataInstanceHandle{ 32u };

        const UInt32 sizeOfActionData(sizeof(RenderableHandle)
                                    + sizeof(NodeHandle)
                                    + sizeof(UInt32)
                                    + sizeof(UInt32)
                                    + sizeof(ResourceContentHash)
                                    + sizeof(RenderStateHandle)
                                    + sizeof(Bool)
                                    + sizeof(UInt32)
                                    + sizeof(DataInstanceHandle)
                                    + sizeof(DataInstanceHandle));

        creator.compoundRenderable(renderableHandle, renderable);

        ASSERT_EQ(sizeOfActionData, collection.collectionData().size());

        EXPECT_CALL(scene, allocateRenderable(renderable.node, renderableHandle)).WillOnce(Return(renderableHandle));
        EXPECT_CALL(scene, setRenderableStartIndex(renderableHandle, renderable.startIndex));
        EXPECT_CALL(scene, setRenderableIndexCount(renderableHandle, renderable.indexCount));
        EXPECT_CALL(scene, setRenderableEffect(renderableHandle, renderable.effectResource));
        EXPECT_CALL(scene, setRenderableRenderState(renderableHandle, renderable.renderState));
        EXPECT_CALL(scene, setRenderableVisibility(renderableHandle, renderable.isVisible));
        EXPECT_CALL(scene, setRenderableInstanceCount(renderableHandle, renderable.instanceCount));
        EXPECT_CALL(scene, setRenderableDataInstance(renderableHandle, ERenderableDataSlotType_Geometry, renderable.dataInstances[ERenderableDataSlotType_Geometry]));
        EXPECT_CALL(scene, setRenderableDataInstance(renderableHandle, ERenderableDataSlotType_Uniforms, renderable.dataInstances[ERenderableDataSlotType_Uniforms]));

        SceneActionApplier::ApplyActionsOnScene(scene, collection);
    }


    TEST_F(ASceneActionCreatorAndApplier, CanSerializeCompoundRenderableWithDefaultValues)
    {
        const RenderableHandle renderableHandle(43u);
        Renderable renderable;
        renderable.node = NodeHandle{ 23u };
        renderable.startIndex = 0u;
        renderable.indexCount = 87u;
        renderable.effectResource= {};
        renderable.renderState = RenderStateHandle{ 33u };
        renderable.isVisible = true;
        renderable.instanceCount = 1u;
        renderable.dataInstances[ERenderableDataSlotType_Geometry] = DataInstanceHandle{ 12u };
        renderable.dataInstances[ERenderableDataSlotType_Uniforms] = DataInstanceHandle{ 32u };

        const UInt32 sizeOfActionData(sizeof(RenderableHandle)
                                    + sizeof(NodeHandle)
                                    + sizeof(UInt32)
                                    + sizeof(UInt32)
                                    + sizeof(ResourceContentHash)
                                    + sizeof(RenderStateHandle)
                                    + sizeof(Bool)
                                    + sizeof(UInt32)
                                    + sizeof(DataInstanceHandle)
                                    + sizeof(DataInstanceHandle));

        creator.compoundRenderable(renderableHandle, renderable);

        ASSERT_EQ(sizeOfActionData, collection.collectionData().size());

        // default values will not be serialized
        EXPECT_CALL(scene, allocateRenderable(renderable.node, renderableHandle)).WillOnce(Return(renderableHandle));
        EXPECT_CALL(scene, setRenderableStartIndex(renderableHandle, renderable.startIndex)).Times(0);
        EXPECT_CALL(scene, setRenderableIndexCount(renderableHandle, renderable.indexCount));
        EXPECT_CALL(scene, setRenderableEffect(renderableHandle, renderable.effectResource)).Times(0);
        EXPECT_CALL(scene, setRenderableRenderState(renderableHandle, renderable.renderState));
        EXPECT_CALL(scene, setRenderableVisibility(renderableHandle, renderable.isVisible)).Times(0);
        EXPECT_CALL(scene, setRenderableInstanceCount(renderableHandle, renderable.instanceCount)).Times(0);
        EXPECT_CALL(scene, setRenderableDataInstance(renderableHandle, ERenderableDataSlotType_Geometry, renderable.dataInstances[ERenderableDataSlotType_Geometry]));
        EXPECT_CALL(scene, setRenderableDataInstance(renderableHandle, ERenderableDataSlotType_Uniforms, renderable.dataInstances[ERenderableDataSlotType_Uniforms]));

        SceneActionApplier::ApplyActionsOnScene(scene, collection);
    }

    TEST_F(ASceneActionCreatorAndApplier, CanSerializeCompoundRenderableEffectDataWithDefaultValues)
    {
        const RenderableHandle renderable(43u);
        const ResourceContentHash effectHash;
        const RenderStateHandle stateHandle(33u);
        const DataInstanceHandle uniformInstanceHandle(32u);

        const UInt32 sizeOfActionData(sizeof(RenderableHandle)
            + sizeof(ResourceContentHash)
            + sizeof(RenderStateHandle)
            + sizeof(DataInstanceHandle));

        creator.compoundRenderableEffectData(renderable, uniformInstanceHandle, stateHandle, effectHash);

        ASSERT_EQ(sizeOfActionData, collection.collectionData().size());

        EXPECT_CALL(scene, setRenderableDataInstance(renderable, ERenderableDataSlotType_Uniforms, uniformInstanceHandle));
        EXPECT_CALL(scene, setRenderableRenderState(renderable, stateHandle));
        EXPECT_CALL(scene, setRenderableEffect(renderable, effectHash));

        SceneActionApplier::ApplyActionsOnScene(scene, collection);
    }

    TEST_F(ASceneActionCreatorAndApplier, CanSerializeCompoundState)
    {
        const RenderStateHandle state(77u);
        RenderState rs;
        rs.blendFactorSrcColor = EBlendFactor::DstAlpha;
        rs.blendFactorDstColor = EBlendFactor::One;
        rs.blendFactorSrcAlpha = EBlendFactor::OneMinusSrcAlpha;
        rs.blendFactorDstAlpha = EBlendFactor::SrcAlpha;
        rs.blendOperationColor = EBlendOperation::Subtract;
        rs.blendOperationAlpha = EBlendOperation::Max;
        rs.cullMode = ECullMode::BackFacing;
        rs.drawMode = EDrawMode::Lines;
        rs.depthWrite = EDepthWrite::Enabled;
        rs.depthFunc = EDepthFunc::SmallerEqual;
        rs.scissorTest = EScissorTest::Enabled;
        rs.scissorRegion = { 1, 2, 3, 4 };
        rs.stencilFunc = EStencilFunc::NotEqual;
        rs.stencilRefValue = 99u;
        rs.stencilMask = 3u;
        rs.stencilOpFail = EStencilOp::Replace;
        rs.stencilOpDepthFail = EStencilOp::Decrement;
        rs.stencilOpDepthPass = EStencilOp::Zero;
        rs.colorWriteMask = 0x80;

        const UInt32 sizeOfActionData(sizeof(RenderState) + sizeof(RenderStateHandle));

        creator.compoundState(state, rs);

        ASSERT_EQ(sizeOfActionData, collection.collectionData().size());

        EXPECT_CALL(scene, allocateRenderState(state)).WillOnce(Return(state));
        EXPECT_CALL(scene, setRenderStateBlendFactors(state, rs.blendFactorSrcColor, rs.blendFactorDstColor, rs.blendFactorSrcAlpha, rs.blendFactorDstAlpha));
        EXPECT_CALL(scene, setRenderStateBlendOperations(state, rs.blendOperationColor, rs.blendOperationAlpha));
        EXPECT_CALL(scene, setRenderStateCullMode(state, rs.cullMode));
        EXPECT_CALL(scene, setRenderStateDrawMode(state, rs.drawMode));
        EXPECT_CALL(scene, setRenderStateDepthWrite(state, rs.depthWrite));
        EXPECT_CALL(scene, setRenderStateDepthFunc(state, rs.depthFunc));
        EXPECT_CALL(scene, setRenderStateScissorTest(state, rs.scissorTest, rs.scissorRegion));
        EXPECT_CALL(scene, setRenderStateStencilFunc(state, rs.stencilFunc, rs.stencilRefValue, rs.stencilMask));
        EXPECT_CALL(scene, setRenderStateStencilOps(state, rs.stencilOpFail, rs.stencilOpDepthFail, rs.stencilOpDepthPass));
        EXPECT_CALL(scene, setRenderStateColorWriteMask(state, rs.colorWriteMask));

        SceneActionApplier::ApplyActionsOnScene(scene, collection);
    }

    TEST_F(ASceneActionCreatorAndApplier, canEnableAckFlush)
    {
        creator.setAckFlushState(true);
        EXPECT_CALL(scene, setAckFlushState(true));
        SceneActionApplier::ApplyActionsOnScene(scene, collection);
    }

    TEST_F(ASceneActionCreatorAndApplier, canDisableAckFlush)
    {
        creator.setAckFlushState(false);
        EXPECT_CALL(scene, setAckFlushState(false));
        SceneActionApplier::ApplyActionsOnScene(scene, collection);
    }


    template <typename T>
    class ASceneActionCreatorAndApplierForResources : public ASceneActionCreatorAndApplier
    {
    public:
        SceneActionApplier::ResourceVector receivedResources;

        void compareResources(const SceneActionApplier::ResourceVector& sentResources)
        {
            ASSERT_EQ(sentResources.size(), receivedResources.size());
            for (UInt i = 0; i < sentResources.size(); ++i)
            {
                ResourceSerializationTestHelper::CompareResourceValues(*sentResources[i], *receivedResources[i]);
                ResourceSerializationTestHelper::CompareTypedResources(static_cast<const T&>(*sentResources[i]), static_cast<const T&>(*receivedResources[i]));
            }
        }
    };

    TYPED_TEST_CASE(ASceneActionCreatorAndApplierForResources, ResourceSerializationTestHelper::Types);

    TYPED_TEST(ASceneActionCreatorAndApplierForResources, writeSingleSmallResource)
    {
        SceneActionApplier::ResourceVector sendResources;
        sendResources.emplace_back(ResourceSerializationTestHelper::CreateTestResource<TypeParam>(5));
        this->creator.pushResource(*sendResources.front());

        SceneActionApplier::ApplyActionsOnScene(this->scene, this->collection, nullptr, &this->receivedResources);
        this->compareResources(sendResources);
    }

    TYPED_TEST(ASceneActionCreatorAndApplierForResources, writeSingleBigResource)
    {
        SceneActionApplier::ResourceVector sendResources;
        sendResources.emplace_back(ResourceSerializationTestHelper::CreateTestResource<TypeParam>(100 * 1000));
        this->creator.pushResource(*sendResources.front());

        SceneActionApplier::ApplyActionsOnScene(this->scene, this->collection, nullptr, &this->receivedResources);
        this->compareResources(sendResources);
    }

    TYPED_TEST(ASceneActionCreatorAndApplierForResources, writeMultipleResources)
    {
        SceneActionApplier::ResourceVector sendResources;
        sendResources.emplace_back(ResourceSerializationTestHelper::CreateTestResource<TypeParam>(100*1000));
        sendResources.emplace_back(ResourceSerializationTestHelper::CreateTestResource<TypeParam>(100*1000));
        sendResources.emplace_back(ResourceSerializationTestHelper::CreateTestResource<TypeParam>(100*1000));
        sendResources.emplace_back(ResourceSerializationTestHelper::CreateTestResource<TypeParam>(100*1000));
        sendResources.emplace_back(ResourceSerializationTestHelper::CreateTestResource<TypeParam>(100*1000));

        for (const auto& res : sendResources)
        {
            this->creator.pushResource(*res);
        }

        SceneActionApplier::ApplyActionsOnScene(this->scene, this->collection, nullptr, &this->receivedResources);
        this->compareResources(sendResources);
    }
}
