//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gmock/gmock.h"
#include "internal/SceneGraph/Scene/SceneActionCollection.h"
#include "internal/SceneGraph/Scene/SceneDescriber.h"
#include "internal/SceneGraph/Scene/ClientScene.h"
#include "SceneActionUtils.h"
#include "internal/SceneGraph/Scene/SceneActionCollectionCreator.h"
#include "internal/SceneGraph/Scene/SceneActionApplier.h"
#include "TestEqualHelper.h"
#include "internal/Core/Utils/MemoryUtils.h"
#include "internal/SceneGraph/SceneAPI/Handles.h"
#include "internal/SceneGraph/SceneAPI/PixelRectangle.h"
#include "internal/SceneGraph/SceneAPI/RenderBuffer.h"

using namespace testing;

namespace ramses::internal
{
    class SceneDescriberTest : public testing::Test
    {
    protected:
        SceneDescriberTest()
            : creator(actions, EFeatureLevel_Latest)
        {}

        static void ExpectAllocateNodeAction(SceneActionCollection::SceneActionReader action, NodeHandle handle, uint32_t expectedChildrenCount)
        {
            EXPECT_EQ(ESceneActionId::AllocateNode, action.type());
            NodeHandle actualHandle;
            uint32_t actualChidlrenCount = 0;
            action.read(actualChidlrenCount);
            action.read(actualHandle);
            EXPECT_EQ(expectedChildrenCount, actualChidlrenCount);
            EXPECT_EQ(handle, actualHandle);
        }

        static void ExpectAddChildToNodeAction(SceneActionCollection::SceneActionReader action, NodeHandle parent, NodeHandle child)
        {
            EXPECT_EQ(ESceneActionId::AddChildToNode, action.type());
            NodeHandle actualParent;
            NodeHandle actualChild;
            action.read(actualParent);
            action.read(actualChild);
            EXPECT_EQ(parent, actualParent);
            EXPECT_EQ(child, actualChild);
        }

        static void ExpectAllocateRenderBufferAction(SceneActionCollection::SceneActionReader action, RenderBufferHandle handle, const RenderBuffer& renderBuffer)
        {
            ASSERT_EQ(ESceneActionId::AllocateRenderBuffer, action.type());

            RenderBufferHandle actualHandle;
            RenderBuffer actualRenderBuffer;
            uint32_t enumInt = 0;

            action.read(actualRenderBuffer.width);
            action.read(actualRenderBuffer.height);
            action.read(actualHandle);
            action.read(enumInt);
            actualRenderBuffer.format = static_cast<EPixelStorageFormat>(enumInt);
            action.read(enumInt);
            actualRenderBuffer.accessMode = static_cast<ERenderBufferAccessMode>(enumInt);
            action.read(actualRenderBuffer.sampleCount);

            EXPECT_EQ(actualHandle, handle);
            EXPECT_EQ(actualRenderBuffer.width, renderBuffer.width);
            EXPECT_EQ(actualRenderBuffer.height, renderBuffer.height);
            EXPECT_EQ(actualRenderBuffer.format, renderBuffer.format);
            EXPECT_EQ(actualRenderBuffer.accessMode, renderBuffer.accessMode);
            EXPECT_EQ(actualRenderBuffer.sampleCount, renderBuffer.sampleCount);

            EXPECT_TRUE(action.isFullyRead());
        }

        static void ExpectAllocateBlitPassAction(SceneActionCollection::SceneActionReader action, BlitPassHandle handle, RenderBufferHandle sourceRenderBufferHandle, RenderBufferHandle destinationRenderbufferHandle)
        {
            ASSERT_EQ(ESceneActionId::AllocateBlitPass, action.type());

            BlitPassHandle actualHandle;
            RenderBufferHandle actualSourceRenderbufferHandle;
            RenderBufferHandle actualDestinationRenderbufferHandle;

            action.read(actualSourceRenderbufferHandle);
            action.read(actualDestinationRenderbufferHandle);
            action.read(actualHandle);

            EXPECT_EQ(actualHandle, handle);
            EXPECT_EQ(actualSourceRenderbufferHandle, sourceRenderBufferHandle);
            EXPECT_EQ(actualDestinationRenderbufferHandle, destinationRenderbufferHandle);

            EXPECT_TRUE(action.isFullyRead());
        }

        static void ExpectSetBlitPassRegionsAction(SceneActionCollection::SceneActionReader action, BlitPassHandle handle, const PixelRectangle& sourceRegion, const PixelRectangle& destinationRegion)
        {
            ASSERT_EQ(ESceneActionId::SetBlitPassRegions, action.type());

            BlitPassHandle actualHandle;
            PixelRectangle actualSourceRegion;
            PixelRectangle actualDestinationRegion;

            action.read(actualHandle);
            action.read(actualSourceRegion.x);
            action.read(actualSourceRegion.y);
            action.read(actualSourceRegion.width);
            action.read(actualSourceRegion.height);
            action.read(actualDestinationRegion.x);
            action.read(actualDestinationRegion.y);
            action.read(actualDestinationRegion.width);
            action.read(actualDestinationRegion.height);

            EXPECT_EQ(handle, actualHandle);
            EXPECT_EQ(sourceRegion.x, actualSourceRegion.x);
            EXPECT_EQ(sourceRegion.y, actualSourceRegion.y);
            EXPECT_EQ(sourceRegion.width, actualSourceRegion.width);
            EXPECT_EQ(sourceRegion.height, actualSourceRegion.height);
            EXPECT_EQ(destinationRegion.x, actualDestinationRegion.x);
            EXPECT_EQ(destinationRegion.y, actualDestinationRegion.y);
            EXPECT_EQ(destinationRegion.width, actualDestinationRegion.width);
            EXPECT_EQ(destinationRegion.height, actualDestinationRegion.height);

            EXPECT_TRUE(action.isFullyRead());
        }

        static void ExpectSetBlitPassRenderOrderAction(SceneActionCollection::SceneActionReader action, BlitPassHandle handle, int32_t renderOrder)
        {
            ASSERT_EQ(ESceneActionId::SetBlitPassRenderOrder, action.type());

            BlitPassHandle actualHandle;
            int32_t actualRenderOrder {};
            action.read(actualHandle);
            action.read(actualRenderOrder);

            EXPECT_EQ(handle, actualHandle);
            EXPECT_EQ(renderOrder, actualRenderOrder);

            EXPECT_TRUE(action.isFullyRead());
        };

        static void ExpectSetBlitPassEnabledAction(SceneActionCollection::SceneActionReader action, BlitPassHandle handle, bool enabled)
        {
            ASSERT_EQ(ESceneActionId::SetBlitPassEnabled, action.type());

            BlitPassHandle actualHandle;
            bool actualEnabled {};
            action.read(actualHandle);
            action.read(actualEnabled);

            EXPECT_EQ(handle, actualHandle);
            EXPECT_EQ(enabled, actualEnabled);

            EXPECT_TRUE(action.isFullyRead());
        };

        struct RenderableCreationData
        {
            RenderableCreationData() {} // NOLINT(modernize-use-equals-default): build issues with clang-12
            uint32_t              startIndex{3u};
            uint32_t              indexCount{9u};
            uint32_t              instanceCount{7u};
            EVisibilityMode     visibility{EVisibilityMode::Invisible};
            RenderStateHandle   state{23u};
            ResourceContentHash effectHash{47u, 0u};
            DataInstanceHandle  geoInstanceHandle{37u};
            DataInstanceHandle  uniformInstanceHandle{77u};
            uint32_t              startVertex{199u};
        };

        void createRenderable(const RenderableCreationData& data = RenderableCreationData{})
        {
            const NodeHandle        node            = m_scene.allocateNode(0, {});
            const RenderableHandle  renderable      = m_scene.allocateRenderable(node, {});
            m_scene.setRenderableStartIndex(renderable, data.startIndex);
            m_scene.setRenderableIndexCount(renderable, data.indexCount);
            m_scene.setRenderableRenderState(renderable, data.state);
            m_scene.setRenderableVisibility(renderable, data.visibility);
            m_scene.setRenderableInstanceCount(renderable, data.instanceCount);
            m_scene.setRenderableStartVertex(renderable, data.startVertex);
            m_scene.setRenderableDataInstance(renderable, ERenderableDataSlotType_Geometry, data.geoInstanceHandle);
            m_scene.setRenderableDataInstance(renderable, ERenderableDataSlotType_Uniforms, data.uniformInstanceHandle);
        }

        struct StateCreationData
        {
            StateCreationData() {} // NOLINT(modernize-use-equals-default): build issues with clang-12
            EBlendFactor    bfSrcColor{EBlendFactor::DstAlpha};
            EBlendFactor    bfDstColor{EBlendFactor::One};
            EBlendFactor    bfSrcAlpha{EBlendFactor::OneMinusSrcAlpha};
            EBlendFactor    bfDstAlpha{EBlendFactor::SrcAlpha};
            EBlendOperation boColor{EBlendOperation::Subtract};
            EBlendOperation boAlpha{EBlendOperation::Max};
            glm::vec4       blendColor{0.1f, 0.2f, 0.3f, 0.4f};
            ECullMode       cullMode{ECullMode::BackFacing};
            EDrawMode       drawMode{EDrawMode::Lines};
            EDepthWrite     depthWrite{EDepthWrite::Enabled};
            EDepthFunc      depthFunc{EDepthFunc::LessEqual};
            EScissorTest    scissorTest{EScissorTest::Enabled};
            RenderState::ScissorRegion scissorRegion{12, 14, 16, 18};
            EStencilFunc    stencilFunc{EStencilFunc::NotEqual};
            uint8_t         stencilRefValue{99u};
            uint8_t         stencilMask{3u};
            EStencilOp      stencilOpFail{EStencilOp::Replace};
            EStencilOp      stencilOpDepthFail{EStencilOp::Decrement};
            EStencilOp      stencilOpDepthPass{EStencilOp::Zero};
            ColorWriteMask  colorWriteMask{0x80};
        };

        void createState(const StateCreationData& data = StateCreationData())
        {
            const RenderStateHandle state = m_scene.allocateRenderState({});
            m_scene.setRenderStateBlendFactors(   state, data.bfSrcColor, data.bfDstColor, data.bfSrcAlpha, data.bfDstAlpha);
            m_scene.setRenderStateBlendOperations(state, data.boColor, data.boAlpha);
            m_scene.setRenderStateBlendColor(     state, data.blendColor);
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
        NodeHandle parent = m_scene.allocateNode(0, {});
        NodeHandle child2 = m_scene.allocateNode(0, {});
        NodeHandle child3 = m_scene.allocateNode(0, {});
        NodeHandle child1 = m_scene.allocateNode(0, {});

        m_scene.addChildToNode(parent, child1);
        m_scene.addChildToNode(parent, child2);
        m_scene.addChildToNode(parent, child3);

        SceneDescriber::describeScene<IScene>(m_scene, creator);

        ASSERT_EQ(7u, actions.numberOfActions());
        uint32_t actionIdx = 0u;

        ExpectAllocateNodeAction  (actions[actionIdx++], parent, 3u);
        ExpectAllocateNodeAction  (actions[actionIdx++], child2, 0u);
        ExpectAllocateNodeAction  (actions[actionIdx++], child3, 0u);
        ExpectAllocateNodeAction  (actions[actionIdx++], child1, 0u);

        ExpectAddChildToNodeAction(actions[actionIdx++], parent, child1);
        ExpectAddChildToNodeAction(actions[actionIdx++], parent, child2);
        ExpectAddChildToNodeAction(actions[actionIdx++], parent, child3);
    }

    TEST_F(SceneDescriberTest, skipSceneActionForDataInstancesWithBinaryDataEqualZero)
    {
        const uint32_t dataFieldElementCount = 3u;
        const DataFieldInfoVector dataFieldInfos =
        {
            DataFieldInfo{ EDataType::Int32,     dataFieldElementCount, EFixedSemantics::Invalid },
            DataFieldInfo{ EDataType::Float,     dataFieldElementCount, EFixedSemantics::Invalid },
            DataFieldInfo{ EDataType::Vector2F,  dataFieldElementCount, EFixedSemantics::Invalid },
            DataFieldInfo{ EDataType::Vector3F,  dataFieldElementCount, EFixedSemantics::Invalid },
            DataFieldInfo{ EDataType::Vector4F,  dataFieldElementCount, EFixedSemantics::Invalid },
            DataFieldInfo{ EDataType::Vector2I,  dataFieldElementCount, EFixedSemantics::Invalid },
            DataFieldInfo{ EDataType::Vector3I,  dataFieldElementCount, EFixedSemantics::Invalid },
            DataFieldInfo{ EDataType::Vector4I,  dataFieldElementCount, EFixedSemantics::Invalid },
            DataFieldInfo{ EDataType::Matrix22F, dataFieldElementCount, EFixedSemantics::Invalid },
            DataFieldInfo{ EDataType::Matrix33F, dataFieldElementCount, EFixedSemantics::Invalid },
            DataFieldInfo{ EDataType::Matrix44F, dataFieldElementCount, EFixedSemantics::Invalid },
            DataFieldInfo{ EDataType::Bool,      dataFieldElementCount, EFixedSemantics::Invalid }
        };

        const DataLayoutHandle dataLayout = m_scene.allocateDataLayout(dataFieldInfos, ResourceContentHash::Invalid(), {});
        const DataInstanceHandle dataInstance = m_scene.allocateDataInstance(dataLayout, {});

        SceneDescriber::describeScene<IScene>(m_scene, creator);

        ASSERT_EQ(2u, actions.numberOfActions());
        uint32_t actionIdx = 0u;
        EXPECT_EQ(ESceneActionId::AllocateDataLayout, actions[actionIdx++].type());
        EXPECT_EQ(ESceneActionId::AllocateDataInstance, actions[actionIdx++].type());
        // no actions for setting the zeroed data types

        Scene newScene;
        SceneActionApplier::ApplyActionsOnScene(newScene, actions, EFeatureLevel_Latest);

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
        EXPECT_TRUE(MemoryUtils::AreAllBytesZero(newScene.getDataBooleanArray(dataInstance, DataFieldHandle(11u)), dataFieldElementCount));
    }

    TEST_F(SceneDescriberTest, checksDescriptionActionsForSceneWithRenderableAndCompoundAction)
    {
        createRenderable();
        SceneDescriber::describeScene<IScene>(m_scene, creator);

        ASSERT_EQ(2u, actions.numberOfActions());
        EXPECT_EQ(1u, SceneActionCollectionUtils::CountNumberOfActionsOfType(actions, ESceneActionId::CompoundRenderable));
    }

    TEST_F(SceneDescriberTest, checksDescriptionActionsForSceneWithStateAndCompoundAction)
    {
        createState();
        SceneDescriber::describeScene<IScene>(m_scene, creator);

        ASSERT_EQ(1u, actions.numberOfActions());
        EXPECT_EQ(1u, SceneActionCollectionUtils::CountNumberOfActionsOfType(actions, ESceneActionId::CompoundState));
    }

    TEST_F(SceneDescriberTest, sameAmountOfDataLayoutsCreatedIsSerializedToSceneActions_withoutCompacting)
    {
        const DataLayoutHandle dataLayout1 = m_scene.allocateDataLayout({ DataFieldInfo{ EDataType::Float }, DataFieldInfo{ EDataType::Float } }, ResourceContentHash(123u, 0u), {});
        const DataLayoutHandle dataLayout2 = m_scene.allocateDataLayout({ DataFieldInfo{ EDataType::Float }, DataFieldInfo{ EDataType::Float } }, ResourceContentHash(123u, 0u), {});
        const DataLayoutHandle dataLayout3 = m_scene.allocateDataLayout({ DataFieldInfo{ EDataType::Float }, DataFieldInfo{ EDataType::Float } }, ResourceContentHash(123u, 0u), {});

        // the test itself does not need to test that the compacting happened or not
        // but it is actually the main purpose to check that the describer properly expands the actions if compacted
        EXPECT_EQ(3u, m_scene.getDataLayoutCount());
        EXPECT_NE(dataLayout1, dataLayout2);
        EXPECT_NE(dataLayout2, dataLayout3);

        SceneDescriber::describeScene<IScene>(m_scene, creator);

        ASSERT_EQ(3u, actions.numberOfActions());
        EXPECT_EQ(3u, SceneActionCollectionUtils::CountNumberOfActionsOfType(actions, ESceneActionId::AllocateDataLayout));
    }

    TEST_F(SceneDescriberTest, sameAmountOfDataLayoutsCreatedIsSerializedToSceneActions_compactingUsed)
    {
        ClientScene scene;
        const DataLayoutHandle dataLayout1 = scene.allocateDataLayout({ DataFieldInfo{ EDataType::Float }, DataFieldInfo{ EDataType::Float } }, ResourceContentHash(123u, 0u), {});
        const DataLayoutHandle dataLayout2 = scene.allocateDataLayout({ DataFieldInfo{ EDataType::Float }, DataFieldInfo{ EDataType::Float } }, ResourceContentHash(123u, 0u), {});
        const DataLayoutHandle dataLayout3 = scene.allocateDataLayout({ DataFieldInfo{ EDataType::Float }, DataFieldInfo{ EDataType::Float } }, ResourceContentHash(123u, 0u), {});

        // the test itself does not need to test that the compacting happened or not
        // but it is actually the main purpose to check that the describer properly expands the actions if compacted
        EXPECT_EQ(1u, scene.getDataLayoutCount());
        EXPECT_EQ(dataLayout1, dataLayout2);
        EXPECT_EQ(dataLayout2, dataLayout3);
        EXPECT_EQ(3u, scene.getNumDataLayoutReferences(dataLayout1));

        SceneDescriber::describeScene<ClientScene>(scene, creator);

        ASSERT_EQ(3u, actions.numberOfActions());
        EXPECT_EQ(3u, SceneActionCollectionUtils::CountNumberOfActionsOfType(actions, ESceneActionId::AllocateDataLayout));
    }

    TEST_F(SceneDescriberTest, checksDescriptionActionsForBlitPassesAndRenderBuffers)
    {
        const ramses::internal::RenderBuffer renderBuffer {
            120u,
            240u,
            EPixelStorageFormat::RGBA8,
            ERenderBufferAccessMode::ReadWrite,
            1
        };
        const PixelRectangle sourceRegion {
            12, 21, 123, 321
        };
        const PixelRectangle destinationRegion {
            23, 32, 234, 432
        };
        const int32_t renderOrder = 7;
        const bool blitPassEnabled = true;

        const auto renderBufferSourceHandle = m_scene.allocateRenderBuffer(renderBuffer, {});
        const auto renderBufferDestinationHandle = m_scene.allocateRenderBuffer(renderBuffer, {});

        const auto blitPassHandle = m_scene.allocateBlitPass(renderBufferSourceHandle, renderBufferDestinationHandle, {});
        m_scene.setBlitPassRegions(blitPassHandle, sourceRegion, destinationRegion);
        m_scene.setBlitPassRenderOrder(blitPassHandle, renderOrder);
        m_scene.setBlitPassEnabled(blitPassHandle, blitPassEnabled);

        SceneDescriber::describeScene<IScene>(m_scene, creator);

        ASSERT_EQ(6u, actions.numberOfActions());
        uint32_t actionIdx = 0u;

        // order of actions is curcial
        // first we need to allocate render buffers
        // then allocate blit passes
        ExpectAllocateRenderBufferAction(actions[actionIdx++], renderBufferSourceHandle, renderBuffer);
        ExpectAllocateRenderBufferAction(actions[actionIdx++], renderBufferDestinationHandle, renderBuffer);
        ExpectAllocateBlitPassAction(actions[actionIdx++], blitPassHandle, renderBufferSourceHandle, renderBufferDestinationHandle);
        ExpectSetBlitPassRegionsAction(actions[actionIdx++], blitPassHandle, sourceRegion, destinationRegion);
        ExpectSetBlitPassRenderOrderAction(actions[actionIdx++], blitPassHandle, renderOrder);
        ExpectSetBlitPassEnabledAction(actions[actionIdx++], blitPassHandle, blitPassEnabled);
    }
}
