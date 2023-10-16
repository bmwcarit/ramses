//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/client/RamsesClient.h"
#include "ramses/framework/TextureEnums.h"
#include "ramses/client/MipLevelData.h"
#include "ramses/client/TextureSwizzle.h"
#include "internal/SceneGraph/SceneAPI/TextureEnums.h"
#include "internal/SceneGraph/Resource/TextureMetaInfo.h"
#include "internal/PlatformAbstraction/PlatformMemory.h"
#include "internal/Core/Utils/LogMacros.h"

namespace ramses
{
    class Texture2D;
}

namespace ramses::internal
{
    class TextureUtils
    {
    public:
        static ramses::internal::EPixelStorageFormat GetTextureFormatInternal(ETextureFormat textureformat)
        {
            switch (textureformat)
            {
            case ETextureFormat::R8:
                return ramses::internal::EPixelStorageFormat::R8;

            case ETextureFormat::RG8:
                return ramses::internal::EPixelStorageFormat::RG8;

            case ETextureFormat::RGB8:
                return ramses::internal::EPixelStorageFormat::RGB8;
            case ETextureFormat::RGB565:
                return ramses::internal::EPixelStorageFormat::RGB565;

            case ETextureFormat::RGBA8:
                return ramses::internal::EPixelStorageFormat::RGBA8;
            case ETextureFormat::RGBA4:
                return ramses::internal::EPixelStorageFormat::RGBA4;
            case ETextureFormat::RGBA5551:
                return ramses::internal::EPixelStorageFormat::RGBA5551;

            case ETextureFormat::ETC2RGB:
                return ramses::internal::EPixelStorageFormat::ETC2RGB;
            case ETextureFormat::ETC2RGBA:
                return ramses::internal::EPixelStorageFormat::ETC2RGBA;
            case ETextureFormat::ASTC_RGBA_4x4:
                return ramses::internal::EPixelStorageFormat::ASTC_RGBA_4x4;
            case ETextureFormat::ASTC_RGBA_5x4:
                return ramses::internal::EPixelStorageFormat::ASTC_RGBA_5x4;
            case ETextureFormat::ASTC_RGBA_5x5:
                return ramses::internal::EPixelStorageFormat::ASTC_RGBA_5x5;
            case ETextureFormat::ASTC_RGBA_6x5:
                return ramses::internal::EPixelStorageFormat::ASTC_RGBA_6x5;
            case ETextureFormat::ASTC_RGBA_6x6:
                return ramses::internal::EPixelStorageFormat::ASTC_RGBA_6x6;
            case ETextureFormat::ASTC_RGBA_8x5:
                return ramses::internal::EPixelStorageFormat::ASTC_RGBA_8x5;
            case ETextureFormat::ASTC_RGBA_8x6:
                return ramses::internal::EPixelStorageFormat::ASTC_RGBA_8x6;
            case ETextureFormat::ASTC_RGBA_8x8:
                return ramses::internal::EPixelStorageFormat::ASTC_RGBA_8x8;
            case ETextureFormat::ASTC_RGBA_10x5:
                return ramses::internal::EPixelStorageFormat::ASTC_RGBA_10x5;
            case ETextureFormat::ASTC_RGBA_10x6:
                return ramses::internal::EPixelStorageFormat::ASTC_RGBA_10x6;
            case ETextureFormat::ASTC_RGBA_10x8:
                return ramses::internal::EPixelStorageFormat::ASTC_RGBA_10x8;
            case ETextureFormat::ASTC_RGBA_10x10:
                return ramses::internal::EPixelStorageFormat::ASTC_RGBA_10x10;
            case ETextureFormat::ASTC_RGBA_12x10:
                return ramses::internal::EPixelStorageFormat::ASTC_RGBA_12x10;
            case ETextureFormat::ASTC_RGBA_12x12:
                return ramses::internal::EPixelStorageFormat::ASTC_RGBA_12x12;
            case ETextureFormat::ASTC_SRGBA_4x4:
                return ramses::internal::EPixelStorageFormat::ASTC_SRGBA_4x4;
            case ETextureFormat::ASTC_SRGBA_5x4:
                return ramses::internal::EPixelStorageFormat::ASTC_SRGBA_5x4;
            case ETextureFormat::ASTC_SRGBA_5x5:
                return ramses::internal::EPixelStorageFormat::ASTC_SRGBA_5x5;
            case ETextureFormat::ASTC_SRGBA_6x5:
                return ramses::internal::EPixelStorageFormat::ASTC_SRGBA_6x5;
            case ETextureFormat::ASTC_SRGBA_6x6:
                return ramses::internal::EPixelStorageFormat::ASTC_SRGBA_6x6;
            case ETextureFormat::ASTC_SRGBA_8x5:
                return ramses::internal::EPixelStorageFormat::ASTC_SRGBA_8x5;
            case ETextureFormat::ASTC_SRGBA_8x6:
                return ramses::internal::EPixelStorageFormat::ASTC_SRGBA_8x6;
            case ETextureFormat::ASTC_SRGBA_8x8:
                return ramses::internal::EPixelStorageFormat::ASTC_SRGBA_8x8;
            case ETextureFormat::ASTC_SRGBA_10x5:
                return ramses::internal::EPixelStorageFormat::ASTC_SRGBA_10x5;
            case ETextureFormat::ASTC_SRGBA_10x6:
                return ramses::internal::EPixelStorageFormat::ASTC_SRGBA_10x6;
            case ETextureFormat::ASTC_SRGBA_10x8:
                return ramses::internal::EPixelStorageFormat::ASTC_SRGBA_10x8;
            case ETextureFormat::ASTC_SRGBA_10x10:
                return ramses::internal::EPixelStorageFormat::ASTC_SRGBA_10x10;
            case ETextureFormat::ASTC_SRGBA_12x10:
                return ramses::internal::EPixelStorageFormat::ASTC_SRGBA_12x10;
            case ETextureFormat::ASTC_SRGBA_12x12:
                return ramses::internal::EPixelStorageFormat::ASTC_SRGBA_12x12;
            case ETextureFormat::R16F:
                return ramses::internal::EPixelStorageFormat::R16F;
            case ETextureFormat::R32F:
                return ramses::internal::EPixelStorageFormat::R32F;
            case ETextureFormat::RG16F:
                return ramses::internal::EPixelStorageFormat::RG16F;
            case ETextureFormat::RG32F:
                return ramses::internal::EPixelStorageFormat::RG32F;
            case ETextureFormat::RGB16F:
                return ramses::internal::EPixelStorageFormat::RGB16F;
            case ETextureFormat::RGB32F:
                return ramses::internal::EPixelStorageFormat::RGB32F;
            case ETextureFormat::RGBA16F:
                return ramses::internal::EPixelStorageFormat::RGBA16F;
            case ETextureFormat::RGBA32F:
                return ramses::internal::EPixelStorageFormat::RGBA32F;

            case ETextureFormat::SRGB8:
                return ramses::internal::EPixelStorageFormat::SRGB8;
            case ETextureFormat::SRGB8_ALPHA8:
                return ramses::internal::EPixelStorageFormat::SRGB8_ALPHA8;
            }

            assert(false);
            return ramses::internal::EPixelStorageFormat::RGBA8;
        }

        static ETextureFormat GetTextureFormatFromInternal(ramses::internal::EPixelStorageFormat textureformat)
        {
            switch (textureformat)
            {
            case ramses::internal::EPixelStorageFormat::R8:
                return ETextureFormat::R8;

            case ramses::internal::EPixelStorageFormat::RG8:
                return ETextureFormat::RG8;

            case ramses::internal::EPixelStorageFormat::RGB8:
                return ETextureFormat::RGB8;
            case ramses::internal::EPixelStorageFormat::RGB565:
                return ETextureFormat::RGB565;

            case ramses::internal::EPixelStorageFormat::RGBA8:
                return ETextureFormat::RGBA8;
            case ramses::internal::EPixelStorageFormat::RGBA4:
                return ETextureFormat::RGBA4;
            case ramses::internal::EPixelStorageFormat::RGBA5551:
                return ETextureFormat::RGBA5551;

            case ramses::internal::EPixelStorageFormat::ETC2RGB:
                return ETextureFormat::ETC2RGB;
            case ramses::internal::EPixelStorageFormat::ETC2RGBA:
                return ETextureFormat::ETC2RGBA;

            case ramses::internal::EPixelStorageFormat::ASTC_RGBA_4x4:
                return ETextureFormat::ASTC_RGBA_4x4;
            case ramses::internal::EPixelStorageFormat::ASTC_RGBA_5x4:
                return ETextureFormat::ASTC_RGBA_5x4;
            case ramses::internal::EPixelStorageFormat::ASTC_RGBA_5x5:
                return ETextureFormat::ASTC_RGBA_5x5;
            case ramses::internal::EPixelStorageFormat::ASTC_RGBA_6x5:
                return ETextureFormat::ASTC_RGBA_6x5;
            case ramses::internal::EPixelStorageFormat::ASTC_RGBA_6x6:
                return ETextureFormat::ASTC_RGBA_6x6;
            case ramses::internal::EPixelStorageFormat::ASTC_RGBA_8x5:
                return ETextureFormat::ASTC_RGBA_8x5;
            case ramses::internal::EPixelStorageFormat::ASTC_RGBA_8x6:
                return ETextureFormat::ASTC_RGBA_8x6;
            case ramses::internal::EPixelStorageFormat::ASTC_RGBA_8x8:
                return ETextureFormat::ASTC_RGBA_8x8;
            case ramses::internal::EPixelStorageFormat::ASTC_RGBA_10x5:
                return ETextureFormat::ASTC_RGBA_10x5;
            case ramses::internal::EPixelStorageFormat::ASTC_RGBA_10x6:
                return ETextureFormat::ASTC_RGBA_10x6;
            case ramses::internal::EPixelStorageFormat::ASTC_RGBA_10x8:
                return ETextureFormat::ASTC_RGBA_10x8;
            case ramses::internal::EPixelStorageFormat::ASTC_RGBA_10x10:
                return ETextureFormat::ASTC_RGBA_10x10;
            case ramses::internal::EPixelStorageFormat::ASTC_RGBA_12x10:
                return ETextureFormat::ASTC_RGBA_12x10;
            case ramses::internal::EPixelStorageFormat::ASTC_RGBA_12x12:
                return ETextureFormat::ASTC_RGBA_12x12;
            case ramses::internal::EPixelStorageFormat::ASTC_SRGBA_4x4:
                return ETextureFormat::ASTC_SRGBA_4x4;
            case ramses::internal::EPixelStorageFormat::ASTC_SRGBA_5x4:
                return ETextureFormat::ASTC_SRGBA_5x4;
            case ramses::internal::EPixelStorageFormat::ASTC_SRGBA_5x5:
                return ETextureFormat::ASTC_SRGBA_5x5;
            case ramses::internal::EPixelStorageFormat::ASTC_SRGBA_6x5:
                return ETextureFormat::ASTC_SRGBA_6x5;
            case ramses::internal::EPixelStorageFormat::ASTC_SRGBA_6x6:
                return ETextureFormat::ASTC_SRGBA_6x6;
            case ramses::internal::EPixelStorageFormat::ASTC_SRGBA_8x5:
                return ETextureFormat::ASTC_SRGBA_8x5;
            case ramses::internal::EPixelStorageFormat::ASTC_SRGBA_8x6:
                return ETextureFormat::ASTC_SRGBA_8x6;
            case ramses::internal::EPixelStorageFormat::ASTC_SRGBA_8x8:
                return ETextureFormat::ASTC_SRGBA_8x8;
            case ramses::internal::EPixelStorageFormat::ASTC_SRGBA_10x5:
                return ETextureFormat::ASTC_SRGBA_10x5;
            case ramses::internal::EPixelStorageFormat::ASTC_SRGBA_10x6:
                return ETextureFormat::ASTC_SRGBA_10x6;
            case ramses::internal::EPixelStorageFormat::ASTC_SRGBA_10x8:
                return ETextureFormat::ASTC_SRGBA_10x8;
            case ramses::internal::EPixelStorageFormat::ASTC_SRGBA_10x10:
                return ETextureFormat::ASTC_SRGBA_10x10;
            case ramses::internal::EPixelStorageFormat::ASTC_SRGBA_12x10:
                return ETextureFormat::ASTC_SRGBA_12x10;
            case ramses::internal::EPixelStorageFormat::ASTC_SRGBA_12x12:
                return ETextureFormat::ASTC_SRGBA_12x12;

            case ramses::internal::EPixelStorageFormat::R16F:
                return ETextureFormat::R16F;
            case ramses::internal::EPixelStorageFormat::R32F:
                return ETextureFormat::R32F;
            case ramses::internal::EPixelStorageFormat::RG16F:
                return ETextureFormat::RG16F;
            case ramses::internal::EPixelStorageFormat::RG32F:
                return ETextureFormat::RG32F;
            case ramses::internal::EPixelStorageFormat::RGB16F:
                return ETextureFormat::RGB16F;
            case ramses::internal::EPixelStorageFormat::RGB32F:
                return ETextureFormat::RGB32F;
            case ramses::internal::EPixelStorageFormat::RGBA16F:
                return ETextureFormat::RGBA16F;
            case ramses::internal::EPixelStorageFormat::RGBA32F:
                return ETextureFormat::RGBA32F;

            case ramses::internal::EPixelStorageFormat::SRGB8:
                return ETextureFormat::SRGB8;
            case ramses::internal::EPixelStorageFormat::SRGB8_ALPHA8:
                return ETextureFormat::SRGB8_ALPHA8;

            case ramses::internal::EPixelStorageFormat::Depth16:
            case ramses::internal::EPixelStorageFormat::Depth24:
            case ramses::internal::EPixelStorageFormat::Depth32:
            case ramses::internal::EPixelStorageFormat::Depth24_Stencil8:
            case ramses::internal::EPixelStorageFormat::Invalid:
                break;
            }
            assert(false);
            return ETextureFormat::RGBA8;
        }

        static ramses::internal::TextureSwizzleArray GetTextureSwizzleInternal(const TextureSwizzle& swizzle)
        {
            return ramses::internal::TextureSwizzleArray{
                swizzle.channelRed,
                swizzle.channelGreen,
                swizzle.channelBlue,
                swizzle.channelAlpha,
            };
        }

        static TextureSwizzle GetTextureSwizzleFromInternal(const ramses::internal::TextureSwizzleArray& swizzle)
        {
            return TextureSwizzle{
                swizzle[0],
                swizzle[1],
                swizzle[2],
                swizzle[3]
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
            }

            assert(false);
            return false;
        }

        static ramses::internal::EPixelStorageFormat GetRenderBufferFormatInternal(ERenderBufferFormat bufferFormat)
        {
            switch (bufferFormat)
            {
            case ERenderBufferFormat::RGBA4:
                return ramses::internal::EPixelStorageFormat::RGBA4;
            case ERenderBufferFormat::R8:
                return ramses::internal::EPixelStorageFormat::R8;
            case ERenderBufferFormat::RG8:
                return ramses::internal::EPixelStorageFormat::RG8;
            case ERenderBufferFormat::RGB8:
                return ramses::internal::EPixelStorageFormat::RGB8;
            case ERenderBufferFormat::RGBA8:
                return ramses::internal::EPixelStorageFormat::RGBA8;
            case ERenderBufferFormat::R16F:
                return ramses::internal::EPixelStorageFormat::R16F;
            case ERenderBufferFormat::R32F:
                return ramses::internal::EPixelStorageFormat::R32F;
            case ERenderBufferFormat::RG16F:
                return ramses::internal::EPixelStorageFormat::RG16F;
            case ERenderBufferFormat::RG32F:
                return ramses::internal::EPixelStorageFormat::RG32F;
            case ERenderBufferFormat::RGB16F:
                return ramses::internal::EPixelStorageFormat::RGB16F;
            case ERenderBufferFormat::RGB32F:
                return ramses::internal::EPixelStorageFormat::RGB32F;
            case ERenderBufferFormat::RGBA16F:
                return ramses::internal::EPixelStorageFormat::RGBA16F;
            case ERenderBufferFormat::RGBA32F:
                return ramses::internal::EPixelStorageFormat::RGBA32F;
            case ERenderBufferFormat::Depth16:
                return ramses::internal::EPixelStorageFormat::Depth16;
            case ERenderBufferFormat::Depth24:
                return ramses::internal::EPixelStorageFormat::Depth24;
            case ERenderBufferFormat::Depth32:
                return ramses::internal::EPixelStorageFormat::Depth32;
            case ERenderBufferFormat::Depth24_Stencil8:
                return ramses::internal::EPixelStorageFormat::Depth24_Stencil8;
            }
            assert(false);
            return ramses::internal::EPixelStorageFormat::Invalid;
        }

        static ERenderBufferFormat GetRenderBufferFormatFromInternal(ramses::internal::EPixelStorageFormat bufferFormat)
        {
            switch (bufferFormat)
            {
            case ramses::internal::EPixelStorageFormat::RGBA4:
                return ERenderBufferFormat::RGBA4;
            case ramses::internal::EPixelStorageFormat::R8:
                return ERenderBufferFormat::R8;
            case ramses::internal::EPixelStorageFormat::RG8:
                return ERenderBufferFormat::RG8;
            case ramses::internal::EPixelStorageFormat::RGB8:
                return ERenderBufferFormat::RGB8;
            case ramses::internal::EPixelStorageFormat::RGBA8:
                return ERenderBufferFormat::RGBA8;
            case ramses::internal::EPixelStorageFormat::R16F:
                return ERenderBufferFormat::R16F;
            case ramses::internal::EPixelStorageFormat::R32F:
                return ERenderBufferFormat::R32F;
            case ramses::internal::EPixelStorageFormat::RG16F:
                return ERenderBufferFormat::RG16F;
            case ramses::internal::EPixelStorageFormat::RG32F:
                return ERenderBufferFormat::RG32F;
            case ramses::internal::EPixelStorageFormat::RGB16F:
                return ERenderBufferFormat::RGB16F;
            case ramses::internal::EPixelStorageFormat::RGB32F:
                return ERenderBufferFormat::RGB32F;
            case ramses::internal::EPixelStorageFormat::RGBA16F:
                return ERenderBufferFormat::RGBA16F;
            case ramses::internal::EPixelStorageFormat::RGBA32F:
                return ERenderBufferFormat::RGBA32F;

            case ramses::internal::EPixelStorageFormat::Depth16:
                return ERenderBufferFormat::Depth16;
            case ramses::internal::EPixelStorageFormat::Depth24:
                return ERenderBufferFormat::Depth24;
            case ramses::internal::EPixelStorageFormat::Depth32:
                return ERenderBufferFormat::Depth32;
            case ramses::internal::EPixelStorageFormat::Depth24_Stencil8:
                return ERenderBufferFormat::Depth24_Stencil8;

            case ramses::internal::EPixelStorageFormat::Invalid:
            case ramses::internal::EPixelStorageFormat::RGB565:
            case ramses::internal::EPixelStorageFormat::RGBA5551:
            case ramses::internal::EPixelStorageFormat::ETC2RGB:
            case ramses::internal::EPixelStorageFormat::ETC2RGBA:
            case ramses::internal::EPixelStorageFormat::ASTC_RGBA_4x4:
            case ramses::internal::EPixelStorageFormat::ASTC_RGBA_5x4:
            case ramses::internal::EPixelStorageFormat::ASTC_RGBA_5x5:
            case ramses::internal::EPixelStorageFormat::ASTC_RGBA_6x5:
            case ramses::internal::EPixelStorageFormat::ASTC_RGBA_6x6:
            case ramses::internal::EPixelStorageFormat::ASTC_RGBA_8x5:
            case ramses::internal::EPixelStorageFormat::ASTC_RGBA_8x6:
            case ramses::internal::EPixelStorageFormat::ASTC_RGBA_8x8:
            case ramses::internal::EPixelStorageFormat::ASTC_RGBA_10x5:
            case ramses::internal::EPixelStorageFormat::ASTC_RGBA_10x6:
            case ramses::internal::EPixelStorageFormat::ASTC_RGBA_10x8:
            case ramses::internal::EPixelStorageFormat::ASTC_RGBA_10x10:
            case ramses::internal::EPixelStorageFormat::ASTC_RGBA_12x10:
            case ramses::internal::EPixelStorageFormat::ASTC_RGBA_12x12:
            case ramses::internal::EPixelStorageFormat::ASTC_SRGBA_4x4:
            case ramses::internal::EPixelStorageFormat::ASTC_SRGBA_5x4:
            case ramses::internal::EPixelStorageFormat::ASTC_SRGBA_5x5:
            case ramses::internal::EPixelStorageFormat::ASTC_SRGBA_6x5:
            case ramses::internal::EPixelStorageFormat::ASTC_SRGBA_6x6:
            case ramses::internal::EPixelStorageFormat::ASTC_SRGBA_8x5:
            case ramses::internal::EPixelStorageFormat::ASTC_SRGBA_8x6:
            case ramses::internal::EPixelStorageFormat::ASTC_SRGBA_8x8:
            case ramses::internal::EPixelStorageFormat::ASTC_SRGBA_10x5:
            case ramses::internal::EPixelStorageFormat::ASTC_SRGBA_10x6:
            case ramses::internal::EPixelStorageFormat::ASTC_SRGBA_10x8:
            case ramses::internal::EPixelStorageFormat::ASTC_SRGBA_10x10:
            case ramses::internal::EPixelStorageFormat::ASTC_SRGBA_12x10:
            case ramses::internal::EPixelStorageFormat::ASTC_SRGBA_12x12:
            case ramses::internal::EPixelStorageFormat::SRGB8:
            case ramses::internal::EPixelStorageFormat::SRGB8_ALPHA8:
                break;
            }
            assert(false);
            return ERenderBufferFormat::RGBA8;
        }

        // NOLINTNEXTLINE(modernize-avoid-c-arrays)
        static void FillMipDataSizes(ramses::internal::MipDataSizeVector& mipDataSizes, uint32_t mipMapCount, const MipLevelData mipLevelData[]);
        // NOLINTNEXTLINE(modernize-avoid-c-arrays)
        static void FillMipDataSizes(ramses::internal::MipDataSizeVector& mipDataSizes, uint32_t mipMapCount, const CubeMipLevelData mipLevelData[]);
        // NOLINTNEXTLINE(modernize-avoid-c-arrays)
        static void FillMipData(std::byte* dest, uint32_t mipMapCount, const MipLevelData mipLevelData[]);
        // NOLINTNEXTLINE(modernize-avoid-c-arrays)
        static void FillMipData(std::byte* dest, uint32_t mipMapCount, const CubeMipLevelData mipLevelData[]);
        // NOLINTNEXTLINE(modernize-avoid-c-arrays)
        static bool MipDataValid(uint32_t width, uint32_t height, uint32_t depth, uint32_t mipMapCount, const MipLevelData mipLevelData[], ETextureFormat format);
        // NOLINTNEXTLINE(modernize-avoid-c-arrays)
        static bool MipDataValid(uint32_t width, uint32_t height, uint32_t depth, uint32_t mipMapCount, const CubeMipLevelData mipLevelData[], ETextureFormat format);
        // NOLINTNEXTLINE(modernize-avoid-c-arrays)
        static bool MipDataValid(uint32_t size, uint32_t mipMapCount, const CubeMipLevelData mipLevelData[], ETextureFormat format);
        static bool TextureParametersValid(uint32_t width, uint32_t height, uint32_t depth, uint32_t mipMapCount);
    };
}
