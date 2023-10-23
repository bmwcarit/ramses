//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/Resource/EffectInputInformation.h"
#include "internal/SceneGraph/SceneAPI/RenderState.h"
#include "internal/SceneGraph/SceneAPI/TextureEnums.h"
#include "internal/SceneGraph/Resource/TextureMetaInfo.h"

namespace ramses::internal
{
    struct TextureUploadParams_GL
    {
        GLint sizedInternalFormat = GL_ZERO;
        GLenum baseInternalFormat = GL_ZERO;
        GLenum type = GL_ZERO;
        bool compressed = false;
        GLint byteAlignment = 0;
        TextureSwizzleArray swizzle = {};
    };

    class TypesConversion_GL
    {
    public:
        static GLenum GetDrawMode(EDrawMode mode);
        static GLenum GetIndexElementType(uint32_t indexElementSizeInBytes);
        static GLenum GetDepthFunc(EDepthFunc func);
        static GLenum GetBlendFactor(EBlendFactor factor);
        static GLenum GetBlendOperation(EBlendOperation operation);
        static GLint GetWrapMode(ETextureAddressMode wrapMode);
        static GLenum GetStencilFunc(EStencilFunc func);
        static GLenum GetStencilOperation(EStencilOp op);
        static GLenum GetCullMode(ECullMode mode);
        static GLenum GetTextureTargetFromTextureInputType(EEffectInputTextureType textureType);
        static GLenum GetCubemapFaceSpecifier(ETextureCubeFace face);
        static GLint GetASTCFormat(EPixelStorageFormat ramsesEnum);
        static EPixelStorageFormat GetTextureFormatFromCompressedGLTextureFormat(GLenum compressedGLTextureFormat);
        static GLint GetGlColorFromTextureChannelColor(ETextureChannelColor color);
        static TextureUploadParams_GL GetTextureUploadParams(EPixelStorageFormat format);
    };
}
