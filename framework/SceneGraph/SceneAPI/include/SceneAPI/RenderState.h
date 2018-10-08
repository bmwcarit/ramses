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

namespace ramses_internal
{
    enum EBlendOperation
    {
        EBlendOperation_Disabled = 0,
        EBlendOperation_Add,
        EBlendOperation_Subtract,
        EBlendOperation_ReverseSubtract,
        EBlendOperation_Min,
        EBlendOperation_Max,
        EBlendOperation_NUMBER_OF_ELEMENTS
    };

    enum EBlendFactor
    {
        EBlendFactor_Zero = 0,
        EBlendFactor_One,
        EBlendFactor_SrcAlpha,
        EBlendFactor_OneMinusSrcAlpha,
        EBlendFactor_DstAlpha,
        EBlendFactor_OneMinusDstAlpha,
        EBlendFactor_Invalid,
        EBlendFactor_NUMBER_OF_ELEMENTS
    };

    enum ECullMode
    {
        ECullMode_Disabled = 0,
        ECullMode_FrontFacing,
        ECullMode_BackFacing,
        ECullMode_FrontAndBackFacing,
        ECullMode_Invalid,
        ECullMode_NUMBER_OF_ELEMENTS
    };

    enum EDepthWrite
    {
        EDepthWrite_Disabled = 0,
        EDepthWrite_Enabled,
        EDepthWrite_NUMBER_OF_ELEMENTS
    };

    enum EDepthFunc
    {
        EDepthFunc_Disabled = 0,
        EDepthFunc_Greater,
        EDepthFunc_GreaterEqual,
        EDepthFunc_Smaller,
        EDepthFunc_SmallerEqual,
        EDepthFunc_Equal,
        EDepthFunc_NotEqual,
        EDepthFunc_Always,
        EDepthFunc_Never,
        EDepthFunc_Invalid,
        EDepthFunc_NUMBER_OF_ELEMENTS
    };

    enum EStencilFunc
    {
        EStencilFunc_Disabled = 0,
        EStencilFunc_Never,
        EStencilFunc_Always,
        EStencilFunc_Equal,
        EStencilFunc_NotEqual,
        EStencilFunc_Less,
        EStencilFunc_LessEqual,
        EStencilFunc_Greater,
        EStencilFunc_GreaterEqual,
        EStencilFunc_NUMBER_OF_ELEMENTS
    };

    enum EStencilOp
    {
        EStencilOp_Keep = 0,
        EStencilOp_Zero,
        EStencilOp_Replace,
        EStencilOp_Increment,
        EStencilOp_IncrementWrap,
        EStencilOp_Decrement,
        EStencilOp_DecrementWrap,
        EStencilOp_Invert,
        EStencilOp_NUMBER_OF_ELEMENTS
    };

    enum EDrawMode
    {
        EDrawMode_Points = 0,
        EDrawMode_Lines,
        EDrawMode_LineLoop,
        EDrawMode_Triangles,
        EDrawMode_TriangleStrip,
        EDrawMode_NUMBER_OF_ELEMENTS
    };

    enum EColorWriteFlag
    {
        EColorWriteFlag_Red = BIT(0),
        EColorWriteFlag_Green = BIT(1),
        EColorWriteFlag_Blue = BIT(2),
        EColorWriteFlag_Alpha = BIT(3),
        EColorWriteFlag_All = (EColorWriteFlag_Red | EColorWriteFlag_Green | EColorWriteFlag_Blue | EColorWriteFlag_Alpha)
    };

    enum EClearFlags
    {
        EClearFlags_None     = 0,
        EClearFlags_Color   = BIT(0),
        EClearFlags_Depth   = BIT(1),
        EClearFlags_Stencil = BIT(2),
        EClearFlags_All     = EClearFlags_Color | EClearFlags_Depth | EClearFlags_Stencil
    };

    typedef UInt32 ColorWriteMask;

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

    ENUM_TO_STRING(EBlendOperation, BlendOperationNames, EBlendOperation_NUMBER_OF_ELEMENTS);
    ENUM_TO_STRING(EBlendFactor, BlendFactorNames, EBlendFactor_NUMBER_OF_ELEMENTS);
    ENUM_TO_STRING(ECullMode, CullModeNames, ECullMode_NUMBER_OF_ELEMENTS);
    ENUM_TO_STRING(EDepthWrite, DepthWriteNames, EDepthWrite_NUMBER_OF_ELEMENTS);
    ENUM_TO_STRING(EDepthFunc, DepthFuncNames, EDepthFunc_NUMBER_OF_ELEMENTS);
    ENUM_TO_STRING(EStencilFunc, StencilFuncNames, EStencilFunc_NUMBER_OF_ELEMENTS);
    ENUM_TO_STRING(EStencilOp, StencilOperationNames, EStencilOp_NUMBER_OF_ELEMENTS);
    ENUM_TO_STRING(EDrawMode, DrawModeNames, EDrawMode_NUMBER_OF_ELEMENTS);

    struct RenderState
    {
        EBlendFactor    blendFactorSrcColor = EBlendFactor_SrcAlpha;
        EBlendFactor    blendFactorDstColor = EBlendFactor_OneMinusSrcAlpha;
        EBlendFactor    blendFactorSrcAlpha = EBlendFactor_One;
        EBlendFactor    blendFactorDstAlpha = EBlendFactor_One;
        EBlendOperation blendOperationColor = EBlendOperation_Disabled;
        EBlendOperation blendOperationAlpha = EBlendOperation_Disabled;
        ECullMode       cullMode = ECullMode_BackFacing;
        EDrawMode       drawMode = EDrawMode_Triangles;
        EDepthFunc      depthFunc = EDepthFunc_SmallerEqual;
        EDepthWrite     depthWrite = EDepthWrite_Enabled;
        EStencilFunc    stencilFunc = EStencilFunc_Disabled;
        UInt32          stencilRefValue = 0u;
        UInt8           stencilMask = 0xFF;
        EStencilOp      stencilOpFail = EStencilOp_Keep;
        EStencilOp      stencilOpDepthFail = EStencilOp_Keep;
        EStencilOp      stencilOpDepthPass = EStencilOp_Keep;
        ColorWriteMask  colorWriteMask = EColorWriteFlag_All;
    };
}

#endif

