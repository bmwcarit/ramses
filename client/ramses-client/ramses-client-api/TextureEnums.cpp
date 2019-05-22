//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-client-api/TextureEnums.h"
#include "Utils/LoggingUtils.h"

namespace ramses
{
    const char* TextureSamplingMethodNames[] =
    {
        "ETextureSamplingMethod_Nearest",
        "ETextureSamplingMethod_Linear",
        "ETextureSamplingMethod_Nearest_MipMapNearest",
        "ETextureSamplingMethod_Nearest_MipMapLinear",
        "ETextureSamplingMethod_Linear_MipMapNearest",
        "ETextureSamplingMethod_Linear_MipMapLinear",
    };


    const char* TextureAddressModeNames[] = {
        "ETextureAddressMode_Clamp",
        "ETextureAddressMode_Repeat",
        "ETextureAddressMode_Mirror"
    };

    const char* TextureFormatNames[] = {
        "ETextureFormat_Invalid",
        "ETextureFormat_R8",
        "ETextureFormat_RG8",
        "ETextureFormat_RGB8",
        "ETextureFormat_RGB565",
        "ETextureFormat_RGBA8",
        "ETextureFormat_RGBA4",
        "ETextureFormat_RGBA5551",
        "ETextureFormat_BGR8",
        "ETextureFormat_BGRA8",
        "ETextureFormat_ETC2RGB",
        "ETextureFormat_ETC2RGBA",
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

    const char* TextureCubeFaceNames[] = {
        "ETextureCubeFace_PositiveX",
        "ETextureCubeFace_NegativeX",
        "ETextureCubeFace_PositiveY",
        "ETextureCubeFace_NegativeY",
        "ETextureCubeFace_PositiveZ",
        "ETextureCubeFace_NegativeZ"
    };

    ENUM_TO_STRING(ETextureSamplingMethod, TextureSamplingMethodNames, ETextureSamplingMethod_NUMBER_OF_ELEMENTS);
    ENUM_TO_STRING(ETextureAddressMode, TextureAddressModeNames, ETextureAddressMode_NUMBER_OF_ELEMENTS);
    ENUM_TO_STRING(ETextureFormat, TextureFormatNames, ETextureFormat_NUMBER_OF_ELEMENTS);
    ENUM_TO_STRING(ETextureCubeFace, TextureCubeFaceNames, ETextureCubeFace_NUMBER_OF_ELEMENTS);

    const char* getTextureSamplingMethodString(ETextureSamplingMethod samplingMethod)
    {
        return EnumToString(samplingMethod);
    }

    const char* getTextureAddressModeString(ETextureAddressMode addressMode)
    {
        return EnumToString(addressMode);
    }

    const char* getTextureFormatString(ETextureFormat format)
    {
        return EnumToString(format);
    }

    const char* getTextureCubeFaceString(ETextureCubeFace face)
    {
        return EnumToString(face);
    }

}
