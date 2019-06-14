//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDEREXECUTORINTERNALRENDERSTATES_H
#define RAMSES_RENDEREXECUTORINTERNALRENDERSTATES_H

#include "SceneAPI/RenderState.h"
#include "PlatformAbstraction/PlatformTypes.h"

namespace ramses_internal
{
    // Initial values of these states must be invalid in the sense that whatever values come
    // from scene's render states set by user must compare NOT equal
    // in order to initiate the device states.

    struct DepthStencilState
    {
        EDepthFunc      m_depthFunc = EDepthFunc::NUMBER_OF_ELEMENTS;
        EDepthWrite     m_depthWrite = EDepthWrite::NUMBER_OF_ELEMENTS;
        EStencilFunc    m_stencilFunc = EStencilFunc::NUMBER_OF_ELEMENTS;
        EStencilOp      m_stencilOpFail = EStencilOp::NUMBER_OF_ELEMENTS;
        EStencilOp      m_stencilOpDepthFail = EStencilOp::NUMBER_OF_ELEMENTS;
        EStencilOp      m_stencilOpDepthPass = EStencilOp::NUMBER_OF_ELEMENTS;
        UInt8           m_stencilMask = 0;
        UInt8           m_stencilRefValue = 0;

        Bool operator!=(const DepthStencilState& other) const
        {
            return m_depthFunc != other.m_depthFunc
                || m_depthWrite != other.m_depthWrite
                || m_stencilFunc != other.m_stencilFunc
                || m_stencilOpFail != other.m_stencilOpFail
                || m_stencilOpDepthFail != other.m_stencilOpDepthFail
                || m_stencilOpDepthPass != other.m_stencilOpDepthPass
                || m_stencilRefValue != other.m_stencilRefValue
                || m_stencilMask != other.m_stencilMask;
        }
    };

    // TODO Mohamed : merge into RasterizerState once caching fixed
    struct ScissorState
    {
        EScissorTest m_scissorTest = EScissorTest::NUMBER_OF_ELEMENTS;
        RenderState::ScissorRegion m_scissorRegion = {0, 0, 0, 0};
    };

    struct BlendState
    {
        EBlendFactor    m_blendFactorSrcColor = EBlendFactor::NUMBER_OF_ELEMENTS;
        EBlendFactor    m_blendFactorDstColor = EBlendFactor::NUMBER_OF_ELEMENTS;
        EBlendFactor    m_blendFactorSrcAlpha = EBlendFactor::NUMBER_OF_ELEMENTS;
        EBlendFactor    m_blendFactorDstAlpha = EBlendFactor::NUMBER_OF_ELEMENTS;
        EBlendOperation m_blendOperationColor = EBlendOperation::NUMBER_OF_ELEMENTS;
        EBlendOperation m_blendOperationAlpha = EBlendOperation::NUMBER_OF_ELEMENTS;
        ColorWriteMask  m_colorWriteMask = 0;

        Bool operator!=(const BlendState& other) const
        {
            return m_blendFactorSrcColor != other.m_blendFactorSrcColor
                || m_blendFactorDstColor != other.m_blendFactorDstColor
                || m_blendFactorSrcAlpha != other.m_blendFactorSrcAlpha
                || m_blendFactorDstAlpha != other.m_blendFactorDstAlpha
                || m_blendOperationColor != other.m_blendOperationColor
                || m_blendOperationAlpha != other.m_blendOperationAlpha
                || m_colorWriteMask != other.m_colorWriteMask;
        }
    };

    struct RasterizerState
    {
        ECullMode       m_cullMode = ECullMode::NUMBER_OF_ELEMENTS;
        EDrawMode       m_drawMode = EDrawMode::NUMBER_OF_ELEMENTS;

        Bool operator!=(const RasterizerState& other) const
        {
            return m_cullMode != other.m_cullMode
                || m_drawMode != other.m_drawMode;
        }
    };
}

#endif
