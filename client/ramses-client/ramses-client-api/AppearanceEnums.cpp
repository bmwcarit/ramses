//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-client-api/AppearanceEnums.h"
#include "Utils/LoggingUtils.h"

namespace ramses
{
    static const char* const InputTypeNames[] =
    {
        "Invalid",
        "Float",
        "Vector2f",
        "Vector3f",
        "Vector4f",
        "Int16",
        "Int32",
        "UInt16",
        "UInt32",
        "Vector2i",
        "Vector3i",
        "Vector4i",
        "Matrix22f",
        "Matrix23f",
        "Matrix24f",
        "Matrix32f",
        "Matrix33f",
        "Matrix34f",
        "Matrix42f",
        "Matrix43f",
        "Matrix44f",
        "TextureSampler",
        "AttributeUInt16",
        "AttributeFloat",
        "AttributeVector2f",
        "AttributeVector3f",
        "AttributeVector4f",
    };

    static const char* const BlendOperationNames[] =
    {
        "Disabled",
        "Add",
        "Subtract",
        "ReverseSubtract",
        "Min",
        "Max",
    };

    static const char* const BlendFactorNames[] =
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

    static const char* const CullModeNames[] =
    {
        "Disabled",
        "FrontFacing",
        "BackFacing",
        "FrontAndBackFacing",
    };

    static const char* const DepthWriteNames[] =
    {
        "Disabled",
        "Enabled",
    };

    static const char* const ScissorTestNames[] =
    {
        "Disabled",
        "Enabled",
    };

    static const char* const DepthFuncNames[] =
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

    static const char* const StencilFuncNames[] =
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

    static const char* const StencilOperationNames[] =
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

    const char* const DrawModeNames[] =
    {
        "Points",
        "Lines",
        "LineLoop",
        "Triangles",
        "TriangleStrip",
        "TriangleFan",
        "LineStrip",
    };

    ENUM_TO_STRING(EInputType, InputTypeNames, EInputType_NUMBER_OF_ELEMENTS);
    ENUM_TO_STRING(EBlendOperation, BlendOperationNames, EBlendOperation_NUMBER_OF_ELEMENTS);
    ENUM_TO_STRING(EBlendFactor, BlendFactorNames, EBlendFactor_NUMBER_OF_ELEMENTS);
    ENUM_TO_STRING(ECullMode, CullModeNames, ECullMode_NUMBER_OF_ELEMENTS);
    ENUM_TO_STRING(EDepthWrite, DepthWriteNames, EDepthWrite_NUMBER_OF_ELEMENTS);
    ENUM_TO_STRING(EScissorTest, ScissorTestNames, EScissorTest_NUMBER_OF_ELEMENTS);
    ENUM_TO_STRING(EDepthFunc, DepthFuncNames, EDepthFunc_NUMBER_OF_ELEMENTS);
    ENUM_TO_STRING(EStencilFunc, StencilFuncNames, EStencilFunc_NUMBER_OF_ELEMENTS);
    ENUM_TO_STRING(EStencilOperation, StencilOperationNames, EStencilOperation_NUMBER_OF_ELEMENTS);
    ENUM_TO_STRING(EDrawMode, DrawModeNames, EDrawMode_NUMBER_OF_ELEMENTS);


    const char* getInputTypeString(EInputType inputType)
    {
        return EnumToString(inputType);
    }

    const char* getBlendOperationString(EBlendOperation blendOperation)
    {
        return EnumToString(blendOperation);
    }

    const char* getBlendFactorString(EBlendFactor blendFactor)
    {
        return EnumToString(blendFactor);
    }

    const char* getCullModeString(ECullMode cullMode)
    {
        return EnumToString(cullMode);
    }

    const char* getDepthWriteString(EDepthWrite depthWrite)
    {
        return EnumToString(depthWrite);
    }

    const char* getScissorTestString(EScissorTest scissorTest)
    {
        return EnumToString(scissorTest);
    }

    const char* getDepthFuncString(EDepthFunc depthFunc)
    {
        return EnumToString(depthFunc);
    }

    const char* getStencilFuncString(EStencilFunc stencilFunc)
    {
        return EnumToString(stencilFunc);
    }

    const char* getStencilOperationString(EStencilOperation stencilOp)
    {
        return EnumToString(stencilOp);
    }

    const char* getDrawModeString(EDrawMode drawMode)
    {
        return EnumToString(drawMode);
    }
}
