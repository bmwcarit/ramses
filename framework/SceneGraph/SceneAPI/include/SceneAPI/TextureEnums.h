//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_FRAMEWORK_TEXTUREENUMS_H
#define RAMSES_FRAMEWORK_TEXTUREENUMS_H

#include <cassert>
#include "PlatformAbstraction/PlatformTypes.h"
#include "Utils/LoggingUtils.h"

namespace ramses_internal
{
    enum class ESamplingMethod : uint8_t
    {
        Nearest = 0,
        Linear,
        Nearest_MipMapNearest,
        Nearest_MipMapLinear,
        Linear_MipMapNearest,
        Linear_MipMapLinear,
        NUMBER_OF_ELEMENTS
    };

    static const char* SamplingMethodNames[] =
    {
        "ESamplingMethod_Nearest",
        "ESamplingMethod_Linear",
        "ESamplingMethod_Nearest_MipMapNearest",
        "ESamplingMethod_Nearest_MipMapLinear",
        "ESamplingMethod_Linear_MipMapNearest",
        "ESamplingMethod_Linear_MipMapLinear",
    };
    ENUM_TO_STRING(ESamplingMethod, SamplingMethodNames, ESamplingMethod::NUMBER_OF_ELEMENTS);

    enum class EWrapMethod : uint8_t
    {
        Clamp = 0,
        Repeat,
        RepeatMirrored,
        NUMBER_OF_ELEMENTS
    };

    static const char* WrapMethodNames[] =
    {
        "EWrapMethod_Clamp",
        "EWrapMethod_Repeat",
        "EWrapMethod_RepeatMirrored"
    };
    ENUM_TO_STRING(EWrapMethod, WrapMethodNames, EWrapMethod::NUMBER_OF_ELEMENTS);

    enum ERenderBufferType
    {
        ERenderBufferType_InvalidBuffer = 0,
        ERenderBufferType_DepthBuffer,
        ERenderBufferType_DepthStencilBuffer,
        ERenderBufferType_ColorBuffer,

        ERenderBufferType_NUMBER_OF_ELEMENTS
    };

    static const char* RenderBufferTypeNames[] =
    {
        "ERenderBufferType_InvalidBuffer",
        "ERenderBufferType_DepthBuffer",
        "ERenderBufferType_DepthStencilBuffer",
        "ERenderBufferType_ColorBuffer"
    };
    ENUM_TO_STRING(ERenderBufferType, RenderBufferTypeNames, ERenderBufferType_NUMBER_OF_ELEMENTS);

    enum ERenderBufferAccessMode
    {
        ERenderBufferAccessMode_Invalid = 0,
        ERenderBufferAccessMode_WriteOnly,
        ERenderBufferAccessMode_ReadWrite,

        ERenderBufferAccessMode_NUMBER_OF_ELEMENTS
    };

    static const char* RenderBufferAccessModeNames[] =
    {
        "ERenderBufferAccessMode_Invalid",
        "ERenderBufferAccessMode_WriteOnly",
        "ERenderBufferAccessMode_ReadWrite"
    };
    ENUM_TO_STRING(ERenderBufferAccessMode, RenderBufferAccessModeNames, ERenderBufferAccessMode_NUMBER_OF_ELEMENTS);

    enum class ETextureFormat
    {
        Invalid = 0,

        R8,
        R16,

        RG8,
        RG16,

        RGB16,
        RGB8,
        RGB565,

        RGBA16,
        RGBA8,
        RGBA4,
        RGBA5551,

        ETC2RGB, // ericsson texture compression 2
        ETC2RGBA, // ericsson texture compression 2 with alpha
        DXT1RGB, // S3 texture compression 1
        DXT3RGBA, // S3 texture compression 3 with alpha
        DXT5RGBA, // S3 texture compression 5 with alpha

        Depth16,
        Depth24,
        Depth24_Stencil8,

        // Floating point formats
        R16F,
        R32F,
        RG16F,
        RG32F,
        RGB16F,
        RGB32F,
        RGBA16F,
        RGBA32F,

        // sRGB formats
        SRGB8,
        SRGB8_ALPHA8,

        // ASTC formats
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

        NUMBER_OF_TYPES
    };

    enum ETextureCubeFace
    {
        ETextureCubeFace_PositiveX = 0,
        ETextureCubeFace_NegativeX,
        ETextureCubeFace_PositiveY,
        ETextureCubeFace_NegativeY,
        ETextureCubeFace_PositiveZ,
        ETextureCubeFace_NegativeZ
    };

    enum class ETextureChannelColor : uint8_t
    {
        Red,
        Green,
        Blue,
        Alpha,
        One,
        Zero,

        NUMBER_OF_ELEMENTS,
    };

    static const char* TextureChannelColorNames[] =
    {
        "Red",
        "Green",
        "Blue",
        "Alpha",
        "One",
        "Zero"
    };
    ENUM_TO_STRING(ETextureChannelColor, TextureChannelColorNames, ETextureChannelColor::NUMBER_OF_ELEMENTS);

    static inline bool IsFormatCompressed(ramses_internal::ETextureFormat textureformat)
    {
        switch (textureformat)
        {
        case ramses_internal::ETextureFormat::ETC2RGB:
        case ramses_internal::ETextureFormat::ETC2RGBA:
        case ramses_internal::ETextureFormat::DXT1RGB:
        case ramses_internal::ETextureFormat::DXT3RGBA:
        case ramses_internal::ETextureFormat::DXT5RGBA:
        case ramses_internal::ETextureFormat::ASTC_RGBA_4x4:
        case ramses_internal::ETextureFormat::ASTC_RGBA_5x4:
        case ramses_internal::ETextureFormat::ASTC_RGBA_5x5:
        case ramses_internal::ETextureFormat::ASTC_RGBA_6x5:
        case ramses_internal::ETextureFormat::ASTC_RGBA_6x6:
        case ramses_internal::ETextureFormat::ASTC_RGBA_8x5:
        case ramses_internal::ETextureFormat::ASTC_RGBA_8x6:
        case ramses_internal::ETextureFormat::ASTC_RGBA_8x8:
        case ramses_internal::ETextureFormat::ASTC_RGBA_10x5:
        case ramses_internal::ETextureFormat::ASTC_RGBA_10x6:
        case ramses_internal::ETextureFormat::ASTC_RGBA_10x8:
        case ramses_internal::ETextureFormat::ASTC_RGBA_10x10:
        case ramses_internal::ETextureFormat::ASTC_RGBA_12x10:
        case ramses_internal::ETextureFormat::ASTC_RGBA_12x12:
        case ramses_internal::ETextureFormat::ASTC_SRGBA_4x4:
        case ramses_internal::ETextureFormat::ASTC_SRGBA_5x4:
        case ramses_internal::ETextureFormat::ASTC_SRGBA_5x5:
        case ramses_internal::ETextureFormat::ASTC_SRGBA_6x5:
        case ramses_internal::ETextureFormat::ASTC_SRGBA_6x6:
        case ramses_internal::ETextureFormat::ASTC_SRGBA_8x5:
        case ramses_internal::ETextureFormat::ASTC_SRGBA_8x6:
        case ramses_internal::ETextureFormat::ASTC_SRGBA_8x8:
        case ramses_internal::ETextureFormat::ASTC_SRGBA_10x5:
        case ramses_internal::ETextureFormat::ASTC_SRGBA_10x6:
        case ramses_internal::ETextureFormat::ASTC_SRGBA_10x8:
        case ramses_internal::ETextureFormat::ASTC_SRGBA_10x10:
        case ramses_internal::ETextureFormat::ASTC_SRGBA_12x10:
        case ramses_internal::ETextureFormat::ASTC_SRGBA_12x12:
            return true;
        default:
            return false;
        }
    }

    static inline UInt32 GetTexelSizeFromFormat(ETextureFormat texelFormat)
    {
        switch (texelFormat)
        {
        case ETextureFormat::R8:
            return 1u;
        case ETextureFormat::R16:
        case ETextureFormat::R16F:
        case ETextureFormat::RG8:
        case ETextureFormat::RGB565:
        case ETextureFormat::RGBA4:
        case ETextureFormat::RGBA5551:
        case ETextureFormat::Depth16:
            return 2u;
        case ETextureFormat::RGB8:
        case ETextureFormat::Depth24:
        case ETextureFormat::SRGB8:
            return 3u;
        case ETextureFormat::RG16:
        case ETextureFormat::RG16F:
        case ETextureFormat::RGBA8:
        case ETextureFormat::Depth24_Stencil8:
        case ETextureFormat::R32F:
        case ETextureFormat::SRGB8_ALPHA8:
            return 4u;
        case ETextureFormat::RGB16:
        case ETextureFormat::RGB16F:
            return 6u;
        case ETextureFormat::RG32F:
        case ETextureFormat::RGBA16:
        case ETextureFormat::RGBA16F:
            return 8u;
        case ETextureFormat::RGB32F:
            return 12u;
        case ETextureFormat::RGBA32F:
            return 16u;

        default:
            assert(false && "Unknown texel size");
            return 1u;
        }
    }

    static const char* TextureFormatNames[] = {
        "ETextureFormat_Invalid",
        "ETextureFormat_R8",
        "ETextureFormat_R16",
        "ETextureFormat_RG8",
        "ETextureFormat_RG16",
        "ETextureFormat_RGB16",
        "ETextureFormat_RGB8",
        "ETextureFormat_RGB565",
        "ETextureFormat_RGBA16",
        "ETextureFormat_RGBA8",
        "ETextureFormat_RGBA4",
        "ETextureFormat_RGBA5551",
        "ETextureFormat_ETC2RGB",
        "ETextureFormat_ETC2RGBA",
        "ETextureFormat_DXT1RGB",
        "ETextureFormat_DXT3RGBA",
        "ETextureFormat_DXT5RGBA",
        "ETextureFormat_Depth16",
        "ETextureFormat_Depth24",
        "ETextureFormat_Depth24_Stencil8",
        "ETextureFormat_R16F",
        "ETextureFormat_R32F",
        "ETextureFormat_RG16F",
        "ETextureFormat_RG32F",
        "ETextureFormat_RGB16F",
        "ETextureFormat_RGB32F",
        "ETextureFormat_RGBA16F",
        "ETextureFormat_RGBA32F",
        "ETextureFormat_SRGB8",
        "ETextureFormat_SRGB8_ALPHA8",
        "ETextureFormat_ASTC_RGBA_4x4",
        "ETextureFormat_ASTC_RGBA_5x4",
        "ETextureFormat_ASTC_RGBA_5x5",
        "ETextureFormat_ASTC_RGBA_6x5",
        "ETextureFormat_ASTC_RGBA_6x6",
        "ETextureFormat_ASTC_RGBA_8x5",
        "ETextureFormat_ASTC_RGBA_8x6",
        "ETextureFormat_ASTC_RGBA_8x8",
        "ETextureFormat_ASTC_RGBA_10x5",
        "ETextureFormat_ASTC_RGBA_10x6",
        "ETextureFormat_ASTC_RGBA_10x8",
        "ETextureFormat_ASTC_RGBA_10x10",
        "ETextureFormat_ASTC_RGBA_12x10",
        "ETextureFormat_ASTC_RGBA_12x12",
        "ETextureFormat_ASTC_SRGBA_4x4",
        "ETextureFormat_ASTC_SRGBA_5x4",
        "ETextureFormat_ASTC_SRGBA_5x5",
        "ETextureFormat_ASTC_SRGBA_6x5",
        "ETextureFormat_ASTC_SRGBA_6x6",
        "ETextureFormat_ASTC_SRGBA_8x5",
        "ETextureFormat_ASTC_SRGBA_8x6",
        "ETextureFormat_ASTC_SRGBA_8x8",
        "ETextureFormat_ASTC_SRGBA_10x5",
        "ETextureFormat_ASTC_SRGBA_10x6",
        "ETextureFormat_ASTC_SRGBA_10x8",
        "ETextureFormat_ASTC_SRGBA_10x10",
        "ETextureFormat_ASTC_SRGBA_12x10",
        "ETextureFormat_ASTC_SRGBA_12x12"
    };

    ENUM_TO_STRING(ETextureFormat, TextureFormatNames, ETextureFormat::NUMBER_OF_TYPES);
}

#endif
