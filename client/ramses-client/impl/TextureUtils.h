//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TEXTUREUTILS_H
#define RAMSES_TEXTUREUTILS_H

#include "ramses-client-api/RamsesClient.h"
#include "ramses-client-api/TextureEnums.h"
#include "ramses-client-api/MipLevelData.h"
#include "ramses-client-api/TextureSwizzle.h"
#include "SceneAPI/TextureEnums.h"
#include "Resource/TextureMetaInfo.h"
#include "PlatformAbstraction/PlatformMemory.h"
#include "Utils/LogMacros.h"

namespace ramses
{
    class Texture2D;

    class TextureUtils
    {
    public:
        static ramses_internal::ETextureFormat GetTextureFormatInternal(ETextureFormat textureformat)
        {
            switch (textureformat)
            {
            case ETextureFormat::R8:
                return ramses_internal::ETextureFormat::R8;

            case ETextureFormat::RG8:
                return ramses_internal::ETextureFormat::RG8;

            case ETextureFormat::RGB8:
                return ramses_internal::ETextureFormat::RGB8;
            case ETextureFormat::RGB565:
                return ramses_internal::ETextureFormat::RGB565;

            case ETextureFormat::RGBA8:
                return ramses_internal::ETextureFormat::RGBA8;
            case ETextureFormat::RGBA4:
                return ramses_internal::ETextureFormat::RGBA4;
            case ETextureFormat::RGBA5551:
                return ramses_internal::ETextureFormat::RGBA5551;

            case ETextureFormat::ETC2RGB:
                return ramses_internal::ETextureFormat::ETC2RGB;
            case ETextureFormat::ETC2RGBA:
                return ramses_internal::ETextureFormat::ETC2RGBA;
            case ETextureFormat::ASTC_RGBA_4x4:
                return ramses_internal::ETextureFormat::ASTC_RGBA_4x4;
            case ETextureFormat::ASTC_RGBA_5x4:
                return ramses_internal::ETextureFormat::ASTC_RGBA_5x4;
            case ETextureFormat::ASTC_RGBA_5x5:
                return ramses_internal::ETextureFormat::ASTC_RGBA_5x5;
            case ETextureFormat::ASTC_RGBA_6x5:
                return ramses_internal::ETextureFormat::ASTC_RGBA_6x5;
            case ETextureFormat::ASTC_RGBA_6x6:
                return ramses_internal::ETextureFormat::ASTC_RGBA_6x6;
            case ETextureFormat::ASTC_RGBA_8x5:
                return ramses_internal::ETextureFormat::ASTC_RGBA_8x5;
            case ETextureFormat::ASTC_RGBA_8x6:
                return ramses_internal::ETextureFormat::ASTC_RGBA_8x6;
            case ETextureFormat::ASTC_RGBA_8x8:
                return ramses_internal::ETextureFormat::ASTC_RGBA_8x8;
            case ETextureFormat::ASTC_RGBA_10x5:
                return ramses_internal::ETextureFormat::ASTC_RGBA_10x5;
            case ETextureFormat::ASTC_RGBA_10x6:
                return ramses_internal::ETextureFormat::ASTC_RGBA_10x6;
            case ETextureFormat::ASTC_RGBA_10x8:
                return ramses_internal::ETextureFormat::ASTC_RGBA_10x8;
            case ETextureFormat::ASTC_RGBA_10x10:
                return ramses_internal::ETextureFormat::ASTC_RGBA_10x10;
            case ETextureFormat::ASTC_RGBA_12x10:
                return ramses_internal::ETextureFormat::ASTC_RGBA_12x10;
            case ETextureFormat::ASTC_RGBA_12x12:
                return ramses_internal::ETextureFormat::ASTC_RGBA_12x12;
            case ETextureFormat::ASTC_SRGBA_4x4:
                return ramses_internal::ETextureFormat::ASTC_SRGBA_4x4;
            case ETextureFormat::ASTC_SRGBA_5x4:
                return ramses_internal::ETextureFormat::ASTC_SRGBA_5x4;
            case ETextureFormat::ASTC_SRGBA_5x5:
                return ramses_internal::ETextureFormat::ASTC_SRGBA_5x5;
            case ETextureFormat::ASTC_SRGBA_6x5:
                return ramses_internal::ETextureFormat::ASTC_SRGBA_6x5;
            case ETextureFormat::ASTC_SRGBA_6x6:
                return ramses_internal::ETextureFormat::ASTC_SRGBA_6x6;
            case ETextureFormat::ASTC_SRGBA_8x5:
                return ramses_internal::ETextureFormat::ASTC_SRGBA_8x5;
            case ETextureFormat::ASTC_SRGBA_8x6:
                return ramses_internal::ETextureFormat::ASTC_SRGBA_8x6;
            case ETextureFormat::ASTC_SRGBA_8x8:
                return ramses_internal::ETextureFormat::ASTC_SRGBA_8x8;
            case ETextureFormat::ASTC_SRGBA_10x5:
                return ramses_internal::ETextureFormat::ASTC_SRGBA_10x5;
            case ETextureFormat::ASTC_SRGBA_10x6:
                return ramses_internal::ETextureFormat::ASTC_SRGBA_10x6;
            case ETextureFormat::ASTC_SRGBA_10x8:
                return ramses_internal::ETextureFormat::ASTC_SRGBA_10x8;
            case ETextureFormat::ASTC_SRGBA_10x10:
                return ramses_internal::ETextureFormat::ASTC_SRGBA_10x10;
            case ETextureFormat::ASTC_SRGBA_12x10:
                return ramses_internal::ETextureFormat::ASTC_SRGBA_12x10;
            case ETextureFormat::ASTC_SRGBA_12x12:
                return ramses_internal::ETextureFormat::ASTC_SRGBA_12x12;
            case ETextureFormat::R16F:
                return ramses_internal::ETextureFormat::R16F;
            case ETextureFormat::R32F:
                return ramses_internal::ETextureFormat::R32F;
            case ETextureFormat::RG16F:
                return ramses_internal::ETextureFormat::RG16F;
            case ETextureFormat::RG32F:
                return ramses_internal::ETextureFormat::RG32F;
            case ETextureFormat::RGB16F:
                return ramses_internal::ETextureFormat::RGB16F;
            case ETextureFormat::RGB32F:
                return ramses_internal::ETextureFormat::RGB32F;
            case ETextureFormat::RGBA16F:
                return ramses_internal::ETextureFormat::RGBA16F;
            case ETextureFormat::RGBA32F:
                return ramses_internal::ETextureFormat::RGBA32F;

            case ETextureFormat::SRGB8:
                return ramses_internal::ETextureFormat::SRGB8;
            case ETextureFormat::SRGB8_ALPHA8:
                return ramses_internal::ETextureFormat::SRGB8_ALPHA8;
            case ETextureFormat::Invalid:
                break;
            }

            assert(false);
            return ramses_internal::ETextureFormat::RGBA8;
        }

        static ETextureFormat GetTextureFormatFromInternal(ramses_internal::ETextureFormat textureformat)
        {
            switch (textureformat)
            {
            case ramses_internal::ETextureFormat::R8:
                return ETextureFormat::R8;

            case ramses_internal::ETextureFormat::RG8:
                return ETextureFormat::RG8;

            case ramses_internal::ETextureFormat::RGB8:
                return ETextureFormat::RGB8;
            case ramses_internal::ETextureFormat::RGB565:
                return ETextureFormat::RGB565;

            case ramses_internal::ETextureFormat::RGBA8:
                return ETextureFormat::RGBA8;
            case ramses_internal::ETextureFormat::RGBA4:
                return ETextureFormat::RGBA4;
            case ramses_internal::ETextureFormat::RGBA5551:
                return ETextureFormat::RGBA5551;

            case ramses_internal::ETextureFormat::ETC2RGB:
                return ETextureFormat::ETC2RGB;
            case ramses_internal::ETextureFormat::ETC2RGBA:
                return ETextureFormat::ETC2RGBA;

            case ramses_internal::ETextureFormat::ASTC_RGBA_4x4:
                return ETextureFormat::ASTC_RGBA_4x4;
            case ramses_internal::ETextureFormat::ASTC_RGBA_5x4:
                return ETextureFormat::ASTC_RGBA_5x4;
            case ramses_internal::ETextureFormat::ASTC_RGBA_5x5:
                return ETextureFormat::ASTC_RGBA_5x5;
            case ramses_internal::ETextureFormat::ASTC_RGBA_6x5:
                return ETextureFormat::ASTC_RGBA_6x5;
            case ramses_internal::ETextureFormat::ASTC_RGBA_6x6:
                return ETextureFormat::ASTC_RGBA_6x6;
            case ramses_internal::ETextureFormat::ASTC_RGBA_8x5:
                return ETextureFormat::ASTC_RGBA_8x5;
            case ramses_internal::ETextureFormat::ASTC_RGBA_8x6:
                return ETextureFormat::ASTC_RGBA_8x6;
            case ramses_internal::ETextureFormat::ASTC_RGBA_8x8:
                return ETextureFormat::ASTC_RGBA_8x8;
            case ramses_internal::ETextureFormat::ASTC_RGBA_10x5:
                return ETextureFormat::ASTC_RGBA_10x5;
            case ramses_internal::ETextureFormat::ASTC_RGBA_10x6:
                return ETextureFormat::ASTC_RGBA_10x6;
            case ramses_internal::ETextureFormat::ASTC_RGBA_10x8:
                return ETextureFormat::ASTC_RGBA_10x8;
            case ramses_internal::ETextureFormat::ASTC_RGBA_10x10:
                return ETextureFormat::ASTC_RGBA_10x10;
            case ramses_internal::ETextureFormat::ASTC_RGBA_12x10:
                return ETextureFormat::ASTC_RGBA_12x10;
            case ramses_internal::ETextureFormat::ASTC_RGBA_12x12:
                return ETextureFormat::ASTC_RGBA_12x12;
            case ramses_internal::ETextureFormat::ASTC_SRGBA_4x4:
                return ETextureFormat::ASTC_SRGBA_4x4;
            case ramses_internal::ETextureFormat::ASTC_SRGBA_5x4:
                return ETextureFormat::ASTC_SRGBA_5x4;
            case ramses_internal::ETextureFormat::ASTC_SRGBA_5x5:
                return ETextureFormat::ASTC_SRGBA_5x5;
            case ramses_internal::ETextureFormat::ASTC_SRGBA_6x5:
                return ETextureFormat::ASTC_SRGBA_6x5;
            case ramses_internal::ETextureFormat::ASTC_SRGBA_6x6:
                return ETextureFormat::ASTC_SRGBA_6x6;
            case ramses_internal::ETextureFormat::ASTC_SRGBA_8x5:
                return ETextureFormat::ASTC_SRGBA_8x5;
            case ramses_internal::ETextureFormat::ASTC_SRGBA_8x6:
                return ETextureFormat::ASTC_SRGBA_8x6;
            case ramses_internal::ETextureFormat::ASTC_SRGBA_8x8:
                return ETextureFormat::ASTC_SRGBA_8x8;
            case ramses_internal::ETextureFormat::ASTC_SRGBA_10x5:
                return ETextureFormat::ASTC_SRGBA_10x5;
            case ramses_internal::ETextureFormat::ASTC_SRGBA_10x6:
                return ETextureFormat::ASTC_SRGBA_10x6;
            case ramses_internal::ETextureFormat::ASTC_SRGBA_10x8:
                return ETextureFormat::ASTC_SRGBA_10x8;
            case ramses_internal::ETextureFormat::ASTC_SRGBA_10x10:
                return ETextureFormat::ASTC_SRGBA_10x10;
            case ramses_internal::ETextureFormat::ASTC_SRGBA_12x10:
                return ETextureFormat::ASTC_SRGBA_12x10;
            case ramses_internal::ETextureFormat::ASTC_SRGBA_12x12:
                return ETextureFormat::ASTC_SRGBA_12x12;

            case ramses_internal::ETextureFormat::R16F:
                return ETextureFormat::R16F;
            case ramses_internal::ETextureFormat::R32F:
                return ETextureFormat::R32F;
            case ramses_internal::ETextureFormat::RG16F:
                return ETextureFormat::RG16F;
            case ramses_internal::ETextureFormat::RG32F:
                return ETextureFormat::RG32F;
            case ramses_internal::ETextureFormat::RGB16F:
                return ETextureFormat::RGB16F;
            case ramses_internal::ETextureFormat::RGB32F:
                return ETextureFormat::RGB32F;
            case ramses_internal::ETextureFormat::RGBA16F:
                return ETextureFormat::RGBA16F;
            case ramses_internal::ETextureFormat::RGBA32F:
                return ETextureFormat::RGBA32F;

            case ramses_internal::ETextureFormat::SRGB8:
                return ETextureFormat::SRGB8;
            case ramses_internal::ETextureFormat::SRGB8_ALPHA8:
                return ETextureFormat::SRGB8_ALPHA8;
            case ramses_internal::ETextureFormat::Invalid:
            case ramses_internal::ETextureFormat::R16:
            case ramses_internal::ETextureFormat::RG16:
            case ramses_internal::ETextureFormat::RGB16:
            case ramses_internal::ETextureFormat::RGBA16:
            case ramses_internal::ETextureFormat::DXT1RGB:
            case ramses_internal::ETextureFormat::DXT3RGBA:
            case ramses_internal::ETextureFormat::DXT5RGBA:
            case ramses_internal::ETextureFormat::Depth16:
            case ramses_internal::ETextureFormat::Depth24:
            case ramses_internal::ETextureFormat::Depth24_Stencil8:
            case ramses_internal::ETextureFormat::NUMBER_OF_TYPES:
                break;
            }
            assert(false);
            return ETextureFormat::RGBA8;
        }

        static ETextureChannelColor GetTextureChannelColorFromInternal(ramses_internal::ETextureChannelColor textureChannelColor)
        {
            switch (textureChannelColor)
            {
            case ramses_internal::ETextureChannelColor::Red:
                return ETextureChannelColor::Red;
            case ramses_internal::ETextureChannelColor::Green:
                return ETextureChannelColor::Green;
            case ramses_internal::ETextureChannelColor::Blue:
                return ETextureChannelColor::Blue;
            case ramses_internal::ETextureChannelColor::Alpha:
                return ETextureChannelColor::Alpha;
            case ramses_internal::ETextureChannelColor::One:
                return ETextureChannelColor::One;
            case ramses_internal::ETextureChannelColor::Zero:
                return ETextureChannelColor::Zero;
            case ramses_internal::ETextureChannelColor::NUMBER_OF_ELEMENTS:
                break;
            }
            assert(false);
            return ETextureChannelColor::Red;
        }

        static ramses_internal::ETextureChannelColor GetTextureChannelColorInternal(ETextureChannelColor textureChannelColor)
        {
            switch (textureChannelColor)
            {
            case ETextureChannelColor::Red:
                return ramses_internal::ETextureChannelColor::Red;
            case ETextureChannelColor::Green:
                return ramses_internal::ETextureChannelColor::Green;
            case ETextureChannelColor::Blue:
                return ramses_internal::ETextureChannelColor::Blue;
            case ETextureChannelColor::Alpha:
                return ramses_internal::ETextureChannelColor::Alpha;
            case ETextureChannelColor::One:
                return ramses_internal::ETextureChannelColor::One;
            case ETextureChannelColor::Zero:
                return ramses_internal::ETextureChannelColor::Zero;
            }
            assert(false);
            return ramses_internal::ETextureChannelColor::Red;
        }

        static ramses_internal::TextureSwizzleArray GetTextureSwizzleInternal(const TextureSwizzle& swizzle)
        {
            return ramses_internal::TextureSwizzleArray{
                GetTextureChannelColorInternal(swizzle.channelRed),
                GetTextureChannelColorInternal(swizzle.channelGreen),
                GetTextureChannelColorInternal(swizzle.channelBlue),
                GetTextureChannelColorInternal(swizzle.channelAlpha)
            };
        }

        static TextureSwizzle GetTextureSwizzleFromInternal(const ramses_internal::TextureSwizzleArray& swizzle)
        {
            return TextureSwizzle{
                GetTextureChannelColorFromInternal(swizzle[0]),
                GetTextureChannelColorFromInternal(swizzle[1]),
                GetTextureChannelColorFromInternal(swizzle[2]),
                GetTextureChannelColorFromInternal(swizzle[3])
            };
        }

        static bool IsTextureSizeSupportedByFormat(uint32_t width, uint32_t height, ETextureFormat textureformat)
        {
            switch (textureformat)
            {
            case ETextureFormat::R8:
            case ETextureFormat::RG8:
            case ETextureFormat::RGB8:
            case ETextureFormat::RGB565:
            case ETextureFormat::RGBA8:
            case ETextureFormat::RGBA4:
            case ETextureFormat::RGBA5551:
            case ETextureFormat::R16F:
            case ETextureFormat::R32F:
            case ETextureFormat::RG16F:
            case ETextureFormat::RG32F:
            case ETextureFormat::RGB16F:
            case ETextureFormat::RGB32F:
            case ETextureFormat::RGBA16F:
            case ETextureFormat::RGBA32F:
            case ETextureFormat::SRGB8:
            case ETextureFormat::SRGB8_ALPHA8:
                // no special requirements
                return true;

            case ETextureFormat::ETC2RGB:
            case ETextureFormat::ETC2RGBA:
            case ETextureFormat::ASTC_RGBA_4x4:
            case ETextureFormat::ASTC_SRGBA_4x4:
                return (width % 4 == 0) && (height % 4 == 0);

            case ETextureFormat::ASTC_RGBA_5x4:
            case ETextureFormat::ASTC_SRGBA_5x4:
                return (width % 5 == 0) && (height % 4 == 0);

            case ETextureFormat::ASTC_RGBA_5x5:
            case ETextureFormat::ASTC_SRGBA_5x5:
                return (width % 5 == 0) && (height % 5 == 0);

            case ETextureFormat::ASTC_RGBA_6x5:
            case ETextureFormat::ASTC_SRGBA_6x5:
                return (width % 6 == 0) && (height % 5 == 0);

            case ETextureFormat::ASTC_RGBA_6x6:
            case ETextureFormat::ASTC_SRGBA_6x6:
                return (width % 6 == 0) && (height % 6 == 0);

            case ETextureFormat::ASTC_RGBA_8x5:
            case ETextureFormat::ASTC_SRGBA_8x5:
                return (width % 8 == 0) && (height % 5 == 0);

            case ETextureFormat::ASTC_RGBA_8x6:
            case ETextureFormat::ASTC_SRGBA_8x6:
                return (width % 8 == 0) && (height % 6 == 0);

            case ETextureFormat::ASTC_RGBA_8x8:
            case ETextureFormat::ASTC_SRGBA_8x8:
                return (width % 8 == 0) && (height % 8 == 0);

            case ETextureFormat::ASTC_RGBA_10x5:
            case ETextureFormat::ASTC_SRGBA_10x5:
                return (width % 10 == 0) && (height % 5 == 0);

            case ETextureFormat::ASTC_RGBA_10x6:
            case ETextureFormat::ASTC_SRGBA_10x6:
                return (width % 10 == 0) && (height % 6 == 0);

            case ETextureFormat::ASTC_RGBA_10x8:
            case ETextureFormat::ASTC_SRGBA_10x8:
                return (width % 10 == 0) && (height % 8 == 0);

            case ETextureFormat::ASTC_RGBA_10x10:
            case ETextureFormat::ASTC_SRGBA_10x10:
                return (width % 10 == 0) && (height % 10 == 0);

            case ETextureFormat::ASTC_RGBA_12x10:
            case ETextureFormat::ASTC_SRGBA_12x10:
                return (width % 12 == 0) && (height % 10 == 0);

            case ETextureFormat::ASTC_RGBA_12x12:
            case ETextureFormat::ASTC_SRGBA_12x12:
                return (width % 12 == 0) && (height % 12 == 0);

            case ETextureFormat::Invalid:
                assert(false);
                return false;
            }

            assert(false);
            return false;
        }

        static ramses_internal::EWrapMethod GetTextureAddressModeInternal(ETextureAddressMode addressMode)
        {
            switch (addressMode)
            {
            case ETextureAddressMode::Clamp:
                return ramses_internal::EWrapMethod::Clamp;
            case ETextureAddressMode::Repeat:
                return ramses_internal::EWrapMethod::Repeat;
            case ETextureAddressMode::Mirror:
                return ramses_internal::EWrapMethod::RepeatMirrored;
            }
            assert(false);
            return ramses_internal::EWrapMethod::Clamp;
        }

        static ETextureAddressMode GetTextureAddressModeFromInternal(ramses_internal::EWrapMethod addressMode)
        {
            switch (addressMode)
            {
            case ramses_internal::EWrapMethod::Clamp:
                return ETextureAddressMode::Clamp;
            case ramses_internal::EWrapMethod::Repeat:
                return ETextureAddressMode::Repeat;
            case ramses_internal::EWrapMethod::RepeatMirrored:
                return ETextureAddressMode::Mirror;
            case ramses_internal::EWrapMethod::NUMBER_OF_ELEMENTS:
                break;
            }
            assert(false);
            return ETextureAddressMode::Clamp;
        }

        static ramses_internal::ETextureCubeFace GetTextureCubeFaceInternal(ETextureCubeFace face)
        {
            switch (face)
            {
            case ETextureCubeFace::PositiveX:
                return ramses_internal::ETextureCubeFace_PositiveX;
            case ETextureCubeFace::NegativeX:
                return ramses_internal::ETextureCubeFace_NegativeX;
            case ETextureCubeFace::PositiveY:
                return ramses_internal::ETextureCubeFace_PositiveY;
            case ETextureCubeFace::NegativeY:
                return ramses_internal::ETextureCubeFace_NegativeY;
            case ETextureCubeFace::PositiveZ:
                return ramses_internal::ETextureCubeFace_PositiveZ;
            case ETextureCubeFace::NegativeZ:
                return ramses_internal::ETextureCubeFace_NegativeZ;
            }
            assert(false);
            return ramses_internal::ETextureCubeFace_PositiveX;
        }

        static ramses_internal::ESamplingMethod GetTextureSamplingInternal(ETextureSamplingMethod sampling)
        {
            switch (sampling)
            {
            case ETextureSamplingMethod::Nearest:
                return ramses_internal::ESamplingMethod::Nearest;
            case ETextureSamplingMethod::Nearest_MipMapNearest:
                return ramses_internal::ESamplingMethod::Nearest_MipMapNearest;
            case ETextureSamplingMethod::Nearest_MipMapLinear:
                return ramses_internal::ESamplingMethod::Nearest_MipMapLinear;
            case ETextureSamplingMethod::Linear:
                return ramses_internal::ESamplingMethod::Linear;
            case ETextureSamplingMethod::Linear_MipMapNearest:
                return ramses_internal::ESamplingMethod::Linear_MipMapNearest;
            case ETextureSamplingMethod::Linear_MipMapLinear:
                return ramses_internal::ESamplingMethod::Linear_MipMapLinear;
            }
            assert(false);
            return ramses_internal::ESamplingMethod::Linear;
        }

        static ETextureSamplingMethod GetTextureSamplingFromInternal(ramses_internal::ESamplingMethod sampling)
        {
            switch (sampling)
            {
            case ramses_internal::ESamplingMethod::Nearest:
                return ETextureSamplingMethod::Nearest;
            case ramses_internal::ESamplingMethod::Nearest_MipMapNearest:
                return ETextureSamplingMethod::Nearest_MipMapNearest;
            case ramses_internal::ESamplingMethod::Nearest_MipMapLinear:
                return ETextureSamplingMethod::Nearest_MipMapLinear;
            case ramses_internal::ESamplingMethod::Linear:
                return ETextureSamplingMethod::Linear;
            case ramses_internal::ESamplingMethod::Linear_MipMapNearest:
                return ETextureSamplingMethod::Linear_MipMapNearest;
            case ramses_internal::ESamplingMethod::Linear_MipMapLinear:
                return ETextureSamplingMethod::Linear_MipMapLinear;
            case ramses_internal::ESamplingMethod::NUMBER_OF_ELEMENTS:
                break;
            }
            assert(false);
            return ETextureSamplingMethod::Linear;
        }

        static ramses_internal::ERenderBufferType GetRenderBufferTypeInternal(ERenderBufferType bufferType)
        {
            switch (bufferType)
            {
            case ERenderBufferType::Color:
                return ramses_internal::ERenderBufferType_ColorBuffer;
            case ERenderBufferType::Depth:
                return ramses_internal::ERenderBufferType_DepthBuffer;
            case ERenderBufferType::DepthStencil:
                return ramses_internal::ERenderBufferType_DepthStencilBuffer;
            }
            assert(false);
            return ramses_internal::ERenderBufferType_InvalidBuffer;
        }

        static ERenderBufferType GetRenderBufferTypeFromInternal(ramses_internal::ERenderBufferType bufferType)
        {
            switch (bufferType)
            {
            case ramses_internal::ERenderBufferType_ColorBuffer:
                return ERenderBufferType::Color;
            case ramses_internal::ERenderBufferType_DepthBuffer:
                return ERenderBufferType::Depth;
            case ramses_internal::ERenderBufferType_DepthStencilBuffer:
                return ERenderBufferType::DepthStencil;
            case ramses_internal::ERenderBufferType_InvalidBuffer:
            case ramses_internal::ERenderBufferType_NUMBER_OF_ELEMENTS:
                break;
            }
            assert(false);
            return ERenderBufferType::Color;
        }

        static ramses_internal::ETextureFormat GetRenderBufferFormatInternal(ERenderBufferFormat bufferFormat)
        {
            switch (bufferFormat)
            {
            case ramses::ERenderBufferFormat::RGBA4:
                return ramses_internal::ETextureFormat::RGBA4;
            case ramses::ERenderBufferFormat::R8:
                return ramses_internal::ETextureFormat::R8;
            case ramses::ERenderBufferFormat::RG8:
                return ramses_internal::ETextureFormat::RG8;
            case ramses::ERenderBufferFormat::RGB8:
                return ramses_internal::ETextureFormat::RGB8;
            case ramses::ERenderBufferFormat::RGBA8:
                return ramses_internal::ETextureFormat::RGBA8;
            case ramses::ERenderBufferFormat::R16F:
                return ramses_internal::ETextureFormat::R16F;
            case ramses::ERenderBufferFormat::R32F:
                return ramses_internal::ETextureFormat::R32F;
            case ramses::ERenderBufferFormat::RG16F:
                return ramses_internal::ETextureFormat::RG16F;
            case ramses::ERenderBufferFormat::RG32F:
                return ramses_internal::ETextureFormat::RG32F;
            case ramses::ERenderBufferFormat::RGB16F:
                return ramses_internal::ETextureFormat::RGB16F;
            case ramses::ERenderBufferFormat::RGB32F:
                return ramses_internal::ETextureFormat::RGB32F;
            case ramses::ERenderBufferFormat::RGBA16F:
                return ramses_internal::ETextureFormat::RGBA16F;
            case ramses::ERenderBufferFormat::RGBA32F:
                return ramses_internal::ETextureFormat::RGBA32F;
            case ramses::ERenderBufferFormat::Depth24:
                return ramses_internal::ETextureFormat::Depth24;
            case ramses::ERenderBufferFormat::Depth24_Stencil8:
                return ramses_internal::ETextureFormat::Depth24_Stencil8;
            }
            assert(false);
            return ramses_internal::ETextureFormat::Invalid;
        }

        static ramses_internal::ERenderBufferAccessMode GetRenderBufferAccessModeInternal(ERenderBufferAccessMode accessMode)
        {
            switch (accessMode)
            {
            case ramses::ERenderBufferAccessMode::WriteOnly:
                return ramses_internal::ERenderBufferAccessMode_WriteOnly;
            case ramses::ERenderBufferAccessMode::ReadWrite:
                return ramses_internal::ERenderBufferAccessMode_ReadWrite;
            }
            assert(false);
            return ramses_internal::ERenderBufferAccessMode_Invalid;
        }

        static ERenderBufferFormat GetRenderBufferFormatFromInternal(ramses_internal::ETextureFormat bufferFormat)
        {
            switch (bufferFormat)
            {
            case ramses_internal::ETextureFormat::RGBA4:
                return ERenderBufferFormat::RGBA4;
            case ramses_internal::ETextureFormat::R8:
                return ERenderBufferFormat::R8;
            case ramses_internal::ETextureFormat::RG8:
                return ERenderBufferFormat::RG8;
            case ramses_internal::ETextureFormat::RGB8:
                return ERenderBufferFormat::RGB8;
            case ramses_internal::ETextureFormat::RGBA8:
                return ERenderBufferFormat::RGBA8;
            case ramses_internal::ETextureFormat::R16F:
                return ERenderBufferFormat::R16F;
            case ramses_internal::ETextureFormat::R32F:
                return ERenderBufferFormat::R32F;
            case ramses_internal::ETextureFormat::RG16F:
                return ERenderBufferFormat::RG16F;
            case ramses_internal::ETextureFormat::RG32F:
                return ERenderBufferFormat::RG32F;
            case ramses_internal::ETextureFormat::RGB16F:
                return ERenderBufferFormat::RGB16F;
            case ramses_internal::ETextureFormat::RGB32F:
                return ERenderBufferFormat::RGB32F;
            case ramses_internal::ETextureFormat::RGBA16F:
                return ERenderBufferFormat::RGBA16F;
            case ramses_internal::ETextureFormat::RGBA32F:
                return ERenderBufferFormat::RGBA32F;

            case ramses_internal::ETextureFormat::Depth24:
                return ERenderBufferFormat::Depth24;
            case ramses_internal::ETextureFormat::Depth24_Stencil8:
                return ERenderBufferFormat::Depth24_Stencil8;
            case ramses_internal::ETextureFormat::Invalid:
            case ramses_internal::ETextureFormat::RGB565:
            case ramses_internal::ETextureFormat::RGBA5551:
            case ramses_internal::ETextureFormat::ETC2RGB:
            case ramses_internal::ETextureFormat::ETC2RGBA:
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
            case ramses_internal::ETextureFormat::SRGB8:
            case ramses_internal::ETextureFormat::SRGB8_ALPHA8:
            case ramses_internal::ETextureFormat::R16:
            case ramses_internal::ETextureFormat::RG16:
            case ramses_internal::ETextureFormat::RGB16:
            case ramses_internal::ETextureFormat::RGBA16:
            case ramses_internal::ETextureFormat::DXT1RGB:
            case ramses_internal::ETextureFormat::DXT3RGBA:
            case ramses_internal::ETextureFormat::DXT5RGBA:
            case ramses_internal::ETextureFormat::Depth16:
            case ramses_internal::ETextureFormat::NUMBER_OF_TYPES:
                break;
            }
            assert(false);
            return ERenderBufferFormat::RGBA8;
        }

        static ERenderBufferAccessMode GetRenderBufferAccessModeFromInternal(ramses_internal::ERenderBufferAccessMode accessMode)
        {
            switch (accessMode)
            {
            case ramses_internal::ERenderBufferAccessMode_WriteOnly:
                return ERenderBufferAccessMode::WriteOnly;
            case ramses_internal::ERenderBufferAccessMode_ReadWrite:
                return ERenderBufferAccessMode::ReadWrite;
            case ramses_internal::ERenderBufferAccessMode_Invalid:
            case ramses_internal::ERenderBufferAccessMode_NUMBER_OF_ELEMENTS:
                break;
            }
            assert(false);
            return ERenderBufferAccessMode::ReadWrite;
        }

        static bool IsRenderBufferTypeCompatibleWithFormat(ERenderBufferType bufferType, ERenderBufferFormat bufferFormat)
        {
            switch (bufferType)
            {
            case ERenderBufferType::Color:
                return
                    bufferFormat == ERenderBufferFormat::RGBA4 ||
                    bufferFormat == ERenderBufferFormat::R8 ||
                    bufferFormat == ERenderBufferFormat::RG8 ||
                    bufferFormat == ERenderBufferFormat::RGB8 ||
                    bufferFormat == ERenderBufferFormat::RGBA8 ||
                    bufferFormat == ERenderBufferFormat::R16F ||
                    bufferFormat == ERenderBufferFormat::R32F ||
                    bufferFormat == ERenderBufferFormat::RG16F ||
                    bufferFormat == ERenderBufferFormat::RG32F ||
                    bufferFormat == ERenderBufferFormat::RGB16F ||
                    bufferFormat == ERenderBufferFormat::RGB32F ||
                    bufferFormat == ERenderBufferFormat::RGBA16F ||
                    bufferFormat == ERenderBufferFormat::RGBA32F;
            case ERenderBufferType::Depth:
                return bufferFormat == ERenderBufferFormat::Depth24;
            case ERenderBufferType::DepthStencil:
                return bufferFormat == ERenderBufferFormat::Depth24_Stencil8;
            }
            assert(false);
            return false;
        }

        // NOLINTNEXTLINE(modernize-avoid-c-arrays)
        static void FillMipDataSizes(ramses_internal::MipDataSizeVector& mipDataSizes, uint32_t mipMapCount, const MipLevelData mipLevelData[]);
        // NOLINTNEXTLINE(modernize-avoid-c-arrays)
        static void FillMipDataSizes(ramses_internal::MipDataSizeVector& mipDataSizes, uint32_t mipMapCount, const CubeMipLevelData mipLevelData[]);
        // NOLINTNEXTLINE(modernize-avoid-c-arrays)
        static void FillMipData(uint8_t* dest, uint32_t mipMapCount, const MipLevelData mipLevelData[]);
        // NOLINTNEXTLINE(modernize-avoid-c-arrays)
        static void FillMipData(uint8_t* dest, uint32_t mipMapCount, const CubeMipLevelData mipLevelData[]);
        // NOLINTNEXTLINE(modernize-avoid-c-arrays)
        static bool MipDataValid(uint32_t width, uint32_t height, uint32_t depth, uint32_t mipMapCount, const MipLevelData mipLevelData[], ETextureFormat format);
        // NOLINTNEXTLINE(modernize-avoid-c-arrays)
        static bool MipDataValid(uint32_t width, uint32_t height, uint32_t depth, uint32_t mipMapCount, const CubeMipLevelData mipLevelData[], ETextureFormat format);
        // NOLINTNEXTLINE(modernize-avoid-c-arrays)
        static bool MipDataValid(uint32_t size, uint32_t mipMapCount, const CubeMipLevelData mipLevelData[], ETextureFormat format);
        static bool TextureParametersValid(uint32_t width, uint32_t height, uint32_t depth, uint32_t mipMapCount);
    };
}

#endif
