//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses-framework-api/AppearanceEnums.h"
#include "Utils/LoggingUtils.h"

namespace ramses
{
    const std::array BlendOperationNames =
    {
        "Disabled",
        "Add",
        "Subtract",
        "ReverseSubtract",
        "Min",
        "Max",
    };

    const std::array BlendFactorNames =
    {
        "Zero",
        "One",
        "SrcAlpha",
        "OneMinusSrcAlpha",
        "DstAlpha",
        "OneMinustDstAlpha",
        "SrcColor",
        "OneMinusSrcColor",
        "DstColor",
        "OneMinusDstColor",
        "ConstColor",
        "OneMinusConstColor",
        "ConstAlpha",
        "OneMinusConstAlpha",
        "AlphaSaturate",
    };

    const std::array CullModeNames =
    {
        "Disabled",
        "FrontFacing",
        "BackFacing",
        "FrontAndBackFacing",
    };

    const std::array DepthWriteNames =
    {
        "Disabled",
        "Enabled",
    };

    const std::array ScissorTestNames =
    {
        "Disabled",
        "Enabled",
    };

    const std::array DepthFuncNames =
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
    };

    const std::array StencilFuncNames =
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

    const std::array StencilOperationNames =
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

    const std::array DrawModeNames =
    {
        "Points",
        "Lines",
        "LineLoop",
        "Triangles",
        "TriangleStrip",
        "TriangleFan",
        "LineStrip",
    };

    ENUM_TO_STRING_NO_EXTRA_LAST(EBlendOperation, BlendOperationNames, EBlendOperation::Max);
    ENUM_TO_STRING_NO_EXTRA_LAST(EBlendFactor, BlendFactorNames, EBlendFactor::AlphaSaturate);
    ENUM_TO_STRING_NO_EXTRA_LAST(ECullMode, CullModeNames, ECullMode::FrontAndBackFacing);
    ENUM_TO_STRING_NO_EXTRA_LAST(EDepthWrite, DepthWriteNames, EDepthWrite::Enabled);
    ENUM_TO_STRING_NO_EXTRA_LAST(EScissorTest, ScissorTestNames, EScissorTest::Enabled);
    ENUM_TO_STRING_NO_EXTRA_LAST(EDepthFunc, DepthFuncNames, EDepthFunc::Never);
    ENUM_TO_STRING_NO_EXTRA_LAST(EStencilFunc, StencilFuncNames, EStencilFunc::GreaterEqual);
    ENUM_TO_STRING_NO_EXTRA_LAST(EStencilOperation, StencilOperationNames, EStencilOperation::Invert);
    ENUM_TO_STRING_NO_EXTRA_LAST(EDrawMode, DrawModeNames, EDrawMode::LineStrip);
}
