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
#include "Math3d/Vector4.h"

namespace ramses_internal
{
    enum class EBlendOperation : uint8_t
    {
        Disabled = 0,
        Add,
        Subtract,
        ReverseSubtract,
        Min,
        Max,
        NUMBER_OF_ELEMENTS
    };

    enum class EBlendFactor : uint8_t
    {
        Zero = 0,
        One,
        SrcAlpha,
        OneMinusSrcAlpha,
        DstAlpha,
        OneMinusDstAlpha,
        SrcColor,
        OneMinusSrcColor,
        DstColor,
        OneMinusDstColor,
        ConstColor,
        OneMinusConstColor,
        ConstAlpha,
        OneMinusConstAlpha,
        AlphaSaturate,
        Invalid,
        NUMBER_OF_ELEMENTS
    };

    enum class ECullMode : uint8_t
    {
        Disabled = 0,
        FrontFacing,
        BackFacing,
        FrontAndBackFacing,
        Invalid,
        NUMBER_OF_ELEMENTS
    };

    enum class EDepthWrite : uint8_t
    {
        Disabled = 0,
        Enabled,
        NUMBER_OF_ELEMENTS
    };

    enum class EDepthFunc : uint8_t
    {
        Disabled = 0,
        Greater,
        GreaterEqual,
        Smaller,
        SmallerEqual,
        Equal,
        NotEqual,
        AlwaysPass,
        NeverPass,
        Invalid,
        NUMBER_OF_ELEMENTS
    };

    enum class EScissorTest : uint8_t
    {
        Disabled = 0,
        Enabled,
        NUMBER_OF_ELEMENTS
    };

    enum class EStencilFunc : uint8_t
    {
        Disabled = 0,
        NeverPass,
        AlwaysPass,
        Equal,
        NotEqual,
        Less,
        LessEqual,
        Greater,
        GreaterEqual,
        NUMBER_OF_ELEMENTS
    };

    enum class EStencilOp : uint8_t
    {
        Keep = 0,
        Zero,
        Replace,
        Increment,
        IncrementWrap,
        Decrement,
        DecrementWrap,
        Invert,
        NUMBER_OF_ELEMENTS
    };

    enum class EDrawMode : uint8_t
    {
        Points = 0,
        Lines,
        LineLoop,
        Triangles,
        TriangleStrip,
        TriangleFan,
        LineStrip,
        NUMBER_OF_ELEMENTS
    };

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

    static const char* BlendOperationNames[] =
    {
        "Disabled",
        "Add",
        "Subtract",
        "ReverseSubtract",
        "Min",
        "Max",
    };

    static const char* BlendFactorNames[] =
    {
        "Zero",
        "One",
        "SrcAlpha",
        "OneMinusSrcAlpha",
        "DstAlpha",
        "OneMinusDstAlpha",
        "SrcColor",
        "OneMinusSrcColor",
        "DstColor",
        "OneMinusDstColor",
        "ConstColor",
        "OneMinusConstColor",
        "ConstAlpha",
        "OneMinusConstAlpha",
        "AlphaSaturate",
        "Invalid",
    };

    static const char* CullModeNames[] =
    {
        "Disabled",
        "FrontFacing",
        "BackFacing",
        "FrontAndBackFacing",
        "Invalid",
    };

    static const char* DepthWriteNames[] =
    {
        "Disabled",
        "Enabled",
    };

    static const char* DepthFuncNames[] =
    {
        "Disabled",
        "Greater",
        "GreaterEqual",
        "Less",
        "LessEqual",
        "Equal",
        "NotEqual",
        "Always",
        "Never",
        "Invalid",
    };

    static const char* ScissorTestNames[] =
    {
        "Disabled",
        "Enabled",
    };

    static const char* StencilFuncNames[] =
    {
        "Disabled",
        "Never",
        "Always",
        "Equal",
        "NotEqual",
        "Less",
        "LessEqual",
        "Greater",
        "GreaterEqual",
    };

    static const char* StencilOperationNames[] =
    {
        "Keep",
        "Zero",
        "Replace",
        "Increment",
        "IncrementWrap",
        "Decrement",
        "DecrementWrap",
        "Invert",
    };

    static const char* DrawModeNames[] =
    {
        "Points",
        "Lines",
        "LineLoop",
        "Triangles",
        "TriangleStrip",
        "TriangleFan",
        "LineStrip",
    };

    ENUM_TO_STRING(EBlendOperation, BlendOperationNames, EBlendOperation::NUMBER_OF_ELEMENTS);
    ENUM_TO_STRING(EBlendFactor, BlendFactorNames, EBlendFactor::NUMBER_OF_ELEMENTS);
    ENUM_TO_STRING(ECullMode, CullModeNames, ECullMode::NUMBER_OF_ELEMENTS);
    ENUM_TO_STRING(EDepthWrite, DepthWriteNames, EDepthWrite::NUMBER_OF_ELEMENTS);
    ENUM_TO_STRING(EDepthFunc, DepthFuncNames, EDepthFunc::NUMBER_OF_ELEMENTS);
    ENUM_TO_STRING(EStencilFunc, StencilFuncNames, EStencilFunc::NUMBER_OF_ELEMENTS);
    ENUM_TO_STRING(EScissorTest, ScissorTestNames, EScissorTest::NUMBER_OF_ELEMENTS);
    ENUM_TO_STRING(EStencilOp, StencilOperationNames, EStencilOp::NUMBER_OF_ELEMENTS);
    ENUM_TO_STRING(EDrawMode, DrawModeNames, EDrawMode::NUMBER_OF_ELEMENTS);

    struct RenderState
    {
        struct ScissorRegion
        {
            Int16 x = 0;
            Int16 y = 0;
            UInt16 width = 0u;
            UInt16 height = 0u;

            bool operator==(const ScissorRegion& o) const
            {
                return x == o.x && y == o.y && width == o.width && height == o.height;
            }
        };

        ScissorRegion   scissorRegion;
        Vector4         blendColor{ 0.f, 0.f, 0.f, 0.f };
        EBlendFactor    blendFactorSrcColor = EBlendFactor::SrcAlpha;
        EBlendFactor    blendFactorDstColor = EBlendFactor::OneMinusSrcAlpha;
        EBlendFactor    blendFactorSrcAlpha = EBlendFactor::One;
        EBlendFactor    blendFactorDstAlpha = EBlendFactor::One;
        EBlendOperation blendOperationColor = EBlendOperation::Disabled;
        EBlendOperation blendOperationAlpha = EBlendOperation::Disabled;
        ColorWriteMask  colorWriteMask = EColorWriteFlag_All;
        ECullMode       cullMode = ECullMode::BackFacing;
        EDrawMode       drawMode = EDrawMode::Triangles;
        EDepthFunc      depthFunc = EDepthFunc::SmallerEqual;
        EDepthWrite     depthWrite = EDepthWrite::Enabled;
        EScissorTest    scissorTest = EScissorTest::Disabled;
        EStencilFunc    stencilFunc = EStencilFunc::Disabled;
        EStencilOp      stencilOpFail = EStencilOp::Keep;
        EStencilOp      stencilOpDepthFail = EStencilOp::Keep;
        EStencilOp      stencilOpDepthPass = EStencilOp::Keep;
        uint8_t         stencilRefValue = 0;
        uint8_t         stencilMask = 0xFF;
        // explicit padding, consume when adding members
        uint8_t padding[2] = { 0 };
    };

    static constexpr size_t ExpectedSize = sizeof(RenderState::ScissorRegion) + sizeof(Vector4) + 4 * sizeof(EBlendFactor) + 2 * sizeof(EBlendOperation) + sizeof(ColorWriteMask)
        + sizeof(ECullMode) + sizeof(EDrawMode) + sizeof(EDepthFunc) + sizeof(EDepthWrite) + sizeof(EScissorTest) + sizeof(EStencilFunc)
        + 3 * sizeof(EStencilOp) + sizeof(RenderState::stencilRefValue) + sizeof(RenderState::stencilMask) + sizeof(RenderState::padding);
    static_assert(sizeof(RenderState) == ExpectedSize, "Avoid padding in this struct, add padding explicitly as member");
}

#endif

