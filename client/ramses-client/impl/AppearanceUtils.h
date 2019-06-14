//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_APPEARANCEUTILS_H
#define RAMSES_APPEARANCEUTILS_H

#include "ramses-client-api/AppearanceEnums.h"
#include "SceneAPI/RenderState.h"

namespace ramses
{
    class AppearanceUtils
    {
    public:
        static ramses_internal::EBlendOperation GetBlendOperationInternal(EBlendOperation blendOp)
        {
            switch (blendOp)
            {
            case EBlendOperation_Disabled:
                return ramses_internal::EBlendOperation::Disabled;
            case EBlendOperation_Add:
                return ramses_internal::EBlendOperation::Add;
            case EBlendOperation_Subtract:
                return ramses_internal::EBlendOperation::Subtract;
            case EBlendOperation_ReverseSubtract:
                return ramses_internal::EBlendOperation::ReverseSubtract;
            case EBlendOperation_Min:
                return ramses_internal::EBlendOperation::Min;
            case EBlendOperation_Max:
                return ramses_internal::EBlendOperation::Max;
            default:
                assert(false);
                return ramses_internal::EBlendOperation::Disabled;
            }
        }

        static EBlendOperation GetBlendOperationFromInternal(ramses_internal::EBlendOperation blendOp)
        {
            switch (blendOp)
            {
            case ramses_internal::EBlendOperation::Disabled:
                return EBlendOperation_Disabled;
            case ramses_internal::EBlendOperation::Add:
                return EBlendOperation_Add;
            case ramses_internal::EBlendOperation::Subtract:
                return EBlendOperation_Subtract;
            case ramses_internal::EBlendOperation::ReverseSubtract:
                return EBlendOperation_ReverseSubtract;
            case ramses_internal::EBlendOperation::Min:
                return EBlendOperation_Min;
            case ramses_internal::EBlendOperation::Max:
                return EBlendOperation_Max;
            default:
                assert(false);
                return EBlendOperation_Disabled;
            }
        }

        static ramses_internal::EBlendFactor GetBlendFactorInternal(EBlendFactor blendFactor)
        {
            switch (blendFactor)
            {
            case EBlendFactor_Zero:
                return ramses_internal::EBlendFactor::Zero;
            case EBlendFactor_One:
                return ramses_internal::EBlendFactor::One;
            case EBlendFactor_SrcAlpha:
                return ramses_internal::EBlendFactor::SrcAlpha;
            case EBlendFactor_OneMinusSrcAlpha:
                return ramses_internal::EBlendFactor::OneMinusSrcAlpha;
            case EBlendFactor_DstAlpha:
                return ramses_internal::EBlendFactor::DstAlpha;
            case EBlendFactor_OneMinusDstAlpha:
                return ramses_internal::EBlendFactor::OneMinusDstAlpha;
            default:
                assert(false);
                return ramses_internal::EBlendFactor::One;
            }
        }

        static EBlendFactor GetBlendFactorFromInternal(ramses_internal::EBlendFactor blendFactor)
        {
            switch (blendFactor)
            {
            case ramses_internal::EBlendFactor::Zero:
                return EBlendFactor_Zero;
            case ramses_internal::EBlendFactor::One:
                return EBlendFactor_One;
            case ramses_internal::EBlendFactor::SrcAlpha:
                return EBlendFactor_SrcAlpha;
            case ramses_internal::EBlendFactor::OneMinusSrcAlpha:
                return EBlendFactor_OneMinusSrcAlpha;
            case ramses_internal::EBlendFactor::DstAlpha:
                return EBlendFactor_DstAlpha;
            case ramses_internal::EBlendFactor::OneMinusDstAlpha:
                return EBlendFactor_OneMinusDstAlpha;
            default:
                assert(false);
                return EBlendFactor_One;
            }
        }

        static ramses_internal::ECullMode GetCullModeInternal(ECullMode cullMode)
        {
            switch (cullMode)
            {
            case ECullMode_Disabled:
                return ramses_internal::ECullMode::Disabled;
            case ECullMode_FrontFacing:
                return ramses_internal::ECullMode::FrontFacing;
            case ECullMode_BackFacing:
                return ramses_internal::ECullMode::BackFacing;
            case ECullMode_FrontAndBackFacing:
                return ramses_internal::ECullMode::FrontAndBackFacing;
            default:
                assert(false);
                return ramses_internal::ECullMode::Invalid;
            }
        }

        static ECullMode GetCullModeFromInternal(ramses_internal::ECullMode cullMode)
        {
            switch (cullMode)
            {
            case ramses_internal::ECullMode::Disabled:
                return ECullMode_Disabled;
            case ramses_internal::ECullMode::FrontFacing:
                return ECullMode_FrontFacing;
            case ramses_internal::ECullMode::BackFacing:
                return ECullMode_BackFacing;
            case ramses_internal::ECullMode::FrontAndBackFacing:
                return ECullMode_FrontAndBackFacing;
            default:
                assert(false);
                return ECullMode_Disabled;
            }
        }

        static ramses_internal::EDepthWrite GetDepthWriteInternal(EDepthWrite depthWrite)
        {
            switch (depthWrite)
            {
            case EDepthWrite_Disabled:
                return ramses_internal::EDepthWrite::Disabled;
            case EDepthWrite_Enabled:
                return ramses_internal::EDepthWrite::Enabled;
            default:
                assert(false);
                return ramses_internal::EDepthWrite::Enabled;
            }
        }

        static EDepthWrite GetDepthWriteFromInternal(ramses_internal::EDepthWrite depthWrite)
        {
            switch (depthWrite)
            {
            case ramses_internal::EDepthWrite::Disabled:
                return EDepthWrite_Disabled;
            case ramses_internal::EDepthWrite::Enabled:
                return EDepthWrite_Enabled;
            default:
                assert(false);
                return EDepthWrite_Enabled;
            }
        }

        static ramses_internal::EScissorTest GetScissorTestInternal(EScissorTest scissorTest)
        {
            switch (scissorTest)
            {
            case EScissorTest_Disabled:
                return ramses_internal::EScissorTest::Disabled;
            case EScissorTest_Enabled:
                return ramses_internal::EScissorTest::Enabled;
            default:
                assert(false);
                return ramses_internal::EScissorTest::Disabled;
            }
        }

        static EScissorTest GetScissorTestFromInternal(ramses_internal::EScissorTest scissorTest)
        {
            switch (scissorTest)
            {
            case ramses_internal::EScissorTest::Disabled:
                return EScissorTest_Disabled;
            case ramses_internal::EScissorTest::Enabled:
                return EScissorTest_Enabled;
            default:
                assert(false);
                return EScissorTest_Disabled;
            }
        }

        static ramses_internal::EDepthFunc GetDepthFuncInternal(EDepthFunc depthFunc)
        {
            switch (depthFunc)
            {
            case EDepthFunc_Disabled:
                return ramses_internal::EDepthFunc::Disabled;
            case EDepthFunc_Greater:
                return ramses_internal::EDepthFunc::Greater;
            case EDepthFunc_GreaterEqual:
                return ramses_internal::EDepthFunc::GreaterEqual;
            case EDepthFunc_Less:
                return ramses_internal::EDepthFunc::Smaller;
            case EDepthFunc_LessEqual:
                return ramses_internal::EDepthFunc::SmallerEqual;
            case EDepthFunc_Always:
                return ramses_internal::EDepthFunc::AlwaysPass;
            case EDepthFunc_Never:
                return ramses_internal::EDepthFunc::NeverPass;
            case EDepthFunc_Equal:
                return ramses_internal::EDepthFunc::Equal;
            case EDepthFunc_NotEqual:
                return ramses_internal::EDepthFunc::NotEqual;
            default:
                assert(false);
                return ramses_internal::EDepthFunc::Invalid;
            }
        }

        static EDepthFunc GetDepthFuncFromInternal(ramses_internal::EDepthFunc depthFunc)
        {
            switch (depthFunc)
            {
            case ramses_internal::EDepthFunc::Disabled:
                return EDepthFunc_Disabled;
            case ramses_internal::EDepthFunc::Greater:
                return EDepthFunc_Greater;
            case ramses_internal::EDepthFunc::GreaterEqual:
                return EDepthFunc_GreaterEqual;
            case ramses_internal::EDepthFunc::Smaller:
                return EDepthFunc_Less;
            case ramses_internal::EDepthFunc::SmallerEqual:
                return EDepthFunc_LessEqual;
            case ramses_internal::EDepthFunc::AlwaysPass:
                return EDepthFunc_Always;
            case ramses_internal::EDepthFunc::NeverPass:
                return EDepthFunc_Never;
            case ramses_internal::EDepthFunc::Equal:
                return EDepthFunc_Equal;
            case ramses_internal::EDepthFunc::NotEqual:
                return EDepthFunc_NotEqual;
            default:
                assert(false);
                return EDepthFunc_LessEqual;
            }
        }

        static ramses_internal::EStencilFunc GetStencilFuncInternal(EStencilFunc stencilFunc)
        {
            switch (stencilFunc)
            {
            case EStencilFunc_Disabled:
                return ramses_internal::EStencilFunc::Disabled;
            case EStencilFunc_Never:
                return ramses_internal::EStencilFunc::NeverPass;
            case EStencilFunc_Always:
                return ramses_internal::EStencilFunc::AlwaysPass;
            case EStencilFunc_Equal:
                return ramses_internal::EStencilFunc::Equal;
            case EStencilFunc_NotEqual:
                return ramses_internal::EStencilFunc::NotEqual;
            case EStencilFunc_Less:
                return ramses_internal::EStencilFunc::Less;
            case EStencilFunc_LessEqual:
                return ramses_internal::EStencilFunc::LessEqual;
            case EStencilFunc_Greater:
                return ramses_internal::EStencilFunc::Greater;
            case EStencilFunc_GreaterEqual:
                return ramses_internal::EStencilFunc::GreaterEqual;
            default:
                assert(false);
                return ramses_internal::EStencilFunc::Disabled;
            }
        }

        static EStencilFunc GetStencilFuncFromInternal(ramses_internal::EStencilFunc stencilFunc)
        {
            switch (stencilFunc)
            {
            case ramses_internal::EStencilFunc::Disabled:
                return EStencilFunc_Disabled;
            case ramses_internal::EStencilFunc::NeverPass:
                return EStencilFunc_Never;
            case ramses_internal::EStencilFunc::AlwaysPass:
                return EStencilFunc_Always;
            case ramses_internal::EStencilFunc::Equal:
                return EStencilFunc_Equal;
            case ramses_internal::EStencilFunc::NotEqual:
                return EStencilFunc_NotEqual;
            case ramses_internal::EStencilFunc::Less:
                return EStencilFunc_Less;
            case ramses_internal::EStencilFunc::LessEqual:
                return EStencilFunc_LessEqual;
            case ramses_internal::EStencilFunc::Greater:
                return EStencilFunc_Greater;
            case ramses_internal::EStencilFunc::GreaterEqual:
                return EStencilFunc_GreaterEqual;
            default:
                assert(false);
                return EStencilFunc_Disabled;
            }
        }

        static ramses_internal::EStencilOp GetStencilOperationInternal(EStencilOperation stencilOp)
        {
            switch (stencilOp)
            {
            case EStencilOperation_Keep:
                return ramses_internal::EStencilOp::Keep;
            case EStencilOperation_Zero:
                return ramses_internal::EStencilOp::Zero;
            case EStencilOperation_Replace:
                return ramses_internal::EStencilOp::Replace;
            case EStencilOperation_Increment:
                return ramses_internal::EStencilOp::Increment;
            case EStencilOperation_IncrementWrap:
                return ramses_internal::EStencilOp::IncrementWrap;
            case EStencilOperation_Decrement:
                return ramses_internal::EStencilOp::Decrement;
            case EStencilOperation_DecrementWrap:
                return ramses_internal::EStencilOp::DecrementWrap;
            case EStencilOperation_Invert:
                return ramses_internal::EStencilOp::Invert;
            default:
                assert(false);
                return ramses_internal::EStencilOp::Keep;
            }
        }

        static EStencilOperation GetStencilOperationFromInternal(ramses_internal::EStencilOp stencilOp)
        {
            switch (stencilOp)
            {
            case ramses_internal::EStencilOp::Keep:
                return EStencilOperation_Keep;
            case ramses_internal::EStencilOp::Zero:
                return EStencilOperation_Zero;
            case ramses_internal::EStencilOp::Replace:
                return EStencilOperation_Replace;
            case ramses_internal::EStencilOp::Increment:
                return EStencilOperation_Increment;
            case ramses_internal::EStencilOp::IncrementWrap:
                return EStencilOperation_IncrementWrap;
            case ramses_internal::EStencilOp::Decrement:
                return EStencilOperation_Decrement;
            case ramses_internal::EStencilOp::DecrementWrap:
                return EStencilOperation_DecrementWrap;
            case ramses_internal::EStencilOp::Invert:
                return EStencilOperation_Invert;
            default:
                assert(false);
                return EStencilOperation_Keep;
            }
        }

        static ramses_internal::EDrawMode GetDrawModeInternal(EDrawMode mode)
        {
            switch (mode)
            {
            case EDrawMode_Points:
                return ramses_internal::EDrawMode::Points;
            case EDrawMode_Lines:
                return ramses_internal::EDrawMode::Lines;
            case EDrawMode_LineLoop:
                return ramses_internal::EDrawMode::LineLoop;
            case EDrawMode_Triangles:
                return ramses_internal::EDrawMode::Triangles;
            case EDrawMode_TriangleStrip:
                return ramses_internal::EDrawMode::TriangleStrip;
            default:
                assert(false);
                return ramses_internal::EDrawMode::Triangles;
            }
        }

        static EDrawMode GetDrawModeFromInternal(ramses_internal::EDrawMode drawMode)
        {
            switch (drawMode)
            {
            case ramses_internal::EDrawMode::Points:
                return EDrawMode_Points;
            case ramses_internal::EDrawMode::Lines:
                return EDrawMode_Lines;
            case ramses_internal::EDrawMode::LineLoop:
                return EDrawMode_LineLoop;
            case ramses_internal::EDrawMode::Triangles:
                return EDrawMode_Triangles;
            case ramses_internal::EDrawMode::TriangleStrip:
                return EDrawMode_TriangleStrip;
            default:
                assert(false);
                return EDrawMode_Triangles;
            }
        }
    };
}

#endif
