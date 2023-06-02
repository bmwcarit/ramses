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
    /**
     * @addtogroup CoreAPI
     * @{
     */

    /// Texture sampling method
    enum class ETextureSamplingMethod
    {
        Nearest,
        Linear,
        Nearest_MipMapNearest,
        Nearest_MipMapLinear,
        Linear_MipMapNearest,
        Linear_MipMapLinear,
    };

    /// Texture address mode
    enum class ETextureAddressMode
    {
        Clamp,
        Repeat,
        Mirror,
    };

    /// Texture data format
    enum class ETextureFormat
    {
        Invalid,

        R8,

        RG8,

        RGB8,
        RGB565,

        RGBA8,
        RGBA4,
        RGBA5551,

        ETC2RGB,
        ETC2RGBA,

        R16F,
        R32F,
        RG16F,
        RG32F,
        RGB16F,
        RGB32F,
        RGBA16F,
        RGBA32F,

        SRGB8,
        SRGB8_ALPHA8,

        // ASTC compressed texture formats - USE WITH CAUTION!
        // ASTC depends on GL ES 3.0 extensions "GL_KHR_texture_compression_astc_hdr" and "GL_KHR_texture_compression_astc_ldr"
        // If the drivers don't support these extensions, expect asserts in the renderer (or black screen in release build)
        // Important note: the "RGBA" in the name only distinguishes between RGBA and sRGBA. The channel count and format are
        // defined internally by the compression algorithm. Refer to the documentation of ASTC extension from above for more details
        ASTC_RGBA_4x4,
        ASTC_RGBA_5x4,
        ASTC_RGBA_5x5,
        ASTC_RGBA_6x5,
        ASTC_RGBA_6x6,
        ASTC_RGBA_8x5,
        ASTC_RGBA_8x6,
        ASTC_RGBA_8x8,
        ASTC_RGBA_10x5,
        ASTC_RGBA_10x6,
        ASTC_RGBA_10x8,
        ASTC_RGBA_10x10,
        ASTC_RGBA_12x10,
        ASTC_RGBA_12x12,
        ASTC_SRGBA_4x4,
        ASTC_SRGBA_5x4,
        ASTC_SRGBA_5x5,
        ASTC_SRGBA_6x5,
        ASTC_SRGBA_6x6,
        ASTC_SRGBA_8x5,
        ASTC_SRGBA_8x6,
        ASTC_SRGBA_8x8,
        ASTC_SRGBA_10x5,
        ASTC_SRGBA_10x6,
        ASTC_SRGBA_10x8,
        ASTC_SRGBA_10x10,
        ASTC_SRGBA_12x10,
        ASTC_SRGBA_12x12,
    };

    /// Cube texture face identifier
    enum class ETextureCubeFace
    {
        PositiveX,
        NegativeX,
        PositiveY,
        NegativeY,
        PositiveZ,
        NegativeZ,
    };

    /// Enum for type of depth buffer created within a RenderTarget
    enum class ERenderTargetDepthBufferType
    {
        None,
        Depth,
        DepthStencil
    };

    /// Enum for type of a RenderBuffer
    enum class ERenderBufferType
    {
        Color,
        Depth,
        DepthStencil
    };

    /// Enum for format of a RenderBuffer
    enum class ERenderBufferFormat
    {
        RGBA4,
        R8,
        RG8,
        RGB8,
        RGBA8,
        R16F,
        R32F,
        RG16F,
        RG32F,
        RGB16F,
        RGB32F,
        RGBA16F,
        RGBA32F,

        Depth24,
        Depth24_Stencil8
    };

    ///Enum for color of texture channel
    enum class ETextureChannelColor : uint8_t
    {
        Red,
        Green,
        Blue,
        Alpha,
        One,
        Zero,
    };


    /// Enum for access mode of a RenderBuffer
    enum class ERenderBufferAccessMode
    {
        WriteOnly,  ///< RenderBuffer with this access can only be used in RenderTarget
        ReadWrite   ///< RenderBuffer with this access can be used both in RenderTarget and TextureSampler
    };

    /**
     * @brief Returns string representation for sampling method
     * @details Useful for logging, etc.
     *
     * @param samplingMethod The enum parameter for which you will get the string
     * @return String representation of the sampling method
     */
    RAMSES_API const char* toString(ETextureSamplingMethod samplingMethod);

    /**
     * @brief Returns string representation for address mode
     * @details Useful for logging, etc.
     *
     * @param addressMode The enum parameter for which you will get the string
     * @return String representation of the address mode
     */
    RAMSES_API const char* toString(ETextureAddressMode addressMode);

    /**
     * @brief Returns string representation for texture format
     * @details Useful for logging, etc.
     *
     * @param format The enum parameter for which you will get the string
     * @return String representation of the texture format
     */
    RAMSES_API const char* toString(ETextureFormat format);

    /**
     * @brief Returns string representation for texture's cube face
     * @details Useful for logging, etc.
     *
     * @param face The enum parameter for which you will get the string
     * @return String representation of the cube face
     */
    RAMSES_API const char* toString(ETextureCubeFace face);

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
        case ETextureFormat::ETC2RGB:
        case ETextureFormat::ETC2RGBA:
        case ETextureFormat::ASTC_RGBA_4x4:
        case ETextureFormat::ASTC_RGBA_5x4:
        case ETextureFormat::ASTC_RGBA_5x5:
        case ETextureFormat::ASTC_RGBA_6x5:
        case ETextureFormat::ASTC_RGBA_6x6:
        case ETextureFormat::ASTC_RGBA_8x5:
        case ETextureFormat::ASTC_RGBA_8x6:
        case ETextureFormat::ASTC_RGBA_8x8:
        case ETextureFormat::ASTC_RGBA_10x5:
        case ETextureFormat::ASTC_RGBA_10x6:
        case ETextureFormat::ASTC_RGBA_10x8:
        case ETextureFormat::ASTC_RGBA_10x10:
        case ETextureFormat::ASTC_RGBA_12x10:
        case ETextureFormat::ASTC_RGBA_12x12:
        case ETextureFormat::ASTC_SRGBA_4x4:
        case ETextureFormat::ASTC_SRGBA_5x4:
        case ETextureFormat::ASTC_SRGBA_5x5:
        case ETextureFormat::ASTC_SRGBA_6x5:
        case ETextureFormat::ASTC_SRGBA_6x6:
        case ETextureFormat::ASTC_SRGBA_8x5:
        case ETextureFormat::ASTC_SRGBA_8x6:
        case ETextureFormat::ASTC_SRGBA_8x8:
        case ETextureFormat::ASTC_SRGBA_10x5:
        case ETextureFormat::ASTC_SRGBA_10x6:
        case ETextureFormat::ASTC_SRGBA_10x8:
        case ETextureFormat::ASTC_SRGBA_10x10:
        case ETextureFormat::ASTC_SRGBA_12x10:
        case ETextureFormat::ASTC_SRGBA_12x12:
            return false;
        default:
            return true;
        }
    }

    /**
     * @}
     */
}

#endif
