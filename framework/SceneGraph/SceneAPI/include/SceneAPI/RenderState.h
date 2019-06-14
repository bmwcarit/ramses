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
        NUMBER_OF_ELEMENTS
    };

    enum EColorWriteFlag
    {
        EColorWriteFlag_Red = BIT(0),
        EColorWriteFlag_Green = BIT(1),
        EColorWriteFlag_Blue = BIT(2),
        EColorWriteFlag_Alpha = BIT(3),
        EColorWriteFlag_All = (EColorWriteFlag_Red | EColorWriteFlag_Green | EColorWriteFlag_Blue | EColorWriteFlag_Alpha)
    };
    using ColorWriteMask = uint8_t;

    enum EClearFlags
    {
        EClearFlags_None     = 0,
        EClearFlags_Color   = BIT(0),
        EClearFlags_Depth   = BIT(1),
        EClearFlags_Stencil = BIT(2),
        EClearFlags_All     = EClearFlags_Color | EClearFlags_Depth | EClearFlags_Stencil
    };

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
        "OneMinustDstAlpha",
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
        "TriangleStrip"
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
    };
}

#endif

