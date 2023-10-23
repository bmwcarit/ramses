//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Core/Utils/LoggingUtils.h"
#include "ramses/framework/TextureEnums.h"
#include "impl/TextureEnumsImpl.h"

#include <cassert>
#include <cstdint>

namespace ramses::internal
{
    using ramses::ETextureSamplingMethod;
    using ramses::ETextureAddressMode;
    using ramses::ETextureChannelColor;
    using ramses::ETextureCubeFace;
    using ramses::ERenderBufferAccessMode;

    enum class EPixelStorageFormat : uint8_t
    {
        Invalid = 0,

        R8,
        RG8,
        RGB8,
        RGB565,

        RGBA8,
        RGBA4,
        RGBA5551,

        ETC2RGB, // ericsson texture compression 2
        ETC2RGBA, // ericsson texture compression 2 with alpha

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

        // Depth
        Depth16,
        Depth24,
        Depth32,
        Depth24_Stencil8,
    };

    static inline bool IsFormatCompressed(EPixelStorageFormat textureformat)
    {
        switch (textureformat)
        {
        case EPixelStorageFormat::ETC2RGB:
        case EPixelStorageFormat::ETC2RGBA:
        case EPixelStorageFormat::ASTC_RGBA_4x4:
        case EPixelStorageFormat::ASTC_RGBA_5x4:
        case EPixelStorageFormat::ASTC_RGBA_5x5:
        case EPixelStorageFormat::ASTC_RGBA_6x5:
        case EPixelStorageFormat::ASTC_RGBA_6x6:
        case EPixelStorageFormat::ASTC_RGBA_8x5:
        case EPixelStorageFormat::ASTC_RGBA_8x6:
        case EPixelStorageFormat::ASTC_RGBA_8x8:
        case EPixelStorageFormat::ASTC_RGBA_10x5:
        case EPixelStorageFormat::ASTC_RGBA_10x6:
        case EPixelStorageFormat::ASTC_RGBA_10x8:
        case EPixelStorageFormat::ASTC_RGBA_10x10:
        case EPixelStorageFormat::ASTC_RGBA_12x10:
        case EPixelStorageFormat::ASTC_RGBA_12x12:
        case EPixelStorageFormat::ASTC_SRGBA_4x4:
        case EPixelStorageFormat::ASTC_SRGBA_5x4:
        case EPixelStorageFormat::ASTC_SRGBA_5x5:
        case EPixelStorageFormat::ASTC_SRGBA_6x5:
        case EPixelStorageFormat::ASTC_SRGBA_6x6:
        case EPixelStorageFormat::ASTC_SRGBA_8x5:
        case EPixelStorageFormat::ASTC_SRGBA_8x6:
        case EPixelStorageFormat::ASTC_SRGBA_8x8:
        case EPixelStorageFormat::ASTC_SRGBA_10x5:
        case EPixelStorageFormat::ASTC_SRGBA_10x6:
        case EPixelStorageFormat::ASTC_SRGBA_10x8:
        case EPixelStorageFormat::ASTC_SRGBA_10x10:
        case EPixelStorageFormat::ASTC_SRGBA_12x10:
        case EPixelStorageFormat::ASTC_SRGBA_12x12:
            return true;
        default:
            return false;
        }
    }

    static inline uint32_t GetTexelSizeFromFormat(EPixelStorageFormat texelFormat)
    {
        switch (texelFormat)
        {
        case EPixelStorageFormat::R8:
            return 1u;
        case EPixelStorageFormat::R16F:
        case EPixelStorageFormat::RG8:
        case EPixelStorageFormat::RGB565:
        case EPixelStorageFormat::RGBA4:
        case EPixelStorageFormat::RGBA5551:
        case EPixelStorageFormat::Depth16:
            return 2u;
        case EPixelStorageFormat::RGB8:
        case EPixelStorageFormat::Depth24:
        case EPixelStorageFormat::SRGB8:
            return 3u;
        case EPixelStorageFormat::RG16F:
        case EPixelStorageFormat::RGBA8:
        case EPixelStorageFormat::R32F:
        case EPixelStorageFormat::SRGB8_ALPHA8:
        case EPixelStorageFormat::Depth32:
        case EPixelStorageFormat::Depth24_Stencil8:
            return 4u;
        case EPixelStorageFormat::RGB16F:
            return 6u;
        case EPixelStorageFormat::RG32F:
        case EPixelStorageFormat::RGBA16F:
            return 8u;
        case EPixelStorageFormat::RGB32F:
            return 12u;
        case EPixelStorageFormat::RGBA32F:
            return 16u;

        default:
            assert(false && "Unknown texel size");
            return 1u;
        }
    }

    static inline bool IsDepthOrStencilFormat(EPixelStorageFormat textureformat)
    {
        switch (textureformat)
        {
        case EPixelStorageFormat::Depth16:
        case EPixelStorageFormat::Depth24:
        case EPixelStorageFormat::Depth32:
        case EPixelStorageFormat::Depth24_Stencil8:
            return true;
        default:
            return false;
        }
    }

    static inline bool IsDepthOnlyFormat(EPixelStorageFormat textureformat)
    {
        switch (textureformat)
        {
        case EPixelStorageFormat::Depth16:
        case EPixelStorageFormat::Depth24:
        case EPixelStorageFormat::Depth32:
            return true;
        default:
            return false;
        }
    }

    const std::array TextureFormatNames = {
        "Invalid",
        "R8",
        "RG8",
        "RGB8",
        "RGB565",
        "RGBA8",
        "RGBA4",
        "RGBA5551",
        "ETC2RGB",
        "ETC2RGBA",
        "R16F",
        "R32F",
        "RG16F",
        "RG32F",
        "RGB16F",
        "RGB32F",
        "RGBA16F",
        "RGBA32F",
        "SRGB8",
        "SRGB8_ALPHA8",
        "ASTC_RGBA_4x4",
        "ASTC_RGBA_5x4",
        "ASTC_RGBA_5x5",
        "ASTC_RGBA_6x5",
        "ASTC_RGBA_6x6",
        "ASTC_RGBA_8x5",
        "ASTC_RGBA_8x6",
        "ASTC_RGBA_8x8",
        "ASTC_RGBA_10x5",
        "ASTC_RGBA_10x6",
        "ASTC_RGBA_10x8",
        "ASTC_RGBA_10x10",
        "ASTC_RGBA_12x10",
        "ASTC_RGBA_12x12",
        "ASTC_SRGBA_4x4",
        "ASTC_SRGBA_5x4",
        "ASTC_SRGBA_5x5",
        "ASTC_SRGBA_6x5",
        "ASTC_SRGBA_6x6",
        "ASTC_SRGBA_8x5",
        "ASTC_SRGBA_8x6",
        "ASTC_SRGBA_8x8",
        "ASTC_SRGBA_10x5",
        "ASTC_SRGBA_10x6",
        "ASTC_SRGBA_10x8",
        "ASTC_SRGBA_10x10",
        "ASTC_SRGBA_12x10",
        "ASTC_SRGBA_12x12",
        "Depth16",
        "Depth24",
        "Depth32",
        "Depth24_Stencil8",
    };

    ENUM_TO_STRING(EPixelStorageFormat, TextureFormatNames, EPixelStorageFormat::Depth24_Stencil8);
}
