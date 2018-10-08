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
                return ramses_internal::EBlendOperation_Disabled;
            case EBlendOperation_Add:
                return ramses_internal::EBlendOperation_Add;
            case EBlendOperation_Subtract:
                return ramses_internal::EBlendOperation_Subtract;
            case EBlendOperation_ReverseSubtract:
                return ramses_internal::EBlendOperation_ReverseSubtract;
            case EBlendOperation_Min:
                return ramses_internal::EBlendOperation_Min;
            case EBlendOperation_Max:
                return ramses_internal::EBlendOperation_Max;
            default:
                assert(false);
                return ramses_internal::EBlendOperation_Disabled;
            }
        }

        static EBlendOperation GetBlendOperationFromInternal(ramses_internal::EBlendOperation blendOp)
        {
            switch (blendOp)
            {
            case ramses_internal::EBlendOperation_Disabled:
                return EBlendOperation_Disabled;
            case ramses_internal::EBlendOperation_Add:
                return EBlendOperation_Add;
            case ramses_internal::EBlendOperation_Subtract:
                return EBlendOperation_Subtract;
            case ramses_internal::EBlendOperation_ReverseSubtract:
                return EBlendOperation_ReverseSubtract;
            case ramses_internal::EBlendOperation_Min:
                return EBlendOperation_Min;
            case ramses_internal::EBlendOperation_Max:
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
                return ramses_internal::EBlendFactor_Zero;
            case EBlendFactor_One:
                return ramses_internal::EBlendFactor_One;
            case EBlendFactor_SrcAlpha:
                return ramses_internal::EBlendFactor_SrcAlpha;
            case EBlendFactor_OneMinusSrcAlpha:
                return ramses_internal::EBlendFactor_OneMinusSrcAlpha;
            case EBlendFactor_DstAlpha:
                return ramses_internal::EBlendFactor_DstAlpha;
            case EBlendFactor_OneMinusDstAlpha:
                return ramses_internal::EBlendFactor_OneMinusDstAlpha;
            default:
                assert(false);
                return ramses_internal::EBlendFactor_One;
            }
        }

        static EBlendFactor GetBlendFactorFromInternal(ramses_internal::EBlendFactor blendFactor)
        {
            switch (blendFactor)
            {
            case ramses_internal::EBlendFactor_Zero:
                return EBlendFactor_Zero;
            case ramses_internal::EBlendFactor_One:
                return EBlendFactor_One;
            case ramses_internal::EBlendFactor_SrcAlpha:
                return EBlendFactor_SrcAlpha;
            case ramses_internal::EBlendFactor_OneMinusSrcAlpha:
                return EBlendFactor_OneMinusSrcAlpha;
            case ramses_internal::EBlendFactor_DstAlpha:
                return EBlendFactor_DstAlpha;
            case ramses_internal::EBlendFactor_OneMinusDstAlpha:
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
                return ramses_internal::ECullMode_Disabled;
            case ECullMode_FrontFacing:
                return ramses_internal::ECullMode_FrontFacing;
            case ECullMode_BackFacing:
                return ramses_internal::ECullMode_BackFacing;
            case ECullMode_FrontAndBackFacing:
                return ramses_internal::ECullMode_FrontAndBackFacing;
            default:
                assert(false);
                return ramses_internal::ECullMode_Invalid;
            }
        }

        static ECullMode GetCullModeFromInternal(ramses_internal::ECullMode cullMode)
        {
            switch (cullMode)
            {
            case ramses_internal::ECullMode_Disabled:
                return ECullMode_Disabled;
            case ramses_internal::ECullMode_FrontFacing:
                return ECullMode_FrontFacing;
            case ramses_internal::ECullMode_BackFacing:
                return ECullMode_BackFacing;
            case ramses_internal::ECullMode_FrontAndBackFacing:
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
                return ramses_internal::EDepthWrite_Disabled;
            case EDepthWrite_Enabled:
                return ramses_internal::EDepthWrite_Enabled;
            default:
                assert(false);
                return ramses_internal::EDepthWrite_Enabled;
            }
        }

        static EDepthWrite GetDepthWriteFromInternal(ramses_internal::EDepthWrite depthWrite)
        {
            switch (depthWrite)
            {
            case ramses_internal::EDepthWrite_Disabled:
                return EDepthWrite_Disabled;
            case ramses_internal::EDepthWrite_Enabled:
                return EDepthWrite_Enabled;
            default:
                assert(false);
                return EDepthWrite_Enabled;
            }
        }

        static ramses_internal::EDepthFunc GetDepthFuncInternal(EDepthFunc depthFunc)
        {
            switch (depthFunc)
            {
            case EDepthFunc_Disabled:
                return ramses_internal::EDepthFunc_Disabled;
            case EDepthFunc_Greater:
                return ramses_internal::EDepthFunc_Greater;
            case EDepthFunc_GreaterEqual:
                return ramses_internal::EDepthFunc_GreaterEqual;
            case EDepthFunc_Less:
                return ramses_internal::EDepthFunc_Smaller;
            case EDepthFunc_LessEqual:
                return ramses_internal::EDepthFunc_SmallerEqual;
            case EDepthFunc_Always:
                return ramses_internal::EDepthFunc_Always;
            case EDepthFunc_Never:
                return ramses_internal::EDepthFunc_Never;
            case EDepthFunc_Equal:
                return ramses_internal::EDepthFunc_Equal;
            case EDepthFunc_NotEqual:
                return ramses_internal::EDepthFunc_NotEqual;
            default:
                assert(false);
                return ramses_internal::EDepthFunc_Invalid;
            }
        }

        static EDepthFunc GetDepthFuncFromInternal(ramses_internal::EDepthFunc depthFunc)
        {
            switch (depthFunc)
            {
            case ramses_internal::EDepthFunc_Disabled:
                return EDepthFunc_Disabled;
            case ramses_internal::EDepthFunc_Greater:
                return EDepthFunc_Greater;
            case ramses_internal::EDepthFunc_GreaterEqual:
                return EDepthFunc_GreaterEqual;
            case ramses_internal::EDepthFunc_Smaller:
                return EDepthFunc_Less;
            case ramses_internal::EDepthFunc_SmallerEqual:
                return EDepthFunc_LessEqual;
            case ramses_internal::EDepthFunc_Always:
                return EDepthFunc_Always;
            case ramses_internal::EDepthFunc_Never:
                return EDepthFunc_Never;
            case ramses_internal::EDepthFunc_Equal:
                return EDepthFunc_Equal;
            case ramses_internal::EDepthFunc_NotEqual:
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
                return ramses_internal::EStencilFunc_Disabled;
            case EStencilFunc_Never:
                return ramses_internal::EStencilFunc_Never;
            case EStencilFunc_Always:
                return ramses_internal::EStencilFunc_Always;
            case EStencilFunc_Equal:
                return ramses_internal::EStencilFunc_Equal;
            case EStencilFunc_NotEqual:
                return ramses_internal::EStencilFunc_NotEqual;
            case EStencilFunc_Less:
                return ramses_internal::EStencilFunc_Less;
            case EStencilFunc_LessEqual:
                return ramses_internal::EStencilFunc_LessEqual;
            case EStencilFunc_Greater:
                return ramses_internal::EStencilFunc_Greater;
            case EStencilFunc_GreaterEqual:
                return ramses_internal::EStencilFunc_GreaterEqual;
            default:
                assert(false);
                return ramses_internal::EStencilFunc_Disabled;
            }
        }

        static EStencilFunc GetStencilFuncFromInternal(ramses_internal::EStencilFunc stencilFunc)
        {
            switch (stencilFunc)
            {
            case ramses_internal::EStencilFunc_Disabled:
                return EStencilFunc_Disabled;
            case ramses_internal::EStencilFunc_Never:
                return EStencilFunc_Never;
            case ramses_internal::EStencilFunc_Always:
                return EStencilFunc_Always;
            case ramses_internal::EStencilFunc_Equal:
                return EStencilFunc_Equal;
            case ramses_internal::EStencilFunc_NotEqual:
                return EStencilFunc_NotEqual;
            case ramses_internal::EStencilFunc_Less:
                return EStencilFunc_Less;
            case ramses_internal::EStencilFunc_LessEqual:
                return EStencilFunc_LessEqual;
            case ramses_internal::EStencilFunc_Greater:
                return EStencilFunc_Greater;
            case ramses_internal::EStencilFunc_GreaterEqual:
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
                return ramses_internal::EStencilOp_Keep;
            case EStencilOperation_Zero:
                return ramses_internal::EStencilOp_Zero;
            case EStencilOperation_Replace:
                return ramses_internal::EStencilOp_Replace;
            case EStencilOperation_Increment:
                return ramses_internal::EStencilOp_Increment;
            case EStencilOperation_IncrementWrap:
                return ramses_internal::EStencilOp_IncrementWrap;
            case EStencilOperation_Decrement:
                return ramses_internal::EStencilOp_Decrement;
            case EStencilOperation_DecrementWrap:
                return ramses_internal::EStencilOp_DecrementWrap;
            case EStencilOperation_Invert:
                return ramses_internal::EStencilOp_Invert;
            default:
                assert(false);
                return ramses_internal::EStencilOp_Keep;
            }
        }

        static EStencilOperation GetStencilOperationFromInternal(ramses_internal::EStencilOp stencilOp)
        {
            switch (stencilOp)
            {
            case ramses_internal::EStencilOp_Keep:
                return EStencilOperation_Keep;
            case ramses_internal::EStencilOp_Zero:
                return EStencilOperation_Zero;
            case ramses_internal::EStencilOp_Replace:
                return EStencilOperation_Replace;
            case ramses_internal::EStencilOp_Increment:
                return EStencilOperation_Increment;
            case ramses_internal::EStencilOp_IncrementWrap:
                return EStencilOperation_IncrementWrap;
            case ramses_internal::EStencilOp_Decrement:
                return EStencilOperation_Decrement;
            case ramses_internal::EStencilOp_DecrementWrap:
                return EStencilOperation_DecrementWrap;
            case ramses_internal::EStencilOp_Invert:
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
                return ramses_internal::EDrawMode_Points;
            case EDrawMode_Lines:
                return ramses_internal::EDrawMode_Lines;
            case EDrawMode_LineLoop:
                return ramses_internal::EDrawMode_LineLoop;
            case EDrawMode_Triangles:
                return ramses_internal::EDrawMode_Triangles;
            case EDrawMode_TriangleStrip:
                return ramses_internal::EDrawMode_TriangleStrip;
            default:
                assert(false);
                return ramses_internal::EDrawMode_Triangles;
            }
        }

        static EDrawMode GetDrawModeFromInternal(ramses_internal::EDrawMode drawMode)
        {
            switch (drawMode)
            {
            case ramses_internal::EDrawMode_Points:
                return EDrawMode_Points;
            case ramses_internal::EDrawMode_Lines:
                return EDrawMode_Lines;
            case ramses_internal::EDrawMode_LineLoop:
                return EDrawMode_LineLoop;
            case ramses_internal::EDrawMode_Triangles:
                return EDrawMode_Triangles;
            case ramses_internal::EDrawMode_TriangleStrip:
                return EDrawMode_TriangleStrip;
            default:
                assert(false);
                return EDrawMode_Triangles;
            }
        }
    };
}

#endif
