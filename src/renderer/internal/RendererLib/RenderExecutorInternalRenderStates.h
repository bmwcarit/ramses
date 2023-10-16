//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/SceneAPI/RenderState.h"

#include <cstdint>

namespace ramses::internal
{
    // Initial values of these states must be invalid in the sense that whatever values come
    // from scene's render states set by user must compare NOT equal
    // in order to initiate the device states.

    struct StencilState
    {
        EStencilFunc    m_stencilFunc{};
        EStencilOp      m_stencilOpFail{};
        EStencilOp      m_stencilOpDepthFail{};
        EStencilOp      m_stencilOpDepthPass{};
        uint8_t         m_stencilMask = 0;
        uint8_t         m_stencilRefValue = 0;

        bool operator!=(const StencilState& other) const
        {
            return m_stencilFunc != other.m_stencilFunc
                || m_stencilOpFail != other.m_stencilOpFail
                || m_stencilOpDepthFail != other.m_stencilOpDepthFail
                || m_stencilOpDepthPass != other.m_stencilOpDepthPass
                || m_stencilRefValue != other.m_stencilRefValue
                || m_stencilMask != other.m_stencilMask;
        }
    };

    struct ScissorState
    {
        EScissorTest m_scissorTest{};
        RenderState::ScissorRegion m_scissorRegion = {0, 0, 0, 0};

        bool operator!=(const ScissorState& other) const
        {
            return m_scissorTest != other.m_scissorTest
                || !(m_scissorRegion == other.m_scissorRegion);
        }

    };

    struct BlendOperationsState
    {
        EBlendOperation m_blendOperationColor{};
        EBlendOperation m_blendOperationAlpha{};

        bool operator!=(const BlendOperationsState& other) const
        {
            return m_blendOperationColor != other.m_blendOperationColor
                || m_blendOperationAlpha != other.m_blendOperationAlpha;
        }
    };

    struct BlendFactorsState
    {
        EBlendFactor    m_blendFactorSrcColor{};
        EBlendFactor    m_blendFactorDstColor{};
        EBlendFactor    m_blendFactorSrcAlpha{};
        EBlendFactor    m_blendFactorDstAlpha{};

        bool operator!=(const BlendFactorsState& other) const
        {
            return m_blendFactorSrcColor != other.m_blendFactorSrcColor
                || m_blendFactorDstColor != other.m_blendFactorDstColor
                || m_blendFactorSrcAlpha != other.m_blendFactorSrcAlpha
                || m_blendFactorDstAlpha != other.m_blendFactorDstAlpha;
        }
    };
}
