//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Device_GL/Device_GL_platform.h"
#include "Device_GL/TypesConversion_GL.h"

namespace ramses_internal
{
    TextureUploadParams_GL TypesConversion_GL::GetTextureUploadParams(ETextureFormat format)
    {
        TextureUploadParams_GL uploadParams;

        switch (format)
        {
        case ETextureFormat_RGBA4:
            uploadParams.sizedInternalFormat = GL_RGBA4;
            uploadParams.baseInternalFormat = GL_RGBA;
            uploadParams.type = GL_UNSIGNED_SHORT_4_4_4_4;
            uploadParams.byteAlignment = 2u;
            break;
        case ETextureFormat_BGR8:
            // Treat the same as RGB8, but swizzle input data
            uploadParams.swizzleBGRXtoRGBX = true;
        case ETextureFormat_RGB8:
            uploadParams.sizedInternalFormat = GL_RGB8;
            uploadParams.baseInternalFormat = GL_RGB;
            uploadParams.type = GL_UNSIGNED_BYTE;
            uploadParams.byteAlignment = 1u;
            break;
        case ETextureFormat_BGRA8:
            // Treat the same as RGBA8, but swizzle input data
            uploadParams.swizzleBGRXtoRGBX = true;
        case ETextureFormat_RGBA8:
            uploadParams.sizedInternalFormat = GL_RGBA8;
            uploadParams.baseInternalFormat = GL_RGBA;
            uploadParams.type = GL_UNSIGNED_BYTE;
            uploadParams.byteAlignment = 4u;
            break;
        case ETextureFormat_RGB16:
            uploadParams.sizedInternalFormat = GL_RGB16UI;
            uploadParams.baseInternalFormat = GL_RGB;
            uploadParams.type = GL_UNSIGNED_SHORT;
            uploadParams.byteAlignment = 2u;
            break;
        case ETextureFormat_RGBA16:
            uploadParams.sizedInternalFormat = GL_RGBA16UI;
            uploadParams.baseInternalFormat = GL_RGBA;
            uploadParams.type = GL_UNSIGNED_SHORT;
            uploadParams.byteAlignment = 8u;
            break;

        case ETextureFormat_RGBA5551:
            uploadParams.sizedInternalFormat = GL_RGB5_A1;
            uploadParams.baseInternalFormat = GL_RGBA;
            uploadParams.type = GL_UNSIGNED_SHORT_5_5_5_1;
            uploadParams.byteAlignment = 2u;
            break;
        case ETextureFormat_RGB565:
            uploadParams.sizedInternalFormat = GL_RGB565;
            uploadParams.baseInternalFormat = GL_RGB;
            uploadParams.type = GL_UNSIGNED_SHORT_5_6_5;
            uploadParams.byteAlignment = 2u;
            break;
        case ETextureFormat_DXT1RGB: // GL_EXT_texture_compression_dxt1
            uploadParams.compressed = true;
            uploadParams.sizedInternalFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
            uploadParams.baseInternalFormat = GL_RGB;
            uploadParams.byteAlignment = 8u;
            break;
        case ETextureFormat_DXT3RGBA:
            // DXT3 (extension)
            uploadParams.compressed = true;
            uploadParams.sizedInternalFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
            uploadParams.baseInternalFormat = GL_RGBA;
            uploadParams.byteAlignment = 8u; // actually 16 bytes but glPixelStore accepts maximum 8 bytes alignment
            break;
        case ETextureFormat_DXT5RGBA:
            // DXT5 (extension)
            uploadParams.compressed = true;
            uploadParams.sizedInternalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
            uploadParams.baseInternalFormat = GL_RGBA;
            uploadParams.byteAlignment = 8u; // actually 16 bytes but glPixelStore accepts maximum 8 bytes alignment
            break;
        case ETextureFormat_R8:
            uploadParams.sizedInternalFormat = GL_R8;
            uploadParams.baseInternalFormat = GL_RED;
            uploadParams.type = GL_UNSIGNED_BYTE;
            uploadParams.byteAlignment = 1u;
            break;
        case ETextureFormat_RG8:
            uploadParams.sizedInternalFormat = GL_RG8;
            uploadParams.baseInternalFormat = GL_RG;
            uploadParams.type = GL_UNSIGNED_BYTE;
            uploadParams.byteAlignment = 2u;
            break;
        case ETextureFormat_R16:
            uploadParams.sizedInternalFormat = GL_R16UI;
            uploadParams.baseInternalFormat = GL_RED;
            uploadParams.type = GL_UNSIGNED_SHORT;
            uploadParams.byteAlignment = 2u;
            break;
        case ETextureFormat_RG16:
            uploadParams.sizedInternalFormat = GL_RG16UI;
            uploadParams.baseInternalFormat = GL_RG;
            uploadParams.type = GL_UNSIGNED_SHORT;
            uploadParams.byteAlignment = 4u;
            break;
        case ETextureFormat_R16F:
            uploadParams.sizedInternalFormat = GL_R16F;
            uploadParams.baseInternalFormat = GL_RED;
            uploadParams.type = GL_HALF_FLOAT;
            uploadParams.byteAlignment = 2u;
            break;
        case ETextureFormat_R32F:
            uploadParams.sizedInternalFormat = GL_R32F;
            uploadParams.baseInternalFormat = GL_RED;
            uploadParams.type = GL_FLOAT;
            uploadParams.byteAlignment = 4u;
            break;
        case ETextureFormat_RG16F:
            uploadParams.sizedInternalFormat = GL_RG16F;
            uploadParams.baseInternalFormat = GL_RG;
            uploadParams.type = GL_HALF_FLOAT;
            uploadParams.byteAlignment = 4u;
            break;
        case ETextureFormat_RG32F:
            uploadParams.sizedInternalFormat = GL_RG32F;
            uploadParams.baseInternalFormat = GL_RG;
            uploadParams.type = GL_FLOAT;
            uploadParams.byteAlignment = 8u;
            break;
        case ETextureFormat_RGB16F:
            uploadParams.sizedInternalFormat = GL_RGB16F;
            uploadParams.baseInternalFormat = GL_RGB;
            uploadParams.type = GL_HALF_FLOAT;
            uploadParams.byteAlignment = 2u;
            break;
        case ETextureFormat_RGB32F:
            uploadParams.sizedInternalFormat = GL_RGB32F;
            uploadParams.baseInternalFormat = GL_RGB;
            uploadParams.type = GL_FLOAT;
            uploadParams.byteAlignment = 4u;
            break;
        case ETextureFormat_RGBA16F:
            uploadParams.sizedInternalFormat = GL_RGBA16F;
            uploadParams.baseInternalFormat = GL_RGBA;
            uploadParams.type = GL_HALF_FLOAT;
            uploadParams.byteAlignment = 8u;
            break;
        case ETextureFormat_RGBA32F:
            uploadParams.sizedInternalFormat = GL_RGBA32F;
            uploadParams.baseInternalFormat = GL_RGBA;
            uploadParams.type = GL_FLOAT;
            uploadParams.byteAlignment = 8u; // actually 16 bytes but glPixelStore accepts maximum 8 bytes alignment
            break;
        case ETextureFormat_SRGB8:
            uploadParams.sizedInternalFormat = GL_SRGB8;
            uploadParams.baseInternalFormat = GL_RGB;
            uploadParams.type = GL_UNSIGNED_BYTE;
            uploadParams.byteAlignment = 1u;
            break;
        case ETextureFormat_SRGB8_ALPHA8:
            uploadParams.sizedInternalFormat = GL_SRGB8_ALPHA8;
            uploadParams.baseInternalFormat = GL_RGBA;
            uploadParams.type = GL_UNSIGNED_BYTE;
            uploadParams.byteAlignment = 4u;
            break;

        case ETextureFormat_ETC2RGB:
            uploadParams.compressed = true;
            uploadParams.sizedInternalFormat = GL_COMPRESSED_RGB8_ETC2;
            uploadParams.baseInternalFormat = GL_RGB;
            uploadParams.byteAlignment = 8u;
            break;
        case ETextureFormat_ETC2RGBA:
            uploadParams.compressed = true;
            uploadParams.sizedInternalFormat = GL_COMPRESSED_RGBA8_ETC2_EAC;
            uploadParams.baseInternalFormat = GL_RGBA;
            uploadParams.byteAlignment = 8u;
            break;
            // TODO Violin remove redundant list of ASTC enums from this switch and below GetASTCFormat() method
        case ETextureFormat_ASTC_RGBA_4x4:
        case ETextureFormat_ASTC_RGBA_5x4:
        case ETextureFormat_ASTC_RGBA_5x5:
        case ETextureFormat_ASTC_RGBA_6x5:
        case ETextureFormat_ASTC_RGBA_6x6:
        case ETextureFormat_ASTC_RGBA_8x5:
        case ETextureFormat_ASTC_RGBA_8x6:
        case ETextureFormat_ASTC_RGBA_8x8:
        case ETextureFormat_ASTC_RGBA_10x5:
        case ETextureFormat_ASTC_RGBA_10x6:
        case ETextureFormat_ASTC_RGBA_10x8:
        case ETextureFormat_ASTC_RGBA_10x10:
        case ETextureFormat_ASTC_RGBA_12x10:
        case ETextureFormat_ASTC_RGBA_12x12:
        case ETextureFormat_ASTC_SRGBA_4x4:
        case ETextureFormat_ASTC_SRGBA_5x4:
        case ETextureFormat_ASTC_SRGBA_5x5:
        case ETextureFormat_ASTC_SRGBA_6x5:
        case ETextureFormat_ASTC_SRGBA_6x6:
        case ETextureFormat_ASTC_SRGBA_8x5:
        case ETextureFormat_ASTC_SRGBA_8x6:
        case ETextureFormat_ASTC_SRGBA_8x8:
        case ETextureFormat_ASTC_SRGBA_10x5:
        case ETextureFormat_ASTC_SRGBA_10x6:
        case ETextureFormat_ASTC_SRGBA_10x8:
        case ETextureFormat_ASTC_SRGBA_10x10:
        case ETextureFormat_ASTC_SRGBA_12x10:
        case ETextureFormat_ASTC_SRGBA_12x12:
            uploadParams.compressed = true;
            uploadParams.sizedInternalFormat = GetASTCFormat(format);
            uploadParams.baseInternalFormat = GL_RGBA; // not used, but to be consistent with the other formats
            uploadParams.byteAlignment = 8u; // actually 16 bytes but glPixelStore accepts maximum 8 bytes alignment
            break;
        case ETextureFormat_Depth16:
            uploadParams.sizedInternalFormat = GL_DEPTH_COMPONENT16;
            uploadParams.baseInternalFormat = GL_DEPTH_COMPONENT;
            uploadParams.byteAlignment = 2u;
            break;
        case ETextureFormat_Depth24:
            uploadParams.sizedInternalFormat = GL_DEPTH_COMPONENT24;
            uploadParams.baseInternalFormat = GL_DEPTH_COMPONENT;
            uploadParams.byteAlignment = 1u;
            break;
        case ETextureFormat_Depth24_Stencil8:
            uploadParams.sizedInternalFormat = GL_DEPTH24_STENCIL8;
            uploadParams.baseInternalFormat = GL_DEPTH_STENCIL;
            uploadParams.byteAlignment = 4u;
            break;
        default:
            assert(false);
            break;
        }

        return uploadParams;
    }

    GLenum TypesConversion_GL::GetDrawMode(EDrawMode mode)
    {
        switch (mode)
        {
        case EDrawMode::Points:
            return GL_POINTS;
        case EDrawMode::Lines:
            return GL_LINES;
        case EDrawMode::LineLoop:
            return GL_LINE_LOOP;
        case EDrawMode::Triangles:
            return GL_TRIANGLES;
        case EDrawMode::TriangleStrip:
            return GL_TRIANGLE_STRIP;
        default:
            assert(false && "Invalid draw mode");
            return GL_ZERO;
        }
    }

    GLenum TypesConversion_GL::GetIndexElementType(UInt32 indexElementSizeInBytes)
    {
        return (indexElementSizeInBytes == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT);
    }

    GLenum TypesConversion_GL::GetDepthFunc(EDepthFunc func)
    {
        switch (func)
        {
        case EDepthFunc::Greater:
            return GL_GREATER;
        case EDepthFunc::GreaterEqual:
            return GL_GEQUAL;
        case EDepthFunc::Smaller:
            return GL_LESS;
        case EDepthFunc::SmallerEqual:
            return GL_LEQUAL;
        case EDepthFunc::Equal:
            return GL_EQUAL;
        case EDepthFunc::NotEqual:
            return GL_NOTEQUAL;
        case EDepthFunc::AlwaysPass:
            return GL_ALWAYS;
        case EDepthFunc::NeverPass:
            return GL_NEVER;
        case EDepthFunc::Disabled:
        default:
            assert(false && "Invalid depth function");
            return GL_NEVER;
        }
    }

    GLenum TypesConversion_GL::GetBlendFactor(EBlendFactor factor)
    {
        switch (factor)
        {
        case EBlendFactor::One:
            return GL_ONE;
        case EBlendFactor::Zero:
            return GL_ZERO;
        case EBlendFactor::OneMinusSrcAlpha:
            return GL_ONE_MINUS_SRC_ALPHA;
        case EBlendFactor::SrcAlpha:
            return GL_SRC_ALPHA;
        case EBlendFactor::DstAlpha:
            return GL_DST_ALPHA;
        case EBlendFactor::OneMinusDstAlpha:
            return GL_ONE_MINUS_DST_ALPHA;
        default:
            assert(false && "Invalid blend factor");
            return GL_ZERO;
        }
    }

    GLenum TypesConversion_GL::GetBlendOperation(EBlendOperation operation)
    {
        switch (operation)
        {
        case EBlendOperation::Add:
            return GL_FUNC_ADD;
        case EBlendOperation::Subtract:
            return GL_FUNC_SUBTRACT;
        case EBlendOperation::ReverseSubtract:
            return GL_FUNC_REVERSE_SUBTRACT;
        case EBlendOperation::Min:
            return GL_MIN;
        case EBlendOperation::Max:
            return GL_MAX;
        case EBlendOperation::Disabled:
        default:
            assert(false && "Invalid blend operation");
            return GL_ZERO;
        }
    }

    GLenum TypesConversion_GL::GetWrapMode(EWrapMethod wrapMode)
    {
        switch (wrapMode)
        {
        case EWrapMethod::Repeat:
            return GL_REPEAT;
        case EWrapMethod::Clamp:
            return GL_CLAMP_TO_EDGE;
        case EWrapMethod::RepeatMirrored:
            return GL_MIRRORED_REPEAT;
        default:
            assert(false && "Invalid wrap mode");
            return GL_REPEAT;
        }
    }

    GLenum TypesConversion_GL::GetStencilFunc(EStencilFunc func)
    {
        switch (func)
        {
        case EStencilFunc::NeverPass:
            return GL_NEVER;
        case EStencilFunc::Equal:
            return GL_EQUAL;
        case EStencilFunc::NotEqual:
            return GL_NOTEQUAL;
        case EStencilFunc::Less:
            return GL_LESS;
        case EStencilFunc::LessEqual:
            return GL_LEQUAL;
        case EStencilFunc::GreaterEqual:
            return GL_GEQUAL;
        case EStencilFunc::Greater:
            return GL_GREATER;
        case EStencilFunc::AlwaysPass:
            return GL_ALWAYS;
        case EStencilFunc::Disabled:
        default:
            assert(false && "Invalid stencil function");
            return GL_NEVER;
        }
    }

    GLenum TypesConversion_GL::GetStencilOperation(EStencilOp op)
    {
        switch (op)
        {
        case EStencilOp::Zero:
            return GL_ZERO;
        case EStencilOp::Replace:
            return GL_REPLACE;
        case EStencilOp::Increment:
            return GL_INCR;
        case EStencilOp::IncrementWrap:
            return GL_INCR_WRAP;
        case EStencilOp::Decrement:
            return GL_DECR;
        case EStencilOp::DecrementWrap:
            return GL_DECR_WRAP;
        case EStencilOp::Invert:
            return GL_INVERT;
        case EStencilOp::Keep:
            return GL_KEEP;
        default:
            assert(false && "Invalid stencil operation");
            return GL_ZERO;
        }
    }

    GLenum TypesConversion_GL::GetCullMode(ECullMode mode)
    {
        switch (mode)
        {
        case ECullMode::FrontFacing:
            return GL_FRONT;
        case ECullMode::BackFacing:
            return GL_BACK;
        case ECullMode::FrontAndBackFacing:
            return GL_FRONT_AND_BACK;
        case ECullMode::Disabled:
        default:
            assert(false && "Invalid cull mode");
            return GL_ZERO;
        }
    }

    GLenum TypesConversion_GL::GetTextureTargetFromTextureInputType(EEffectInputTextureType textureType)
    {
        switch (textureType)
        {
        case EEffectInputTextureType_TextureCube:
            return GL_TEXTURE_CUBE_MAP;
        case EEffectInputTextureType_Texture2D:
            return GL_TEXTURE_2D;
        case EEffectInputTextureType_Texture3D:
            return GL_TEXTURE_3D;
        default:
            assert("Unsupported texture target!" && false);
            return GL_TEXTURE_2D;
        }
    }

    GLenum TypesConversion_GL::GetCubemapFaceSpecifier(ETextureCubeFace face)
    {
        switch (face)
        {
        case ETextureCubeFace_PositiveX:
            return GL_TEXTURE_CUBE_MAP_POSITIVE_X;
        case ETextureCubeFace_NegativeX:
            return GL_TEXTURE_CUBE_MAP_NEGATIVE_X;
        case ETextureCubeFace_PositiveY:
            return GL_TEXTURE_CUBE_MAP_POSITIVE_Y;
        case ETextureCubeFace_NegativeY:
            return GL_TEXTURE_CUBE_MAP_NEGATIVE_Y;
        case ETextureCubeFace_PositiveZ:
            return GL_TEXTURE_CUBE_MAP_POSITIVE_Z;
        case ETextureCubeFace_NegativeZ:
            return GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
        default:
            assert(false);
            return 0;
        }
    }

    GLenum TypesConversion_GL::GetASTCFormat(ETextureFormat ramsesEnum)
    {
        switch (ramsesEnum)
        {
        case ETextureFormat_ASTC_RGBA_4x4:
            return GL_COMPRESSED_RGBA_ASTC_4x4_KHR;
        case ETextureFormat_ASTC_RGBA_5x4:
            return GL_COMPRESSED_RGBA_ASTC_5x4_KHR;
        case ETextureFormat_ASTC_RGBA_5x5:
            return GL_COMPRESSED_RGBA_ASTC_5x5_KHR;
        case ETextureFormat_ASTC_RGBA_6x5:
            return GL_COMPRESSED_RGBA_ASTC_6x5_KHR;
        case ETextureFormat_ASTC_RGBA_6x6:
            return GL_COMPRESSED_RGBA_ASTC_6x6_KHR;
        case ETextureFormat_ASTC_RGBA_8x5:
            return GL_COMPRESSED_RGBA_ASTC_8x5_KHR;
        case ETextureFormat_ASTC_RGBA_8x6:
            return GL_COMPRESSED_RGBA_ASTC_8x6_KHR;
        case ETextureFormat_ASTC_RGBA_8x8:
            return GL_COMPRESSED_RGBA_ASTC_8x8_KHR;
        case ETextureFormat_ASTC_RGBA_10x5:
            return GL_COMPRESSED_RGBA_ASTC_10x5_KHR;
        case ETextureFormat_ASTC_RGBA_10x6:
            return GL_COMPRESSED_RGBA_ASTC_10x6_KHR;
        case ETextureFormat_ASTC_RGBA_10x8:
            return GL_COMPRESSED_RGBA_ASTC_10x8_KHR;
        case ETextureFormat_ASTC_RGBA_10x10:
            return GL_COMPRESSED_RGBA_ASTC_10x10_KHR;
        case ETextureFormat_ASTC_RGBA_12x10:
            return GL_COMPRESSED_RGBA_ASTC_12x10_KHR;
        case ETextureFormat_ASTC_RGBA_12x12:
            return GL_COMPRESSED_RGBA_ASTC_12x12_KHR;
        case ETextureFormat_ASTC_SRGBA_4x4:
            return GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR;
        case ETextureFormat_ASTC_SRGBA_5x4:
            return GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR;
        case ETextureFormat_ASTC_SRGBA_5x5:
            return GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR;
        case ETextureFormat_ASTC_SRGBA_6x5:
            return GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR;
        case ETextureFormat_ASTC_SRGBA_6x6:
            return GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR;
        case ETextureFormat_ASTC_SRGBA_8x5:
            return GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR;
        case ETextureFormat_ASTC_SRGBA_8x6:
            return GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR;
        case ETextureFormat_ASTC_SRGBA_8x8:
            return GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR;
        case ETextureFormat_ASTC_SRGBA_10x5:
            return GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR;
        case ETextureFormat_ASTC_SRGBA_10x6:
            return GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR;
        case ETextureFormat_ASTC_SRGBA_10x8:
            return GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR;
        case ETextureFormat_ASTC_SRGBA_10x10:
            return GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR;
        case ETextureFormat_ASTC_SRGBA_12x10:
            return GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR;
        case ETextureFormat_ASTC_SRGBA_12x12:
            return GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR;
        default:
            assert(false && "Using unsupported enum in GetASTCFormat()");
            return GL_ZERO;
        }
    }

    ETextureFormat TypesConversion_GL::GetTextureFormatFromCompressedGLTextureFormat(GLenum compressedGLTextureFormat)
    {
        switch (compressedGLTextureFormat)
        {
        case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
            return ETextureFormat_DXT1RGB;
        case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
            return ETextureFormat_DXT3RGBA;
        case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
            return ETextureFormat_DXT5RGBA;
        case GL_COMPRESSED_RGBA_ASTC_4x4_KHR:
            return ETextureFormat_ASTC_RGBA_4x4;
        case GL_COMPRESSED_RGBA_ASTC_5x4_KHR:
            return ETextureFormat_ASTC_RGBA_5x4;
        case GL_COMPRESSED_RGBA_ASTC_5x5_KHR:
            return ETextureFormat_ASTC_RGBA_5x5;
        case GL_COMPRESSED_RGBA_ASTC_6x5_KHR:
            return ETextureFormat_ASTC_RGBA_6x5;
        case GL_COMPRESSED_RGBA_ASTC_6x6_KHR:
            return ETextureFormat_ASTC_RGBA_6x6;
        case GL_COMPRESSED_RGBA_ASTC_8x5_KHR:
            return ETextureFormat_ASTC_RGBA_8x5;
        case GL_COMPRESSED_RGBA_ASTC_8x6_KHR:
            return ETextureFormat_ASTC_RGBA_8x6;
        case GL_COMPRESSED_RGBA_ASTC_8x8_KHR:
            return ETextureFormat_ASTC_RGBA_8x8;
        case GL_COMPRESSED_RGBA_ASTC_10x5_KHR:
            return ETextureFormat_ASTC_RGBA_10x5;
        case GL_COMPRESSED_RGBA_ASTC_10x6_KHR:
            return ETextureFormat_ASTC_RGBA_10x6;
        case GL_COMPRESSED_RGBA_ASTC_10x8_KHR:
            return ETextureFormat_ASTC_RGBA_10x8;
        case GL_COMPRESSED_RGBA_ASTC_10x10_KHR:
            return ETextureFormat_ASTC_RGBA_10x10;
        case GL_COMPRESSED_RGBA_ASTC_12x10_KHR:
            return ETextureFormat_ASTC_RGBA_12x10;
        case GL_COMPRESSED_RGBA_ASTC_12x12_KHR:
            return ETextureFormat_ASTC_RGBA_12x12;
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR:
            return ETextureFormat_ASTC_SRGBA_4x4;
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR:
            return ETextureFormat_ASTC_SRGBA_5x4;
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR:
            return ETextureFormat_ASTC_SRGBA_5x5;
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR:
            return ETextureFormat_ASTC_SRGBA_6x5;
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR:
            return ETextureFormat_ASTC_SRGBA_6x6;
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR:
            return ETextureFormat_ASTC_SRGBA_8x5;
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR:
            return ETextureFormat_ASTC_SRGBA_8x6;
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR:
            return ETextureFormat_ASTC_SRGBA_8x8;
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR:
            return ETextureFormat_ASTC_SRGBA_10x5;
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR:
            return ETextureFormat_ASTC_SRGBA_10x6;
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR:
            return ETextureFormat_ASTC_SRGBA_10x8;
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR:
            return ETextureFormat_ASTC_SRGBA_10x10;
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR:
            return ETextureFormat_ASTC_SRGBA_12x10;
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR:
            return ETextureFormat_ASTC_SRGBA_12x12;
        case GL_COMPRESSED_RGB8_ETC2:
            return ETextureFormat_ETC2RGB;
        case GL_COMPRESSED_RGBA8_ETC2_EAC:
            return ETextureFormat_ETC2RGBA;
        default:
            return ETextureFormat_Invalid;
        }
    }
}
