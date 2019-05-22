//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_FRAMEWORK_TEXTUREENUMS_H
#define RAMSES_FRAMEWORK_TEXTUREENUMS_H

#include <assert.h>
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

    enum ETextureFormat
    {
        ETextureFormat_Invalid = 0,

        ETextureFormat_R8,
        ETextureFormat_R16,

        ETextureFormat_RG8,
        ETextureFormat_RG16,

        ETextureFormat_RGB16,
        ETextureFormat_RGB8,
        ETextureFormat_RGB565,

        ETextureFormat_RGBA16,
        ETextureFormat_RGBA8,
        ETextureFormat_RGBA4,
        ETextureFormat_RGBA5551,

        ETextureFormat_BGR8,
        ETextureFormat_BGRA8,

        ETextureFormat_ETC2RGB, // ericsson texture compression 2
        ETextureFormat_ETC2RGBA, // ericsson texture compression 2 with alpha
        ETextureFormat_DXT1RGB, // S3 texture compression 1
        ETextureFormat_DXT3RGBA, // S3 texture compression 3 with alpha
        ETextureFormat_DXT5RGBA, // S3 texture compression 5 with alpha

        ETextureFormat_Depth16,
        ETextureFormat_Depth24,
        ETextureFormat_Depth24_Stencil8,

        // Floating point formats
        ETextureFormat_R16F,
        ETextureFormat_R32F,
        ETextureFormat_RG16F,
        ETextureFormat_RG32F,
        ETextureFormat_RGB16F,
        ETextureFormat_RGB32F,
        ETextureFormat_RGBA16F,
        ETextureFormat_RGBA32F,

        // sRGB formats
        ETextureFormat_SRGB8,
        ETextureFormat_SRGB8_ALPHA8,

        // ASTC formats
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

        ETextureFormat_NUMBER_OF_TYPES
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

    static inline bool IsFormatCompressed(ramses_internal::ETextureFormat textureformat)
    {
        switch (textureformat)
        {
        case ramses_internal::ETextureFormat_ETC2RGB:
        case ramses_internal::ETextureFormat_ETC2RGBA:
        case ramses_internal::ETextureFormat_DXT1RGB:
        case ramses_internal::ETextureFormat_DXT3RGBA:
        case ramses_internal::ETextureFormat_DXT5RGBA:
        case ramses_internal::ETextureFormat_ASTC_RGBA_4x4:
        case ramses_internal::ETextureFormat_ASTC_RGBA_5x4:
        case ramses_internal::ETextureFormat_ASTC_RGBA_5x5:
        case ramses_internal::ETextureFormat_ASTC_RGBA_6x5:
        case ramses_internal::ETextureFormat_ASTC_RGBA_6x6:
        case ramses_internal::ETextureFormat_ASTC_RGBA_8x5:
        case ramses_internal::ETextureFormat_ASTC_RGBA_8x6:
        case ramses_internal::ETextureFormat_ASTC_RGBA_8x8:
        case ramses_internal::ETextureFormat_ASTC_RGBA_10x5:
        case ramses_internal::ETextureFormat_ASTC_RGBA_10x6:
        case ramses_internal::ETextureFormat_ASTC_RGBA_10x8:
        case ramses_internal::ETextureFormat_ASTC_RGBA_10x10:
        case ramses_internal::ETextureFormat_ASTC_RGBA_12x10:
        case ramses_internal::ETextureFormat_ASTC_RGBA_12x12:
        case ramses_internal::ETextureFormat_ASTC_SRGBA_4x4:
        case ramses_internal::ETextureFormat_ASTC_SRGBA_5x4:
        case ramses_internal::ETextureFormat_ASTC_SRGBA_5x5:
        case ramses_internal::ETextureFormat_ASTC_SRGBA_6x5:
        case ramses_internal::ETextureFormat_ASTC_SRGBA_6x6:
        case ramses_internal::ETextureFormat_ASTC_SRGBA_8x5:
        case ramses_internal::ETextureFormat_ASTC_SRGBA_8x6:
        case ramses_internal::ETextureFormat_ASTC_SRGBA_8x8:
        case ramses_internal::ETextureFormat_ASTC_SRGBA_10x5:
        case ramses_internal::ETextureFormat_ASTC_SRGBA_10x6:
        case ramses_internal::ETextureFormat_ASTC_SRGBA_10x8:
        case ramses_internal::ETextureFormat_ASTC_SRGBA_10x10:
        case ramses_internal::ETextureFormat_ASTC_SRGBA_12x10:
        case ramses_internal::ETextureFormat_ASTC_SRGBA_12x12:
            return true;
        default:
            return false;
        }
    }

    static inline UInt32 GetTexelSizeFromFormat(ETextureFormat texelFormat)
    {
        switch (texelFormat)
        {
        case ETextureFormat_R8:
            return 1u;
        case ETextureFormat_R16:
        case ETextureFormat_R16F:
        case ETextureFormat_RG8:
        case ETextureFormat_RGB565:
        case ETextureFormat_RGBA4:
        case ETextureFormat_RGBA5551:
        case ETextureFormat_Depth16:
            return 2u;
        case ETextureFormat_BGR8:
        case ETextureFormat_RGB8:
        case ETextureFormat_Depth24:
        case ETextureFormat_SRGB8:
            return 3u;
        case ETextureFormat_RG16:
        case ETextureFormat_RG16F:
        case ETextureFormat_BGRA8:
        case ETextureFormat_RGBA8:
        case ETextureFormat_Depth24_Stencil8:
        case ETextureFormat_R32F:
        case ETextureFormat_SRGB8_ALPHA8:
            return 4u;
        case ETextureFormat_RGB16:
        case ETextureFormat_RGB16F:
            return 6u;
        case ETextureFormat_RG32F:
        case ETextureFormat_RGBA16:
        case ETextureFormat_RGBA16F:
            return 8u;
        case ETextureFormat_RGB32F:
            return 12u;
        case ETextureFormat_RGBA32F:
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
        "ETextureFormat_BGR8",
        "ETextureFormat_BGRA8",
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

    ENUM_TO_STRING(ETextureFormat, TextureFormatNames, ETextureFormat_NUMBER_OF_TYPES);
}

#endif
