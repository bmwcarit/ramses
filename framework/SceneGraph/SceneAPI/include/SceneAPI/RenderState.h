//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENEAPI_RENDERSTATE_H
#define RAMSES_SCENEAPI_RENDERSTATE_H

#include "Common/BitForgeMacro.h"
#include "Utils/LoggingUtils.h"
#include "Math3d/Quad.h"
#include "DataTypesImpl.h"
#include "ramses-framework-api/AppearanceEnums.h"

namespace ramses_internal
{
    using ramses::EBlendOperation;
    using ramses::EBlendFactor;
    using ramses::ECullMode;
    using ramses::EDepthWrite;
    using ramses::EDepthFunc;
    using ramses::EScissorTest;
    using ramses::EStencilFunc;
    using EStencilOp = ramses::EStencilOperation;
    using ramses::EDrawMode;

    enum EColorWriteFlag : uint32_t
    {
        EColorWriteFlag_Red = BIT(0u),
        EColorWriteFlag_Green = BIT(1u),
        EColorWriteFlag_Blue = BIT(2u),
        EColorWriteFlag_Alpha = BIT(3u),
        EColorWriteFlag_All = (EColorWriteFlag_Red | EColorWriteFlag_Green | EColorWriteFlag_Blue | EColorWriteFlag_Alpha)
    };
    using ColorWriteMask = uint8_t;

    enum EClearFlags : uint32_t
    {
        EClearFlags_None    = 0,
        EClearFlags_Color   = BIT(0u),
        EClearFlags_Depth   = BIT(1u),
        EClearFlags_Stencil = BIT(2u),
        EClearFlags_All     = EClearFlags_Color | EClearFlags_Depth | EClearFlags_Stencil
    };

    constexpr bool IsClearFlagSet(uint32_t clearFlags, EClearFlags clearFlagToCheckIfSet)
    {
        return (clearFlags & clearFlagToCheckIfSet) == clearFlagToCheckIfSet;
    }

    struct RenderState
    {
        struct ScissorRegion
        {
            int16_t x = 0;
            int16_t y = 0;
            UInt16 width = 0u;
            UInt16 height = 0u;

            bool operator==(const ScissorRegion& o) const
            {
                return x == o.x && y == o.y && width == o.width && height == o.height;
            }
        };

        ScissorRegion   scissorRegion;
        glm::vec4       blendColor{ 0.f, 0.f, 0.f, 0.f };
        EBlendFactor    blendFactorSrcColor = EBlendFactor::SrcAlpha;
        EBlendFactor    blendFactorDstColor = EBlendFactor::OneMinusSrcAlpha;
        EBlendFactor    blendFactorSrcAlpha = EBlendFactor::One;
        EBlendFactor    blendFactorDstAlpha = EBlendFactor::One;
        EBlendOperation blendOperationColor = EBlendOperation::Disabled;
        EBlendOperation blendOperationAlpha = EBlendOperation::Disabled;
        ColorWriteMask  colorWriteMask = EColorWriteFlag_All;
        ECullMode       cullMode = ECullMode::BackFacing;
        EDrawMode       drawMode = EDrawMode::Triangles;
        EDepthFunc      depthFunc = EDepthFunc::LessEqual;
        EDepthWrite     depthWrite = EDepthWrite::Enabled;
        EScissorTest    scissorTest = EScissorTest::Disabled;
        EStencilFunc    stencilFunc = EStencilFunc::Disabled;
        EStencilOp      stencilOpFail = EStencilOp::Keep;
        EStencilOp      stencilOpDepthFail = EStencilOp::Keep;
        EStencilOp      stencilOpDepthPass = EStencilOp::Keep;
        uint8_t         stencilRefValue = 0;
        uint8_t         stencilMask = 0xFF;
        // explicit padding, consume when adding members
        uint8_t padding[2] = { 0 };  // NOLINT(modernize-avoid-c-arrays)
    };

    static constexpr size_t ExpectedSize = sizeof(RenderState::ScissorRegion) + sizeof(glm::vec4) + 4 * sizeof(EBlendFactor) + 2 * sizeof(EBlendOperation) + sizeof(ColorWriteMask)
        + sizeof(ECullMode) + sizeof(EDrawMode) + sizeof(EDepthFunc) + sizeof(EDepthWrite) + sizeof(EScissorTest) + sizeof(EStencilFunc)
        + 3 * sizeof(EStencilOp) + sizeof(RenderState::stencilRefValue) + sizeof(RenderState::stencilMask) + sizeof(RenderState::padding);
    static_assert(sizeof(RenderState) == ExpectedSize, "Avoid padding in this struct, add padding explicitly as member");
}

#endif

