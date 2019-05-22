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
            case ETextureFormat_R8:
                return ramses_internal::ETextureFormat_R8;

            case ETextureFormat_RG8:
                return ramses_internal::ETextureFormat_RG8;

            case ETextureFormat_RGB8:
                return ramses_internal::ETextureFormat_RGB8;
            case ETextureFormat_RGB565:
                return ramses_internal::ETextureFormat_RGB565;

            case ETextureFormat_RGBA8:
                return ramses_internal::ETextureFormat_RGBA8;
            case ETextureFormat_RGBA4:
                return ramses_internal::ETextureFormat_RGBA4;
            case ETextureFormat_RGBA5551:
                return ramses_internal::ETextureFormat_RGBA5551;

            case ETextureFormat_BGR8:
                return ramses_internal::ETextureFormat_BGR8;
            case ETextureFormat_BGRA8:
                return ramses_internal::ETextureFormat_BGRA8;
            case ETextureFormat_ETC2RGB:
                return ramses_internal::ETextureFormat_ETC2RGB;
            case ETextureFormat_ETC2RGBA:
                return ramses_internal::ETextureFormat_ETC2RGBA;
            case ETextureFormat_ASTC_RGBA_4x4:
                return ramses_internal::ETextureFormat_ASTC_RGBA_4x4;
            case ETextureFormat_ASTC_RGBA_5x4:
                return ramses_internal::ETextureFormat_ASTC_RGBA_5x4;
            case ETextureFormat_ASTC_RGBA_5x5:
                return ramses_internal::ETextureFormat_ASTC_RGBA_5x5;
            case ETextureFormat_ASTC_RGBA_6x5:
                return ramses_internal::ETextureFormat_ASTC_RGBA_6x5;
            case ETextureFormat_ASTC_RGBA_6x6:
                return ramses_internal::ETextureFormat_ASTC_RGBA_6x6;
            case ETextureFormat_ASTC_RGBA_8x5:
                return ramses_internal::ETextureFormat_ASTC_RGBA_8x5;
            case ETextureFormat_ASTC_RGBA_8x6:
                return ramses_internal::ETextureFormat_ASTC_RGBA_8x6;
            case ETextureFormat_ASTC_RGBA_8x8:
                return ramses_internal::ETextureFormat_ASTC_RGBA_8x8;
            case ETextureFormat_ASTC_RGBA_10x5:
                return ramses_internal::ETextureFormat_ASTC_RGBA_10x5;
            case ETextureFormat_ASTC_RGBA_10x6:
                return ramses_internal::ETextureFormat_ASTC_RGBA_10x6;
            case ETextureFormat_ASTC_RGBA_10x8:
                return ramses_internal::ETextureFormat_ASTC_RGBA_10x8;
            case ETextureFormat_ASTC_RGBA_10x10:
                return ramses_internal::ETextureFormat_ASTC_RGBA_10x10;
            case ETextureFormat_ASTC_RGBA_12x10:
                return ramses_internal::ETextureFormat_ASTC_RGBA_12x10;
            case ETextureFormat_ASTC_RGBA_12x12:
                return ramses_internal::ETextureFormat_ASTC_RGBA_12x12;
            case ETextureFormat_ASTC_SRGBA_4x4:
                return ramses_internal::ETextureFormat_ASTC_SRGBA_4x4;
            case ETextureFormat_ASTC_SRGBA_5x4:
                return ramses_internal::ETextureFormat_ASTC_SRGBA_5x4;
            case ETextureFormat_ASTC_SRGBA_5x5:
                return ramses_internal::ETextureFormat_ASTC_SRGBA_5x5;
            case ETextureFormat_ASTC_SRGBA_6x5:
                return ramses_internal::ETextureFormat_ASTC_SRGBA_6x5;
            case ETextureFormat_ASTC_SRGBA_6x6:
                return ramses_internal::ETextureFormat_ASTC_SRGBA_6x6;
            case ETextureFormat_ASTC_SRGBA_8x5:
                return ramses_internal::ETextureFormat_ASTC_SRGBA_8x5;
            case ETextureFormat_ASTC_SRGBA_8x6:
                return ramses_internal::ETextureFormat_ASTC_SRGBA_8x6;
            case ETextureFormat_ASTC_SRGBA_8x8:
                return ramses_internal::ETextureFormat_ASTC_SRGBA_8x8;
            case ETextureFormat_ASTC_SRGBA_10x5:
                return ramses_internal::ETextureFormat_ASTC_SRGBA_10x5;
            case ETextureFormat_ASTC_SRGBA_10x6:
                return ramses_internal::ETextureFormat_ASTC_SRGBA_10x6;
            case ETextureFormat_ASTC_SRGBA_10x8:
                return ramses_internal::ETextureFormat_ASTC_SRGBA_10x8;
            case ETextureFormat_ASTC_SRGBA_10x10:
                return ramses_internal::ETextureFormat_ASTC_SRGBA_10x10;
            case ETextureFormat_ASTC_SRGBA_12x10:
                return ramses_internal::ETextureFormat_ASTC_SRGBA_12x10;
            case ETextureFormat_ASTC_SRGBA_12x12:
                return ramses_internal::ETextureFormat_ASTC_SRGBA_12x12;
            case ETextureFormat_R16F:
                return ramses_internal::ETextureFormat_R16F;
            case ETextureFormat_R32F:
                return ramses_internal::ETextureFormat_R32F;
            case ETextureFormat_RG16F:
                return ramses_internal::ETextureFormat_RG16F;
            case ETextureFormat_RG32F:
                return ramses_internal::ETextureFormat_RG32F;
            case ETextureFormat_RGB16F:
                return ramses_internal::ETextureFormat_RGB16F;
            case ETextureFormat_RGB32F:
                return ramses_internal::ETextureFormat_RGB32F;
            case ETextureFormat_RGBA16F:
                return ramses_internal::ETextureFormat_RGBA16F;
            case ETextureFormat_RGBA32F:
                return ramses_internal::ETextureFormat_RGBA32F;

            case ETextureFormat_SRGB8:
                return ramses_internal::ETextureFormat_SRGB8;
            case ETextureFormat_SRGB8_ALPHA8:
                return ramses_internal::ETextureFormat_SRGB8_ALPHA8;

            default:
                assert(false);
                return ramses_internal::ETextureFormat_RGBA8;
            }
        }

        static ETextureFormat GetTextureFormatFromInternal(ramses_internal::ETextureFormat textureformat)
        {
            switch (textureformat)
            {
            case ramses_internal::ETextureFormat_R8:
                return ETextureFormat_R8;

            case ramses_internal::ETextureFormat_RG8:
                return ETextureFormat_RG8;

            case ramses_internal::ETextureFormat_RGB8:
                return ETextureFormat_RGB8;
            case ramses_internal::ETextureFormat_RGB565:
                return ETextureFormat_RGB565;

            case ramses_internal::ETextureFormat_RGBA8:
                return ETextureFormat_RGBA8;
            case ramses_internal::ETextureFormat_RGBA4:
                return ETextureFormat_RGBA4;
            case ramses_internal::ETextureFormat_RGBA5551:
                return ETextureFormat_RGBA5551;

            case ramses_internal::ETextureFormat_BGR8:
                return ETextureFormat_BGR8;
            case ramses_internal::ETextureFormat_BGRA8:
                return ETextureFormat_BGRA8;

            case ramses_internal::ETextureFormat_ETC2RGB:
                return ETextureFormat_ETC2RGB;
            case ramses_internal::ETextureFormat_ETC2RGBA:
                return ETextureFormat_ETC2RGBA;

            case ramses_internal::ETextureFormat_ASTC_RGBA_4x4:
                return ETextureFormat_ASTC_RGBA_4x4;
            case ramses_internal::ETextureFormat_ASTC_RGBA_5x4:
                return ETextureFormat_ASTC_RGBA_5x4;
            case ramses_internal::ETextureFormat_ASTC_RGBA_5x5:
                return ETextureFormat_ASTC_RGBA_5x5;
            case ramses_internal::ETextureFormat_ASTC_RGBA_6x5:
                return ETextureFormat_ASTC_RGBA_6x5;
            case ramses_internal::ETextureFormat_ASTC_RGBA_6x6:
                return ETextureFormat_ASTC_RGBA_6x6;
            case ramses_internal::ETextureFormat_ASTC_RGBA_8x5:
                return ETextureFormat_ASTC_RGBA_8x5;
            case ramses_internal::ETextureFormat_ASTC_RGBA_8x6:
                return ETextureFormat_ASTC_RGBA_8x6;
            case ramses_internal::ETextureFormat_ASTC_RGBA_8x8:
                return ETextureFormat_ASTC_RGBA_8x8;
            case ramses_internal::ETextureFormat_ASTC_RGBA_10x5:
                return ETextureFormat_ASTC_RGBA_10x5;
            case ramses_internal::ETextureFormat_ASTC_RGBA_10x6:
                return ETextureFormat_ASTC_RGBA_10x6;
            case ramses_internal::ETextureFormat_ASTC_RGBA_10x8:
                return ETextureFormat_ASTC_RGBA_10x8;
            case ramses_internal::ETextureFormat_ASTC_RGBA_10x10:
                return ETextureFormat_ASTC_RGBA_10x10;
            case ramses_internal::ETextureFormat_ASTC_RGBA_12x10:
                return ETextureFormat_ASTC_RGBA_12x10;
            case ramses_internal::ETextureFormat_ASTC_RGBA_12x12:
                return ETextureFormat_ASTC_RGBA_12x12;
            case ramses_internal::ETextureFormat_ASTC_SRGBA_4x4:
                return ETextureFormat_ASTC_SRGBA_4x4;
            case ramses_internal::ETextureFormat_ASTC_SRGBA_5x4:
                return ETextureFormat_ASTC_SRGBA_5x4;
            case ramses_internal::ETextureFormat_ASTC_SRGBA_5x5:
                return ETextureFormat_ASTC_SRGBA_5x5;
            case ramses_internal::ETextureFormat_ASTC_SRGBA_6x5:
                return ETextureFormat_ASTC_SRGBA_6x5;
            case ramses_internal::ETextureFormat_ASTC_SRGBA_6x6:
                return ETextureFormat_ASTC_SRGBA_6x6;
            case ramses_internal::ETextureFormat_ASTC_SRGBA_8x5:
                return ETextureFormat_ASTC_SRGBA_8x5;
            case ramses_internal::ETextureFormat_ASTC_SRGBA_8x6:
                return ETextureFormat_ASTC_SRGBA_8x6;
            case ramses_internal::ETextureFormat_ASTC_SRGBA_8x8:
                return ETextureFormat_ASTC_SRGBA_8x8;
            case ramses_internal::ETextureFormat_ASTC_SRGBA_10x5:
                return ETextureFormat_ASTC_SRGBA_10x5;
            case ramses_internal::ETextureFormat_ASTC_SRGBA_10x6:
                return ETextureFormat_ASTC_SRGBA_10x6;
            case ramses_internal::ETextureFormat_ASTC_SRGBA_10x8:
                return ETextureFormat_ASTC_SRGBA_10x8;
            case ramses_internal::ETextureFormat_ASTC_SRGBA_10x10:
                return ETextureFormat_ASTC_SRGBA_10x10;
            case ramses_internal::ETextureFormat_ASTC_SRGBA_12x10:
                return ETextureFormat_ASTC_SRGBA_12x10;
            case ramses_internal::ETextureFormat_ASTC_SRGBA_12x12:
                return ETextureFormat_ASTC_SRGBA_12x12;

            case ramses_internal::ETextureFormat_R16F:
                return ETextureFormat_R16F;
            case ramses_internal::ETextureFormat_R32F:
                return ETextureFormat_R32F;
            case ramses_internal::ETextureFormat_RG16F:
                return ETextureFormat_RG16F;
            case ramses_internal::ETextureFormat_RG32F:
                return ETextureFormat_RG32F;
            case ramses_internal::ETextureFormat_RGB16F:
                return ETextureFormat_RGB16F;
            case ramses_internal::ETextureFormat_RGB32F:
                return ETextureFormat_RGB32F;
            case ramses_internal::ETextureFormat_RGBA16F:
                return ETextureFormat_RGBA16F;
            case ramses_internal::ETextureFormat_RGBA32F:
                return ETextureFormat_RGBA32F;

            case ramses_internal::ETextureFormat_SRGB8:
                return ETextureFormat_SRGB8;
            case ramses_internal::ETextureFormat_SRGB8_ALPHA8:
                return ETextureFormat_SRGB8_ALPHA8;

            default:
                assert(false);
                return ETextureFormat_RGBA8;
            }
        }

        static ramses_internal::EWrapMethod GetTextureAddressModeInternal(ETextureAddressMode addressMode)
        {
            switch (addressMode)
            {
            case ETextureAddressMode_Clamp:
                return ramses_internal::EWrapMethod::Clamp;
            case ETextureAddressMode_Repeat:
                return ramses_internal::EWrapMethod::Repeat;
            case ETextureAddressMode_Mirror:
                return ramses_internal::EWrapMethod::RepeatMirrored;
            default:
                assert(false);
                return ramses_internal::EWrapMethod::Clamp;
            }
        }

        static ETextureAddressMode GetTextureAddressModeFromInternal(ramses_internal::EWrapMethod addressMode)
        {
            switch (addressMode)
            {
            case ramses_internal::EWrapMethod::Clamp:
                return ETextureAddressMode_Clamp;
            case ramses_internal::EWrapMethod::Repeat:
                return ETextureAddressMode_Repeat;
            case ramses_internal::EWrapMethod::RepeatMirrored:
                return ETextureAddressMode_Mirror;
            default:
                assert(false);
                return ETextureAddressMode_Clamp;
            }
        }

        static ramses_internal::ETextureCubeFace GetTextureCubeFaceInternal(ETextureCubeFace face)
        {
            switch (face)
            {
            case ETextureCubeFace_PositiveX:
                return ramses_internal::ETextureCubeFace_PositiveX;
            case ETextureCubeFace_NegativeX:
                return ramses_internal::ETextureCubeFace_NegativeX;
            case ETextureCubeFace_PositiveY:
                return ramses_internal::ETextureCubeFace_PositiveY;
            case ETextureCubeFace_NegativeY:
                return ramses_internal::ETextureCubeFace_NegativeY;
            case ETextureCubeFace_PositiveZ:
                return ramses_internal::ETextureCubeFace_PositiveZ;
            case ETextureCubeFace_NegativeZ:
                return ramses_internal::ETextureCubeFace_NegativeZ;
            default:
                assert(false);
                return ramses_internal::ETextureCubeFace_PositiveX;
            }
        }

        static ramses_internal::ESamplingMethod GetTextureSamplingInternal(ETextureSamplingMethod sampling)
        {
            switch (sampling)
            {
            case ETextureSamplingMethod_Nearest:
                return ramses_internal::ESamplingMethod::Nearest;
            case ETextureSamplingMethod_Nearest_MipMapNearest:
                return ramses_internal::ESamplingMethod::Nearest_MipMapNearest;
            case ETextureSamplingMethod_Nearest_MipMapLinear:
                return ramses_internal::ESamplingMethod::Nearest_MipMapLinear;
            case ETextureSamplingMethod_Linear:
                return ramses_internal::ESamplingMethod::Linear;
            case ETextureSamplingMethod_Linear_MipMapNearest:
                return ramses_internal::ESamplingMethod::Linear_MipMapNearest;
            case ETextureSamplingMethod_Linear_MipMapLinear:
                return ramses_internal::ESamplingMethod::Linear_MipMapLinear;
            default:
                assert(false);
                return ramses_internal::ESamplingMethod::Linear;
            }
        }

        static ETextureSamplingMethod GetTextureSamplingFromInternal(ramses_internal::ESamplingMethod sampling)
        {
            switch (sampling)
            {
            case ramses_internal::ESamplingMethod::Nearest:
                return ETextureSamplingMethod_Nearest;
            case ramses_internal::ESamplingMethod::Nearest_MipMapNearest:
                return ETextureSamplingMethod_Nearest_MipMapNearest;
            case ramses_internal::ESamplingMethod::Nearest_MipMapLinear:
                return ETextureSamplingMethod_Nearest_MipMapLinear;
            case ramses_internal::ESamplingMethod::Linear:
                return ETextureSamplingMethod_Linear;
            case ramses_internal::ESamplingMethod::Linear_MipMapNearest:
                return ETextureSamplingMethod_Linear_MipMapNearest;
            case ramses_internal::ESamplingMethod::Linear_MipMapLinear:
                return ETextureSamplingMethod_Linear_MipMapLinear;
            default:
                assert(false);
                return ETextureSamplingMethod_Linear;
            }
        }

        static ramses_internal::ERenderBufferType GetRenderBufferTypeInternal(ERenderBufferType bufferType)
        {
            switch (bufferType)
            {
            case ERenderBufferType_Color:
                return ramses_internal::ERenderBufferType_ColorBuffer;
            case ERenderBufferType_Depth:
                return ramses_internal::ERenderBufferType_DepthBuffer;
            case ERenderBufferType_DepthStencil:
                return ramses_internal::ERenderBufferType_DepthStencilBuffer;
            default:
                assert(false);
                return ramses_internal::ERenderBufferType_InvalidBuffer;
            }
        }

        static ERenderBufferType GetRenderBufferTypeFromInternal(ramses_internal::ERenderBufferType bufferType)
        {
            switch (bufferType)
            {
            case ramses_internal::ERenderBufferType_ColorBuffer:
                return ERenderBufferType_Color;
            case ramses_internal::ERenderBufferType_DepthBuffer:
                return ERenderBufferType_Depth;
            case ramses_internal::ERenderBufferType_DepthStencilBuffer:
                return ERenderBufferType_DepthStencil;
            default:
                assert(false);
                return ERenderBufferType_Color;
            }
        }

        static ramses_internal::ETextureFormat GetRenderBufferFormatInternal(ERenderBufferFormat bufferFormat)
        {
            switch (bufferFormat)
            {
            case ramses::ERenderBufferFormat_R8:
                return ramses_internal::ETextureFormat_R8;
            case ramses::ERenderBufferFormat_RG8:
                return ramses_internal::ETextureFormat_RG8;
            case ramses::ERenderBufferFormat_RGB8:
                return ramses_internal::ETextureFormat_RGB8;
            case ramses::ERenderBufferFormat_RGBA8:
                return ramses_internal::ETextureFormat_RGBA8;
            case ramses::ERenderBufferFormat_R16F:
                return ramses_internal::ETextureFormat_R16F;
            case ramses::ERenderBufferFormat_R32F:
                return ramses_internal::ETextureFormat_R32F;
            case ramses::ERenderBufferFormat_RG16F:
                return ramses_internal::ETextureFormat_RG16F;
            case ramses::ERenderBufferFormat_RG32F:
                return ramses_internal::ETextureFormat_RG32F;
            case ramses::ERenderBufferFormat_RGB16F:
                return ramses_internal::ETextureFormat_RGB16F;
            case ramses::ERenderBufferFormat_RGB32F:
                return ramses_internal::ETextureFormat_RGB32F;
            case ramses::ERenderBufferFormat_RGBA16F:
                return ramses_internal::ETextureFormat_RGBA16F;
            case ramses::ERenderBufferFormat_RGBA32F:
                return ramses_internal::ETextureFormat_RGBA32F;
            case ramses::ERenderBufferFormat_Depth24:
                return ramses_internal::ETextureFormat_Depth24;
            case ramses::ERenderBufferFormat_Depth24_Stencil8:
                return ramses_internal::ETextureFormat_Depth24_Stencil8;
            default:
                assert(false);
                return ramses_internal::ETextureFormat_Invalid;
            }
        }

        static ramses_internal::ERenderBufferAccessMode GetRenderBufferAccessModeInternal(ERenderBufferAccessMode accessMode)
        {
            switch (accessMode)
            {
            case ramses::ERenderBufferAccessMode_WriteOnly:
                return ramses_internal::ERenderBufferAccessMode_WriteOnly;
            case ramses::ERenderBufferAccessMode_ReadWrite:
                return ramses_internal::ERenderBufferAccessMode_ReadWrite;
            default:
                assert(false);
                return ramses_internal::ERenderBufferAccessMode_Invalid;
            }
        }

        static ERenderBufferFormat GetRenderBufferFormatFromInternal(ramses_internal::ETextureFormat bufferFormat)
        {
            switch (bufferFormat)
            {
            case ramses_internal::ETextureFormat_R8:
                return ERenderBufferFormat_R8;
            case ramses_internal::ETextureFormat_RG8:
                return ERenderBufferFormat_RG8;
            case ramses_internal::ETextureFormat_RGB8:
                return ERenderBufferFormat_RGB8;
            case ramses_internal::ETextureFormat_RGBA8:
                return ERenderBufferFormat_RGBA8;
            case ramses_internal::ETextureFormat_R16F:
                return ERenderBufferFormat_R16F;
            case ramses_internal::ETextureFormat_R32F:
                return ERenderBufferFormat_R32F;
            case ramses_internal::ETextureFormat_RG16F:
                return ERenderBufferFormat_RG16F;
            case ramses_internal::ETextureFormat_RG32F:
                return ERenderBufferFormat_RG32F;
            case ramses_internal::ETextureFormat_RGB16F:
                return ERenderBufferFormat_RGB16F;
            case ramses_internal::ETextureFormat_RGB32F:
                return ERenderBufferFormat_RGB32F;
            case ramses_internal::ETextureFormat_RGBA16F:
                return ERenderBufferFormat_RGBA16F;
            case ramses_internal::ETextureFormat_RGBA32F:
                return ERenderBufferFormat_RGBA32F;

            case ramses_internal::ETextureFormat_Depth24:
                return ERenderBufferFormat_Depth24;
            case ramses_internal::ETextureFormat_Depth24_Stencil8:
                return ERenderBufferFormat_Depth24_Stencil8;
            default:
                assert(false);
                return ERenderBufferFormat_RGBA8;
            }
        }

        static ERenderBufferAccessMode GetRenderBufferAccessModeFromInternal(ramses_internal::ERenderBufferAccessMode accessMode)
        {
            switch (accessMode)
            {
            case ramses_internal::ERenderBufferAccessMode_WriteOnly:
                return ERenderBufferAccessMode_WriteOnly;
            case ramses_internal::ERenderBufferAccessMode_ReadWrite:
                return ERenderBufferAccessMode_ReadWrite;
            default:
                assert(false);
                return ERenderBufferAccessMode_ReadWrite;
            }
        }

        static bool IsRenderBufferTypeCompatibleWithFormat(ERenderBufferType bufferType, ERenderBufferFormat bufferFormat)
        {
            switch (bufferType)
            {
            case ERenderBufferType_Color:
                return
                    bufferFormat == ERenderBufferFormat_R8 ||
                    bufferFormat == ERenderBufferFormat_RG8 ||
                    bufferFormat == ERenderBufferFormat_RGB8 ||
                    bufferFormat == ERenderBufferFormat_RGBA8 ||
                    bufferFormat == ERenderBufferFormat_R16F ||
                    bufferFormat == ERenderBufferFormat_R32F ||
                    bufferFormat == ERenderBufferFormat_RG16F ||
                    bufferFormat == ERenderBufferFormat_RG32F ||
                    bufferFormat == ERenderBufferFormat_RGB16F ||
                    bufferFormat == ERenderBufferFormat_RGB32F ||
                    bufferFormat == ERenderBufferFormat_RGBA16F ||
                    bufferFormat == ERenderBufferFormat_RGBA32F;
            case ERenderBufferType_Depth:
                return bufferFormat == ERenderBufferFormat_Depth24;
            case ERenderBufferType_DepthStencil:
                return bufferFormat == ERenderBufferFormat_Depth24_Stencil8;
            default:
                assert(false);
                return false;
            }
        }

        static void FillMipDataSizes(ramses_internal::MipDataSizeVector& mipDataSizes, uint32_t mipMapCount, const MipLevelData mipLevelData[]);
        static void FillMipDataSizes(ramses_internal::MipDataSizeVector& mipDataSizes, uint32_t mipMapCount, const CubeMipLevelData mipLevelData[]);
        static void FillMipData(uint8_t* dest, uint32_t mipMapCount, const MipLevelData mipLevelData[]);
        static void FillMipData(uint8_t* dest, uint32_t mipMapCount, const CubeMipLevelData mipLevelData[]);
        static bool MipDataValid(uint32_t width, uint32_t height, uint32_t depth, uint32_t mipMapCount, const MipLevelData mipLevelData[], ETextureFormat format);
        static bool MipDataValid(uint32_t width, uint32_t height, uint32_t depth, uint32_t mipMapCount, const CubeMipLevelData mipLevelData[], ETextureFormat format);
        static bool MipDataValid(uint32_t size, uint32_t mipMapCount, const CubeMipLevelData mipLevelData[], ETextureFormat format);
        static bool TextureParametersValid(uint32_t width, uint32_t height, uint32_t depth, uint32_t mipMapCount);
        static Texture2D* CreateTextureResourceFromPng(const char* filePath, RamsesClient& client, const char* name = nullptr);
    };
}

#endif
