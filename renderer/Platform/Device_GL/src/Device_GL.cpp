//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Device_GL/Device_GL.h"

#include "Platform_Base/RenderTargetGpuResource.h"
#include "Platform_Base/RenderBufferGPUResource.h"
#include "Platform_Base/IndexBufferGPUResource.h"
#include "Platform_Base/VertexBufferGPUResource.h"
#include "Platform_Base/TextureSamplerGPUResource.h"

#include "Device_GL/Device_GL_platform.h"
#include "Device_GL/ShaderGPUResource_GL.h"
#include "Device_GL/ShaderUploader_GL.h"
#include "Device_GL/ShaderProgramInfo.h"
#include "Device_GL/TypesConversion_GL.h"

#include "SceneAPI/PixelRectangle.h"
#include "RendererAPI/IContext.h"
#include "SceneAPI/RenderBuffer.h"
#include "RendererLib/TextureData.h"

#include "Math3d/Vector2.h"
#include "Math3d/Vector3.h"
#include "Math3d/Vector4.h"
#include "Math3d/Vector2i.h"
#include "Math3d/Vector3i.h"
#include "Math3d/Vector4i.h"
#include "Math3d/Matrix22f.h"
#include "Math3d/Matrix33f.h"
#include "Math3d/Matrix44f.h"

#include "Utils/LogMacros.h"
#include "Utils/TextureMathUtils.h"
#include "PlatformAbstraction/PlatformStringUtils.h"

#include "Platform_Base/GpuResource.h"
#include "SceneAPI/TextureEnums.h"

namespace ramses_internal
{
    // TODO Violin move again to other files, once GL headers are consolidated
    struct GLTextureInfo
    {
        GLenum target = GL_ZERO;
        UInt32 width = 0u;
        UInt32 height = 0u;
        UInt32 depth = 0u;
        TextureUploadParams_GL uploadParams;
    };

    // TODO Violin move again to other files, once GL headers are consolidated
    class TextureGPUResource_GL : public GPUResource
    {
    public:
        TextureGPUResource_GL(const GLTextureInfo& textureInfo, UInt32 gpuAddress, UInt32 dataSizeInBytes)
            : GPUResource(gpuAddress, dataSizeInBytes)
            , m_textureInfo(textureInfo)
        {

        }
        const GLTextureInfo m_textureInfo;
    };

    Device_GL::Device_GL(IContext& context, UInt8 majorApiVersion, UInt8 minorApiVersion, bool isEmbedded)
        : Device_Base()
        , m_context(context)
        , m_resourceMapper(context.getResources())
        , m_activeShader(0)
        , m_activePrimitiveDrawMode(EDrawMode::Triangles)
        , m_activeIndexArrayElementSizeBytes(2u)
        , m_majorApiVersion(majorApiVersion)
        , m_minorApiVersion(minorApiVersion)
        , m_isEmbedded(isEmbedded)
        , m_debugOutput()
    {
#if defined _DEBUG
        m_debugOutput.enable(context);
#endif

        m_limits.addTextureFormat(ETextureFormat_Depth16);
        m_limits.addTextureFormat(ETextureFormat_Depth24);
        m_limits.addTextureFormat(ETextureFormat_Depth24_Stencil8);

        m_limits.addTextureFormat(ETextureFormat_RGBA8);
        m_limits.addTextureFormat(ETextureFormat_RGB8);
        m_limits.addTextureFormat(ETextureFormat_RGBA5551);
        m_limits.addTextureFormat(ETextureFormat_RGBA4);
        m_limits.addTextureFormat(ETextureFormat_RGB565);

        m_limits.addTextureFormat(ETextureFormat_R8);
        m_limits.addTextureFormat(ETextureFormat_R16);
        m_limits.addTextureFormat(ETextureFormat_RG8);
        m_limits.addTextureFormat(ETextureFormat_RG16);
        m_limits.addTextureFormat(ETextureFormat_RGB16);
        m_limits.addTextureFormat(ETextureFormat_RGBA16);

        m_limits.addTextureFormat(ETextureFormat_R16F);
        m_limits.addTextureFormat(ETextureFormat_R32F);
        m_limits.addTextureFormat(ETextureFormat_RG16F);
        m_limits.addTextureFormat(ETextureFormat_RG32F);
        m_limits.addTextureFormat(ETextureFormat_RGB16F);
        m_limits.addTextureFormat(ETextureFormat_RGB32F);
        m_limits.addTextureFormat(ETextureFormat_RGBA16F);
        m_limits.addTextureFormat(ETextureFormat_RGBA32F);

        m_limits.addTextureFormat(ETextureFormat_SRGB8);
        m_limits.addTextureFormat(ETextureFormat_SRGB8_ALPHA8);

        m_limits.addTextureFormat(ETextureFormat_BGR8);
        m_limits.addTextureFormat(ETextureFormat_BGRA8);
    }

    Device_GL::~Device_GL()
    {
        m_resourceMapper.deleteResource(m_framebufferRenderTarget);
    }

    Bool Device_GL::init()
    {
        LOG_DEBUG(CONTEXT_RENDERER, "Device_GL::init:");

        LOAD_ALL_API_PROCS;

        const Char* tmp = NULL;

        tmp = reinterpret_cast<const Char*>(glGetString(GL_VENDOR));
        LOG_INFO(CONTEXT_RENDERER, "Device_GL::init:  OpenGL vendor is " << tmp);

        tmp = reinterpret_cast<const Char*>(glGetString(GL_RENDERER));
        LOG_INFO(CONTEXT_RENDERER, "    OpenGL renderer is " << tmp);

        tmp = reinterpret_cast<const Char*>(glGetString(GL_VERSION));
        LOG_INFO(CONTEXT_RENDERER, "     OpenGL version is " << tmp);

        tmp = reinterpret_cast<const Char*>(glGetString(GL_SHADING_LANGUAGE_VERSION));
        LOG_INFO(CONTEXT_RENDERER, "     GLSL version " << tmp);

        loadOpenGLExtensions();
        loadExtensionDependentFeatures();

        const RenderTargetGPUResource& framebufferRenderTarget = *new RenderTargetGPUResource(0);
        m_framebufferRenderTarget = m_resourceMapper.registerResource(framebufferRenderTarget);

// This is required for proper smoothing of cube sides. This feature is enabled by default on ES 3.0,
// but needs explicit enabling for Desktop GL
#ifdef DESKTOP_GL
        if (!m_isEmbedded)
        {
            glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
        }
#endif

        m_limits.logLimits();

        return true;
    }

    EDeviceTypeId Device_GL::getDeviceTypeId() const
    {
        if (m_majorApiVersion == 3 && m_minorApiVersion == 0 && m_isEmbedded)
            return EDeviceTypeId_GL_ES_3_0;
        else if (m_majorApiVersion == 4 && m_minorApiVersion == 2 && !m_isEmbedded)
            return EDeviceTypeId_GL_4_2_CORE;
        else if (m_majorApiVersion == 4 && m_minorApiVersion == 5 && !m_isEmbedded)
            return EDeviceTypeId_GL_4_5;
        else
            return EDeviceTypeId_INVALID;
    }

    void Device_GL::drawIndexedTriangles(Int32 startOffset, Int32 elementCount, UInt32 instanceCount)
    {
        const UInt startOffsetAddressAsUInt = startOffset * m_activeIndexArrayElementSizeBytes;
        const GLvoid* startOffsetAddress = reinterpret_cast<void*>(startOffsetAddressAsUInt);

        const GLenum drawModeGL = TypesConversion_GL::GetDrawMode(m_activePrimitiveDrawMode);
        const GLenum elementTypeGL = TypesConversion_GL::GetIndexElementType(m_activeIndexArrayElementSizeBytes);
        if (instanceCount > 1u)
        {
            glDrawElementsInstanced(drawModeGL, elementCount, elementTypeGL, startOffsetAddress, static_cast<GLsizei>(instanceCount));
        }
        else
        {
            glDrawElements(drawModeGL, elementCount, elementTypeGL, startOffsetAddress);
        }

        // For profiling/tests
        Device_Base::drawIndexedTriangles(startOffset, elementCount, instanceCount);
    }

    void Device_GL::drawTriangles(Int32 startOffset, Int32 elementCount, UInt32 instanceCount)
    {
        const GLenum drawModeGL = TypesConversion_GL::GetDrawMode(m_activePrimitiveDrawMode);
        if (instanceCount > 1u)
        {
            glDrawArraysInstanced(drawModeGL, startOffset, elementCount, static_cast<GLsizei>(instanceCount));
        }
        else
        {
            glDrawArrays(drawModeGL, startOffset, elementCount);
        }

        // For profiling/tests
        Device_Base::drawTriangles(startOffset, elementCount, instanceCount);
    }

    void Device_GL::clear(UInt32 clearFlags)
    {
        GLbitfield deviceClearFlags = 0;
        if (clearFlags & EClearFlags_Color)
        {
            deviceClearFlags |= GL_COLOR_BUFFER_BIT;
        }
        if (clearFlags & EClearFlags_Depth)
        {
            deviceClearFlags |= GL_DEPTH_BUFFER_BIT;
        }
        if (clearFlags & EClearFlags_Stencil)
        {
            deviceClearFlags |= GL_STENCIL_BUFFER_BIT;
        }

        glClear(deviceClearFlags);
    }

    void Device_GL::clearColor(const Vector4& clearColor)
    {
        glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
    }

    void Device_GL::colorMask(Bool r, Bool g, Bool b, Bool a)
    {
        glColorMask(r, g, b, a);
    }

    void Device_GL::clearDepth(Float d)
    {
        glClearDepthf(d);
    }

    void Device_GL::clearStencil(Int32 s)
    {
        glClearStencil(s);
    }

    void Device_GL::depthFunc(EDepthFunc func)
    {
        if (func == EDepthFunc::Disabled)
        {
            glDisable(GL_DEPTH_TEST);
        }
        else
        {
            glEnable(GL_DEPTH_TEST);
            const GLenum depthFuncGL = TypesConversion_GL::GetDepthFunc(func);
            glDepthFunc(depthFuncGL);
        }
    }

    void Device_GL::depthWrite(EDepthWrite flag)
    {
        glDepthMask(flag == EDepthWrite::Enabled);
    }

    void Device_GL::scissorTest(EScissorTest state, const RenderState::ScissorRegion& region)
    {
        if (state == EScissorTest::Disabled)
        {
            glDisable(GL_SCISSOR_TEST);
        }
        else
        {
            glEnable(GL_SCISSOR_TEST);
            glScissor(region.x, region.y, region.width, region.height);
        }
    }

    void Device_GL::blendFactors(EBlendFactor sourceColor, EBlendFactor destinationColor, EBlendFactor sourceAlpha, EBlendFactor destinationAlpha)
    {
        const GLenum glSourceColor = TypesConversion_GL::GetBlendFactor(sourceColor);
        const GLenum glDestinationColor = TypesConversion_GL::GetBlendFactor(destinationColor);
        const GLenum glSourceAlpha = TypesConversion_GL::GetBlendFactor(sourceAlpha);
        const GLenum glDestinationAlpha = TypesConversion_GL::GetBlendFactor(destinationAlpha);

        glBlendFuncSeparate(glSourceColor, glDestinationColor, glSourceAlpha, glDestinationAlpha);
    }

    void Device_GL::blendOperations(EBlendOperation operationColor, EBlendOperation operationAlpha)
    {
        if (operationColor == EBlendOperation::Disabled || operationAlpha == EBlendOperation::Disabled)
        {
            glDisable(GL_BLEND);
        }
        else
        {
            const GLenum glColorOperation = TypesConversion_GL::GetBlendOperation(operationColor);
            const GLenum glAlphaOperation = TypesConversion_GL::GetBlendOperation(operationAlpha);

            glEnable(GL_BLEND);
            glBlendEquationSeparate(glColorOperation, glAlphaOperation);
        }
    }

    void Device_GL::cullMode(ECullMode mode)
    {
        if (mode == ECullMode::Disabled)
        {
            glDisable(GL_CULL_FACE);
        }
        else
        {
            glEnable(GL_CULL_FACE);
            const GLenum cullModeGL = TypesConversion_GL::GetCullMode(mode);
            glCullFace(cullModeGL);
        }
    }

    void Device_GL::stencilFunc(EStencilFunc func, UInt8 ref, UInt8 mask)
    {
        if (func == EStencilFunc::Disabled)
        {
            glDisable(GL_STENCIL_TEST);
        }
        else
        {
            glEnable(GL_STENCIL_TEST);
            const GLenum stencilFuncGL = TypesConversion_GL::GetStencilFunc(func);
            glStencilFunc(stencilFuncGL, ref, mask);
        }
    }

    void Device_GL::stencilOp(EStencilOp sfail, EStencilOp dpfail, EStencilOp dppass)
    {
        const GLenum glSfail = TypesConversion_GL::GetStencilOperation(sfail);
        const GLenum glDpfail = TypesConversion_GL::GetStencilOperation(dpfail);
        const GLenum glDppass = TypesConversion_GL::GetStencilOperation(dppass);

        glStencilOp(glSfail, glDpfail, glDppass);
    }

    void Device_GL::drawMode(EDrawMode mode)
    {
        m_activePrimitiveDrawMode = mode;
    }

    void Device_GL::setViewport(UInt32 x, UInt32 y, UInt32 width, UInt32 height)
    {
        glViewport(x, y, width, height);
    }

    GLHandle Device_GL::createTexture(UInt32 width, UInt32 height, ETextureFormat storageFormat) const
    {
        LOG_DEBUG(CONTEXT_RENDERER, "Device_GL::createTexture:  creating a new texture (texture render target)");

        const GLHandle texID = generateAndBindTexture(GL_TEXTURE_2D);
        GLTextureInfo texInfo;
        fillGLInternalTextureInfo(GL_TEXTURE_2D, width, height, 1u, storageFormat, texInfo);
        allocateTextureStorage(texInfo, 1u);

        return texID;
    }

    GLHandle Device_GL::createRenderBuffer(UInt32 width, UInt32 height, ETextureFormat format, UInt32 sampleCount)
    {
        LOG_TRACE(CONTEXT_RENDERER, "Creating a new render buffer");

        GLHandle renderbufferHandle = InvalidGLHandle;
        glGenRenderbuffers(1, &renderbufferHandle);
        glBindRenderbuffer(GL_RENDERBUFFER, renderbufferHandle);

        GLenum internalFormat(0);
        switch (format)
        {
        case ETextureFormat_Depth24:
            internalFormat = GL_DEPTH_COMPONENT24;
            break;
        case ETextureFormat_Depth24_Stencil8:
            internalFormat = GL_DEPTH24_STENCIL8;
            break;
        case ETextureFormat_RGBA8:
            internalFormat = GL_RGBA8;
            break;
        default:
            assert(false && "Unknown render buffer format");
        }

        glRenderbufferStorageMultisample(GL_RENDERBUFFER, sampleCount, internalFormat, width, height);

        return renderbufferHandle;
    }

    Bool Device_GL::getUniformLocation(DataFieldHandle field, GLInputLocation& location) const
    {
        assert(0 != m_activeShader);
        location = m_activeShader->getUniformLocation(field);
        return location != GLInputLocationInvalid;
    }

    Bool Device_GL::getAttributeLocation(DataFieldHandle field, GLInputLocation& location) const
    {
        assert(0 != m_activeShader);
        location = m_activeShader->getAttributeLocation(field);
        return location != GLInputLocationInvalid;
    }

    void Device_GL::setConstant(DataFieldHandle field, UInt32 count, const Float* value)
    {
        GLInputLocation uniformLocation;
        if (getUniformLocation(field, uniformLocation))
        {
            assert(0 != value);
            glUniform1fv(uniformLocation.getValue(), count, value);
        }
    }

    void Device_GL::setConstant(DataFieldHandle field, UInt32 count, const Vector2* value)
    {
        GLInputLocation uniformLocation;
        if (getUniformLocation(field, uniformLocation))
        {
            assert(0 != value);
            glUniform2fv(uniformLocation.getValue(), count, value[0].data);
        }
    }

    void Device_GL::setConstant(DataFieldHandle field, UInt32 count, const Vector3* value)
    {
        GLInputLocation uniformLocation;
        if (getUniformLocation(field, uniformLocation))
        {
            assert(0 != value);
            glUniform3fv(uniformLocation.getValue(), count, value[0].data);
        }
    }

    void Device_GL::setConstant(DataFieldHandle field, UInt32 count, const Vector4* value)
    {
        GLInputLocation uniformLocation;
        if (getUniformLocation(field, uniformLocation))
        {
            assert(0 != value);
            glUniform4fv(uniformLocation.getValue(), count, value[0].data);
        }
    }

    void Device_GL::setConstant(DataFieldHandle field, UInt32 count, const Int32* value)
    {
        GLInputLocation uniformLocation;
        if (getUniformLocation(field, uniformLocation))
        {
            assert(0 != value);
            glUniform1iv(uniformLocation.getValue(), count, value);
        }
    }

    void Device_GL::setConstant(DataFieldHandle field, UInt32 count, const Vector2i* value)
    {
        GLInputLocation uniformLocation;
        if (getUniformLocation(field, uniformLocation))
        {
            assert(0 != value);
            glUniform2iv(uniformLocation.getValue(), count, value[0].data);
        }
    }

    void Device_GL::setConstant(DataFieldHandle field, UInt32 count, const Vector3i* value)
    {
        GLInputLocation uniformLocation;
        if (getUniformLocation(field, uniformLocation))
        {
            assert(0 != value);
            glUniform3iv(uniformLocation.getValue(), count, value[0].data);
        }
    }

    void Device_GL::setConstant(DataFieldHandle field, UInt32 count, const Vector4i* value)
    {
        GLInputLocation uniformLocation;
        if (getUniformLocation(field, uniformLocation))
        {
            assert(0 != value);
            glUniform4iv(uniformLocation.getValue(), count, value[0].data);
        }
    }

    void Device_GL::setConstant(DataFieldHandle field, UInt32 count, const Matrix22f* value)
    {
        GLInputLocation uniformLocation;
        if (getUniformLocation(field, uniformLocation))
        {
            assert(0 != value);
            glUniformMatrix2fv(uniformLocation.getValue(), count, false, value[0].data);
        }
    }

    void Device_GL::setConstant(DataFieldHandle field, UInt32 count, const Matrix33f* value)
    {
        GLInputLocation uniformLocation;
        if (getUniformLocation(field, uniformLocation))
        {
            assert(0 != value);
            glUniformMatrix3fv(uniformLocation.getValue(), count, false, value[0].data);
        }
    }

    void Device_GL::setConstant(DataFieldHandle field, UInt32 count, const Matrix44f* value)
    {
        GLInputLocation uniformLocation;
        if (getUniformLocation(field, uniformLocation))
        {
            assert(0 != value);
            glUniformMatrix4fv(uniformLocation.getValue(), count, false, value[0].data);
        }
    }

    DeviceResourceHandle Device_GL::allocateTexture2D(UInt32 width, UInt32 height, ETextureFormat textureFormat, UInt32 mipLevelCount, UInt32 totalSizeInBytes)
    {
        const GLHandle texID = generateAndBindTexture(GL_TEXTURE_2D);
        GLTextureInfo texInfo;
        fillGLInternalTextureInfo(GL_TEXTURE_2D, width, height, 1u, textureFormat, texInfo);
        allocateTextureStorage(texInfo, mipLevelCount);

        const GPUResource& gpuResource = *new TextureGPUResource_GL(texInfo, texID, totalSizeInBytes);
        return m_resourceMapper.registerResource(gpuResource);
    }

    DeviceResourceHandle Device_GL::allocateTexture3D(UInt32 width, UInt32 height, UInt32 depth, ETextureFormat textureFormat, UInt32 mipLevelCount, UInt32 totalSizeInBytes)
    {
        const GLHandle texID = generateAndBindTexture(GL_TEXTURE_3D);
        GLTextureInfo texInfo;
        fillGLInternalTextureInfo(GL_TEXTURE_3D, width, height, depth, textureFormat, texInfo);
        allocateTextureStorage(texInfo, mipLevelCount);

        const GPUResource& gpuResource = *new TextureGPUResource_GL(texInfo, texID, totalSizeInBytes);
        return m_resourceMapper.registerResource(gpuResource);
    }

    DeviceResourceHandle Device_GL::allocateTextureCube(UInt32 faceSize, ETextureFormat textureFormat, UInt32 mipLevelCount, UInt32 totalSizeInBytes)
    {
        const GLHandle texID = generateAndBindTexture(GL_TEXTURE_CUBE_MAP);
        GLTextureInfo texInfo;
        fillGLInternalTextureInfo(GL_TEXTURE_CUBE_MAP, faceSize, faceSize, 1u, textureFormat, texInfo);
        allocateTextureStorage(texInfo, mipLevelCount);

        const GPUResource& gpuResource = *new TextureGPUResource_GL(texInfo, texID, totalSizeInBytes);
        return m_resourceMapper.registerResource(gpuResource);
    }

    void Device_GL::bindTexture(DeviceResourceHandle handle)
    {
        const TextureGPUResource_GL& gpuResource = m_resourceMapper.getResourceAs<TextureGPUResource_GL>(handle);
        glBindTexture(gpuResource.m_textureInfo.target, gpuResource.getGPUAddress());
    }

    void Device_GL::generateMipmaps(DeviceResourceHandle handle)
    {
        const TextureGPUResource_GL& gpuResource = m_resourceMapper.getResourceAs<TextureGPUResource_GL>(handle);
        glGenerateMipmap(gpuResource.m_textureInfo.target);
    }

    void Device_GL::uploadTextureData(DeviceResourceHandle handle, UInt32 mipLevel, UInt32 x, UInt32 y, UInt32 z, UInt32 width, UInt32 height, UInt32 depth, const Byte* data, UInt32 dataSize)
    {
        const TextureGPUResource_GL& gpuResource = m_resourceMapper.getResourceAs<TextureGPUResource_GL>(handle);

        GLTextureInfo texInfo = gpuResource.m_textureInfo;
        // in case of cube texture faceID is encoded in Z offset
        if (texInfo.target == GL_TEXTURE_CUBE_MAP)
        {
            texInfo.target = TypesConversion_GL::GetCubemapFaceSpecifier(static_cast<ETextureCubeFace>(z));
            z = 0u;
        }
        uploadTextureMipMapData(mipLevel, x, y, z, width, height, depth, texInfo, data, dataSize);
    }

    DeviceResourceHandle Device_GL::uploadStreamTexture2D(DeviceResourceHandle handle, UInt32 width, UInt32 height, ETextureFormat format, const UInt8* data)
    {
        if (!handle.isValid())
        {
            // generate texID and register texture resource
            assert(data == nullptr);
            GLHandle texID = InvalidGLHandle;
            glGenTextures(1, &texID);
            const GPUResource& gpuResource = *new GPUResource(texID, 0u);

            return m_resourceMapper.registerResource(gpuResource);
        }
        else
        {
            // upload data to registered texture resource
            const GLHandle texID = getTextureAddress(handle);
            assert(texID != InvalidGLHandle);
            assert(data != nullptr);
            LOG_DEBUG(CONTEXT_RENDERER, "Device_GL::uploadStreamTexture2D:  texid: " << texID << " width: " << width << " height: " << height << " format: " << EnumToString(format));

            glBindTexture(GL_TEXTURE_2D, texID);

            GLTextureInfo texInfo;
            fillGLInternalTextureInfo(GL_TEXTURE_2D, width, height, 1u, format, texInfo);
            assert(!texInfo.uploadParams.compressed);
            // For now stream texture upload is using glTexImage2D instead of glStore/glSubimage because its size/format cannot be immutable
            glTexImage2D(texInfo.target, 0, texInfo.uploadParams.sizedInternalFormat, texInfo.width, texInfo.height, 0, texInfo.uploadParams.baseInternalFormat, texInfo.uploadParams.type, data);

            return handle;
        }
    }

    void Device_GL::fillGLInternalTextureInfo(GLenum target, UInt32 width, UInt32 height, UInt32 depth, ETextureFormat textureFormat, GLTextureInfo& glTexInfoOut) const
    {
        glTexInfoOut.target = target;
        glTexInfoOut.width = width;
        glTexInfoOut.height = height;
        glTexInfoOut.depth = depth;

        if (!m_limits.isTextureFormatAvailable(textureFormat))
        {
            LOG_ERROR(CONTEXT_RENDERER, "Device_GL::createGLInternalTextureInfo: Unsupported texture format " << EnumToString(textureFormat));
            assert(false && "Device_GL::createGLInternalTextureInfo unsupported texture format");
        }
        glTexInfoOut.uploadParams = TypesConversion_GL::GetTextureUploadParams(textureFormat);

        if (glTexInfoOut.uploadParams.swizzleBGRXtoRGBX)
        {
            glTexParameteri(target, GL_TEXTURE_SWIZZLE_R, GL_BLUE);
            glTexParameteri(target, GL_TEXTURE_SWIZZLE_B, GL_RED);
        }
    }

    void Device_GL::allocateTextureStorage(const GLTextureInfo& texInfo, UInt32 mipLevels) const
    {
        glPixelStorei(GL_UNPACK_ALIGNMENT, texInfo.uploadParams.byteAlignment);

        switch (texInfo.target)
        {
        case GL_TEXTURE_2D:
        case GL_TEXTURE_CUBE_MAP:
            glTexStorage2D(texInfo.target, mipLevels, texInfo.uploadParams.sizedInternalFormat, texInfo.width, texInfo.height);
            break;
        case GL_TEXTURE_3D:
            glTexStorage3D(texInfo.target, mipLevels, texInfo.uploadParams.sizedInternalFormat, texInfo.width, texInfo.height, texInfo.depth);
            break;
        default:
            assert(false);
            break;
        }
    }

    void Device_GL::uploadTextureMipMapData(UInt32 mipLevel, UInt32 x, UInt32 y, UInt32 z, UInt32 width, UInt32 height, UInt32 depth, const GLTextureInfo& texInfo, const UInt8 *pData, UInt32 dataSize) const
    {
        assert(width > 0 && height > 0 && depth > 0 && "trying to upload texture with 0 width and/or height and/or depth!");
        assert(x + width <= TextureMathUtils::GetMipSize(mipLevel, texInfo.width));
        assert(y + height <= TextureMathUtils::GetMipSize(mipLevel, texInfo.height));
        assert(z + depth <= TextureMathUtils::GetMipSize(mipLevel, texInfo.depth));
        assert(dataSize > 0u || !texInfo.uploadParams.compressed);

        switch (texInfo.target)
        {
        case GL_TEXTURE_2D:
        case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
        case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
        case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
            if (texInfo.uploadParams.compressed)
            {
                glCompressedTexSubImage2D(texInfo.target, mipLevel, x, y, width, height, texInfo.uploadParams.sizedInternalFormat, dataSize, pData);
            }
            else
            {
                glTexSubImage2D(texInfo.target, mipLevel, x, y, width, height, texInfo.uploadParams.baseInternalFormat, texInfo.uploadParams.type, pData);
            }
            break;
        case GL_TEXTURE_3D:
            if (texInfo.uploadParams.compressed)
            {
                glCompressedTexSubImage3D(texInfo.target, mipLevel, x, y, z, width, height, depth, texInfo.uploadParams.sizedInternalFormat, dataSize, pData);
            }
            else
            {
                glTexSubImage3D(texInfo.target, mipLevel, x, y, z, width, height, depth, texInfo.uploadParams.baseInternalFormat, texInfo.uploadParams.type, pData);
            }
            break;
        default:
            assert(false);
            break;
        }
    }

    DeviceResourceHandle Device_GL::uploadRenderBuffer(const RenderBuffer& buffer)
    {
        GLHandle bufferGLHandle = InvalidGLHandle;
        switch (buffer.accessMode)
        {
        case ERenderBufferAccessMode_ReadWrite:
            assert(0u == buffer.sampleCount);
            bufferGLHandle = createTexture(buffer.width, buffer.height, buffer.format);
            break;
        case ERenderBufferAccessMode_WriteOnly:
            bufferGLHandle = createRenderBuffer(buffer.width, buffer.height, buffer.format, buffer.sampleCount);
            break;
        default:
            assert(false);
        }

        if (bufferGLHandle != InvalidGLHandle)
        {
            const GPUResource& bufferGPUResource = *new RenderBufferGPUResource(bufferGLHandle, buffer.width, buffer.height, buffer.type, buffer.format, buffer.accessMode);
            return m_resourceMapper.registerResource(bufferGPUResource);
        }

        return DeviceResourceHandle::Invalid();
    }

    void Device_GL::deleteRenderBuffer(DeviceResourceHandle bufferHandle)
    {
        const RenderBufferGPUResource& resource = m_resourceMapper.getResourceAs<RenderBufferGPUResource>(bufferHandle);
        const GLHandle glAddress = resource.getGPUAddress();
        if (ERenderBufferAccessMode_ReadWrite == resource.getAccessMode())
        {
            glDeleteTextures(1, &glAddress);
        }
        else if (ERenderBufferAccessMode_WriteOnly == resource.getAccessMode())
        {
            glDeleteRenderbuffers(1, &glAddress);
        }
        else
        {
            assert(false);
        }

        m_resourceMapper.deleteResource(bufferHandle);
    }

    DeviceResourceHandle Device_GL::uploadTextureSampler(EWrapMethod wrapU, EWrapMethod wrapV, EWrapMethod wrapR, ESamplingMethod minSampling, ESamplingMethod magSampling, UInt32 anisotropyLevel)
    {
        GLuint sampler;
        glGenSamplers(1, &sampler);

        setTextureFiltering(sampler, wrapU, wrapV, wrapR, minSampling, magSampling, anisotropyLevel);

        const GPUResource& textureSamplerGPUResource = *new TextureSamplerGPUResource(wrapU, wrapV, wrapR, minSampling, magSampling, anisotropyLevel, sampler, 0);
        return m_resourceMapper.registerResource(textureSamplerGPUResource);
    }

    void Device_GL::deleteTextureSampler(DeviceResourceHandle handle)
    {
        const TextureSamplerGPUResource& resource = m_resourceMapper.getResourceAs<TextureSamplerGPUResource>(handle);
        const GLHandle glAddress = resource.getGPUAddress();
        glDeleteSamplers(1, &glAddress);

        m_resourceMapper.deleteResource(handle);
    }

    void Device_GL::activateTextureSampler(DeviceResourceHandle handle, DataFieldHandle field)
    {
        const TextureSlot textureSlot = m_activeShader->getTextureSlot(field).slot;
        assert(static_cast<UInt32>(textureSlot) < m_limits.getMaximumTextureUnits());
        const GLHandle glAddress = m_resourceMapper.getResource(handle).getGPUAddress();
        glBindSampler(textureSlot, glAddress);
    }

    Bool Device_GL::allBuffersHaveTheSameSize(const DeviceHandleVector& renderBuffers) const
    {
        assert(!renderBuffers.empty());
        assert(renderBuffers.size() <= 16u);

        UInt32 width = 0u;
        UInt32 height = 0u;

        for (UInt i = 0; i < renderBuffers.size(); ++i)
        {
            const RenderBufferGPUResource& bufferGPUResource = m_resourceMapper.getResourceAs<RenderBufferGPUResource>(renderBuffers[i]);

            if (0 == i)
            {
                width = bufferGPUResource.getWidth();
                height = bufferGPUResource.getHeight();
            }
            else if (width != bufferGPUResource.getWidth() || height != bufferGPUResource.getHeight())
            {
                return false;
            }
        }

        return true;
    }

    DeviceResourceHandle Device_GL::uploadRenderTarget(const DeviceHandleVector& renderBuffers)
    {
        assert(allBuffersHaveTheSameSize(renderBuffers));

        GLHandle fboAddress = InvalidGLHandle;
        glGenFramebuffers(1, &fboAddress);
        assert(fboAddress != InvalidGLHandle);
        glBindFramebuffer(GL_FRAMEBUFFER, fboAddress);

        GLenum drawBuffers[16];
        UInt32 colorBufferSlot = 0;

        for (UInt i = 0; i < renderBuffers.size(); ++i)
        {
            const RenderBufferGPUResource& bufferGPUResource = m_resourceMapper.getResourceAs<RenderBufferGPUResource>(renderBuffers[i]);
            bindRenderBufferToRenderTarget(bufferGPUResource, colorBufferSlot);

            if (ERenderBufferType_ColorBuffer == bufferGPUResource.getType())
            {
                drawBuffers[colorBufferSlot] = GL_COLOR_ATTACHMENT0 + colorBufferSlot;
                colorBufferSlot++;
            }
        }

        // Always check that our framebuffer is ok
        const GLenum FBOstatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (FBOstatus != GL_FRAMEBUFFER_COMPLETE)
        {
            LOG_ERROR(CONTEXT_RENDERER, "Device_GL::createRenderTargetComponents Framebuffer status not complete! GL error code: " << FBOstatus);
            return DeviceResourceHandle::Invalid();
        }
        glDrawBuffers(colorBufferSlot, drawBuffers);

        const RenderTargetGPUResource& fboGpuResource = *new RenderTargetGPUResource(fboAddress);
        const DeviceResourceHandle fboHandle = m_resourceMapper.registerResource(fboGpuResource);
        return fboHandle;
    }

    void Device_GL::deleteRenderTarget(DeviceResourceHandle handle)
    {
        const RenderTargetGPUResource& rtResource = m_resourceMapper.getResourceAs<RenderTargetGPUResource>(handle);
        const GLHandle rtGlAddress = rtResource.getGPUAddress();
        assert(rtGlAddress != InvalidGLHandle);
        glDeleteFramebuffers(1, &rtGlAddress);

        m_resourceMapper.deleteResource(handle);
    }

    void Device_GL::bindRenderBufferToRenderTarget(const RenderBufferGPUResource& renderBufferGpuResource, const UInt32 colorBufferSlot)
    {
        switch (renderBufferGpuResource.getAccessMode())
        {
        case ramses_internal::ERenderBufferAccessMode_WriteOnly:
            bindWriteOnlyRenderBufferToRenderTarget(renderBufferGpuResource.getType(), colorBufferSlot, renderBufferGpuResource.getGPUAddress());
            break;
        case ramses_internal::ERenderBufferAccessMode_ReadWrite:
            bindReadWriteRenderBufferToRenderTarget(renderBufferGpuResource.getType(), colorBufferSlot, renderBufferGpuResource.getGPUAddress());
            break;
        default:
            assert(false && "invalid render buffer access mode");
            break;
        }
    }

    void Device_GL::bindReadWriteRenderBufferToRenderTarget(const ERenderBufferType bufferType, const UInt32 colorBufferSlot, const GLHandle bufferGLHandle)
    {
        switch (bufferType)
        {
        case ERenderBufferType_DepthBuffer:
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, bufferGLHandle, 0);
            break;
        case ERenderBufferType_DepthStencilBuffer:
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, bufferGLHandle, 0);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, bufferGLHandle, 0);
            break;
        case ERenderBufferType_ColorBuffer:
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + colorBufferSlot, GL_TEXTURE_2D, bufferGLHandle, 0);
            break;
        case ERenderBufferType_InvalidBuffer:
        default:
            assert(false && "invalid render buffer type");
            break;
        }
    }

    void Device_GL::bindWriteOnlyRenderBufferToRenderTarget(const ERenderBufferType bufferType, const UInt32 colorBufferSlot, const GLHandle bufferGLHandle)
    {
        switch (bufferType)
        {
        case ERenderBufferType_DepthBuffer:
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, bufferGLHandle);
            break;
        case ERenderBufferType_DepthStencilBuffer:
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, bufferGLHandle);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, bufferGLHandle);
            break;
        case ERenderBufferType_ColorBuffer:
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + colorBufferSlot, GL_RENDERBUFFER, bufferGLHandle);
            break;
        case ERenderBufferType_InvalidBuffer:
        default:
            assert(false && "invalid render buffer type");
            break;
        }
    }

    void Device_GL::activateRenderTarget(DeviceResourceHandle renderTarget)
    {
        const auto renderTargetPair = std::find_if(m_pairedRenderTargets.cbegin(), m_pairedRenderTargets.cend(), [renderTarget](const RenderTargetPair& rtPair) -> bool {return rtPair.renderTargets[0] == renderTarget; });

        const GPUResource* rtResource = nullptr;
        if (renderTargetPair != m_pairedRenderTargets.cend())
        {
            const UInt8 writingIndex = (renderTargetPair->readingIndex + 1) % 2;
            rtResource = &m_resourceMapper.getResource(renderTargetPair->renderTargets[writingIndex]);
        }
        else
        {
            rtResource = &m_resourceMapper.getResource(renderTarget);
        }

        const GLHandle rtGlAddress = rtResource->getGPUAddress();
        glBindFramebuffer(GL_FRAMEBUFFER, rtGlAddress);
    }

    void Device_GL::pairRenderTargetsForDoubleBuffering(DeviceResourceHandle renderTargets[2], DeviceResourceHandle colorBuffers[2])
    {
        m_pairedRenderTargets.push_back({ { renderTargets[0], renderTargets[1] },{ colorBuffers[0], colorBuffers[1] }, 0u });
    }

    void Device_GL::unpairRenderTargets(DeviceResourceHandle renderTarget)
    {
        auto renderTargetPair = std::find_if(m_pairedRenderTargets.begin(), m_pairedRenderTargets.end(), [renderTarget](const RenderTargetPair& rtPair) -> bool {return rtPair.renderTargets[0] == renderTarget; });
        assert(renderTargetPair != m_pairedRenderTargets.end());
        m_pairedRenderTargets.erase(renderTargetPair);
    }

    void Device_GL::swapDoubleBufferedRenderTarget(DeviceResourceHandle renderTarget)
    {
        auto renderTargetPair = std::find_if(m_pairedRenderTargets.begin(), m_pairedRenderTargets.end(), [renderTarget](const RenderTargetPair& rtPair) -> bool {return rtPair.renderTargets[0] == renderTarget; });
        assert(renderTargetPair != m_pairedRenderTargets.end());
        renderTargetPair->readingIndex = (renderTargetPair->readingIndex + 1) % 2;
    }

    void Device_GL::setTextureFiltering(GLenum target, EWrapMethod wrapU, EWrapMethod wrapV, EWrapMethod wrapR, ESamplingMethod minSampling, ESamplingMethod magSampling, UInt32 anisotropyLevel)
    {
        const GLenum wrappingModeR = TypesConversion_GL::GetWrapMode(wrapR);
        const GLenum wrappingModeU = TypesConversion_GL::GetWrapMode(wrapU);
        const GLenum wrappingModeV = TypesConversion_GL::GetWrapMode(wrapV);

        glTexParameteri(target, GL_TEXTURE_WRAP_S, wrappingModeU);
        glTexParameteri(target, GL_TEXTURE_WRAP_T, wrappingModeV);
        glTexParameteri(target, GL_TEXTURE_WRAP_R, wrappingModeR);

        switch (minSampling)
        {
        case ESamplingMethod::Nearest:
            glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            break;
        case ESamplingMethod::Linear:
            glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            break;
        case ESamplingMethod::Nearest_MipMapNearest:
            glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
            break;
        case ESamplingMethod::Nearest_MipMapLinear:
            glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
            break;
        case ESamplingMethod::Linear_MipMapNearest:
            glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
            break;
        case ESamplingMethod::Linear_MipMapLinear:
            glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            break;
        default:
            assert(false && "Unsupported texture sampling method");
            break;
        }

        switch (magSampling)
        {
        case ESamplingMethod::Nearest:
            glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            break;
        case ESamplingMethod::Linear:
            glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            break;
        default:
            assert(false && "Unsupported texture sampling method");
            break;
        }

        // set anisotropy only
        // - if not a 3D texture
        // - if feature is supported
        if ((target != GL_TEXTURE_3D) && m_limits.getMaximumAnisotropy() > 1u)
        {
            // clamp anisotropy value to max supported range
            anisotropyLevel = min(anisotropyLevel, m_limits.getMaximumAnisotropy());
            glTexParameteri(target, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropyLevel);
        }
    }

    GLHandle Device_GL::generateAndBindTexture(GLenum target) const
    {
        GLHandle texID = InvalidGLHandle;
        glGenTextures(1, &texID);
        assert(texID != InvalidGLHandle);
        glBindTexture(target, texID);

        return texID;
    }

    void Device_GL::setTextureSampling(DataFieldHandle field, EWrapMethod wrapU, EWrapMethod wrapV, EWrapMethod wrapR, ESamplingMethod minSampling, ESamplingMethod magSampling, UInt32 anisotropyLevel)
    {
        // TODO violin try to remove dependency of sampling state to uniform input. Idea: provide texture explicitly, or use more modern sampler objects
        const GLenum target = TypesConversion_GL::GetTextureTargetFromTextureInputType(m_activeShader->getTextureSlot(field).textureType);

        setTextureFiltering(target, wrapU, wrapV, wrapR, minSampling, magSampling, anisotropyLevel);
    }

    DeviceResourceHandle Device_GL::allocateVertexBuffer(EDataType dataType, UInt32 sizeInBytes)
    {
        GLHandle glAddress = InvalidGLHandle;
        glGenBuffers(1, &glAddress);
        assert(glAddress != InvalidGLHandle);

        return m_resourceMapper.registerResource(*new VertexBufferGPUResource(glAddress, sizeInBytes, EnumToNumComponents(dataType)));
    }

    void Device_GL::uploadVertexBufferData(DeviceResourceHandle handle, const Byte* data, UInt32 dataSize)
    {
        const auto& vertexBuffer = m_resourceMapper.getResource(handle);
        assert(dataSize <= vertexBuffer.getTotalSizeInBytes());

        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer.getGPUAddress());
        glBufferData(GL_ARRAY_BUFFER, dataSize, data, GL_STATIC_DRAW);
    }

    void Device_GL::deleteVertexBuffer(DeviceResourceHandle handle)
    {
        const GLHandle resourceAddress = m_resourceMapper.getResource(handle).getGPUAddress();
        glDeleteBuffers(1, &resourceAddress);
        m_resourceMapper.deleteResource(handle);
    }

    void Device_GL::activateVertexBuffer(DeviceResourceHandle handle, DataFieldHandle field, UInt32 instancingDivisor)
    {
        GLInputLocation vertexInputAddress;
        if (getAttributeLocation(field, vertexInputAddress))
        {
            assert(m_activeShader != NULL);
            const VertexBufferGPUResource& arrayResource = m_resourceMapper.getResourceAs<VertexBufferGPUResource>(handle);

            glBindBuffer(GL_ARRAY_BUFFER, arrayResource.getGPUAddress());
            glEnableVertexAttribArray(vertexInputAddress.getValue());

            glVertexAttribPointer(vertexInputAddress.getValue(), arrayResource.getNumComponentsPerElement(), GL_FLOAT, GL_FALSE, 0, NULL);

            glVertexAttribDivisor(vertexInputAddress.getValue(), instancingDivisor);
        }
    }

    DeviceResourceHandle Device_GL::allocateIndexBuffer(EDataType dataType, UInt32 sizeInBytes)
    {
        GLHandle glAddress = InvalidGLHandle;
        glGenBuffers(1, &glAddress);
        assert(glAddress != InvalidGLHandle);
        assert(dataType == EDataType_UInt16 || dataType == EDataType_UInt32);

        return m_resourceMapper.registerResource(*new IndexBufferGPUResource(glAddress, sizeInBytes, dataType == EDataType_UInt16 ? 2 : 4));
    }

    void Device_GL::uploadIndexBufferData(DeviceResourceHandle handle, const Byte* data, UInt32 dataSize)
    {
        const auto& indexBuffer = m_resourceMapper.getResource(handle);
        assert(dataSize <= indexBuffer.getTotalSizeInBytes());

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer.getGPUAddress());
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, dataSize, data, GL_STATIC_DRAW);
    }

    void Device_GL::deleteIndexBuffer(DeviceResourceHandle handle)
    {
        const GLHandle resourceAddress = m_resourceMapper.getResource(handle).getGPUAddress();
        glDeleteBuffers(1, &resourceAddress);
        m_resourceMapper.deleteResource(handle);
    }

    void Device_GL::activateIndexBuffer(DeviceResourceHandle handle)
    {
        const IndexBufferGPUResource& indexBufferGPUResource = m_resourceMapper.getResourceAs<IndexBufferGPUResource>(handle);
        const GLHandle resourceAddress = indexBufferGPUResource.getGPUAddress();
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, resourceAddress);

        m_activeIndexArrayElementSizeBytes = indexBufferGPUResource.getElementSizeInBytes();
        assert(m_activeIndexArrayElementSizeBytes == 2 || m_activeIndexArrayElementSizeBytes == 4);
    }

    DeviceResourceHandle Device_GL::uploadShader(const EffectResource& effect)
    {
        ShaderProgramInfo programInfo;
        String debugErrorLog;
        const Bool uploadSuccessful = ShaderUploader_GL::UploadShaderProgramFromSource(effect, programInfo, debugErrorLog);

        if (uploadSuccessful)
        {
            const ShaderGPUResource_GL& shaderGpuResource = *new ShaderGPUResource_GL(effect, programInfo);
            return m_resourceMapper.registerResource(shaderGpuResource);
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "Device_GL::uploadShader: shader upload failed: " << debugErrorLog);
            return DeviceResourceHandle::Invalid();
        }
    }

    DeviceResourceHandle Device_GL::uploadBinaryShader(const EffectResource& effect, const UInt8* binaryShaderData, UInt32 binaryShaderDataSize, UInt32 binaryShaderFormat)
    {
        ShaderProgramInfo programInfo;
        String debugErrorLog;
        const Bool uploadSuccessful = ShaderUploader_GL::UploadShaderProgramFromBinary(binaryShaderData, binaryShaderDataSize, binaryShaderFormat, programInfo, debugErrorLog);

        if (uploadSuccessful)
        {
            LOG_INFO(CONTEXT_SMOKETEST, "Device_GL::uploadShader: renderer successfully uploaded binary shader for effect " << effect.getName());
            const ShaderGPUResource_GL& shaderGpuResource = *new ShaderGPUResource_GL(effect, programInfo);
            return m_resourceMapper.registerResource(shaderGpuResource);
        }
        else
        {
            LOG_INFO(CONTEXT_RENDERER, "Device_GL::uploadShader: renderer failed to upload binary shader for effect " << effect.getName() << "\n Error was: " << debugErrorLog);
            return DeviceResourceHandle::Invalid();
        }
    }

    Bool Device_GL::getBinaryShader(DeviceResourceHandle handle, UInt8Vector& binaryShader, UInt32& binaryShaderFormat)
    {
        binaryShader.clear();

        const ShaderGPUResource_GL& shaderProgramGL = m_resourceMapper.getResourceAs<ShaderGPUResource_GL>(handle);
        LOG_TRACE(CONTEXT_RENDERER, "Device_GL::getBinaryShader:  retrieving shader binary for effect with handle " << handle.asMemoryHandle());
        return shaderProgramGL.getBinaryInfo(binaryShader, binaryShaderFormat);
    }

    void Device_GL::deleteShader(DeviceResourceHandle handle)
    {
        const ShaderGPUResource_GL& shaderProgramGL = m_resourceMapper.getResourceAs<ShaderGPUResource_GL>(handle);
        if (m_activeShader == &shaderProgramGL)
        {
            m_activeShader = NULL;
        }

        m_resourceMapper.deleteResource(handle);
    }

    void Device_GL::activateShader(DeviceResourceHandle handle)
    {
        const ShaderGPUResource_GL& shaderProgramGL = m_resourceMapper.getResourceAs<ShaderGPUResource_GL>(handle);
        glUseProgram(shaderProgramGL.getGPUAddress());
        m_activeShader = &shaderProgramGL;
    }

    void Device_GL::deleteTexture(DeviceResourceHandle handle)
    {
        const GPUResource& resource = m_resourceMapper.getResource(handle);
        const GLHandle glAddress = resource.getGPUAddress();
        glDeleteTextures(1, &glAddress);
        m_resourceMapper.deleteResource(handle);
    }

    void Device_GL::activateTexture(DeviceResourceHandle textureResource, DataFieldHandle field)
    {
        GLInputLocation uniformLocation;
        if (getUniformLocation(field, uniformLocation))
        {
            const TextureSlotInfo textureSlot = m_activeShader->getTextureSlot(field);

            assert(static_cast<UInt32>(textureSlot.slot) < m_limits.getMaximumTextureUnits());
            glActiveTexture(GL_TEXTURE0 + textureSlot.slot);

            const auto renderTargetPair = std::find_if(m_pairedRenderTargets.cbegin(), m_pairedRenderTargets.cend(), [textureResource](const RenderTargetPair& rtPair) -> bool {return rtPair.colorBuffers[0] == textureResource; });

            const GPUResource* resource = nullptr;
            if (renderTargetPair != m_pairedRenderTargets.cend())
            {
                resource = &m_resourceMapper.getResource(renderTargetPair->colorBuffers[renderTargetPair->readingIndex]);
            }
            else
            {
                resource = &m_resourceMapper.getResource(textureResource);
            }

            const GLenum target = TypesConversion_GL::GetTextureTargetFromTextureInputType(textureSlot.textureType);
            glBindTexture(target, resource->getGPUAddress());
            glUniform1i(uniformLocation.getValue(), textureSlot.slot);
        }
        else
        {
            LOG_DEBUG(CONTEXT_RENDERER, "Device_GL::activateTexture could not find uniform location for field :" << field.asMemoryHandle() << ").\n");
        }
    }

    int Device_GL::getTextureAddress(DeviceResourceHandle handle) const
    {
        return m_resourceMapper.getResource(handle).getGPUAddress();
    }

    DeviceResourceHandle Device_GL::getFramebufferRenderTarget() const
    {
        return m_framebufferRenderTarget;
    }

    void Device_GL::blitRenderTargets(DeviceResourceHandle rtSrc, DeviceResourceHandle rtDst, const PixelRectangle& srcRect, const PixelRectangle& dstRect, Bool colorOnly)
    {
        const GPUResource& rtSrcResource = m_resourceMapper.getResource(rtSrc);
        const GPUResource& rtDstResource = m_resourceMapper.getResource(rtDst);

        const GLuint blittingSourceFrameBuffer = rtSrcResource.getGPUAddress();
        const GLuint blittingDestinationFrameBuffer = rtDstResource.getGPUAddress();

        const GLenum blittingMask = (colorOnly ? GL_COLOR_BUFFER_BIT : (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));
        glBindFramebuffer(GL_READ_FRAMEBUFFER, blittingSourceFrameBuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, blittingDestinationFrameBuffer);
        glBlitFramebuffer(srcRect.x,
            srcRect.y,
            srcRect.x + srcRect.width,
            srcRect.y + srcRect.height,

            dstRect.x,
            dstRect.y,
            dstRect.x + dstRect.width,
            dstRect.y + dstRect.height,

            blittingMask,
            GL_NEAREST);
    }

    Bool Device_GL::isApiExtensionAvailable(const String& extensionName) const
    {
        return m_apiExtensions.hasElement(extensionName);
    }

    void Device_GL::loadOpenGLExtensions()
    {
        GLint numExtensions = 0;
        glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
        if (numExtensions > 0)
        {
            m_apiExtensions.reserve(numExtensions);
            uint32_t sumExtensionStringLength = 0;
            for (auto i = 0; i < numExtensions; i++)
            {
                const auto tmp = reinterpret_cast<const Char*>(glGetStringi(GL_EXTENSIONS, i));
                sumExtensionStringLength += PlatformStringUtils::StrLen(tmp);
                m_apiExtensions.put(tmp);
            }

            LOG_INFO_F(CONTEXT_RENDERER, ([&](StringOutputStream& sos) {
                sos << "Device_GL::init: OpenGL extensions: ";
                sos.reserve(sos.capacity() + sumExtensionStringLength + numExtensions);
                for (const auto& extensionString : m_apiExtensions)
                    sos << extensionString << " ";
            }));
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "Device_GL::init: No OpenGL extensions available");
        }
    }

    void Device_GL::loadExtensionDependentFeatures()
    {
        GLint max_textures(0);
        glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &max_textures);
        m_limits.setMaximumTextureUnits(max_textures);

        GLint numCompressedTextureFormats(0);
        glGetIntegerv(GL_NUM_COMPRESSED_TEXTURE_FORMATS, &numCompressedTextureFormats);
        std::vector<GLint> compressedTextureFormats(numCompressedTextureFormats);
        glGetIntegerv(GL_COMPRESSED_TEXTURE_FORMATS, compressedTextureFormats.data());

        for (GLint compressedGLTextureFormat : compressedTextureFormats)
        {
            const ETextureFormat textureFormat = TypesConversion_GL::GetTextureFormatFromCompressedGLTextureFormat(compressedGLTextureFormat);
            if (ETextureFormat_Invalid != textureFormat)
            {
                m_limits.addTextureFormat(textureFormat);
            }
        }

        if (isApiExtensionAvailable("GL_EXT_texture_filter_anisotropic"))
        {
            GLint anisotropy = 0;
            glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &anisotropy);
            m_limits.setMaximumAnisotropy(anisotropy);
        }
        else
        {
            LOG_WARN(CONTEXT_RENDERER, "Device_GL::loadExtensionDependentFeatures:  anisotropic filtering not available on this device");
        }
    }

    void Device_GL::readPixels(UInt8* buffer, UInt32 x, UInt32 y, UInt32 width, UInt32 height)
    {
        glReadPixels(x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, static_cast<void*>(buffer));
    }

    UInt32 Device_GL::getTotalGpuMemoryUsageInKB() const
    {
        return m_resourceMapper.getTotalGpuMemoryUsageInKB();
    }

    Bool Device_GL::isDeviceStatusHealthy() const
    {
        if (m_debugOutput.isAvailable())
        {
            return !m_debugOutput.checkAndResetError();
        }

        const GLenum errorStatus = glGetError();
        if (GL_NO_ERROR != errorStatus)
        {
            LOG_ERROR(CONTEXT_RENDERER, "Device_GL::validateDeviceStatusHealthy:  GL Error detected :" << errorStatus);
            return false;
        }

        return true;
    }

    void Device_GL::validateDeviceStatusHealthy() const
    {
        assert(isDeviceStatusHealthy());
    }

    void Device_GL::finish()
    {
        glFinish();
    }
}
