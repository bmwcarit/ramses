//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_APPEARANCEENUMS_H
#define RAMSES_APPEARANCEENUMS_H

#include "ramses-framework-api/APIExport.h"
#include <cstdint>

namespace ramses
{
    /**
     * @addtogroup CoreAPI
     * @{
     */

    /**
    * Specifies the blending operation.
    * @ingroup CoreAPI
    */
    enum class EBlendOperation : uint8_t
    {
        Disabled,
        Add,
        Subtract,
        ReverseSubtract,
        Min,
        Max,
    };

    /**
    * Specifies the blending factor used with blending operation.
    * @ingroup CoreAPI
    */
    enum class EBlendFactor : uint8_t
    {
        Zero,
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
    };

    /**
    * Specifies the culling mode.
    */
    enum class ECullMode : uint8_t
    {
        Disabled,
        FrontFacing,
        BackFacing,
        FrontAndBackFacing,
    };

    /**
    * Specifies the depth write state.
    */
    enum class EDepthWrite : uint8_t
    {
        Disabled,
        Enabled,
    };

    /**
    * Specifies the scissor test state.
    */
    enum class EScissorTest : uint8_t
    {
        Disabled,
        Enabled,
    };

    /**
    * Specifies the depth function.
    */
    enum class EDepthFunc : uint8_t
    {
        Disabled,
        Greater,
        GreaterEqual,
        Less,
        LessEqual,
        Equal,
        NotEqual,
        Always,
        Never,
    };

    /**
    * Specifies the stencil function.
    */
    enum class EStencilFunc : uint8_t
    {
        Disabled,
        Never,
        Always,
        Equal,
        NotEqual,
        Less,
        LessEqual,
        Greater,
        GreaterEqual,
    };

    /**
    * Specifies the stencil operation.
    */
    enum class EStencilOperation : uint8_t
    {
        Keep,
        Zero,
        Replace,
        Increment,
        IncrementWrap,
        Decrement,
        DecrementWrap,
        Invert,
    };

    /**
    * Specifies the drawing mode.
    */
    enum class EDrawMode : uint8_t
    {
        Points,
        Lines,
        LineLoop,
        Triangles,
        TriangleStrip,
        TriangleFan,
        LineStrip,
    };

    /**
     * @brief Returns string representation for blend operation
     * @details Useful for logging, etc.
     *
     * @param blendOperation The enum parameter for which you will get the string
     * @return String representation of the blend operation
     */
    RAMSES_API const char* toString(EBlendOperation blendOperation);

    /**
     * @brief Returns string representation for blend factor
     * @details Useful for logging, etc.
     *
     * @param blendFactor The enum parameter for which you will get the string
     * @return String representation of the blend factor
     */
    RAMSES_API const char* toString(EBlendFactor blendFactor);

    /**
     * @brief Returns string representation for cull mode
     * @details Useful for logging, etc.
     *
     * @param cullMode The enum parameter for which you will get the string
     * @return String representation of the cull mode
     */
    RAMSES_API const char* toString(ECullMode cullMode);

    /**
     * @brief Returns string representation for depth write
     * @details Useful for logging, etc.
     *
     * @param depthWrite The enum parameter for which you will get the string
     * @return String representation of the depth write
     */
    RAMSES_API const char* toString(EDepthWrite depthWrite);

    /**
     * @brief Returns string representation for scissor test
     * @details Useful for logging, etc.
     *
     * @param scissorTest The enum parameter for which you will get the string
     * @return String representation of the scissor test
     */
    RAMSES_API const char* toString(EScissorTest scissorTest);

    /**
     * @brief Returns string representation for depth function
     * @details Useful for logging, etc.
     *
     * @param depthFunc The enum parameter for which you will get the string
     * @return String representation of the depth function
     */
    RAMSES_API const char* toString(EDepthFunc depthFunc);

    /**
     * @brief Returns string representation for stencil function
     * @details Useful for logging, etc.
     *
     * @param stencilFunc The enum parameter for which you will get the string
     * @return String representation of the stencil function
     */
    RAMSES_API const char* toString(EStencilFunc stencilFunc);

    /**
     * @brief Returns string representation for stencil operation
     * @details Useful for logging, etc.
     *
     * @param stencilOp The enum parameter for which you will get the string
     * @return String representation of the stencil operation
     */
    RAMSES_API const char* toString(EStencilOperation stencilOp);

    /**
     * @brief Returns string representation for draw mode
     * @details Useful for logging, etc.
     *
     * @param drawMode The enum parameter for which you will get the string
     * @return String representation of the draw mode
     */
    RAMSES_API const char* toString(EDrawMode drawMode);

    /**
     * @}
     */
}

#endif  //RAMSES_APPEARANCEENUMS_H
