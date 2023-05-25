//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TYPESCONVERSION_GL_H
#define RAMSES_TYPESCONVERSION_GL_H

#include "Resource/EffectInputInformation.h"
#include "SceneAPI/RenderState.h"
#include "SceneAPI/TextureEnums.h"
#include "Resource/TextureMetaInfo.h"

namespace ramses_internal
{
    struct TextureUploadParams_GL
    {
        GLenum sizedInternalFormat = GL_ZERO;
        GLenum baseInternalFormat = GL_ZERO;
        GLenum type = GL_ZERO;
        bool compressed = false;
        UInt32 byteAlignment = 0u;
        TextureSwizzleArray swizzle = {};
    };

    class TypesConversion_GL
    {
    public:
        static GLenum GetDrawMode(EDrawMode mode);
        static GLenum GetIndexElementType(UInt32 indexElementSizeInBytes);
        static GLenum GetDepthFunc(EDepthFunc func);
        static GLenum GetBlendFactor(EBlendFactor factor);
        static GLenum GetBlendOperation(EBlendOperation operation);
        static GLenum GetWrapMode(EWrapMethod wrapMode);
        static GLenum GetStencilFunc(EStencilFunc func);
        static GLenum GetStencilOperation(EStencilOp op);
        static GLenum GetCullMode(ECullMode mode);
        static GLenum GetTextureTargetFromTextureInputType(EEffectInputTextureType textureType);
        static GLenum GetCubemapFaceSpecifier(ETextureCubeFace face);
        static GLenum GetASTCFormat(ETextureFormat ramsesEnum);
        static ETextureFormat GetTextureFormatFromCompressedGLTextureFormat(GLenum compressedGLTextureFormat);
        static GLenum GetGlColorFromTextureChannelColor(ETextureChannelColor color);
        static TextureUploadParams_GL GetTextureUploadParams(ETextureFormat format);
    };
}

#endif
