//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TEXTUREENUMS_H
#define RAMSES_TEXTUREENUMS_H

#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "ramses-framework-api/APIExport.h"

namespace ramses
{
    /// Texture sampling method
    enum ETextureSamplingMethod
    {
        ETextureSamplingMethod_Nearest = 0,
        ETextureSamplingMethod_Linear,
        ETextureSamplingMethod_Nearest_MipMapNearest,
        ETextureSamplingMethod_Nearest_MipMapLinear,
        ETextureSamplingMethod_Linear_MipMapNearest,
        ETextureSamplingMethod_Linear_MipMapLinear,
        ETextureSamplingMethod_NUMBER_OF_ELEMENTS
    };

    /// Texture address mode
    enum ETextureAddressMode
    {
        ETextureAddressMode_Clamp = 0,
        ETextureAddressMode_Repeat,
        ETextureAddressMode_Mirror,
        ETextureAddressMode_NUMBER_OF_ELEMENTS
    };

    /// Texture data format
    enum ETextureFormat
    {
        ETextureFormat_Invalid = 0,

        ETextureFormat_R8,

        ETextureFormat_RG8,

        ETextureFormat_RGB8,
        ETextureFormat_RGB565,

        ETextureFormat_RGBA8,
        ETextureFormat_RGBA4,
        ETextureFormat_RGBA5551,

        ETextureFormat_BGR8,
        ETextureFormat_BGRA8,

        ETextureFormat_ETC2RGB,
        ETextureFormat_ETC2RGBA,

        ETextureFormat_R16F,
        ETextureFormat_R32F,
        ETextureFormat_RG16F,
        ETextureFormat_RG32F,
        ETextureFormat_RGB16F,
        ETextureFormat_RGB32F,
        ETextureFormat_RGBA16F,
        ETextureFormat_RGBA32F,

        ETextureFormat_SRGB8,
        ETextureFormat_SRGB8_ALPHA8,

        // ASTC compressed texture formats - USE WITH CAUTION!
        // ASTC depends on GL ES 3.0 extensions "GL_KHR_texture_compression_astc_hdr" and "GL_KHR_texture_compression_astc_ldr"
        // If the drivers don't support these extensions, expect asserts in the renderer (or black screen in release build)
        // Important note: the "RGBA" in the name only distinguishes between RGBA and sRGBA. The channel count and format are
        // defined internally by the compression algorithm. Refer to the documentation of ASTC extension from above for more details
        ETextureFormat_ASTC_RGBA_4x4,
        ETextureFormat_ASTC_RGBA_5x4,
        ETextureFormat_ASTC_RGBA_5x5,
        ETextureFormat_ASTC_RGBA_6x5,
        ETextureFormat_ASTC_RGBA_6x6,
        ETextureFormat_ASTC_RGBA_8x5,
        ETextureFormat_ASTC_RGBA_8x6,
        ETextureFormat_ASTC_RGBA_8x8,
        ETextureFormat_ASTC_RGBA_10x5,
        ETextureFormat_ASTC_RGBA_10x6,
        ETextureFormat_ASTC_RGBA_10x8,
        ETextureFormat_ASTC_RGBA_10x10,
        ETextureFormat_ASTC_RGBA_12x10,
        ETextureFormat_ASTC_RGBA_12x12,
        ETextureFormat_ASTC_SRGBA_4x4,
        ETextureFormat_ASTC_SRGBA_5x4,
        ETextureFormat_ASTC_SRGBA_5x5,
        ETextureFormat_ASTC_SRGBA_6x5,
        ETextureFormat_ASTC_SRGBA_6x6,
        ETextureFormat_ASTC_SRGBA_8x5,
        ETextureFormat_ASTC_SRGBA_8x6,
        ETextureFormat_ASTC_SRGBA_8x8,
        ETextureFormat_ASTC_SRGBA_10x5,
        ETextureFormat_ASTC_SRGBA_10x6,
        ETextureFormat_ASTC_SRGBA_10x8,
        ETextureFormat_ASTC_SRGBA_10x10,
        ETextureFormat_ASTC_SRGBA_12x10,
        ETextureFormat_ASTC_SRGBA_12x12,

        ETextureFormat_NUMBER_OF_ELEMENTS
    };

    /// Cube texture face identifier
    enum ETextureCubeFace
    {
        ETextureCubeFace_PositiveX = 0,
        ETextureCubeFace_NegativeX,
        ETextureCubeFace_PositiveY,
        ETextureCubeFace_NegativeY,
        ETextureCubeFace_PositiveZ,
        ETextureCubeFace_NegativeZ,
        ETextureCubeFace_NUMBER_OF_ELEMENTS
    };

    /// Enum for type of depth buffer created within a RenderTarget
    enum ERenderTargetDepthBufferType
    {
        ERenderTargetDepthBufferType_None = 0,
        ERenderTargetDepthBufferType_Depth,
        ERenderTargetDepthBufferType_DepthStencil
    };

    /// Enum for type of a RenderBuffer
    enum ERenderBufferType
    {
        ERenderBufferType_Color = 0,
        ERenderBufferType_Depth,
        ERenderBufferType_DepthStencil
    };

    /// Enum for format of a RenderBuffer
    enum ERenderBufferFormat
    {
        ERenderBufferFormat_R8 = 0,
        ERenderBufferFormat_RG8,
        ERenderBufferFormat_RGB8,
        ERenderBufferFormat_RGBA8,
        ERenderBufferFormat_R16F,
        ERenderBufferFormat_R32F,
        ERenderBufferFormat_RG16F,
        ERenderBufferFormat_RG32F,
        ERenderBufferFormat_RGB16F,
        ERenderBufferFormat_RGB32F,
        ERenderBufferFormat_RGBA16F,
        ERenderBufferFormat_RGBA32F,

        ERenderBufferFormat_Depth24,
        ERenderBufferFormat_Depth24_Stencil8
    };

    /// Enum for access mode of a RenderBuffer
    enum ERenderBufferAccessMode
    {
        ERenderBufferAccessMode_WriteOnly = 0, /// RenderBuffer with this access can only be used in RenderTarget
        ERenderBufferAccessMode_ReadWrite /// RenderBuffer with this access can be used both in RenderTarget and TextureSampler
    };

    /**
     * @brief Returns string representation for sampling method
     * @details Useful for logging, etc.
     *
     * @param samplingMethod The enum parameter for which you will get the string
     * @return String representation of the sampling method
     */
    RAMSES_API const char* getTextureSamplingMethodString(ETextureSamplingMethod samplingMethod);

    /**
     * @brief Returns string representation for address mode
     * @details Useful for logging, etc.
     *
     * @param addressMode The enum parameter for which you will get the string
     * @return String representation of the address mode
     */
    RAMSES_API const char* getTextureAddressModeString(ETextureAddressMode addressMode);

    /**
     * @brief Returns string representation for texture format
     * @details Useful for logging, etc.
     *
     * @param format The enum parameter for which you will get the string
     * @return String representation of the texture format
     */
    RAMSES_API const char* getTextureFormatString(ETextureFormat format);

    /**
     * @brief Returns string representation for texture's cube face
     * @details Useful for logging, etc.
     *
     * @param face The enum parameter for which you will get the string
     * @return String representation of the cube face
     */
    RAMSES_API const char* getTextureCubeFaceString(ETextureCubeFace face);

    /**
    * Returns if given texture format supports mipmap chain generation
    *
    * @param format the format in question
    * @return true if given format supports generation of mipchain
    **/
    inline bool FormatSupportsMipChainGeneration(ETextureFormat format)
    {
        switch (format)
        {
        case ETextureFormat_ETC2RGB:
        case ETextureFormat_ETC2RGBA:
        case ETextureFormat_ASTC_RGBA_4x4:
        case ETextureFormat_ASTC_RGBA_5x4:
        case ETextureFormat_ASTC_RGBA_5x5:
        case ETextureFormat_ASTC_RGBA_6x5:
        case ETextureFormat_ASTC_RGBA_6x6:
        case ETextureFormat_ASTC_RGBA_8x5:
        case ETextureFormat_ASTC_RGBA_8x6:
        case ETextureFormat_ASTC_RGBA_8x8:
        case ETextureFormat_ASTC_RGBA_10x5:
        case ETextureFormat_ASTC_RGBA_10x6:
        case ETextureFormat_ASTC_RGBA_10x8:
        case ETextureFormat_ASTC_RGBA_10x10:
        case ETextureFormat_ASTC_RGBA_12x10:
        case ETextureFormat_ASTC_RGBA_12x12:
        case ETextureFormat_ASTC_SRGBA_4x4:
        case ETextureFormat_ASTC_SRGBA_5x4:
        case ETextureFormat_ASTC_SRGBA_5x5:
        case ETextureFormat_ASTC_SRGBA_6x5:
        case ETextureFormat_ASTC_SRGBA_6x6:
        case ETextureFormat_ASTC_SRGBA_8x5:
        case ETextureFormat_ASTC_SRGBA_8x6:
        case ETextureFormat_ASTC_SRGBA_8x8:
        case ETextureFormat_ASTC_SRGBA_10x5:
        case ETextureFormat_ASTC_SRGBA_10x6:
        case ETextureFormat_ASTC_SRGBA_10x8:
        case ETextureFormat_ASTC_SRGBA_10x10:
        case ETextureFormat_ASTC_SRGBA_12x10:
        case ETextureFormat_ASTC_SRGBA_12x12:
            return false;
        default:
            return true;
        }
    }
}

#endif
