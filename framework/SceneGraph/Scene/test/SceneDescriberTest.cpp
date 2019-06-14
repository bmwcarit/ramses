//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "framework_common_gmock_header.h"
#include "gmock/gmock.h"
#include "Scene/SceneDescriber.h"
#include "Scene/ClientScene.h"
#include "Scene/SceneActionUtils.h"
#include "Scene/SceneActionCollectionCreator.h"
#include "Scene/SceneActionApplierHelper.h"
#include "TestEqualHelper.h"
#include "Math3d/Matrix22f.h"
#include "Math3d/Vector2i.h"
#include "Math3d/Vector3i.h"
#include "Math3d/Vector4i.h"
#include "Utils/MemoryUtils.h"

using namespace testing;

namespace ramses_internal
{
    class SceneDescriberTest : public testing::Test
    {
    protected:
        SceneDescriberTest()
            : creator(actions)
        {}

        void expectAllocateNodeAction(SceneActionCollection::SceneActionReader action, NodeHandle handle, UInt32 expectedChildrenCount)
        {
            EXPECT_EQ(ESceneActionId_AllocateNode, action.type());
            NodeHandle actualHandle;
            UInt32 actualChidlrenCount = 0;
            action.read(actualChidlrenCount);
            action.read(actualHandle);
            EXPECT_EQ(expectedChildrenCount, actualChidlrenCount);
            EXPECT_EQ(handle, actualHandle);
        }

        void expectAddChildToNodeAction(SceneActionCollection::SceneActionReader action, NodeHandle parent, NodeHandle child)
        {
            EXPECT_EQ(ESceneActionId_AddChildToNode, action.type());
            NodeHandle actualParent;
            NodeHandle actualChild;
            action.read(actualParent);
            action.read(actualChild);
            EXPECT_EQ(parent, actualParent);
            EXPECT_EQ(child, actualChild);
        }

        struct RenderableCreationData
        {
            RenderableCreationData()
            : startIndex(3u)
            , indexCount(9u)
            , instanceCount(7u)
            , visibility(false)
            , state(23u)
            , effectHash(47u, 0u)
            , geoInstanceHandle(37u)
            , uniformInstanceHandle(77u)
            {}

            UInt32              startIndex;
            UInt32              indexCount;
            UInt32              instanceCount;
            Bool                visibility;
            RenderStateHandle   state;
            ResourceContentHash effectHash;
            DataInstanceHandle  geoInstanceHandle;
            DataInstanceHandle  uniformInstanceHandle;
        };

        void createRenderable(const RenderableCreationData& data = RenderableCreationData())
        {
            const NodeHandle        node       = m_scene.allocateNode();
            const RenderableHandle  renderable = m_scene.allocateRenderable(node);
            m_scene.setRenderableStartIndex(renderable, data.startIndex);
            m_scene.setRenderableIndexCount(renderable, data.indexCount);
            m_scene.setRenderableRenderState(renderable, data.state);
            m_scene.setRenderableVisibility(renderable, data.visibility);
            m_scene.setRenderableInstanceCount(renderable, data.instanceCount);
            m_scene.setRenderableDataInstance(renderable, ERenderableDataSlotType_Geometry, data.geoInstanceHandle);
            m_scene.setRenderableDataInstance(renderable, ERenderableDataSlotType_Uniforms, data.uniformInstanceHandle);
        }

        struct StateCreationData
        {
            StateCreationData()
            : bfSrcColor(EBlendFactor::DstAlpha)
            , bfDstColor(EBlendFactor::One)
            , bfSrcAlpha(EBlendFactor::OneMinusSrcAlpha)
            , bfDstAlpha(EBlendFactor::SrcAlpha)
            , boColor(EBlendOperation::Subtract)
            , boAlpha(EBlendOperation::Max)
            , cullMode(ECullMode::BackFacing)
            , drawMode(EDrawMode::Lines)
            , depthWrite(EDepthWrite::Enabled)
            , depthFunc(EDepthFunc::SmallerEqual)
            , scissorTest(EScissorTest::Enabled)
            , scissorRegion{
                12, 14, 16, 18
            }
            , stencilFunc(EStencilFunc::NotEqual)
            , stencilRefValue(99u)
            , stencilMask(3u)
            , stencilOpFail(EStencilOp::Replace)
            , stencilOpDepthFail(EStencilOp::Decrement)
            , stencilOpDepthPass(EStencilOp::Zero)
            , colorWriteMask(0x80)
            {}

            EBlendFactor    bfSrcColor;
            EBlendFactor    bfDstColor;
            EBlendFactor    bfSrcAlpha;
            EBlendFactor    bfDstAlpha;
            EBlendOperation boColor;
            EBlendOperation boAlpha;
            ECullMode       cullMode;
            EDrawMode       drawMode;
            EDepthWrite     depthWrite;
            EDepthFunc      depthFunc;
            EScissorTest    scissorTest;
            RenderState::ScissorRegion scissorRegion;
            EStencilFunc    stencilFunc;
            UInt8           stencilRefValue;
            UInt8           stencilMask;
            EStencilOp      stencilOpFail;
            EStencilOp      stencilOpDepthFail;
            EStencilOp      stencilOpDepthPass;
            ColorWriteMask  colorWriteMask;
        };

        void createState(const StateCreationData& data = StateCreationData())
        {
            const RenderStateHandle state = m_scene.allocateRenderState();
            m_scene.setRenderStateBlendFactors(   state, data.bfSrcColor, data.bfDstColor, data.bfSrcAlpha, data.bfDstAlpha);
            m_scene.setRenderStateBlendOperations(state, data.boColor, data.boAlpha);
            m_scene.setRenderStateCullMode(       state, data.cullMode);
            m_scene.setRenderStateDrawMode(       state, data.drawMode);
            m_scene.setRenderStateDepthWrite(     state, data.depthWrite);
            m_scene.setRenderStateDepthFunc(      state, data.depthFunc);
            m_scene.setRenderStateScissorTest(    state, data.scissorTest, data.scissorRegion);
            m_scene.setRenderStateStencilFunc(    state, data.stencilFunc, data.stencilRefValue, data.stencilMask);
            m_scene.setRenderStateStencilOps(     state, data.stencilOpFail, data.stencilOpDepthFail, data.stencilOpDepthPass);
            m_scene.setRenderStateColorWriteMask( state, data.colorWriteMask);
        }

        Scene m_scene;
        SceneActionCollection actions;
        SceneActionCollectionCreator creator;
    };

    TEST_F(SceneDescriberTest, checksDescriptionActionsForSceneWithParentAndThreeChildren)
    {
        NodeHandle parent = m_scene.allocateNode();
        NodeHandle child2 = m_scene.allocateNode();
        NodeHandle child3 = m_scene.allocateNode();
        NodeHandle child1 = m_scene.allocateNode();

        m_scene.addChildToNode(parent, child1);
        m_scene.addChildToNode(parent, child2);
        m_scene.addChildToNode(parent, child3);

        SceneDescriber::describeScene<IScene>(m_scene, creator);

        ASSERT_EQ(7u, actions.numberOfActions());
        uint32_t actionIdx = 0u;

        expectAllocateNodeAction  (actions[actionIdx++], parent, 3u);
        expectAllocateNodeAction  (actions[actionIdx++], child2, 0u);
        expectAllocateNodeAction  (actions[actionIdx++], child3, 0u);
        expectAllocateNodeAction  (actions[actionIdx++], child1, 0u);

        expectAddChildToNodeAction(actions[actionIdx++], parent, child1);
        expectAddChildToNodeAction(actions[actionIdx++], parent, child2);
        expectAddChildToNodeAction(actions[actionIdx++], parent, child3);
    }

    TEST_F(SceneDescriberTest, skipSceneActionForDataInstancesWithBinaryDataEqualZero)
    {
        const UInt32 dataFieldElementCount = 3u;
        const DataFieldInfoVector dataFieldInfos =
        {
            { EDataType_Int32,     dataFieldElementCount, EFixedSemantics_Invalid },
            { EDataType_Float,     dataFieldElementCount, EFixedSemantics_Invalid },
            { EDataType_Vector2F,  dataFieldElementCount, EFixedSemantics_Invalid },
            { EDataType_Vector3F,  dataFieldElementCount, EFixedSemantics_Invalid },
            { EDataType_Vector4F,  dataFieldElementCount, EFixedSemantics_Invalid },
            { EDataType_Vector2I,  dataFieldElementCount, EFixedSemantics_Invalid },
            { EDataType_Vector3I,  dataFieldElementCount, EFixedSemantics_Invalid },
            { EDataType_Vector4I,  dataFieldElementCount, EFixedSemantics_Invalid },
            { EDataType_Matrix22F, dataFieldElementCount, EFixedSemantics_Invalid },
            { EDataType_Matrix33F, dataFieldElementCount, EFixedSemantics_Invalid },
            { EDataType_Matrix44F, dataFieldElementCount, EFixedSemantics_Invalid }
        };

        const DataLayoutHandle dataLayout = m_scene.allocateDataLayout(dataFieldInfos);
        const DataInstanceHandle dataInstance = m_scene.allocateDataInstance(dataLayout);

        SceneDescriber::describeScene<IScene>(m_scene, creator);

        ASSERT_EQ(2u, actions.numberOfActions());
        uint32_t actionIdx = 0u;
        EXPECT_EQ(ESceneActionId_AllocateDataLayout, actions[actionIdx++].type());
        EXPECT_EQ(ESceneActionId_AllocateDataInstance, actions[actionIdx++].type());
        // no actions for setting the zeroed data types

        Scene newScene;
        SceneActionApplierHelper sceneCreator(newScene);
        sceneCreator.applyActionsOnScene(actions);

        // validate skipped actions still result in nulled data
        EXPECT_TRUE(MemoryUtils::AreAllBytesZero(newScene.getDataIntegerArray(dataInstance, DataFieldHandle(0u)), dataFieldElementCount));
        EXPECT_TRUE(MemoryUtils::AreAllBytesZero(newScene.getDataFloatArray(dataInstance, DataFieldHandle(1u)), dataFieldElementCount));
        EXPECT_TRUE(MemoryUtils::AreAllBytesZero(newScene.getDataVector2fArray(dataInstance, DataFieldHandle(2u)), dataFieldElementCount));
        EXPECT_TRUE(MemoryUtils::AreAllBytesZero(newScene.getDataVector3fArray(dataInstance, DataFieldHandle(3u)), dataFieldElementCount));
        EXPECT_TRUE(MemoryUtils::AreAllBytesZero(newScene.getDataVector4fArray(dataInstance, DataFieldHandle(4u)), dataFieldElementCount));
        EXPECT_TRUE(MemoryUtils::AreAllBytesZero(newScene.getDataVector2iArray(dataInstance, DataFieldHandle(5u)), dataFieldElementCount));
        EXPECT_TRUE(MemoryUtils::AreAllBytesZero(newScene.getDataVector3iArray(dataInstance, DataFieldHandle(6u)), dataFieldElementCount));
        EXPECT_TRUE(MemoryUtils::AreAllBytesZero(newScene.getDataVector4iArray(dataInstance, DataFieldHandle(7u)), dataFieldElementCount));
        EXPECT_TRUE(MemoryUtils::AreAllBytesZero(newScene.getDataMatrix22fArray(dataInstance, DataFieldHandle(8u)), dataFieldElementCount));
        EXPECT_TRUE(MemoryUtils::AreAllBytesZero(newScene.getDataMatrix33fArray(dataInstance, DataFieldHandle(9u)), dataFieldElementCount));
        EXPECT_TRUE(MemoryUtils::AreAllBytesZero(newScene.getDataMatrix44fArray(dataInstance, DataFieldHandle(10u)), dataFieldElementCount));
    }

    TEST_F(SceneDescriberTest, checksDescriptionActionsForSceneWithRenderableAndCompoundAction)
    {
        createRenderable();
        SceneDescriber::describeScene<IScene>(m_scene, creator);

        ASSERT_EQ(2u, actions.numberOfActions());
        EXPECT_EQ(1u, SceneActionCollectionUtils::CountNumberOfActionsOfType(actions, ESceneActionId_CompoundRenderable));
    }

    TEST_F(SceneDescriberTest, checksDescriptionActionsForSceneWithStateAndCompoundAction)
    {
        createState();
        SceneDescriber::describeScene<IScene>(m_scene, creator);

        ASSERT_EQ(1u, actions.numberOfActions());
        EXPECT_EQ(1u, SceneActionCollectionUtils::CountNumberOfActionsOfType(actions, ESceneActionId_CompoundState));
    }

    TEST_F(SceneDescriberTest, sameAmountOfDataLayoutsCreatedIsSerializedToSceneActions_withoutCompacting)
    {
        const DataLayoutHandle dataLayout1 = m_scene.allocateDataLayout({ { EDataType_Float },{ EDataType_Float } });
        const DataLayoutHandle dataLayout2 = m_scene.allocateDataLayout({ { EDataType_Float },{ EDataType_Float } });
        const DataLayoutHandle dataLayout3 = m_scene.allocateDataLayout({ { EDataType_Float },{ EDataType_Float } });

        // the test itself does not need to test that the compacting happened or not
        // but it is actually the main purpose to check that the describer properly expands the actions if compacted
        EXPECT_EQ(3u, m_scene.getDataLayoutCount());
        EXPECT_NE(dataLayout1, dataLayout2);
        EXPECT_NE(dataLayout2, dataLayout3);

        SceneDescriber::describeScene<IScene>(m_scene, creator);

        ASSERT_EQ(3u, actions.numberOfActions());
        EXPECT_EQ(3u, SceneActionCollectionUtils::CountNumberOfActionsOfType(actions, ESceneActionId_AllocateDataLayout));
    }

    TEST_F(SceneDescriberTest, sameAmountOfDataLayoutsCreatedIsSerializedToSceneActions_compactingUsed)
    {
        ClientScene scene;
        const DataLayoutHandle dataLayout1 = scene.allocateDataLayout({ { EDataType_Float },{ EDataType_Float } });
        const DataLayoutHandle dataLayout2 = scene.allocateDataLayout({ { EDataType_Float },{ EDataType_Float } });
        const DataLayoutHandle dataLayout3 = scene.allocateDataLayout({ { EDataType_Float },{ EDataType_Float } });

        // the test itself does not need to test that the compacting happened or not
        // but it is actually the main purpose to check that the describer properly expands the actions if compacted
        EXPECT_EQ(1u, scene.getDataLayoutCount());
        EXPECT_EQ(dataLayout1, dataLayout2);
        EXPECT_EQ(dataLayout2, dataLayout3);
        EXPECT_EQ(3u, scene.getNumDataLayoutReferences(dataLayout1));

        SceneDescriber::describeScene<ClientScene>(scene, creator);

        ASSERT_EQ(3u, actions.numberOfActions());
        EXPECT_EQ(3u, SceneActionCollectionUtils::CountNumberOfActionsOfType(actions, ESceneActionId_AllocateDataLayout));
    }
}
