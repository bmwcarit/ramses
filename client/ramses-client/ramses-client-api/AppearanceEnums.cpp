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
    const char* InputTypeNames[] =
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

    const char* BlendOperationNames[] =
    {
        "Disabled",
        "Add",
        "Subtract",
        "ReverseSubtract",
        "Min",
        "Max",
    };

    const char* BlendFactorNames[] =
    {
        "Zero",
        "One",
        "SrcAlpha",
        "OneMinusSrcAlpha",
        "DstAlpha",
        "OneMinustDstAlpha",
    };

    const char* CullModeNames[] =
    {
        "Disabled",
        "FrontFacing",
        "BackFacing",
        "FrontAndBackFacing",
    };

    const char* DepthWriteNames[] =
    {
        "Disabled",
        "Enabled",
    };

    const char* ScissorTestNames[] =
    {
        "Disabled",
        "Enabled",
    };

    const char* DepthFuncNames[] =
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

    const char* StencilFuncNames[] =
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

    const char* StencilOperationNames[] =
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

    const char* DrawModeNames[] =
    {
        "Points",
        "Lines",
        "LineLoop",
        "Triangles",
        "TriangleStrip",
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
