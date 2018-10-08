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
        EXPECT_EQ(EBlendFactor_SrcAlpha     , rs.blendFactorSrcColor);
        EXPECT_EQ(EBlendFactor_OneMinusSrcAlpha, rs.blendFactorDstColor);
        EXPECT_EQ(EBlendFactor_One          , rs.blendFactorSrcAlpha);
        EXPECT_EQ(EBlendFactor_One          , rs.blendFactorDstAlpha);
        EXPECT_EQ(EBlendOperation_Disabled  , rs.blendOperationColor);
        EXPECT_EQ(EBlendOperation_Disabled  , rs.blendOperationAlpha);
        EXPECT_EQ(ECullMode_BackFacing      , rs.cullMode);
        EXPECT_EQ(EDrawMode_Triangles       , rs.drawMode);
        EXPECT_EQ(EDepthFunc_SmallerEqual   , rs.depthFunc);
        EXPECT_EQ(EDepthWrite_Enabled       , rs.depthWrite);
        EXPECT_EQ(EStencilFunc_Disabled     , rs.stencilFunc);
        EXPECT_EQ(0u                        , rs.stencilRefValue);
        EXPECT_EQ(0xFF                      , rs.stencilMask);
        EXPECT_EQ(EStencilOp_Keep           , rs.stencilOpFail);
        EXPECT_EQ(EStencilOp_Keep           , rs.stencilOpDepthFail);
        EXPECT_EQ(EStencilOp_Keep           , rs.stencilOpDepthPass);
    }

    TYPED_TEST(AScene, SetsStateBlendFactors)
    {
        const RenderStateHandle state = this->m_scene.allocateRenderState();
        this->m_scene.setRenderStateBlendFactors(state, EBlendFactor_One, EBlendFactor_SrcAlpha, EBlendFactor_OneMinusSrcAlpha, EBlendFactor_DstAlpha);

        const RenderState& rs = this->m_scene.getRenderState(state);
        EXPECT_EQ(EBlendFactor_One, rs.blendFactorSrcColor);
        EXPECT_EQ(EBlendFactor_SrcAlpha, rs.blendFactorDstColor);
        EXPECT_EQ(EBlendFactor_OneMinusSrcAlpha, rs.blendFactorSrcAlpha);
        EXPECT_EQ(EBlendFactor_DstAlpha, rs.blendFactorDstAlpha);
    }

    TYPED_TEST(AScene, SetsStateBlendOperations)
    {
        const RenderStateHandle state = this->m_scene.allocateRenderState();
        this->m_scene.setRenderStateBlendOperations(state, EBlendOperation_Add, EBlendOperation_Subtract);

        const RenderState& rs = this->m_scene.getRenderState(state);
        EXPECT_EQ(EBlendOperation_Add, rs.blendOperationColor);
        EXPECT_EQ(EBlendOperation_Subtract, rs.blendOperationAlpha);
    }

    TYPED_TEST(AScene, SetsStateCullMode)
    {
        const RenderStateHandle state = this->m_scene.allocateRenderState();
        this->m_scene.setRenderStateCullMode(state, ECullMode_FrontFacing);

        const RenderState& rs = this->m_scene.getRenderState(state);
        EXPECT_EQ(ECullMode_FrontFacing, rs.cullMode);
    }

    TYPED_TEST(AScene, SetsStateDrawMode)
    {
        const RenderStateHandle state = this->m_scene.allocateRenderState();
        this->m_scene.setRenderStateDrawMode(state, EDrawMode_Triangles);

        const RenderState& rs = this->m_scene.getRenderState(state);
        EXPECT_EQ(EDrawMode_Triangles, rs.drawMode);
    }

    TYPED_TEST(AScene, SetsStateDepthWrite)
    {
        const RenderStateHandle state = this->m_scene.allocateRenderState();
        this->m_scene.setRenderStateDepthWrite(state, EDepthWrite_Disabled);

        const RenderState& rs = this->m_scene.getRenderState(state);
        EXPECT_EQ(EDepthWrite_Disabled, rs.depthWrite);
    }

    TYPED_TEST(AScene, SetsStateDepthFunc)
    {
        const RenderStateHandle state = this->m_scene.allocateRenderState();
        this->m_scene.setRenderStateDepthFunc(state, EDepthFunc_GreaterEqual);

        const RenderState& rs = this->m_scene.getRenderState(state);
        EXPECT_EQ(EDepthFunc_GreaterEqual, rs.depthFunc);
    }

    TYPED_TEST(AScene, SetsStateStencilFunc)
    {
        const RenderStateHandle state = this->m_scene.allocateRenderState();
        this->m_scene.setRenderStateStencilFunc(state, EStencilFunc_NotEqual, 12u, 0xAA);

        const RenderState& rs = this->m_scene.getRenderState(state);
        EXPECT_EQ(EStencilFunc_NotEqual     , rs.stencilFunc);
        EXPECT_EQ(12u                       , rs.stencilRefValue);
        EXPECT_EQ(0xAA                      , rs.stencilMask);
    }

    TYPED_TEST(AScene, SetsStateStencilOps)
    {
        const RenderStateHandle state = this->m_scene.allocateRenderState();
        this->m_scene.setRenderStateStencilOps(state, EStencilOp_Increment, EStencilOp_IncrementWrap, EStencilOp_Decrement);

        const RenderState& rs = this->m_scene.getRenderState(state);
        EXPECT_EQ(EStencilOp_Increment           , rs.stencilOpFail);
        EXPECT_EQ(EStencilOp_IncrementWrap      , rs.stencilOpDepthFail);
        EXPECT_EQ(EStencilOp_Decrement           , rs.stencilOpDepthPass);
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
