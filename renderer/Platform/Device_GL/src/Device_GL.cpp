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
#include "Platform_Base/VertexArrayGPUResource.h"

#include "Device_GL/Device_GL_platform.h"
#include "Device_GL/ShaderGPUResource_GL.h"
#include "Device_GL/ShaderUploader_GL.h"
#include "Device_GL/ShaderProgramInfo.h"
#include "Device_GL/TypesConversion_GL.h"

#include "SceneAPI/PixelRectangle.h"
#include "RendererAPI/IContext.h"
#include "RendererAPI/IDeviceExtension.h"
#include "Resource/EffectResource.h"

#include "Math3d/Vector2.h"
#include "Math3d/Vector3.h"
#include "Math3d/Vector4.h"
#include "Math3d/Vector2i.h"
#include "Math3d/Vector3i.h"
#include "Math3d/Vector4i.h"
#include "Math3d/Matrix22f.h"
#include "Math3d/Matrix33f.h"
#include "Math3d/Matrix44f.h"

#include "Utils/ThreadLocalLogForced.h"
#include "Utils/TextureMathUtils.h"
#include "PlatformAbstraction/PlatformStringUtils.h"
#include "PlatformAbstraction/Macros.h"


namespace ramses_internal
{
    static constexpr GLboolean ToGLboolean(bool b)
    {
        return b ? GL_TRUE : GL_FALSE;
    }

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

    Device_GL::Device_GL(IContext& context, UInt8 majorApiVersion, UInt8 minorApiVersion, bool isEmbedded, IDeviceExtension* deviceExtension)
        : Device_Base(context)
        , m_activeShader(nullptr)
        , m_activePrimitiveDrawMode(EDrawMode::Triangles)
        , m_activeIndexArrayElementSizeBytes(2u)
        , m_majorApiVersion(majorApiVersion)
        , m_minorApiVersion(minorApiVersion)
        , m_isEmbedded(isEmbedded)
        , m_debugOutput()
        , m_deviceExtension(deviceExtension)
    {
#if defined _DEBUG
        m_debugOutput.enable(context);
#endif

        m_limits.addTextureFormat(ETextureFormat::Depth16);
        m_limits.addTextureFormat(ETextureFormat::Depth24);
        m_limits.addTextureFormat(ETextureFormat::Depth24_Stencil8);

        m_limits.addTextureFormat(ETextureFormat::RGBA8);
        m_limits.addTextureFormat(ETextureFormat::RGB8);
        m_limits.addTextureFormat(ETextureFormat::RGBA5551);
        m_limits.addTextureFormat(ETextureFormat::RGBA4);
        m_limits.addTextureFormat(ETextureFormat::RGB565);

        m_limits.addTextureFormat(ETextureFormat::R8);
        m_limits.addTextureFormat(ETextureFormat::R16);
        m_limits.addTextureFormat(ETextureFormat::RG8);
        m_limits.addTextureFormat(ETextureFormat::RG16);
        m_limits.addTextureFormat(ETextureFormat::RGB16);
        m_limits.addTextureFormat(ETextureFormat::RGBA16);

        m_limits.addTextureFormat(ETextureFormat::R16F);
        m_limits.addTextureFormat(ETextureFormat::R32F);
        m_limits.addTextureFormat(ETextureFormat::RG16F);
        m_limits.addTextureFormat(ETextureFormat::RG32F);
        m_limits.addTextureFormat(ETextureFormat::RGB16F);
        m_limits.addTextureFormat(ETextureFormat::RGB32F);
        m_limits.addTextureFormat(ETextureFormat::RGBA16F);
        m_limits.addTextureFormat(ETextureFormat::RGBA32F);

        m_limits.addTextureFormat(ETextureFormat::SRGB8);
        m_limits.addTextureFormat(ETextureFormat::SRGB8_ALPHA8);
        m_limits.addTextureFormat(ETextureFormat::DXT3RGBA);
    }

    Device_GL::~Device_GL()
    {
        for (const auto& it : m_textureSamplerObjectsCache)
            deleteTextureSampler(it.second);

        m_resourceMapper.deleteResource(m_framebufferRenderTarget);
    }

    Bool Device_GL::init()
    {
        LOAD_ALL_API_PROCS(m_context);

        const Char* tmp = nullptr;

        tmp = reinterpret_cast<const Char*>(glGetString(GL_VENDOR));
        LOG_INFO(CONTEXT_RENDERER, "Device_GL::init:  OpenGL vendor is " << tmp);

        tmp = reinterpret_cast<const Char*>(glGetString(GL_RENDERER));
        LOG_INFO(CONTEXT_RENDERER, "    OpenGL renderer is " << tmp);

        tmp = reinterpret_cast<const Char*>(glGetString(GL_VERSION));
        LOG_INFO(CONTEXT_RENDERER, "     OpenGL version is " << tmp);

        tmp = reinterpret_cast<const Char*>(glGetString(GL_SHADING_LANGUAGE_VERSION));
        LOG_INFO(CONTEXT_RENDERER, "     GLSL version " << tmp);

        loadOpenGLExtensions();
        queryDeviceDependentFeatures();

        m_framebufferRenderTarget = m_resourceMapper.registerResource(std::make_unique<RenderTargetGPUResource>(0));

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
        assert(m_activeIndexArraySizeBytes != 0 && m_activeIndexArrayElementSizeBytes != 0);
        if (m_activeIndexArraySizeBytes < (startOffset + elementCount) * m_activeIndexArrayElementSizeBytes)
        {
            LOG_ERROR_P(CONTEXT_RENDERER, "Device_GL::drawIndexedTriangles: index buffer access out of bounds "
                "[drawStartOffset={} drawElementCount={} IndexBufferElementCount={}]",
                startOffset, elementCount, m_activeIndexArraySizeBytes / m_activeIndexArrayElementSizeBytes);
            return;
        }

        const UInt startOffsetAddressAsUInt = startOffset * m_activeIndexArrayElementSizeBytes;
        const GLvoid* startOffsetAddress = reinterpret_cast<void*>(startOffsetAddressAsUInt);

        const GLenum drawModeGL = TypesConversion_GL::GetDrawMode(m_activePrimitiveDrawMode);
        const GLenum elementTypeGL = TypesConversion_GL::GetIndexElementType(m_activeIndexArrayElementSizeBytes);
        if (instanceCount > 1u)
            glDrawElementsInstanced(drawModeGL, elementCount, elementTypeGL, startOffsetAddress, static_cast<GLsizei>(instanceCount));
        else
            glDrawElements(drawModeGL, elementCount, elementTypeGL, startOffsetAddress);

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
        glColorMask(ToGLboolean(r),
                    ToGLboolean(g),
                    ToGLboolean(b),
                    ToGLboolean(a));
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
        glDepthMask(ToGLboolean(flag == EDepthWrite::Enabled));
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

    void Device_GL::blendColor(const Vector4& color)
    {
        glBlendColor(color.r, color.g, color.b, color.a);
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

    void Device_GL::setViewport(int32_t x, int32_t y, uint32_t width, uint32_t height)
    {
        if (width > m_limits.getMaxViewportWidth() || height > m_limits.getMaxViewportHeight())
        {
            LOG_WARN_P(CONTEXT_RENDERER, "Device_GL::setViewport: viewport size out of bounds "
                "[width={} height={}], clamping to [maxW={} maxH={}]",
                width, height, m_limits.getMaxViewportWidth(), m_limits.getMaxViewportHeight());
        }

        glViewport(x, y, std::min(width, m_limits.getMaxViewportWidth()), std::min(height, m_limits.getMaxViewportHeight()));
    }

    GLHandle Device_GL::createTexture(UInt32 width, UInt32 height, ETextureFormat storageFormat, UInt32 sampleCount) const
    {
        LOG_DEBUG(CONTEXT_RENDERER, "Device_GL::createTexture:  creating a new texture (texture render target)");

        const GLHandle texID = generateAndBindTexture((sampleCount) ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D);
        GLTextureInfo texInfo;
        fillGLInternalTextureInfo((sampleCount) ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, width, height, 1u, storageFormat, {ETextureChannelColor::Red, ETextureChannelColor::Green, ETextureChannelColor::Blue, ETextureChannelColor::Alpha}, texInfo);
        allocateTextureStorage(texInfo, 1u, sampleCount);

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
        case ETextureFormat::Depth24:
            internalFormat = GL_DEPTH_COMPONENT24;
            break;
        case ETextureFormat::Depth24_Stencil8:
            internalFormat = GL_DEPTH24_STENCIL8;
            break;
        case ETextureFormat::RGBA8:
            internalFormat = GL_RGBA8;
            break;
        default:
            assert(false && "Unknown render buffer format");
        }

        sampleCount = checkAndClampNumberOfSamples(internalFormat, sampleCount);

        glRenderbufferStorageMultisample(GL_RENDERBUFFER, sampleCount, internalFormat, width, height);

        return renderbufferHandle;
    }

    Bool Device_GL::getUniformLocation(DataFieldHandle field, GLInputLocation& location) const
    {
        assert(nullptr != m_activeShader);
        location = m_activeShader->getUniformLocation(field);
        return location != GLInputLocationInvalid;
    }

    Bool Device_GL::getAttributeLocation(DataFieldHandle field, GLInputLocation& location) const
    {
        assert(nullptr != m_activeShader);
        location = m_activeShader->getAttributeLocation(field);
        return location != GLInputLocationInvalid;
    }

    void Device_GL::setConstant(DataFieldHandle field, UInt32 count, const Float* value)
    {
        GLInputLocation uniformLocation;
        if (getUniformLocation(field, uniformLocation))
        {
            assert(nullptr != value);
            glUniform1fv(uniformLocation.getValue(), count, value);
        }
    }

    void Device_GL::setConstant(DataFieldHandle field, UInt32 count, const Vector2* value)
    {
        GLInputLocation uniformLocation;
        if (getUniformLocation(field, uniformLocation))
        {
            assert(nullptr != value);
            glUniform2fv(uniformLocation.getValue(), count, value[0].data);
        }
    }

    void Device_GL::setConstant(DataFieldHandle field, UInt32 count, const Vector3* value)
    {
        GLInputLocation uniformLocation;
        if (getUniformLocation(field, uniformLocation))
        {
            assert(nullptr != value);
            glUniform3fv(uniformLocation.getValue(), count, value[0].data);
        }
    }

    void Device_GL::setConstant(DataFieldHandle field, UInt32 count, const Vector4* value)
    {
        GLInputLocation uniformLocation;
        if (getUniformLocation(field, uniformLocation))
        {
            assert(nullptr != value);
            glUniform4fv(uniformLocation.getValue(), count, value[0].data);
        }
    }

    void Device_GL::setConstant(DataFieldHandle field, UInt32 count, const Int32* value)
    {
        GLInputLocation uniformLocation;
        if (getUniformLocation(field, uniformLocation))
        {
            assert(nullptr != value);
            glUniform1iv(uniformLocation.getValue(), count, value);
        }
    }

    void Device_GL::setConstant(DataFieldHandle field, UInt32 count, const Vector2i* value)
    {
        GLInputLocation uniformLocation;
        if (getUniformLocation(field, uniformLocation))
        {
            assert(nullptr != value);
            glUniform2iv(uniformLocation.getValue(), count, value[0].data);
        }
    }

    void Device_GL::setConstant(DataFieldHandle field, UInt32 count, const Vector3i* value)
    {
        GLInputLocation uniformLocation;
        if (getUniformLocation(field, uniformLocation))
        {
            assert(nullptr != value);
            glUniform3iv(uniformLocation.getValue(), count, value[0].data);
        }
    }

    void Device_GL::setConstant(DataFieldHandle field, UInt32 count, const Vector4i* value)
    {
        GLInputLocation uniformLocation;
        if (getUniformLocation(field, uniformLocation))
        {
            assert(nullptr != value);
            glUniform4iv(uniformLocation.getValue(), count, value[0].data);
        }
    }

    void Device_GL::setConstant(DataFieldHandle field, UInt32 count, const Matrix22f* value)
    {
        GLInputLocation uniformLocation;
        if (getUniformLocation(field, uniformLocation))
        {
            assert(nullptr != value);
            glUniformMatrix2fv(uniformLocation.getValue(), count, ToGLboolean(false), value[0].data);
        }
    }

    void Device_GL::setConstant(DataFieldHandle field, UInt32 count, const Matrix33f* value)
    {
        GLInputLocation uniformLocation;
        if (getUniformLocation(field, uniformLocation))
        {
            assert(nullptr != value);
            glUniformMatrix3fv(uniformLocation.getValue(), count, ToGLboolean(false), value[0].data);
        }
    }

    void Device_GL::setConstant(DataFieldHandle field, UInt32 count, const Matrix44f* value)
    {
        GLInputLocation uniformLocation;
        if (getUniformLocation(field, uniformLocation))
        {
            assert(nullptr != value);
            glUniformMatrix4fv(uniformLocation.getValue(), count, ToGLboolean(false), value[0].data);
        }
    }

    DeviceResourceHandle Device_GL::allocateTexture2D(UInt32 width, UInt32 height, ETextureFormat textureFormat, const TextureSwizzleArray& swizzle, UInt32 mipLevelCount, UInt32 totalSizeInBytes)
    {
        const GLHandle texID = generateAndBindTexture(GL_TEXTURE_2D);
        GLTextureInfo texInfo;
        fillGLInternalTextureInfo(GL_TEXTURE_2D, width, height, 1u, textureFormat, swizzle, texInfo);
        allocateTextureStorage(texInfo, mipLevelCount);

        return m_resourceMapper.registerResource(std::make_unique<TextureGPUResource_GL>(texInfo, texID, totalSizeInBytes));
    }

    DeviceResourceHandle Device_GL::allocateTexture3D(UInt32 width, UInt32 height, UInt32 depth, ETextureFormat textureFormat, UInt32 mipLevelCount, UInt32 totalSizeInBytes)
    {
        const GLHandle texID = generateAndBindTexture(GL_TEXTURE_3D);
        GLTextureInfo texInfo;
        fillGLInternalTextureInfo(GL_TEXTURE_3D, width, height, depth, textureFormat, DefaultTextureSwizzleArray, texInfo);
        allocateTextureStorage(texInfo, mipLevelCount);

        return m_resourceMapper.registerResource(std::make_unique<TextureGPUResource_GL>(texInfo, texID, totalSizeInBytes));
    }

    DeviceResourceHandle Device_GL::allocateTextureCube(UInt32 faceSize, ETextureFormat textureFormat, const TextureSwizzleArray& swizzle, UInt32 mipLevelCount, UInt32 totalSizeInBytes)
    {
        const GLHandle texID = generateAndBindTexture(GL_TEXTURE_CUBE_MAP);
        GLTextureInfo texInfo;
        fillGLInternalTextureInfo(GL_TEXTURE_CUBE_MAP, faceSize, faceSize, 1u, textureFormat, swizzle, texInfo);
        allocateTextureStorage(texInfo, mipLevelCount);

        return m_resourceMapper.registerResource(std::make_unique<TextureGPUResource_GL>(texInfo, texID, totalSizeInBytes));
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

    DeviceResourceHandle Device_GL::uploadStreamTexture2D(DeviceResourceHandle handle, UInt32 width, UInt32 height, ETextureFormat format, const UInt8* data, const TextureSwizzleArray& swizzle)
    {
        if (!handle.isValid())
        {
            // generate texID and register texture resource
            assert(data == nullptr);
            GLHandle texID = InvalidGLHandle;
            glGenTextures(1, &texID);

            return m_resourceMapper.registerResource(std::make_unique<GPUResource>(texID, 0u));
        }
        else
        {
            // upload data to registered texture resource
            const GLHandle texID = getTextureAddress(handle);
            assert(texID != InvalidGLHandle);
            assert(data != nullptr);
            LOG_DEBUG(CONTEXT_RENDERER, "Device_GL::uploadStreamTexture2D:  texid: " << texID << " width: " << width << " height: " << height << " format: " << EnumToString(format) << " textureSwizzle: " << EnumToString(swizzle[0]) << "," << EnumToString(swizzle[1]) << "," << EnumToString(swizzle[2]) << "," << EnumToString(swizzle[3]));

            glBindTexture(GL_TEXTURE_2D, texID);

            GLTextureInfo texInfo;
            fillGLInternalTextureInfo(GL_TEXTURE_2D, width, height, 1u, format, swizzle, texInfo);

            assert(4 == swizzle.size());
            glTexParameteri(texInfo.target, GL_TEXTURE_SWIZZLE_R, TypesConversion_GL::GetGlColorFromTextureChannelColor(texInfo.uploadParams.swizzle[0]));
            glTexParameteri(texInfo.target, GL_TEXTURE_SWIZZLE_G, TypesConversion_GL::GetGlColorFromTextureChannelColor(texInfo.uploadParams.swizzle[1]));
            glTexParameteri(texInfo.target, GL_TEXTURE_SWIZZLE_B, TypesConversion_GL::GetGlColorFromTextureChannelColor(texInfo.uploadParams.swizzle[2]));
            glTexParameteri(texInfo.target, GL_TEXTURE_SWIZZLE_A, TypesConversion_GL::GetGlColorFromTextureChannelColor(texInfo.uploadParams.swizzle[3]));
            assert(!texInfo.uploadParams.compressed);
            // For now stream texture upload is using glTexImage2D instead of glStore/glSubimage because its size/format cannot be immutable
            glTexImage2D(texInfo.target, 0, texInfo.uploadParams.sizedInternalFormat, texInfo.width, texInfo.height, 0, texInfo.uploadParams.baseInternalFormat, texInfo.uploadParams.type, data);

            return handle;
        }
    }

    void Device_GL::fillGLInternalTextureInfo(GLenum target, UInt32 width, UInt32 height, UInt32 depth, ETextureFormat textureFormat, const TextureSwizzleArray& swizzle, GLTextureInfo& glTexInfoOut) const
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
        glTexInfoOut.uploadParams.swizzle = swizzle;
    }

    uint32_t Device_GL::checkAndClampNumberOfSamples(GLenum internalFormat, uint32_t numSamples) const
    {
        if (numSamples > 1)
        {
            GLint maxNumSamplesGL = 0;
            glGetInternalformativ(GL_RENDERBUFFER, internalFormat, GL_SAMPLES, 1, &maxNumSamplesGL);
            uint32_t maxNumSamples = static_cast<uint32_t>(maxNumSamplesGL);
            if (numSamples > maxNumSamples)
            {
                LOG_WARN_P(CONTEXT_RENDERER, "Device_GL: clamping requested MSAA sample count {} "
                    "to {}, a maximum number of samples supported by device for this format.", numSamples, maxNumSamples);
                numSamples = maxNumSamples;
            }
        }

        return numSamples;
    }

    void Device_GL::allocateTextureStorage(const GLTextureInfo& texInfo, UInt32 mipLevels, UInt32 sampleCount) const
    {
        assert(!(sampleCount && mipLevels > 1));
        glPixelStorei(GL_UNPACK_ALIGNMENT, texInfo.uploadParams.byteAlignment);

        static_assert(decltype(texInfo.uploadParams.swizzle)().size() == 4, "Wrong size of texture swizzle array");
        // This is NOOP for Cube Map and Texture3D because the swizzle has default values for these types.
        glTexParameteri(texInfo.target, GL_TEXTURE_SWIZZLE_R, TypesConversion_GL::GetGlColorFromTextureChannelColor(texInfo.uploadParams.swizzle[0]));
        glTexParameteri(texInfo.target, GL_TEXTURE_SWIZZLE_G, TypesConversion_GL::GetGlColorFromTextureChannelColor(texInfo.uploadParams.swizzle[1]));
        glTexParameteri(texInfo.target, GL_TEXTURE_SWIZZLE_B, TypesConversion_GL::GetGlColorFromTextureChannelColor(texInfo.uploadParams.swizzle[2]));
        glTexParameteri(texInfo.target, GL_TEXTURE_SWIZZLE_A, TypesConversion_GL::GetGlColorFromTextureChannelColor(texInfo.uploadParams.swizzle[3]));

        switch (texInfo.target)
        {
        case GL_TEXTURE_2D:
        case GL_TEXTURE_CUBE_MAP:
            glTexStorage2D(texInfo.target, mipLevels, texInfo.uploadParams.sizedInternalFormat, texInfo.width, texInfo.height);
            break;
        case GL_TEXTURE_2D_MULTISAMPLE:
            sampleCount = checkAndClampNumberOfSamples(texInfo.uploadParams.sizedInternalFormat, sampleCount);
            glTexStorage2DMultisample(texInfo.target, sampleCount, texInfo.uploadParams.sizedInternalFormat, texInfo.width, texInfo.height, ToGLboolean(true));
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

    DeviceResourceHandle Device_GL::uploadRenderBuffer(uint32_t width, uint32_t height, ERenderBufferType type, ETextureFormat format, ERenderBufferAccessMode accessMode, uint32_t sampleCount)
    {
        GLHandle bufferGLHandle = InvalidGLHandle;
        switch (accessMode)
        {
        case ERenderBufferAccessMode_ReadWrite:
            bufferGLHandle = createTexture(width, height, format, sampleCount);
            break;
        case ERenderBufferAccessMode_WriteOnly:
            bufferGLHandle = createRenderBuffer(width, height, format, sampleCount);
            break;
        default:
            assert(false);
        }

        if (bufferGLHandle != InvalidGLHandle)
            return m_resourceMapper.registerResource(std::make_unique<RenderBufferGPUResource>(bufferGLHandle, width, height, type, format, sampleCount, accessMode));

        return DeviceResourceHandle::Invalid();
    }

    DeviceResourceHandle Device_GL::uploadDmaRenderBuffer(UInt32 width, UInt32 height, DmaBufferFourccFormat fourccFormat, DmaBufferUsageFlags usageFlags, DmaBufferModifiers modifiers)
    {
        if(m_deviceExtension == nullptr)
            return {};

        return m_deviceExtension->createDmaRenderBuffer(width, height, fourccFormat, usageFlags, modifiers);
    }

    int Device_GL::getDmaRenderBufferFD(DeviceResourceHandle handle)
    {
        if(m_deviceExtension == nullptr)
            return -1;

        return m_deviceExtension->getDmaRenderBufferFD(handle);
    }

    uint32_t Device_GL::getDmaRenderBufferStride(DeviceResourceHandle handle)
    {
        if(m_deviceExtension == nullptr)
            return 0u;
        return m_deviceExtension->getDmaRenderBufferStride(handle);
    }

    void Device_GL::destroyDmaRenderBuffer(DeviceResourceHandle handle)
    {
        if(m_deviceExtension == nullptr)
            return;

        m_deviceExtension->destroyDmaRenderBuffer(handle);
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

    DeviceResourceHandle Device_GL::uploadTextureSampler(const TextureSamplerStates& samplerStates)
    {
        GLuint sampler;
        glGenSamplers(1, &sampler);

        const GLenum wrappingModeR = TypesConversion_GL::GetWrapMode(samplerStates.m_addressModeR);
        const GLenum wrappingModeU = TypesConversion_GL::GetWrapMode(samplerStates.m_addressModeU);
        const GLenum wrappingModeV = TypesConversion_GL::GetWrapMode(samplerStates.m_addressModeV);

        glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, wrappingModeU);
        glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, wrappingModeV);
        glSamplerParameteri(sampler, GL_TEXTURE_WRAP_R, wrappingModeR);

        switch (samplerStates.m_minSamplingMode)
        {
        case ESamplingMethod::Nearest:
            glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            break;
        case ESamplingMethod::Linear:
            glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            break;
        case ESamplingMethod::Nearest_MipMapNearest:
            glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
            break;
        case ESamplingMethod::Nearest_MipMapLinear:
            glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
            break;
        case ESamplingMethod::Linear_MipMapNearest:
            glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
            break;
        case ESamplingMethod::Linear_MipMapLinear:
            glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            break;
        case ESamplingMethod::NUMBER_OF_ELEMENTS:
            assert(false);
        }

        switch (samplerStates.m_magSamplingMode)
        {
        case ESamplingMethod::Nearest:
            glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            break;
        case ESamplingMethod::Linear:
            glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            break;
        case ESamplingMethod::Nearest_MipMapNearest:
        case ESamplingMethod::Nearest_MipMapLinear:
        case ESamplingMethod::Linear_MipMapNearest:
        case ESamplingMethod::Linear_MipMapLinear:
        case ESamplingMethod::NUMBER_OF_ELEMENTS:
            assert(false);
        }

        // set anisotropy only if feature is supported
        if (m_limits.getMaximumAnisotropy() > 1u)
        {
            // clamp anisotropy value to max supported range
            const auto anisotropyLevel = std::min(samplerStates.m_anisotropyLevel, m_limits.getMaximumAnisotropy());
            glSamplerParameteri(sampler, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropyLevel);
        }

        return m_resourceMapper.registerResource(std::make_unique<GPUResource>(sampler, 0u));
    }

    void Device_GL::deleteTextureSampler(DeviceResourceHandle handle)
    {
        const GPUResource& resource = m_resourceMapper.getResourceAs<GPUResource>(handle);
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

    void Device_GL::activateTextureSamplerObject(const TextureSamplerStates& samplerStates, DataFieldHandle field)
    {
        const auto samplerStatesHash = samplerStates.hash();
        auto it = m_textureSamplerObjectsCache.find(samplerStatesHash);
        if (it == m_textureSamplerObjectsCache.end())
        {
            m_textureSamplerObjectsCache[samplerStatesHash] = uploadTextureSampler(samplerStates);
            it = m_textureSamplerObjectsCache.find(samplerStatesHash);
            LOG_INFO(CONTEXT_RENDERER, "Device_GL::activateTextureSamplerObject: cached new sampler object, total count: " << m_textureSamplerObjectsCache.size());
        }

        activateTextureSampler(it->second, field);
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

        if (renderBuffers.size() > m_limits.getMaximumDrawBuffers())
        {
            LOG_ERROR_P(CONTEXT_RENDERER, "Device_GL::uploadRenderTarget failed: this device supports at most {} render buffers attached to render target, requested {}.",
                m_limits.getMaximumDrawBuffers(), renderBuffers.size());
            return DeviceResourceHandle::Invalid();
        }

        GLHandle fboAddress = InvalidGLHandle;
        glGenFramebuffers(1, &fboAddress);
        assert(fboAddress != InvalidGLHandle);
        glBindFramebuffer(GL_FRAMEBUFFER, fboAddress);

        // colorBuffers will contain color attachments slots (no depth/stencil), to define fragment shader mapping via glDrawBuffers
        std::vector<GLenum> colorBuffers;
        colorBuffers.reserve(renderBuffers.size());
        for (const DeviceResourceHandle rbHandle : renderBuffers)
        {
            const RenderBufferGPUResource& bufferGPUResource = m_resourceMapper.getResourceAs<RenderBufferGPUResource>(rbHandle);
            bindRenderBufferToRenderTarget(bufferGPUResource, colorBuffers.size());

            if (ERenderBufferType_ColorBuffer == bufferGPUResource.getType())
                colorBuffers.push_back(GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(colorBuffers.size()));
        }

        // Always check that our framebuffer is ok
        const GLenum FBOstatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (FBOstatus != GL_FRAMEBUFFER_COMPLETE)
        {
            LOG_ERROR(CONTEXT_RENDERER, "Device_GL::createRenderTargetComponents Framebuffer status not complete! GL error code: " << FBOstatus);
            return DeviceResourceHandle::Invalid();
        }
        glDrawBuffers(static_cast<GLsizei>(colorBuffers.size()), colorBuffers.data());

        const DeviceResourceHandle fboHandle = m_resourceMapper.registerResource(std::make_unique<RenderTargetGPUResource>(fboAddress));
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

    void Device_GL::discardDepthStencil()
    {
        // Not to be used with default framebuffer which might need (depending on implementation) different enums for attachments
        // https://www.khronos.org/registry/OpenGL-Refpages/es3.0/html/glInvalidateFramebuffer.xhtml
        constexpr std::array<GLenum, 3> depthStencilAttachments = { GL_DEPTH_ATTACHMENT, GL_STENCIL_ATTACHMENT, GL_DEPTH_STENCIL_ATTACHMENT };
        glInvalidateFramebuffer(GL_FRAMEBUFFER, static_cast<GLsizei>(depthStencilAttachments.size()), depthStencilAttachments.data());
    }

    void Device_GL::bindRenderBufferToRenderTarget(const RenderBufferGPUResource& renderBufferGpuResource, size_t colorBufferSlot)
    {
        switch (renderBufferGpuResource.getAccessMode())
        {
        case ramses_internal::ERenderBufferAccessMode_WriteOnly:
            bindWriteOnlyRenderBufferToRenderTarget(renderBufferGpuResource.getType(), colorBufferSlot, renderBufferGpuResource.getGPUAddress());
            break;
        case ramses_internal::ERenderBufferAccessMode_ReadWrite:
            bindReadWriteRenderBufferToRenderTarget(renderBufferGpuResource.getType(), colorBufferSlot, renderBufferGpuResource.getGPUAddress(), renderBufferGpuResource.getSampleCount() != 0);
            break;
        default:
            assert(false && "invalid render buffer access mode");
            break;
        }
    }

    void Device_GL::bindReadWriteRenderBufferToRenderTarget(ERenderBufferType bufferType, size_t colorBufferSlot, GLHandle bufferGLHandle, const bool multiSample)
    {
        const int texTarget = (multiSample) ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
        switch (bufferType)
        {
        case ERenderBufferType_DepthBuffer:
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texTarget, bufferGLHandle, 0);
            break;
        case ERenderBufferType_DepthStencilBuffer:
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texTarget, bufferGLHandle, 0);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, texTarget, bufferGLHandle, 0);
            break;
        case ERenderBufferType_ColorBuffer:
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(colorBufferSlot), texTarget, bufferGLHandle, 0);
            break;
        case ERenderBufferType_InvalidBuffer:
        default:
            assert(false && "invalid render buffer type");
            break;
        }
    }

    void Device_GL::bindWriteOnlyRenderBufferToRenderTarget(ERenderBufferType bufferType, size_t colorBufferSlot, GLHandle bufferGLHandle)
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
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(colorBufferSlot), GL_RENDERBUFFER, bufferGLHandle);
            break;
        case ERenderBufferType_InvalidBuffer:
        default:
            assert(false && "invalid render buffer type");
            break;
        }
    }

    void Device_GL::activateRenderTarget(DeviceResourceHandle handle)
    {
        const auto renderTargetPair = std::find_if(m_pairedRenderTargets.cbegin(), m_pairedRenderTargets.cend(), [handle](const RenderTargetPair& rtPair) -> bool {return rtPair.renderTargets[0] == handle; });

        const GPUResource* rtResource = nullptr;
        if (renderTargetPair != m_pairedRenderTargets.cend())
        {
            const UInt8 writingIndex = (renderTargetPair->readingIndex + 1) % 2;
            rtResource = &m_resourceMapper.getResource(renderTargetPair->renderTargets[writingIndex]);
        }
        else
        {
            rtResource = &m_resourceMapper.getResource(handle);
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

    GLHandle Device_GL::generateAndBindTexture(GLenum target) const
    {
        GLHandle texID = InvalidGLHandle;
        glGenTextures(1, &texID);
        assert(texID != InvalidGLHandle);
        glBindTexture(target, texID);

        return texID;
    }

    DeviceResourceHandle Device_GL::allocateVertexBuffer(UInt32 totalSizeInBytes)
    {
        GLHandle glAddress = InvalidGLHandle;
        glGenBuffers(1, &glAddress);
        assert(glAddress != InvalidGLHandle);

        return m_resourceMapper.registerResource(std::make_unique<GPUResource>(glAddress, totalSizeInBytes));
    }

    void Device_GL::uploadVertexBufferData(DeviceResourceHandle handle, const Byte* data, UInt32 dataSize)
    {
        const auto& vertexBuffer = m_resourceMapper.getResource(handle);
        assert(dataSize <= vertexBuffer.getTotalSizeInBytes());

        glBindVertexArray(0u); // make sure no VAO affected
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer.getGPUAddress());
        glBufferData(GL_ARRAY_BUFFER, dataSize, data, GL_STATIC_DRAW);
    }

    void Device_GL::deleteVertexBuffer(DeviceResourceHandle handle)
    {
        const GLHandle resourceAddress = m_resourceMapper.getResource(handle).getGPUAddress();
        glDeleteBuffers(1, &resourceAddress);
        m_resourceMapper.deleteResource(handle);
    }

    DeviceResourceHandle Device_GL::allocateVertexArray(const VertexArrayInfo& vertexArrayInfo)
    {
        const auto& shaderResource = m_resourceMapper.getResourceAs<const ShaderGPUResource_GL>(vertexArrayInfo.shader);

        GLuint vertexArrayAddress = 0u;
        glGenVertexArrays(1, &vertexArrayAddress);
        glBindVertexArray(vertexArrayAddress);

        if (vertexArrayInfo.indexBuffer.isValid())
        {
            const IndexBufferGPUResource& indexBufferGPUResource = m_resourceMapper.getResourceAs<IndexBufferGPUResource>(vertexArrayInfo.indexBuffer);
            const GLHandle resourceAddress = indexBufferGPUResource.getGPUAddress();
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, resourceAddress);
        }

        for (const auto& vb : vertexArrayInfo.vertexBuffers)
        {
            assert(IsBufferDataType(vb.bufferDataType));

            const GLInputLocation vertexInputAddress = shaderResource.getAttributeLocation(vb.field);
            if (!vertexInputAddress.isValid())
            {
                //In case attribute is optimized out by shader compiler, e.g., because it is unused
                LOG_DEBUG_P(CONTEXT_RENDERER, "Device_GL::allocateVertexArray could not find attrib location for field: {}. Field will be ignored in vertex array.", vb.field);
                continue;
            }

            const GPUResource& arrayResource = m_resourceMapper.getResource(vb.deviceHandle);

            const auto attributeDataType = BufferTypeToElementType(vb.bufferDataType);
            const auto elementSize = (vb.stride != 0u ? vb.stride : EnumToSize(attributeDataType));

            const std::intptr_t offsetInBytes = vb.startVertex * elementSize + vb.offsetWithinElement;
            const void* offsetAsPointer = reinterpret_cast<const void*>(offsetInBytes);
            const auto attributeNumComponents = EnumToNumComponents(attributeDataType);

            glBindBuffer(GL_ARRAY_BUFFER, arrayResource.getGPUAddress());
            glEnableVertexAttribArray(vertexInputAddress.getValue());
            glVertexAttribPointer(vertexInputAddress.getValue(), attributeNumComponents, GL_FLOAT, GL_FALSE, vb.stride, offsetAsPointer);

            glVertexAttribDivisor(vertexInputAddress.getValue(), vb.instancingDivisor);
        }

        glBindVertexArray(0u);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0u);
        glBindBuffer(GL_ARRAY_BUFFER, 0u);

        return m_resourceMapper.registerResource(std::make_unique<VertexArrayGPUResource>(vertexArrayAddress, vertexArrayInfo.indexBuffer));
    }

    void Device_GL::activateVertexArray(DeviceResourceHandle handle)
    {
        assert(handle.isValid());
        const VertexArrayGPUResource& vertexArrayResource = m_resourceMapper.getResourceAs<VertexArrayGPUResource>(handle);
        glBindVertexArray(vertexArrayResource.getGPUAddress());

        const auto indexBuffer = vertexArrayResource.getIndexBufferHandle();
        if (indexBuffer.isValid())
        {
            const IndexBufferGPUResource& indexBufferGPUResource = m_resourceMapper.getResourceAs<IndexBufferGPUResource>(indexBuffer);
            m_activeIndexArrayElementSizeBytes = indexBufferGPUResource.getElementSizeInBytes();
            assert(m_activeIndexArrayElementSizeBytes == 2 || m_activeIndexArrayElementSizeBytes == 4);
            m_activeIndexArraySizeBytes = indexBufferGPUResource.getTotalSizeInBytes();
        }
        else
        {
            m_activeIndexArrayElementSizeBytes = 0u;
            m_activeIndexArraySizeBytes = 0u;
        }
    }

    void Device_GL::deleteVertexArray(DeviceResourceHandle handle)
    {
        const GPUResource& vertexArrayResource = m_resourceMapper.getResource(handle);
        const GLuint vertexArray = vertexArrayResource.getGPUAddress();
        glDeleteVertexArrays(1, &vertexArray);

        m_resourceMapper.deleteResource(handle);
    }

    DeviceResourceHandle Device_GL::allocateIndexBuffer(EDataType dataType, UInt32 sizeInBytes)
    {
        GLHandle glAddress = InvalidGLHandle;
        glGenBuffers(1, &glAddress);
        assert(glAddress != InvalidGLHandle);
        assert(dataType == EDataType::UInt16 || dataType == EDataType::UInt32);

        return m_resourceMapper.registerResource(std::make_unique<IndexBufferGPUResource>(glAddress, sizeInBytes, dataType == EDataType::UInt16 ? 2 : 4));
    }

    void Device_GL::uploadIndexBufferData(DeviceResourceHandle handle, const Byte* data, UInt32 dataSize)
    {
        const auto& indexBuffer = m_resourceMapper.getResource(handle);
        assert(dataSize <= indexBuffer.getTotalSizeInBytes());

        glBindVertexArray(0u); // make sure no VAO affected
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer.getGPUAddress());
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, dataSize, data, GL_STATIC_DRAW);
    }

    void Device_GL::deleteIndexBuffer(DeviceResourceHandle handle)
    {
        const GLHandle resourceAddress = m_resourceMapper.getResource(handle).getGPUAddress();
        glDeleteBuffers(1, &resourceAddress);
        m_resourceMapper.deleteResource(handle);
    }

    std::unique_ptr<const GPUResource> Device_GL::uploadShader(const EffectResource& shader)
    {
        ShaderProgramInfo programInfo;
        String debugErrorLog;
        const Bool uploadSuccessful = ShaderUploader_GL::UploadShaderProgramFromSource(shader, programInfo, debugErrorLog);

        if (uploadSuccessful)
            return std::make_unique<const ShaderGPUResource_GL>(shader, programInfo);
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "Device_GL::uploadShader: shader upload failed: " << debugErrorLog);
            return nullptr;
        }
    }

    DeviceResourceHandle Device_GL::registerShader(std::unique_ptr<const GPUResource> shaderResource)
    {
        return m_resourceMapper.registerResource(std::move(shaderResource));
    }

    DeviceResourceHandle Device_GL::uploadBinaryShader(const EffectResource& shader, const UInt8* binaryShaderData, UInt32 binaryShaderDataSize, BinaryShaderFormatID binaryShaderFormat)
    {
        ShaderProgramInfo programInfo;
        String debugErrorLog;
        const Bool uploadSuccessful = ShaderUploader_GL::UploadShaderProgramFromBinary(binaryShaderData, binaryShaderDataSize, binaryShaderFormat, programInfo, debugErrorLog);

        if (uploadSuccessful)
        {
            LOG_INFO(CONTEXT_SMOKETEST, "Device_GL::uploadShader: renderer successfully uploaded binary shader for effect " << shader.getName());
            return m_resourceMapper.registerResource(std::make_unique<ShaderGPUResource_GL>(shader, programInfo));
        }
        else
        {
            LOG_INFO(CONTEXT_RENDERER, "Device_GL::uploadShader: renderer failed to upload binary shader for effect " << shader.getName() << ". Error was: " << debugErrorLog);
            return DeviceResourceHandle::Invalid();
        }
    }

    Bool Device_GL::getBinaryShader(DeviceResourceHandle handle, UInt8Vector& binaryShader, BinaryShaderFormatID& binaryShaderFormat)
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
            m_activeShader = nullptr;
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

    void Device_GL::activateTexture(DeviceResourceHandle handle, DataFieldHandle field)
    {
        GLInputLocation uniformLocation;
        if (getUniformLocation(field, uniformLocation))
        {
            const TextureSlotInfo textureSlot = m_activeShader->getTextureSlot(field);

            assert(static_cast<UInt32>(textureSlot.slot) < m_limits.getMaximumTextureUnits());
            glActiveTexture(GL_TEXTURE0 + textureSlot.slot);

            const auto renderTargetPair = std::find_if(m_pairedRenderTargets.cbegin(), m_pairedRenderTargets.cend(), [handle](const RenderTargetPair& rtPair) -> bool {return rtPair.colorBuffers[0] == handle; });

            const GPUResource* resource = nullptr;
            if (renderTargetPair != m_pairedRenderTargets.cend())
            {
                resource = &m_resourceMapper.getResource(renderTargetPair->colorBuffers[renderTargetPair->readingIndex]);
            }
            else
            {
                resource = &m_resourceMapper.getResource(handle);
            }

            const GLenum target = TypesConversion_GL::GetTextureTargetFromTextureInputType(textureSlot.textureType);
            glBindTexture(target, resource->getGPUAddress());
            glUniform1i(uniformLocation.getValue(), textureSlot.slot);
        }
        else
        {
            LOG_DEBUG(CONTEXT_RENDERER, "Device_GL::activateTexture could not find uniform location for field :" << field.asMemoryHandle() << ").");
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
        return m_apiExtensions.contains(extensionName);
    }

    void Device_GL::loadOpenGLExtensions()
    {
        GLint numExtensions = 0;
        glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
        if (numExtensions > 0)
        {
            m_apiExtensions.reserve(numExtensions);
            size_t sumExtensionStringLength = 0;
            for (auto i = 0; i < numExtensions; i++)
            {
                const auto tmp = reinterpret_cast<const Char*>(glGetStringi(GL_EXTENSIONS, i));
                sumExtensionStringLength += std::strlen(tmp);
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

    void Device_GL::queryDeviceDependentFeatures()
    {
        GLint max_textures(0);
        glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &max_textures);
        m_limits.setMaximumTextureUnits(max_textures);

        GLint numCompressedTextureFormats(0);
        glGetIntegerv(GL_NUM_COMPRESSED_TEXTURE_FORMATS, &numCompressedTextureFormats);
        if(0 != numCompressedTextureFormats)
        {
            std::vector<GLint> compressedTextureFormats(numCompressedTextureFormats);
            glGetIntegerv(GL_COMPRESSED_TEXTURE_FORMATS, compressedTextureFormats.data());

            for (GLint compressedGLTextureFormat : compressedTextureFormats)
            {
                const ETextureFormat textureFormat = TypesConversion_GL::GetTextureFormatFromCompressedGLTextureFormat(compressedGLTextureFormat);
                if (ETextureFormat::Invalid != textureFormat)
                {
                    m_limits.addTextureFormat(textureFormat);
                }
            }
        }

        GLint maxMSAASamples{ 0 };
        glGetIntegerv(GL_MAX_SAMPLES, &maxMSAASamples);
        m_limits.setMaximumSamples(maxMSAASamples);

        std::array<GLint, 2> maxViewport;
        glGetIntegerv(GL_MAX_VIEWPORT_DIMS, maxViewport.data());
        m_limits.setMaxViewport(maxViewport[0], maxViewport[1]);

        GLint maxNumberOfBinaryFormats = 0;
        // binary shader formats
        glGetIntegerv(GL_NUM_SHADER_BINARY_FORMATS, &maxNumberOfBinaryFormats);
        if(0 != maxNumberOfBinaryFormats)
        {
            std::vector<GLint> suppportedBinaryShaderFormats(maxNumberOfBinaryFormats);
            glGetIntegerv(GL_SHADER_BINARY_FORMATS, suppportedBinaryShaderFormats.data());
            LOG_INFO_F(CONTEXT_RENDERER, ([&](StringOutputStream & sos)
            {
                sos << "Device_GL::queryDeviceDependentFeatures: supported binary shader formats (GL_SHADER_BINARY_FORMATS):";
                for (GLint binaryShaderFormat : suppportedBinaryShaderFormats)
                    sos << "  " << binaryShaderFormat;
            }));
        }

        // binary program formats
        glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &maxNumberOfBinaryFormats);
        if(0 != maxNumberOfBinaryFormats)
        {
            m_supportedBinaryProgramFormats.resize(maxNumberOfBinaryFormats);
            glGetIntegerv(GL_PROGRAM_BINARY_FORMATS, m_supportedBinaryProgramFormats.data());
            LOG_INFO_F(CONTEXT_RENDERER, ([&](StringOutputStream & sos)
            {
                sos << "Device_GL::queryDeviceDependentFeatures: supported binary program formats (GL_PROGRAM_BINARY_FORMATS):";
                for (GLint binaryProgramFormat : m_supportedBinaryProgramFormats)
                    sos << "  " << binaryProgramFormat;
            }));
        }

        if (isApiExtensionAvailable("GL_EXT_texture_filter_anisotropic"))
        {
            GLint anisotropy = 0;
            glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &anisotropy);
            m_limits.setMaximumAnisotropy(anisotropy);
        }
        else
        {
            LOG_WARN(CONTEXT_RENDERER, "Device_GL::queryDeviceDependentFeatures:  anisotropic filtering not available on this device");
        }

        GLint maxDrawBuffers{ 0 };
        glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxDrawBuffers);
        m_limits.setMaximumDrawBuffers(maxDrawBuffers);
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

    void Device_GL::getSupportedBinaryProgramFormats(std::vector<BinaryShaderFormatID>& formats) const
    {
        formats.resize(m_supportedBinaryProgramFormats.size());
        std::transform(m_supportedBinaryProgramFormats.cbegin(), m_supportedBinaryProgramFormats.cend(), formats.begin(), [](GLint id) { return BinaryShaderFormatID{ uint32_t(id) }; });
    }

    void Device_GL::flush()
    {
        glFlush();
    }
}
