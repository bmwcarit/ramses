//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "SceneTest.h"

using namespace testing;

namespace ramses_internal
{
    TYPED_TEST_CASE(AScene, SceneTypes);

    TYPED_TEST(AScene, ContainsCreatedState)
    {
        this->m_scene.allocateRenderState();
        RenderStateHandle state2 = this->m_scene.allocateRenderState();

        EXPECT_TRUE(this->m_scene.isRenderStateAllocated(state2));
    }

    TYPED_TEST(AScene, ContainsZeroTotalStatesUponCreation)
    {
        EXPECT_EQ(0u, this->m_scene.getRenderStateCount());
    }

    TYPED_TEST(AScene, CreatingStateIncreasesStateCount)
    {
        this->m_scene.allocateRenderState();

        EXPECT_EQ(1u, this->m_scene.getRenderStateCount());
    }

    TYPED_TEST(AScene, AssignsDefaultValuesToNewlyCreatedState)
    {
        const RenderStateHandle state = this->m_scene.allocateRenderState();

        const RenderState& rs = this->m_scene.getRenderState(state);
        const RenderState::ScissorRegion scissorRegion{ 0, 0, 0u, 0u };
        EXPECT_EQ(scissorRegion              , rs.scissorRegion);
        EXPECT_EQ(EBlendFactor::SrcAlpha     , rs.blendFactorSrcColor);
        EXPECT_EQ(EBlendFactor::OneMinusSrcAlpha, rs.blendFactorDstColor);
        EXPECT_EQ(EBlendFactor::One          , rs.blendFactorSrcAlpha);
        EXPECT_EQ(EBlendFactor::One          , rs.blendFactorDstAlpha);
        EXPECT_EQ(EBlendOperation::Disabled  , rs.blendOperationColor);
        EXPECT_EQ(EBlendOperation::Disabled  , rs.blendOperationAlpha);
        EXPECT_EQ(ECullMode::BackFacing      , rs.cullMode);
        EXPECT_EQ(EDrawMode::Triangles       , rs.drawMode);
        EXPECT_EQ(EDepthFunc::SmallerEqual   , rs.depthFunc);
        EXPECT_EQ(EDepthWrite::Enabled       , rs.depthWrite);
        EXPECT_EQ(EScissorTest::Disabled     , rs.scissorTest);
        EXPECT_EQ(EStencilFunc::Disabled     , rs.stencilFunc);
        EXPECT_EQ(0u                        , rs.stencilRefValue);
        EXPECT_EQ(0xFF                      , rs.stencilMask);
        EXPECT_EQ(EStencilOp::Keep           , rs.stencilOpFail);
        EXPECT_EQ(EStencilOp::Keep           , rs.stencilOpDepthFail);
        EXPECT_EQ(EStencilOp::Keep           , rs.stencilOpDepthPass);
    }

    TYPED_TEST(AScene, SetsStateBlendFactors)
    {
        const RenderStateHandle state = this->m_scene.allocateRenderState();
        this->m_scene.setRenderStateBlendFactors(state, EBlendFactor::One, EBlendFactor::SrcAlpha, EBlendFactor::OneMinusSrcAlpha, EBlendFactor::DstAlpha);

        const RenderState& rs = this->m_scene.getRenderState(state);
        EXPECT_EQ(EBlendFactor::One, rs.blendFactorSrcColor);
        EXPECT_EQ(EBlendFactor::SrcAlpha, rs.blendFactorDstColor);
        EXPECT_EQ(EBlendFactor::OneMinusSrcAlpha, rs.blendFactorSrcAlpha);
        EXPECT_EQ(EBlendFactor::DstAlpha, rs.blendFactorDstAlpha);
    }

    TYPED_TEST(AScene, SetsStateBlendOperations)
    {
        const RenderStateHandle state = this->m_scene.allocateRenderState();
        this->m_scene.setRenderStateBlendOperations(state, EBlendOperation::Add, EBlendOperation::Subtract);

        const RenderState& rs = this->m_scene.getRenderState(state);
        EXPECT_EQ(EBlendOperation::Add, rs.blendOperationColor);
        EXPECT_EQ(EBlendOperation::Subtract, rs.blendOperationAlpha);
    }

    TYPED_TEST(AScene, SetsStateCullMode)
    {
        const RenderStateHandle state = this->m_scene.allocateRenderState();
        this->m_scene.setRenderStateCullMode(state, ECullMode::FrontFacing);

        const RenderState& rs = this->m_scene.getRenderState(state);
        EXPECT_EQ(ECullMode::FrontFacing, rs.cullMode);
    }

    TYPED_TEST(AScene, SetsStateDrawMode)
    {
        const RenderStateHandle state = this->m_scene.allocateRenderState();
        this->m_scene.setRenderStateDrawMode(state, EDrawMode::Triangles);

        const RenderState& rs = this->m_scene.getRenderState(state);
        EXPECT_EQ(EDrawMode::Triangles, rs.drawMode);
    }

    TYPED_TEST(AScene, SetsStateDepthWrite)
    {
        const RenderStateHandle state = this->m_scene.allocateRenderState();
        this->m_scene.setRenderStateDepthWrite(state, EDepthWrite::Disabled);

        const RenderState& rs = this->m_scene.getRenderState(state);
        EXPECT_EQ(EDepthWrite::Disabled, rs.depthWrite);
    }

    TYPED_TEST(AScene, SetsStateDepthFunc)
    {
        const RenderStateHandle state = this->m_scene.allocateRenderState();
        this->m_scene.setRenderStateDepthFunc(state, EDepthFunc::GreaterEqual);

        const RenderState& rs = this->m_scene.getRenderState(state);
        EXPECT_EQ(EDepthFunc::GreaterEqual, rs.depthFunc);
    }

    TYPED_TEST(AScene, SetsStateScissorTest)
    {
        const RenderStateHandle state = this->m_scene.allocateRenderState();
        this->m_scene.setRenderStateScissorTest(state, EScissorTest::Enabled, { 1, 2, 3, 4 });

        const RenderState& rs = this->m_scene.getRenderState(state);
        EXPECT_EQ(EScissorTest::Enabled, rs.scissorTest);
        const RenderState::ScissorRegion scissorRegion{ 1, 2, 3, 4 };
        EXPECT_EQ(scissorRegion, rs.scissorRegion);
    }

    TYPED_TEST(AScene, SetsStateStencilFunc)
    {
        const RenderStateHandle state = this->m_scene.allocateRenderState();
        this->m_scene.setRenderStateStencilFunc(state, EStencilFunc::NotEqual, 12u, 0xAA);

        const RenderState& rs = this->m_scene.getRenderState(state);
        EXPECT_EQ(EStencilFunc::NotEqual     , rs.stencilFunc);
        EXPECT_EQ(12u                       , rs.stencilRefValue);
        EXPECT_EQ(0xAA                      , rs.stencilMask);
    }

    TYPED_TEST(AScene, SetsStateStencilOps)
    {
        const RenderStateHandle state = this->m_scene.allocateRenderState();
        this->m_scene.setRenderStateStencilOps(state, EStencilOp::Increment, EStencilOp::IncrementWrap, EStencilOp::Decrement);

        const RenderState& rs = this->m_scene.getRenderState(state);
        EXPECT_EQ(EStencilOp::Increment           , rs.stencilOpFail);
        EXPECT_EQ(EStencilOp::IncrementWrap      , rs.stencilOpDepthFail);
        EXPECT_EQ(EStencilOp::Decrement           , rs.stencilOpDepthPass);
    }

    TYPED_TEST(AScene, SetsStateColorWriteMask)
    {
        const RenderStateHandle state = this->m_scene.allocateRenderState();
        const ColorWriteMask colorMask = EColorWriteFlag_Green | EColorWriteFlag_Alpha;
        this->m_scene.setRenderStateColorWriteMask(state, colorMask);

        const RenderState& rs = this->m_scene.getRenderState(state);
        EXPECT_EQ(colorMask, rs.colorWriteMask);
    }
}
