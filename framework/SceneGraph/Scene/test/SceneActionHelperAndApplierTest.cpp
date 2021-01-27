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
#include "Resource/IResource.h"

using namespace testing;

namespace ramses_internal
{
    class SceneForSceneActionApplierTesting : public Scene
    {
    public:
        SceneForSceneActionApplierTesting()
        {}

        MOCK_METHOD(RenderableHandle, allocateRenderable, (NodeHandle, RenderableHandle), (override));
        MOCK_METHOD(void , setRenderableStartIndex, (RenderableHandle, UInt32), (override));
        MOCK_METHOD(void , setRenderableIndexCount, (RenderableHandle, UInt32), (override));
        MOCK_METHOD(void , setRenderableRenderState, (RenderableHandle, RenderStateHandle), (override));
        MOCK_METHOD(void , setRenderableVisibility, (RenderableHandle, EVisibilityMode), (override));
        MOCK_METHOD(void , setRenderableInstanceCount, (RenderableHandle, UInt32), (override));
        MOCK_METHOD(void , setRenderableDataInstance, (RenderableHandle, ERenderableDataSlotType, DataInstanceHandle), (override));
        MOCK_METHOD(void, setRenderableStartVertex, (RenderableHandle, UInt32), (override));

        MOCK_METHOD(RenderStateHandle, allocateRenderState, (RenderStateHandle), (override));
        MOCK_METHOD(void , setRenderStateBlendFactors, (RenderStateHandle, EBlendFactor, EBlendFactor, EBlendFactor, EBlendFactor), (override));
        MOCK_METHOD(void , setRenderStateBlendOperations, (RenderStateHandle, EBlendOperation, EBlendOperation), (override));
        MOCK_METHOD(void , setRenderStateBlendColor, (RenderStateHandle, const Vector4&), (override));
        MOCK_METHOD(void , setRenderStateCullMode, (RenderStateHandle, ECullMode), (override));
        MOCK_METHOD(void , setRenderStateDrawMode, (RenderStateHandle, EDrawMode), (override));
        MOCK_METHOD(void , setRenderStateDepthWrite, (RenderStateHandle, EDepthWrite), (override));
        MOCK_METHOD(void , setRenderStateDepthFunc, (RenderStateHandle, EDepthFunc), (override));
        MOCK_METHOD(void, setRenderStateScissorTest, (RenderStateHandle, EScissorTest, const RenderState::ScissorRegion&), (override));
        MOCK_METHOD(void , setRenderStateStencilFunc, (RenderStateHandle, EStencilFunc, UInt8, UInt8), (override));
        MOCK_METHOD(void , setRenderStateStencilOps, (RenderStateHandle, EStencilOp, EStencilOp, EStencilOp), (override));
        MOCK_METHOD(void , setRenderStateColorWriteMask, (RenderStateHandle, ColorWriteMask), (override));

        MOCK_METHOD(TextureSamplerHandle, allocateTextureSampler, (const TextureSampler& sampler, TextureSamplerHandle handle), (override));
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
        renderable.renderState = RenderStateHandle{ 33u };
        renderable.visibilityMode = EVisibilityMode::Invisible;
        renderable.instanceCount = 7u;
        renderable.startVertex = 124u;
        renderable.dataInstances[ERenderableDataSlotType_Geometry] = DataInstanceHandle{ 12u };
        renderable.dataInstances[ERenderableDataSlotType_Uniforms] = DataInstanceHandle{ 32u };

        const UInt32 sizeOfActionData(sizeof(RenderableHandle)
                                    + sizeof(NodeHandle)
                                    + sizeof(UInt32)
                                    + sizeof(UInt32)
                                    + sizeof(RenderStateHandle)
                                    + sizeof(EVisibilityMode)
                                    + sizeof(UInt32)
                                    + sizeof(UInt32)
                                    + sizeof(DataInstanceHandle)
                                    + sizeof(DataInstanceHandle));

        creator.compoundRenderable(renderableHandle, renderable);

        ASSERT_EQ(sizeOfActionData, collection.collectionData().size());

        EXPECT_CALL(scene, allocateRenderable(renderable.node, renderableHandle)).WillOnce(Return(renderableHandle));
        EXPECT_CALL(scene, setRenderableStartIndex(renderableHandle, renderable.startIndex));
        EXPECT_CALL(scene, setRenderableIndexCount(renderableHandle, renderable.indexCount));
        EXPECT_CALL(scene, setRenderableRenderState(renderableHandle, renderable.renderState));
        EXPECT_CALL(scene, setRenderableVisibility(renderableHandle, renderable.visibilityMode));
        EXPECT_CALL(scene, setRenderableInstanceCount(renderableHandle, renderable.instanceCount));
        EXPECT_CALL(scene, setRenderableStartVertex(renderableHandle, renderable.startVertex));
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
        renderable.renderState = RenderStateHandle{ 33u };
        renderable.visibilityMode = EVisibilityMode::Visible;
        renderable.instanceCount = 1u;
        renderable.startVertex = 0u;
        renderable.dataInstances[ERenderableDataSlotType_Geometry] = DataInstanceHandle{ 12u };
        renderable.dataInstances[ERenderableDataSlotType_Uniforms] = DataInstanceHandle{ 32u };

        const UInt32 sizeOfActionData(sizeof(RenderableHandle)
                                    + sizeof(NodeHandle)
                                    + sizeof(UInt32)
                                    + sizeof(UInt32)
                                    + sizeof(RenderStateHandle)
                                    + sizeof(EVisibilityMode)
                                    + sizeof(UInt32)
                                    + sizeof(UInt32)
                                    + sizeof(DataInstanceHandle)
                                    + sizeof(DataInstanceHandle));

        creator.compoundRenderable(renderableHandle, renderable);

        ASSERT_EQ(sizeOfActionData, collection.collectionData().size());

        // default values will not be serialized
        EXPECT_CALL(scene, allocateRenderable(renderable.node, renderableHandle)).WillOnce(Return(renderableHandle));
        EXPECT_CALL(scene, setRenderableStartIndex(renderableHandle, _)).Times(0);
        EXPECT_CALL(scene, setRenderableIndexCount(renderableHandle, renderable.indexCount));
        EXPECT_CALL(scene, setRenderableRenderState(renderableHandle, renderable.renderState));
        EXPECT_CALL(scene, setRenderableVisibility(renderableHandle, _)).Times(0);
        EXPECT_CALL(scene, setRenderableInstanceCount(renderableHandle, _)).Times(0);
        EXPECT_CALL(scene, setRenderableStartVertex(renderableHandle, _)).Times(0);
        EXPECT_CALL(scene, setRenderableDataInstance(renderableHandle, ERenderableDataSlotType_Geometry, renderable.dataInstances[ERenderableDataSlotType_Geometry]));
        EXPECT_CALL(scene, setRenderableDataInstance(renderableHandle, ERenderableDataSlotType_Uniforms, renderable.dataInstances[ERenderableDataSlotType_Uniforms]));

        SceneActionApplier::ApplyActionsOnScene(scene, collection);
    }

    TEST_F(ASceneActionCreatorAndApplier, CanSerializeCompoundRenderableEffectDataWithDefaultValues)
    {
        const RenderableHandle renderable(43u);
        const RenderStateHandle stateHandle(33u);
        const DataInstanceHandle uniformInstanceHandle(32u);

        const UInt32 sizeOfActionData(sizeof(RenderableHandle)
            + sizeof(RenderStateHandle)
            + sizeof(DataInstanceHandle));

        creator.compoundRenderableData(renderable, uniformInstanceHandle, stateHandle);

        ASSERT_EQ(sizeOfActionData, collection.collectionData().size());

        EXPECT_CALL(scene, setRenderableDataInstance(renderable, ERenderableDataSlotType_Uniforms, uniformInstanceHandle));
        EXPECT_CALL(scene, setRenderableRenderState(renderable, stateHandle));

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
        rs.blendColor = { .1f, .2f, .3f, .4f };
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

        creator.compoundState(state, rs);

        const size_t expectedSize{ sizeof(RenderState) - sizeof(RenderState::padding) + sizeof(RenderStateHandle) };
        ASSERT_EQ(expectedSize, collection.collectionData().size());

        EXPECT_CALL(scene, allocateRenderState(state)).WillOnce(Return(state));
        EXPECT_CALL(scene, setRenderStateBlendFactors(state, rs.blendFactorSrcColor, rs.blendFactorDstColor, rs.blendFactorSrcAlpha, rs.blendFactorDstAlpha));
        EXPECT_CALL(scene, setRenderStateBlendOperations(state, rs.blendOperationColor, rs.blendOperationAlpha));
        EXPECT_CALL(scene, setRenderStateBlendColor(state, Vector4(0.1f, 0.2f, 0.3f, 0.4f)));
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
}
