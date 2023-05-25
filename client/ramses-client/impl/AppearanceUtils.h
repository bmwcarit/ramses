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
            case EBlendOperation::Disabled:
                return ramses_internal::EBlendOperation::Disabled;
            case EBlendOperation::Add:
                return ramses_internal::EBlendOperation::Add;
            case EBlendOperation::Subtract:
                return ramses_internal::EBlendOperation::Subtract;
            case EBlendOperation::ReverseSubtract:
                return ramses_internal::EBlendOperation::ReverseSubtract;
            case EBlendOperation::Min:
                return ramses_internal::EBlendOperation::Min;
            case EBlendOperation::Max:
                return ramses_internal::EBlendOperation::Max;
            }

            assert(false);
            return ramses_internal::EBlendOperation::Disabled;
        }

        static EBlendOperation GetBlendOperationFromInternal(ramses_internal::EBlendOperation blendOp)
        {
            switch (blendOp)
            {
            case ramses_internal::EBlendOperation::Disabled:
                return EBlendOperation::Disabled;
            case ramses_internal::EBlendOperation::Add:
                return EBlendOperation::Add;
            case ramses_internal::EBlendOperation::Subtract:
                return EBlendOperation::Subtract;
            case ramses_internal::EBlendOperation::ReverseSubtract:
                return EBlendOperation::ReverseSubtract;
            case ramses_internal::EBlendOperation::Min:
                return EBlendOperation::Min;
            case ramses_internal::EBlendOperation::Max:
                return EBlendOperation::Max;
            case ramses_internal::EBlendOperation::NUMBER_OF_ELEMENTS:
                break;
            }

            assert(false);
            return EBlendOperation::Disabled;
        }

        static ramses_internal::EBlendFactor GetBlendFactorInternal(EBlendFactor blendFactor)
        {
            switch (blendFactor)
            {
            case EBlendFactor::Zero:
                return ramses_internal::EBlendFactor::Zero;
            case EBlendFactor::One:
                return ramses_internal::EBlendFactor::One;
            case EBlendFactor::SrcAlpha:
                return ramses_internal::EBlendFactor::SrcAlpha;
            case EBlendFactor::OneMinusSrcAlpha:
                return ramses_internal::EBlendFactor::OneMinusSrcAlpha;
            case EBlendFactor::DstAlpha:
                return ramses_internal::EBlendFactor::DstAlpha;
            case EBlendFactor::OneMinusDstAlpha:
                return ramses_internal::EBlendFactor::OneMinusDstAlpha;
            case EBlendFactor::SrcColor:
                return ramses_internal::EBlendFactor::SrcColor;
            case EBlendFactor::OneMinusSrcColor:
                return ramses_internal::EBlendFactor::OneMinusSrcColor;
            case EBlendFactor::DstColor:
                return ramses_internal::EBlendFactor::DstColor;
            case EBlendFactor::OneMinusDstColor:
                return ramses_internal::EBlendFactor::OneMinusDstColor;
            case EBlendFactor::ConstColor:
                return ramses_internal::EBlendFactor::ConstColor;
            case EBlendFactor::OneMinusConstColor:
                return ramses_internal::EBlendFactor::OneMinusConstColor;
            case EBlendFactor::ConstAlpha:
                return ramses_internal::EBlendFactor::ConstAlpha;
            case EBlendFactor::OneMinusConstAlpha:
                return ramses_internal::EBlendFactor::OneMinusConstAlpha;
            case EBlendFactor::AlphaSaturate:
                return ramses_internal::EBlendFactor::AlphaSaturate;
            }

            assert(false);
            return ramses_internal::EBlendFactor::One;
        }

        static EBlendFactor GetBlendFactorFromInternal(ramses_internal::EBlendFactor blendFactor)
        {
            switch (blendFactor)
            {
            case ramses_internal::EBlendFactor::Zero:
                return EBlendFactor::Zero;
            case ramses_internal::EBlendFactor::One:
                return EBlendFactor::One;
            case ramses_internal::EBlendFactor::SrcAlpha:
                return EBlendFactor::SrcAlpha;
            case ramses_internal::EBlendFactor::OneMinusSrcAlpha:
                return EBlendFactor::OneMinusSrcAlpha;
            case ramses_internal::EBlendFactor::DstAlpha:
                return EBlendFactor::DstAlpha;
            case ramses_internal::EBlendFactor::OneMinusDstAlpha:
                return EBlendFactor::OneMinusDstAlpha;

            case ramses_internal::EBlendFactor::SrcColor:
                return EBlendFactor::SrcColor;
            case ramses_internal::EBlendFactor::OneMinusSrcColor:
                return EBlendFactor::OneMinusSrcColor;
            case ramses_internal::EBlendFactor::DstColor:
                return EBlendFactor::DstColor;
            case ramses_internal::EBlendFactor::OneMinusDstColor:
                return EBlendFactor::OneMinusDstColor;
            case ramses_internal::EBlendFactor::ConstColor:
                return EBlendFactor::ConstColor;
            case ramses_internal::EBlendFactor::OneMinusConstColor:
                return EBlendFactor::OneMinusConstColor;
            case ramses_internal::EBlendFactor::ConstAlpha:
                return EBlendFactor::ConstAlpha;
            case ramses_internal::EBlendFactor::OneMinusConstAlpha:
                return EBlendFactor::OneMinusConstAlpha;
            case ramses_internal::EBlendFactor::AlphaSaturate:
                return EBlendFactor::AlphaSaturate;
            case ramses_internal::EBlendFactor::NUMBER_OF_ELEMENTS:
                break;
            }

            assert(false);
            return EBlendFactor::One;
        }

        static ramses_internal::ECullMode GetCullModeInternal(ECullMode cullMode)
        {
            switch (cullMode)
            {
            case ECullMode::Disabled:
                return ramses_internal::ECullMode::Disabled;
            case ECullMode::FrontFacing:
                return ramses_internal::ECullMode::FrontFacing;
            case ECullMode::BackFacing:
                return ramses_internal::ECullMode::BackFacing;
            case ECullMode::FrontAndBackFacing:
                return ramses_internal::ECullMode::FrontAndBackFacing;
            }

            assert(false);
            return ramses_internal::ECullMode::Disabled;
        }

        static ECullMode GetCullModeFromInternal(ramses_internal::ECullMode cullMode)
        {
            switch (cullMode)
            {
            case ramses_internal::ECullMode::Disabled:
                return ECullMode::Disabled;
            case ramses_internal::ECullMode::FrontFacing:
                return ECullMode::FrontFacing;
            case ramses_internal::ECullMode::BackFacing:
                return ECullMode::BackFacing;
            case ramses_internal::ECullMode::FrontAndBackFacing:
                return ECullMode::FrontAndBackFacing;
            case ramses_internal::ECullMode::NUMBER_OF_ELEMENTS:
                break;
            }

            assert(false);
            return ECullMode::Disabled;
        }

        static ramses_internal::EDepthWrite GetDepthWriteInternal(EDepthWrite depthWrite)
        {
            switch (depthWrite)
            {
            case EDepthWrite::Disabled:
                return ramses_internal::EDepthWrite::Disabled;
            case EDepthWrite::Enabled:
                return ramses_internal::EDepthWrite::Enabled;
            }

            assert(false);
            return ramses_internal::EDepthWrite::Enabled;
        }

        static EDepthWrite GetDepthWriteFromInternal(ramses_internal::EDepthWrite depthWrite)
        {
            switch (depthWrite)
            {
            case ramses_internal::EDepthWrite::Disabled:
                return EDepthWrite::Disabled;
            case ramses_internal::EDepthWrite::Enabled:
                return EDepthWrite::Enabled;
            case ramses_internal::EDepthWrite::NUMBER_OF_ELEMENTS:
                break;
            }

            assert(false);
            return EDepthWrite::Enabled;
        }

        static ramses_internal::EScissorTest GetScissorTestInternal(EScissorTest scissorTest)
        {
            switch (scissorTest)
            {
            case EScissorTest::Disabled:
                return ramses_internal::EScissorTest::Disabled;
            case EScissorTest::Enabled:
                return ramses_internal::EScissorTest::Enabled;
            }

            assert(false);
            return ramses_internal::EScissorTest::Disabled;
        }

        static EScissorTest GetScissorTestFromInternal(ramses_internal::EScissorTest scissorTest)
        {
            switch (scissorTest)
            {
            case ramses_internal::EScissorTest::Disabled:
                return EScissorTest::Disabled;
            case ramses_internal::EScissorTest::Enabled:
                return EScissorTest::Enabled;
            case ramses_internal::EScissorTest::NUMBER_OF_ELEMENTS:
                break;
            }

            assert(false);
            return EScissorTest::Disabled;
        }

        static ramses_internal::EDepthFunc GetDepthFuncInternal(EDepthFunc depthFunc)
        {
            switch (depthFunc)
            {
            case EDepthFunc::Disabled:
                return ramses_internal::EDepthFunc::Disabled;
            case EDepthFunc::Greater:
                return ramses_internal::EDepthFunc::Greater;
            case EDepthFunc::GreaterEqual:
                return ramses_internal::EDepthFunc::GreaterEqual;
            case EDepthFunc::Less:
                return ramses_internal::EDepthFunc::Smaller;
            case EDepthFunc::LessEqual:
                return ramses_internal::EDepthFunc::SmallerEqual;
            case EDepthFunc::Always:
                return ramses_internal::EDepthFunc::AlwaysPass;
            case EDepthFunc::Never:
                return ramses_internal::EDepthFunc::NeverPass;
            case EDepthFunc::Equal:
                return ramses_internal::EDepthFunc::Equal;
            case EDepthFunc::NotEqual:
                return ramses_internal::EDepthFunc::NotEqual;
            }

            assert(false);
            return ramses_internal::EDepthFunc::Disabled;
        }

        static EDepthFunc GetDepthFuncFromInternal(ramses_internal::EDepthFunc depthFunc)
        {
            switch (depthFunc)
            {
            case ramses_internal::EDepthFunc::Disabled:
                return EDepthFunc::Disabled;
            case ramses_internal::EDepthFunc::Greater:
                return EDepthFunc::Greater;
            case ramses_internal::EDepthFunc::GreaterEqual:
                return EDepthFunc::GreaterEqual;
            case ramses_internal::EDepthFunc::Smaller:
                return EDepthFunc::Less;
            case ramses_internal::EDepthFunc::SmallerEqual:
                return EDepthFunc::LessEqual;
            case ramses_internal::EDepthFunc::AlwaysPass:
                return EDepthFunc::Always;
            case ramses_internal::EDepthFunc::NeverPass:
                return EDepthFunc::Never;
            case ramses_internal::EDepthFunc::Equal:
                return EDepthFunc::Equal;
            case ramses_internal::EDepthFunc::NotEqual:
                return EDepthFunc::NotEqual;
            case ramses_internal::EDepthFunc::NUMBER_OF_ELEMENTS:
                break;
            }

            assert(false);
            return EDepthFunc::LessEqual;
        }

        static ramses_internal::EStencilFunc GetStencilFuncInternal(EStencilFunc stencilFunc)
        {
            switch (stencilFunc)
            {
            case EStencilFunc::Disabled:
                return ramses_internal::EStencilFunc::Disabled;
            case EStencilFunc::Never:
                return ramses_internal::EStencilFunc::NeverPass;
            case EStencilFunc::Always:
                return ramses_internal::EStencilFunc::AlwaysPass;
            case EStencilFunc::Equal:
                return ramses_internal::EStencilFunc::Equal;
            case EStencilFunc::NotEqual:
                return ramses_internal::EStencilFunc::NotEqual;
            case EStencilFunc::Less:
                return ramses_internal::EStencilFunc::Less;
            case EStencilFunc::LessEqual:
                return ramses_internal::EStencilFunc::LessEqual;
            case EStencilFunc::Greater:
                return ramses_internal::EStencilFunc::Greater;
            case EStencilFunc::GreaterEqual:
                return ramses_internal::EStencilFunc::GreaterEqual;
            }

            assert(false);
            return ramses_internal::EStencilFunc::Disabled;
        }

        static EStencilFunc GetStencilFuncFromInternal(ramses_internal::EStencilFunc stencilFunc)
        {
            switch (stencilFunc)
            {
            case ramses_internal::EStencilFunc::Disabled:
                return EStencilFunc::Disabled;
            case ramses_internal::EStencilFunc::NeverPass:
                return EStencilFunc::Never;
            case ramses_internal::EStencilFunc::AlwaysPass:
                return EStencilFunc::Always;
            case ramses_internal::EStencilFunc::Equal:
                return EStencilFunc::Equal;
            case ramses_internal::EStencilFunc::NotEqual:
                return EStencilFunc::NotEqual;
            case ramses_internal::EStencilFunc::Less:
                return EStencilFunc::Less;
            case ramses_internal::EStencilFunc::LessEqual:
                return EStencilFunc::LessEqual;
            case ramses_internal::EStencilFunc::Greater:
                return EStencilFunc::Greater;
            case ramses_internal::EStencilFunc::GreaterEqual:
                return EStencilFunc::GreaterEqual;
            case ramses_internal::EStencilFunc::NUMBER_OF_ELEMENTS:
                break;
            }

            assert(false);
            return EStencilFunc::Disabled;
        }

        static ramses_internal::EStencilOp GetStencilOperationInternal(EStencilOperation stencilOp)
        {
            switch (stencilOp)
            {
            case EStencilOperation::Keep:
                return ramses_internal::EStencilOp::Keep;
            case EStencilOperation::Zero:
                return ramses_internal::EStencilOp::Zero;
            case EStencilOperation::Replace:
                return ramses_internal::EStencilOp::Replace;
            case EStencilOperation::Increment:
                return ramses_internal::EStencilOp::Increment;
            case EStencilOperation::IncrementWrap:
                return ramses_internal::EStencilOp::IncrementWrap;
            case EStencilOperation::Decrement:
                return ramses_internal::EStencilOp::Decrement;
            case EStencilOperation::DecrementWrap:
                return ramses_internal::EStencilOp::DecrementWrap;
            case EStencilOperation::Invert:
                return ramses_internal::EStencilOp::Invert;
            }

            assert(false);
            return ramses_internal::EStencilOp::Keep;
        }

        static EStencilOperation GetStencilOperationFromInternal(ramses_internal::EStencilOp stencilOp)
        {
            switch (stencilOp)
            {
            case ramses_internal::EStencilOp::Keep:
                return EStencilOperation::Keep;
            case ramses_internal::EStencilOp::Zero:
                return EStencilOperation::Zero;
            case ramses_internal::EStencilOp::Replace:
                return EStencilOperation::Replace;
            case ramses_internal::EStencilOp::Increment:
                return EStencilOperation::Increment;
            case ramses_internal::EStencilOp::IncrementWrap:
                return EStencilOperation::IncrementWrap;
            case ramses_internal::EStencilOp::Decrement:
                return EStencilOperation::Decrement;
            case ramses_internal::EStencilOp::DecrementWrap:
                return EStencilOperation::DecrementWrap;
            case ramses_internal::EStencilOp::Invert:
                return EStencilOperation::Invert;
            case ramses_internal::EStencilOp::NUMBER_OF_ELEMENTS:
                break;
            }

            assert(false);
            return EStencilOperation::Keep;
        }

        static ramses_internal::EDrawMode GetDrawModeInternal(EDrawMode mode)
        {
            switch (mode)
            {
            case EDrawMode::Points:
                return ramses_internal::EDrawMode::Points;
            case EDrawMode::Lines:
                return ramses_internal::EDrawMode::Lines;
            case EDrawMode::LineLoop:
                return ramses_internal::EDrawMode::LineLoop;
            case EDrawMode::Triangles:
                return ramses_internal::EDrawMode::Triangles;
            case EDrawMode::TriangleStrip:
                return ramses_internal::EDrawMode::TriangleStrip;
            case EDrawMode::TriangleFan:
                return ramses_internal::EDrawMode::TriangleFan;
            case EDrawMode::LineStrip:
                return ramses_internal::EDrawMode::LineStrip;
            }

            assert(false);
            return ramses_internal::EDrawMode::Triangles;
        }

        static bool GeometryShaderCompatibleWithDrawMode(EDrawMode geometryShaderInputType, EDrawMode drawMode)
        {
            // only basic 'variant' (i.e. no strip/fan) of a primitive is allowed as GS input declaration
            assert(geometryShaderInputType == EDrawMode::Points || geometryShaderInputType == EDrawMode::Lines || geometryShaderInputType == EDrawMode::Triangles);

            switch (drawMode)
            {
            case EDrawMode::Points:
                return geometryShaderInputType == EDrawMode::Points;
            case EDrawMode::Lines:
            case EDrawMode::LineStrip:
            case EDrawMode::LineLoop:
                return geometryShaderInputType == EDrawMode::Lines;
            case EDrawMode::Triangles:
            case EDrawMode::TriangleStrip:
            case EDrawMode::TriangleFan:
                return geometryShaderInputType == EDrawMode::Triangles;
            }

            assert(false);
            return false;
        }

        static EDrawMode GetDrawModeFromInternal(ramses_internal::EDrawMode drawMode)
        {
            switch (drawMode)
            {
            case ramses_internal::EDrawMode::Points:
                return EDrawMode::Points;
            case ramses_internal::EDrawMode::Lines:
                return EDrawMode::Lines;
            case ramses_internal::EDrawMode::LineLoop:
                return EDrawMode::LineLoop;
            case ramses_internal::EDrawMode::Triangles:
                return EDrawMode::Triangles;
            case ramses_internal::EDrawMode::TriangleStrip:
                return EDrawMode::TriangleStrip;
            case ramses_internal::EDrawMode::TriangleFan:
                return EDrawMode::TriangleFan;
            case ramses_internal::EDrawMode::LineStrip:
                return EDrawMode::LineStrip;
            case ramses_internal::EDrawMode::NUMBER_OF_ELEMENTS:
                break;
            }

            assert(false);
            return EDrawMode::Triangles;
        }
    };
}

#endif
