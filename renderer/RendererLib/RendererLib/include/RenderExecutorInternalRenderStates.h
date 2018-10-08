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
    struct DepthStencilState
    {
        EDepthFunc      m_depthFunc;
        EDepthWrite     m_depthWrite;
        EStencilFunc    m_stencilFunc;
        EStencilOp      m_stencilOpFail;
        EStencilOp      m_stencilOpDepthFail;
        EStencilOp      m_stencilOpDepthPass;
        UInt32          m_stencilRefValue;
        UInt8           m_stencilMask;

        DepthStencilState()
        {
            PlatformMemory::Set(this, 0, sizeof(DepthStencilState));
            m_depthFunc = EDepthFunc_Invalid; // to invalidate initial state
        }

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

    struct BlendState
    {
        EBlendFactor    m_blendFactorSrcColor;
        EBlendFactor    m_blendFactorDstColor;
        EBlendFactor    m_blendFactorSrcAlpha;
        EBlendFactor    m_blendFactorDstAlpha;
        EBlendOperation m_blendOperationColor;
        EBlendOperation m_blendOperationAlpha;
        ColorWriteMask  m_colorWriteMask;

        BlendState()
        {
            PlatformMemory::Set(this, 0, sizeof(BlendState));
            m_blendFactorSrcColor = EBlendFactor_Invalid; // to invalidate initial state
        }

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
        ECullMode       m_cullMode;
        EDrawMode       m_drawMode;

        RasterizerState()
            : m_cullMode(ECullMode_Invalid) // to invalidate initial state
            , m_drawMode(EDrawMode_Triangles)
        {
        }

        Bool operator!=(const RasterizerState& other) const
        {
            return m_cullMode != other.m_cullMode
                || m_drawMode != other.m_drawMode;
        }
    };
}

#endif
