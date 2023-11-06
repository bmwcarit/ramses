//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Platform/OpenGL/Device_GL.h"

#include "internal/RendererLib/PlatformBase/RenderTargetGpuResource.h"
#include "internal/RendererLib/PlatformBase/RenderBufferGPUResource.h"
#include "internal/RendererLib/PlatformBase/IndexBufferGPUResource.h"
#include "internal/RendererLib/PlatformBase/VertexArrayGPUResource.h"

#include "internal/Platform/OpenGL/Device_GL_platform.h"
#include "internal/Platform/OpenGL/ShaderGPUResource_GL.h"
#include "internal/Platform/OpenGL/ShaderUploader_GL.h"
#include "internal/Platform/OpenGL/ShaderProgramInfo.h"
#include "internal/Platform/OpenGL/TypesConversion_GL.h"

#include "internal/SceneGraph/SceneAPI/PixelRectangle.h"
#include "internal/RendererLib/PlatformInterface/IContext.h"
#include "internal/RendererLib/PlatformInterface/IDeviceExtension.h"
#include "internal/SceneGraph/Resource/EffectResource.h"

#include "internal/Core/Utils/LogMacros.h"
#include "internal/Core/Utils/TextureMathUtils.h"
#include "internal/PlatformAbstraction/PlatformStringUtils.h"
#include "internal/PlatformAbstraction/Macros.h"
#include "glm/gtc/type_ptr.hpp"

#include "impl/TextureEnumsImpl.h"

namespace ramses::internal
{
    static constexpr GLboolean ToGLboolean(bool b)
    {
        return b ? GL_TRUE : GL_FALSE;
    }

    // TODO Violin move again to other files, once GL headers are consolidated
    struct GLTextureInfo
    {
        GLenum target = GL_ZERO;
        GLsizei width = 0u;
        GLsizei height = 0u;
        GLsizei depth = 0u;
        TextureUploadParams_GL uploadParams;
    };

    // TODO Violin move again to other files, once GL headers are consolidated
    class TextureGPUResource_GL : public GPUResource
    {
    public:
        TextureGPUResource_GL(const GLTextureInfo& textureInfo, uint32_t gpuAddress, uint32_t dataSizeInBytes)
            : GPUResource(gpuAddress, dataSizeInBytes)
            , m_textureInfo(textureInfo)
        {

        }
        const GLTextureInfo m_textureInfo;
    };

    Device_GL::Device_GL(IContext& context, IDeviceExtension* deviceExtension)
        : Device_Base(context)
        , m_activePrimitiveDrawMode(EDrawMode::Triangles)
        , m_activeIndexArrayElementSizeBytes(2u)
        , m_deviceExtension(deviceExtension)
        , m_emptyExternalTextureResource(m_resourceMapper.registerResource(std::make_unique<GPUResource>(0u, 0u)))
    {
#if defined _DEBUG
        m_debugOutput.enable(context);
#endif

        m_limits.addTextureFormat(EPixelStorageFormat::Depth16);
        m_limits.addTextureFormat(EPixelStorageFormat::Depth24);
        m_limits.addTextureFormat(EPixelStorageFormat::Depth32);
        m_limits.addTextureFormat(EPixelStorageFormat::Depth24_Stencil8);

        m_limits.addTextureFormat(EPixelStorageFormat::RGBA8);
        m_limits.addTextureFormat(EPixelStorageFormat::RGB8);
        m_limits.addTextureFormat(EPixelStorageFormat::RGBA5551);
        m_limits.addTextureFormat(EPixelStorageFormat::RGBA4);
        m_limits.addTextureFormat(EPixelStorageFormat::RGB565);

        m_limits.addTextureFormat(EPixelStorageFormat::R8);
        m_limits.addTextureFormat(EPixelStorageFormat::RG8);

        m_limits.addTextureFormat(EPixelStorageFormat::R16F);
        m_limits.addTextureFormat(EPixelStorageFormat::R32F);
        m_limits.addTextureFormat(EPixelStorageFormat::RG16F);
        m_limits.addTextureFormat(EPixelStorageFormat::RG32F);
        m_limits.addTextureFormat(EPixelStorageFormat::RGB16F);
        m_limits.addTextureFormat(EPixelStorageFormat::RGB32F);
        m_limits.addTextureFormat(EPixelStorageFormat::RGBA16F);
        m_limits.addTextureFormat(EPixelStorageFormat::RGBA32F);

        m_limits.addTextureFormat(EPixelStorageFormat::SRGB8);
        m_limits.addTextureFormat(EPixelStorageFormat::SRGB8_ALPHA8);
    }

    Device_GL::~Device_GL()
    {
        for (const auto& it : m_textureSamplerObjectsCache)
            deleteTextureSampler(it.second);

        m_resourceMapper.deleteResource(m_framebufferRenderTarget);
    }

    bool Device_GL::init()
    {
        LOAD_ALL_API_PROCS(m_context);

        const char* tmp = nullptr;

        tmp = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
        LOG_INFO(CONTEXT_RENDERER, "Device_GL::init:  OpenGL vendor is " << tmp);

        tmp = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
        LOG_INFO(CONTEXT_RENDERER, "    OpenGL renderer is " << tmp);

        tmp = reinterpret_cast<const char*>(glGetString(GL_VERSION));
        LOG_INFO(CONTEXT_RENDERER, "     OpenGL version is " << tmp);

        tmp = reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION));
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

    void Device_GL::drawIndexedTriangles(int32_t startOffset, int32_t elementCount, uint32_t instanceCount)
    {
        assert(m_activeIndexArraySizeBytes != 0 && m_activeIndexArrayElementSizeBytes != 0);
        if (m_activeIndexArraySizeBytes < (startOffset + elementCount) * m_activeIndexArrayElementSizeBytes)
        {
            LOG_ERROR_P(CONTEXT_RENDERER, "Device_GL::drawIndexedTriangles: index buffer access out of bounds "
                "[drawStartOffset={} drawElementCount={} IndexBufferElementCount={}]",
                startOffset, elementCount, m_activeIndexArraySizeBytes / m_activeIndexArrayElementSizeBytes);
            return;
        }

        const size_t startOffsetAddressAsUInt = startOffset * m_activeIndexArrayElementSizeBytes;
        const GLvoid* startOffsetAddress = reinterpret_cast<void*>(startOffsetAddressAsUInt);

        const GLenum drawModeGL = TypesConversion_GL::GetDrawMode(m_activePrimitiveDrawMode);
        const GLenum elementTypeGL = TypesConversion_GL::GetIndexElementType(m_activeIndexArrayElementSizeBytes);
        glDrawElementsInstanced(drawModeGL, elementCount, elementTypeGL, startOffsetAddress, static_cast<GLsizei>(instanceCount));

        // For profiling/tests
        Device_Base::drawIndexedTriangles(startOffset, elementCount, instanceCount);
    }

    void Device_GL::drawTriangles(int32_t startOffset, int32_t elementCount, uint32_t instanceCount)
    {
        const GLenum drawModeGL = TypesConversion_GL::GetDrawMode(m_activePrimitiveDrawMode);
        glDrawArraysInstanced(drawModeGL, startOffset, elementCount, static_cast<GLsizei>(instanceCount));

        // For profiling/tests
        Device_Base::drawTriangles(startOffset, elementCount, instanceCount);
    }

    void Device_GL::clear(ClearFlags clearFlags)
    {
        GLbitfield deviceClearFlags = 0;
        if (clearFlags.isSet(EClearFlag::Color))
        {
            // NOLINTNEXTLINE(hicpp-signed-bitwise)
            deviceClearFlags |= GL_COLOR_BUFFER_BIT;
        }
        if (clearFlags.isSet(EClearFlag::Depth))
        {
            // NOLINTNEXTLINE(hicpp-signed-bitwise)
            deviceClearFlags |= GL_DEPTH_BUFFER_BIT;
        }
        if (clearFlags.isSet(EClearFlag::Stencil))
        {
            // NOLINTNEXTLINE(hicpp-signed-bitwise)
            deviceClearFlags |= GL_STENCIL_BUFFER_BIT;
        }

        glClear(deviceClearFlags);
    }

    void Device_GL::clearColor(const glm::vec4& clearColor)
    {
        glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
    }

    void Device_GL::colorMask(bool r, bool g, bool b, bool a)
    {
        glColorMask(ToGLboolean(r),
                    ToGLboolean(g),
                    ToGLboolean(b),
                    ToGLboolean(a));
    }

    void Device_GL::clearDepth(float d)
    {
        glClearDepthf(d);
    }

    void Device_GL::clearStencil(int32_t s)
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

    void Device_GL::blendColor(const glm::vec4& color)
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

    void Device_GL::stencilFunc(EStencilFunc func, uint8_t ref, uint8_t mask)
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

        glViewport(x, y, static_cast<GLsizei>(std::min(width, m_limits.getMaxViewportWidth())), static_cast<GLsizei>(std::min(height, m_limits.getMaxViewportHeight())));
    }

    GLHandle Device_GL::createTexture(uint32_t width, uint32_t height, EPixelStorageFormat storageFormat, uint32_t sampleCount) const
    {
        LOG_DEBUG(CONTEXT_RENDERER, "Device_GL::createTexture:  creating a new texture (texture render target)");

        const GLHandle texID = GenerateAndBindTexture((sampleCount) != 0u ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D);
        GLTextureInfo texInfo;
        fillGLInternalTextureInfo((sampleCount) != 0u ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D,
                                  width,
                                  height,
                                  1u,
                                  storageFormat,
                                  {ETextureChannelColor::Red, ETextureChannelColor::Green, ETextureChannelColor::Blue, ETextureChannelColor::Alpha},
                                  texInfo);
        AllocateTextureStorage(texInfo, 1u, sampleCount);

        return texID;
    }

    GLHandle Device_GL::CreateRenderBuffer(uint32_t width, uint32_t height, EPixelStorageFormat format, uint32_t sampleCount)
    {
        LOG_TRACE(CONTEXT_RENDERER, "Creating a new render buffer");

        GLHandle renderbufferHandle = InvalidGLHandle;
        glGenRenderbuffers(1, &renderbufferHandle);
        glBindRenderbuffer(GL_RENDERBUFFER, renderbufferHandle);

        GLenum internalFormat(0);
        switch (format)
        {
        case EPixelStorageFormat::Depth16:
            internalFormat = GL_DEPTH_COMPONENT16;
            break;
        case EPixelStorageFormat::Depth24:
            internalFormat = GL_DEPTH_COMPONENT24;
            break;
        case EPixelStorageFormat::Depth32:
            internalFormat = GL_DEPTH_COMPONENT32F;
            break;
        case EPixelStorageFormat::Depth24_Stencil8:
            internalFormat = GL_DEPTH24_STENCIL8;
            break;
        case EPixelStorageFormat::RGBA8:
            internalFormat = GL_RGBA8;
            break;
        default:
            assert(false && "Unknown render buffer format");
        }

        sampleCount = CheckAndClampNumberOfSamples(internalFormat, sampleCount);

        glRenderbufferStorageMultisample(GL_RENDERBUFFER, static_cast<GLsizei>(sampleCount), internalFormat, static_cast<GLsizei>(width), static_cast<GLsizei>(height));

        return renderbufferHandle;
    }

    bool Device_GL::setConstant(DataFieldHandle field, uint32_t count, const float* value)
    {
        const auto uniformLocation = m_activeShader->getUniformLocation(field);
        if (uniformLocation.isValid())
            glUniform1fv(uniformLocation.getValue(), static_cast<GLsizei>(count), value);
        return uniformLocation.isValid();
    }

    bool Device_GL::setConstant(DataFieldHandle field, uint32_t count, const glm::vec2* value)
    {
        const auto uniformLocation = m_activeShader->getUniformLocation(field);
        if (uniformLocation.isValid())
            glUniform2fv(uniformLocation.getValue(), static_cast<GLsizei>(count), glm::value_ptr(value[0]));
        return uniformLocation.isValid();
    }

    bool Device_GL::setConstant(DataFieldHandle field, uint32_t count, const glm::vec3* value)
    {
        const auto uniformLocation = m_activeShader->getUniformLocation(field);
        if (uniformLocation.isValid())
            glUniform3fv(uniformLocation.getValue(), static_cast<GLsizei>(count), glm::value_ptr(value[0]));
        return uniformLocation.isValid();
    }

    bool Device_GL::setConstant(DataFieldHandle field, uint32_t count, const glm::vec4* value)
    {
        const auto uniformLocation = m_activeShader->getUniformLocation(field);
        if (uniformLocation.isValid())
            glUniform4fv(uniformLocation.getValue(), static_cast<GLsizei>(count), glm::value_ptr(value[0]));
        return uniformLocation.isValid();
    }

    bool Device_GL::setConstant(DataFieldHandle field, uint32_t count, const bool* value)
    {
        // GL does not provide native bool on its API but it is a common practice to use integer for setting bool uniforms
        m_containerForBoolValues.assign(count, 0);
        for (uint32_t i = 0; i < count; ++i)
        {
            if (value[i])
                m_containerForBoolValues[i] = 1;
        }

        const auto uniformLocation = m_activeShader->getUniformLocation(field);
        if (uniformLocation.isValid())
            glUniform1iv(uniformLocation.getValue(), static_cast<GLsizei>(count), m_containerForBoolValues.data());
        return uniformLocation.isValid();
    }

    bool Device_GL::setConstant(DataFieldHandle field, uint32_t count, const int32_t* value)
    {
        const auto uniformLocation = m_activeShader->getUniformLocation(field);
        if (uniformLocation.isValid())
            glUniform1iv(uniformLocation.getValue(), static_cast<GLsizei>(count), value);
        return uniformLocation.isValid();
    }

    bool Device_GL::setConstant(DataFieldHandle field, uint32_t count, const glm::ivec2* value)
    {
        const auto uniformLocation = m_activeShader->getUniformLocation(field);
        if (uniformLocation.isValid())
            glUniform2iv(uniformLocation.getValue(), static_cast<GLsizei>(count), glm::value_ptr(value[0]));
        return uniformLocation.isValid();
    }

    bool Device_GL::setConstant(DataFieldHandle field, uint32_t count, const glm::ivec3* value)
    {
        const auto uniformLocation = m_activeShader->getUniformLocation(field);
        if (uniformLocation.isValid())
            glUniform3iv(uniformLocation.getValue(), static_cast<GLsizei>(count), glm::value_ptr(value[0]));
        return uniformLocation.isValid();
    }

    bool Device_GL::setConstant(DataFieldHandle field, uint32_t count, const glm::ivec4* value)
    {
        const auto uniformLocation = m_activeShader->getUniformLocation(field);
        if (uniformLocation.isValid())
            glUniform4iv(uniformLocation.getValue(), static_cast<GLsizei>(count), glm::value_ptr(value[0]));
        return uniformLocation.isValid();
    }

    bool Device_GL::setConstant(DataFieldHandle field, uint32_t count, const glm::mat2* value)
    {
        const auto uniformLocation = m_activeShader->getUniformLocation(field);
        if (uniformLocation.isValid())
            glUniformMatrix2fv(uniformLocation.getValue(), static_cast<GLsizei>(count), ToGLboolean(false), glm::value_ptr(value[0]));
        return uniformLocation.isValid();
    }

    bool Device_GL::setConstant(DataFieldHandle field, uint32_t count, const glm::mat3* value)
    {
        const auto uniformLocation = m_activeShader->getUniformLocation(field);
        if (uniformLocation.isValid())
            glUniformMatrix3fv(uniformLocation.getValue(), static_cast<GLsizei>(count), ToGLboolean(false), glm::value_ptr(value[0]));
        return uniformLocation.isValid();
    }

    bool Device_GL::setConstant(DataFieldHandle field, uint32_t count, const glm::mat4* value)
    {
        const auto uniformLocation = m_activeShader->getUniformLocation(field);
        if (uniformLocation.isValid())
            glUniformMatrix4fv(uniformLocation.getValue(), static_cast<GLsizei>(count), ToGLboolean(false), glm::value_ptr(value[0]));
        return uniformLocation.isValid();
    }

    DeviceResourceHandle Device_GL::allocateTexture2D(uint32_t width, uint32_t height, EPixelStorageFormat textureFormat, const TextureSwizzleArray& swizzle, uint32_t mipLevelCount, uint32_t totalSizeInBytes)
    {
        const GLHandle texID = GenerateAndBindTexture(GL_TEXTURE_2D);
        GLTextureInfo texInfo;
        fillGLInternalTextureInfo(GL_TEXTURE_2D, width, height, 1u, textureFormat, swizzle, texInfo);
        AllocateTextureStorage(texInfo, mipLevelCount);

        return m_resourceMapper.registerResource(std::make_unique<TextureGPUResource_GL>(texInfo, texID, totalSizeInBytes));
    }

    DeviceResourceHandle Device_GL::allocateTexture3D(uint32_t width, uint32_t height, uint32_t depth, EPixelStorageFormat textureFormat, uint32_t mipLevelCount, uint32_t totalSizeInBytes)
    {
        const GLHandle texID = GenerateAndBindTexture(GL_TEXTURE_3D);
        GLTextureInfo texInfo;
        fillGLInternalTextureInfo(GL_TEXTURE_3D, width, height, depth, textureFormat, DefaultTextureSwizzleArray, texInfo);
        AllocateTextureStorage(texInfo, mipLevelCount);

        return m_resourceMapper.registerResource(std::make_unique<TextureGPUResource_GL>(texInfo, texID, totalSizeInBytes));
    }

    DeviceResourceHandle Device_GL::allocateTextureCube(uint32_t faceSize, EPixelStorageFormat textureFormat, const TextureSwizzleArray& swizzle, uint32_t mipLevelCount, uint32_t totalSizeInBytes)
    {
        const GLHandle texID = GenerateAndBindTexture(GL_TEXTURE_CUBE_MAP);
        GLTextureInfo texInfo;
        fillGLInternalTextureInfo(GL_TEXTURE_CUBE_MAP, faceSize, faceSize, 1u, textureFormat, swizzle, texInfo);
        AllocateTextureStorage(texInfo, mipLevelCount);

        return m_resourceMapper.registerResource(std::make_unique<TextureGPUResource_GL>(texInfo, texID, totalSizeInBytes));
    }

    DeviceResourceHandle Device_GL::allocateExternalTexture()
    {
        if (m_limits.isExternalTextureExtensionSupported())
        {
            const auto textureTarget = GL_TEXTURE_EXTERNAL_OES;
            const GLHandle texID = GenerateAndBindTexture(textureTarget);
            GLTextureInfo texInfo;
            fillGLInternalTextureInfo(textureTarget, 0u, 0u, 1u, EPixelStorageFormat::RGBA8, {}, texInfo);

            return m_resourceMapper.registerResource(std::make_unique<TextureGPUResource_GL>(texInfo, texID, 0u));
        }
        LOG_ERROR(CONTEXT_RENDERER, "Device_GL::allocateExternalTexture: feature not supported on platform");
        return {};
    }

    DeviceResourceHandle Device_GL::getEmptyExternalTexture() const
    {
        return m_emptyExternalTextureResource;
    }

    void Device_GL::bindTexture(DeviceResourceHandle handle)
    {
        const auto& gpuResource = m_resourceMapper.getResourceAs<TextureGPUResource_GL>(handle);
        glBindTexture(gpuResource.m_textureInfo.target, gpuResource.getGPUAddress());
    }

    void Device_GL::generateMipmaps(DeviceResourceHandle handle)
    {
        const auto& gpuResource = m_resourceMapper.getResourceAs<TextureGPUResource_GL>(handle);
        glGenerateMipmap(gpuResource.m_textureInfo.target);
    }

    void Device_GL::uploadTextureData(DeviceResourceHandle handle, uint32_t mipLevel, uint32_t x, uint32_t y, uint32_t z, uint32_t width, uint32_t height, uint32_t depth, const std::byte* data, uint32_t dataSize, uint32_t stride)
    {
        const auto& gpuResource = m_resourceMapper.getResourceAs<TextureGPUResource_GL>(handle);

        GLTextureInfo texInfo = gpuResource.m_textureInfo;
        // in case of cube texture faceID is encoded in Z offset
        if (texInfo.target == GL_TEXTURE_CUBE_MAP)
        {
            texInfo.target = TypesConversion_GL::GetCubemapFaceSpecifier(static_cast<ETextureCubeFace>(z));
            z = 0u;
        }
        UploadTextureMipMapData(mipLevel, x, y, z, width, height, depth, texInfo, data, dataSize, stride);
    }

    DeviceResourceHandle Device_GL::uploadStreamTexture2D(DeviceResourceHandle handle, uint32_t width, uint32_t height, EPixelStorageFormat format, const std::byte* data, const TextureSwizzleArray& swizzle)
    {
        if (!handle.isValid())
        {
            // generate texID and register texture resource
            assert(data == nullptr);
            GLHandle texID = InvalidGLHandle;
            glGenTextures(1, &texID);

            return m_resourceMapper.registerResource(std::make_unique<GPUResource>(texID, 0u));
        }
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

    void Device_GL::fillGLInternalTextureInfo(GLenum target, uint32_t width, uint32_t height, uint32_t depth, EPixelStorageFormat textureFormat, const TextureSwizzleArray& swizzle, GLTextureInfo& glTexInfoOut) const
    {
        glTexInfoOut.target = target;
        glTexInfoOut.width = static_cast<GLsizei>(width);
        glTexInfoOut.height = static_cast<GLsizei>(height);
        glTexInfoOut.depth = static_cast<GLsizei>(depth);

        if (!m_limits.isTextureFormatAvailable(textureFormat))
        {
            LOG_ERROR(CONTEXT_RENDERER, "Device_GL::createGLInternalTextureInfo: Unsupported texture format " << EnumToString(textureFormat));
            assert(false && "Device_GL::createGLInternalTextureInfo unsupported texture format");
        }

        glTexInfoOut.uploadParams = TypesConversion_GL::GetTextureUploadParams(textureFormat);
        glTexInfoOut.uploadParams.swizzle = swizzle;
    }

    uint32_t Device_GL::CheckAndClampNumberOfSamples(GLenum internalFormat, uint32_t numSamples)
    {
        if (numSamples > 1)
        {
            GLint maxNumSamplesGL = 0;
            glGetInternalformativ(GL_RENDERBUFFER, internalFormat, GL_SAMPLES, 1, &maxNumSamplesGL);
            auto maxNumSamples = static_cast<uint32_t>(maxNumSamplesGL);
            if (numSamples > maxNumSamples)
            {
                LOG_WARN_P(CONTEXT_RENDERER, "Device_GL: clamping requested MSAA sample count {} "
                    "to {}, a maximum number of samples supported by device for this format.", numSamples, maxNumSamples);
                numSamples = maxNumSamples;
            }
        }

        return numSamples;
    }

    void Device_GL::AllocateTextureStorage(const GLTextureInfo& texInfo, uint32_t mipLevels, uint32_t sampleCount)
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
            glTexStorage2D(texInfo.target, static_cast<GLsizei>(mipLevels), texInfo.uploadParams.sizedInternalFormat, texInfo.width, texInfo.height);
            break;
        case GL_TEXTURE_2D_MULTISAMPLE:
            sampleCount = CheckAndClampNumberOfSamples(texInfo.uploadParams.sizedInternalFormat, sampleCount);
            glTexStorage2DMultisample(texInfo.target, static_cast<GLsizei>(sampleCount), texInfo.uploadParams.sizedInternalFormat, texInfo.width, texInfo.height, ToGLboolean(true));
            break;
        case GL_TEXTURE_3D:
            glTexStorage3D(texInfo.target, static_cast<GLsizei>(mipLevels), texInfo.uploadParams.sizedInternalFormat, texInfo.width, texInfo.height, texInfo.depth);
            break;
        default:
            assert(false);
            break;
        }
    }

    void Device_GL::UploadTextureMipMapData(uint32_t mipLevel, uint32_t x, uint32_t y, uint32_t z, uint32_t width, uint32_t height, uint32_t depth, const GLTextureInfo& texInfo, const std::byte *pData, uint32_t dataSize, uint32_t stride)
    {
        assert(width > 0 && height > 0 && depth > 0 && "trying to upload texture with 0 width and/or height and/or depth!");
        assert(x + width <= TextureMathUtils::GetMipSize(mipLevel, texInfo.width));
        assert(y + height <= TextureMathUtils::GetMipSize(mipLevel, texInfo.height));
        assert(z + depth <= TextureMathUtils::GetMipSize(mipLevel, texInfo.depth));
        assert(dataSize > 0u || !texInfo.uploadParams.compressed);

        glPixelStorei(GL_UNPACK_ROW_LENGTH, static_cast<GLint>(stride));
        glPixelStorei(GL_UNPACK_SKIP_ROWS, static_cast<GLint>(y));
        glPixelStorei(GL_UNPACK_SKIP_PIXELS, static_cast<GLint>(x));

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
                glCompressedTexSubImage2D(texInfo.target,
                    static_cast<GLint>(mipLevel),
                    static_cast<GLint>(x),
                    static_cast<GLint>(y),
                    static_cast<GLsizei>(width),
                    static_cast<GLsizei>(height),
                    texInfo.uploadParams.sizedInternalFormat,
                    static_cast<GLsizei>(dataSize),
                    pData);
            }
            else
            {
                glTexSubImage2D(texInfo.target,
                    static_cast<GLint>(mipLevel),
                    static_cast<GLint>(x),
                    static_cast<GLint>(y),
                    static_cast<GLsizei>(width),
                    static_cast<GLsizei>(height),
                    texInfo.uploadParams.baseInternalFormat,
                    texInfo.uploadParams.type,
                    pData);
            }
            break;
        case GL_TEXTURE_3D:
            if (texInfo.uploadParams.compressed)
            {
                glCompressedTexSubImage3D(texInfo.target,
                    static_cast<GLint>(mipLevel),
                    static_cast<GLint>(x),
                    static_cast<GLint>(y),
                    static_cast<GLint>(z),
                    static_cast<GLsizei>(width),
                    static_cast<GLsizei>(height),
                    static_cast<GLsizei>(depth),
                    texInfo.uploadParams.sizedInternalFormat,
                    static_cast<GLsizei>(dataSize),
                    pData);
            }
            else
            {
                glTexSubImage3D(texInfo.target,
                    static_cast<GLint>(mipLevel),
                    static_cast<GLint>(x),
                    static_cast<GLint>(y),
                    static_cast<GLint>(z),
                    static_cast<GLsizei>(width),
                    static_cast<GLsizei>(height),
                    static_cast<GLsizei>(depth),
                    texInfo.uploadParams.baseInternalFormat,
                    texInfo.uploadParams.type,
                    pData);
            }
            break;
        default:
            assert(false);
            break;
        }

        // restore default values
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
        glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    }

    DeviceResourceHandle Device_GL::uploadRenderBuffer(uint32_t width, uint32_t height, EPixelStorageFormat format, ERenderBufferAccessMode accessMode, uint32_t sampleCount)
    {
        GLHandle bufferGLHandle = InvalidGLHandle;
        switch (accessMode)
        {
        case ERenderBufferAccessMode::ReadWrite:
            bufferGLHandle = createTexture(width, height, format, sampleCount);
            break;
        case ERenderBufferAccessMode::WriteOnly:
            bufferGLHandle = CreateRenderBuffer(width, height, format, sampleCount);
            break;
        default:
            assert(false);
        }

        if (bufferGLHandle != InvalidGLHandle)
            return m_resourceMapper.registerResource(std::make_unique<RenderBufferGPUResource>(bufferGLHandle, width, height, format, sampleCount, accessMode));

        return DeviceResourceHandle::Invalid();
    }

    DeviceResourceHandle Device_GL::uploadDmaRenderBuffer(uint32_t width, uint32_t height, DmaBufferFourccFormat fourccFormat, DmaBufferUsageFlags usageFlags, DmaBufferModifiers modifiers)
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
        const auto& resource = m_resourceMapper.getResourceAs<RenderBufferGPUResource>(bufferHandle);
        const GLHandle glAddress = resource.getGPUAddress();
        if (ERenderBufferAccessMode::ReadWrite == resource.getAccessMode())
        {
            glDeleteTextures(1, &glAddress);
        }
        else if (ERenderBufferAccessMode::WriteOnly == resource.getAccessMode())
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
        GLuint sampler = 0;
        glGenSamplers(1, &sampler);

        const auto wrappingModeR = TypesConversion_GL::GetWrapMode(samplerStates.m_addressModeR);
        const auto wrappingModeU = TypesConversion_GL::GetWrapMode(samplerStates.m_addressModeU);
        const auto wrappingModeV = TypesConversion_GL::GetWrapMode(samplerStates.m_addressModeV);

        glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, wrappingModeU);
        glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, wrappingModeV);
        glSamplerParameteri(sampler, GL_TEXTURE_WRAP_R, wrappingModeR);

        switch (samplerStates.m_minSamplingMode)
        {
        case ETextureSamplingMethod::Nearest:
            glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            break;
        case ETextureSamplingMethod::Linear:
            glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            break;
        case ETextureSamplingMethod::Nearest_MipMapNearest:
            glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
            break;
        case ETextureSamplingMethod::Nearest_MipMapLinear:
            glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
            break;
        case ETextureSamplingMethod::Linear_MipMapNearest:
            glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
            break;
        case ETextureSamplingMethod::Linear_MipMapLinear:
            glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            break;
        }

        switch (samplerStates.m_magSamplingMode)
        {
        case ETextureSamplingMethod::Nearest:
            glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            break;
        case ETextureSamplingMethod::Linear:
            glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            break;
        case ETextureSamplingMethod::Nearest_MipMapNearest:
        case ETextureSamplingMethod::Nearest_MipMapLinear:
        case ETextureSamplingMethod::Linear_MipMapNearest:
        case ETextureSamplingMethod::Linear_MipMapLinear:
            assert(false);
        }

        // set anisotropy only if feature is supported
        if (m_limits.getMaximumAnisotropy() > 1u)
        {
            // clamp anisotropy value to max supported range
            const auto anisotropyLevel = std::min(samplerStates.m_anisotropyLevel, m_limits.getMaximumAnisotropy());
            glSamplerParameteri(sampler, GL_TEXTURE_MAX_ANISOTROPY_EXT, static_cast<GLint>(anisotropyLevel));
        }

        return m_resourceMapper.registerResource(std::make_unique<GPUResource>(sampler, 0u));
    }

    void Device_GL::deleteTextureSampler(DeviceResourceHandle handle)
    {
        const auto& resource = m_resourceMapper.getResourceAs<GPUResource>(handle);
        const GLHandle glAddress = resource.getGPUAddress();
        glDeleteSamplers(1, &glAddress);

        m_resourceMapper.deleteResource(handle);
    }

    void Device_GL::activateTextureSampler(DeviceResourceHandle handle, DataFieldHandle field)
    {
        const TextureSlot textureSlot = m_activeShader->getTextureSlot(field).slot;
        assert(static_cast<uint32_t>(textureSlot) < m_limits.getMaximumTextureUnits());
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

    bool Device_GL::allBuffersHaveTheSameSize(const DeviceHandleVector& renderBuffers) const
    {
        assert(!renderBuffers.empty());
        assert(renderBuffers.size() <= 16u);

        uint32_t width = 0u;
        uint32_t height = 0u;

        for (size_t i = 0; i < renderBuffers.size(); ++i)
        {
            const auto& bufferGPUResource = m_resourceMapper.getResourceAs<RenderBufferGPUResource>(renderBuffers[i]);

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
            const auto& bufferGPUResource = m_resourceMapper.getResourceAs<RenderBufferGPUResource>(rbHandle);
            BindRenderBufferToRenderTarget(bufferGPUResource, colorBuffers.size());

            if (!IsDepthOrStencilFormat(bufferGPUResource.getStorageFormat()))
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
        const auto& rtResource = m_resourceMapper.getResourceAs<RenderTargetGPUResource>(handle);
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

    void Device_GL::BindRenderBufferToRenderTarget(const RenderBufferGPUResource& renderBufferGpuResource, size_t colorBufferSlot)
    {
        switch (renderBufferGpuResource.getAccessMode())
        {
        case ERenderBufferAccessMode::WriteOnly:
            BindWriteOnlyRenderBufferToRenderTarget(renderBufferGpuResource.getStorageFormat(), colorBufferSlot, renderBufferGpuResource.getGPUAddress());
            break;
        case ERenderBufferAccessMode::ReadWrite:
            BindReadWriteRenderBufferToRenderTarget(renderBufferGpuResource.getStorageFormat(), colorBufferSlot, renderBufferGpuResource.getGPUAddress(), renderBufferGpuResource.getSampleCount() != 0);
            break;
        default:
            assert(false && "invalid render buffer access mode");
            break;
        }
    }

    void Device_GL::BindReadWriteRenderBufferToRenderTarget(EPixelStorageFormat bufferFormat, size_t colorBufferSlot, GLHandle bufferGLHandle, const bool multiSample)
    {
        const int texTarget = (multiSample) ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
        if (IsDepthOnlyFormat(bufferFormat))
        {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texTarget, bufferGLHandle, 0);
        }
        else if (IsDepthOrStencilFormat(bufferFormat))
        {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texTarget, bufferGLHandle, 0);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, texTarget, bufferGLHandle, 0);
        }
        else
        {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(colorBufferSlot), texTarget, bufferGLHandle, 0);
        }
    }

    void Device_GL::BindWriteOnlyRenderBufferToRenderTarget(EPixelStorageFormat bufferFormat, size_t colorBufferSlot, GLHandle bufferGLHandle)
    {
        if (IsDepthOnlyFormat(bufferFormat))
        {
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, bufferGLHandle);
        }
        else if (IsDepthOrStencilFormat(bufferFormat))
        {
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, bufferGLHandle);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, bufferGLHandle);
        }
        else
        {
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(colorBufferSlot), GL_RENDERBUFFER, bufferGLHandle);
        }
    }

    void Device_GL::activateRenderTarget(DeviceResourceHandle handle)
    {
        const auto renderTargetPair = std::find_if(m_pairedRenderTargets.cbegin(), m_pairedRenderTargets.cend(), [handle](const RenderTargetPair& rtPair) -> bool {return rtPair.renderTargets[0] == handle; });

        const GPUResource* rtResource = nullptr;
        if (renderTargetPair != m_pairedRenderTargets.cend())
        {
            const uint8_t writingIndex = (renderTargetPair->readingIndex + 1) % 2;
            rtResource = &m_resourceMapper.getResource(renderTargetPair->renderTargets[writingIndex]);
        }
        else
        {
            rtResource = &m_resourceMapper.getResource(handle);
        }

        const GLHandle rtGlAddress = rtResource->getGPUAddress();
        glBindFramebuffer(GL_FRAMEBUFFER, rtGlAddress);
    }

    void Device_GL::pairRenderTargetsForDoubleBuffering(const std::array<DeviceResourceHandle, 2>& renderTargets, const std::array<DeviceResourceHandle, 2>& colorBuffers)
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

    GLHandle Device_GL::GenerateAndBindTexture(GLenum target)
    {
        GLHandle texID = InvalidGLHandle;
        glGenTextures(1, &texID);
        assert(texID != InvalidGLHandle);
        glBindTexture(target, texID);

        return texID;
    }

    DeviceResourceHandle Device_GL::allocateVertexBuffer(uint32_t totalSizeInBytes)
    {
        GLHandle glAddress = InvalidGLHandle;
        glGenBuffers(1, &glAddress);
        assert(glAddress != InvalidGLHandle);

        return m_resourceMapper.registerResource(std::make_unique<GPUResource>(glAddress, totalSizeInBytes));
    }

    void Device_GL::uploadVertexBufferData(DeviceResourceHandle handle, const std::byte* data, uint32_t dataSize)
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
            const auto& indexBufferGPUResource = m_resourceMapper.getResourceAs<IndexBufferGPUResource>(vertexArrayInfo.indexBuffer);
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
            const auto attributeNumComponents = static_cast<GLint>(EnumToNumComponents(attributeDataType));

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
        const auto& vertexArrayResource = m_resourceMapper.getResourceAs<VertexArrayGPUResource>(handle);
        glBindVertexArray(vertexArrayResource.getGPUAddress());

        const auto indexBuffer = vertexArrayResource.getIndexBufferHandle();
        if (indexBuffer.isValid())
        {
            const auto& indexBufferGPUResource = m_resourceMapper.getResourceAs<IndexBufferGPUResource>(indexBuffer);
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

    DeviceResourceHandle Device_GL::allocateIndexBuffer(EDataType dataType, uint32_t sizeInBytes)
    {
        GLHandle glAddress = InvalidGLHandle;
        glGenBuffers(1, &glAddress);
        assert(glAddress != InvalidGLHandle);
        assert(dataType == EDataType::UInt16 || dataType == EDataType::UInt32);

        return m_resourceMapper.registerResource(std::make_unique<IndexBufferGPUResource>(glAddress, sizeInBytes, dataType == EDataType::UInt16 ? 2 : 4));
    }

    void Device_GL::uploadIndexBufferData(DeviceResourceHandle handle, const std::byte* data, uint32_t dataSize)
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
        std::string debugErrorLog;
        const bool uploadSuccessful = ShaderUploader_GL::UploadShaderProgramFromSource(shader, programInfo, debugErrorLog);

        if (uploadSuccessful)
        {
            return std::make_unique<const ShaderGPUResource_GL>(shader, programInfo);
        }
        LOG_ERROR(CONTEXT_RENDERER, "Device_GL::uploadShader: shader upload failed: " << debugErrorLog);
        return nullptr;
    }

    DeviceResourceHandle Device_GL::registerShader(std::unique_ptr<const GPUResource> shaderResource)
    {
        return m_resourceMapper.registerResource(std::move(shaderResource));
    }

    DeviceResourceHandle Device_GL::uploadBinaryShader(const EffectResource& shader, const std::byte* binaryShaderData, uint32_t binaryShaderDataSize, BinaryShaderFormatID binaryShaderFormat)
    {
        ShaderProgramInfo programInfo;
        std::string debugErrorLog;
        const bool uploadSuccessful = ShaderUploader_GL::UploadShaderProgramFromBinary(binaryShaderData, binaryShaderDataSize, binaryShaderFormat, programInfo, debugErrorLog);

        if (uploadSuccessful)
        {
            LOG_DEBUG(CONTEXT_SMOKETEST, "Device_GL::uploadShader: renderer successfully uploaded binary shader for effect " << shader.getName());
            return m_resourceMapper.registerResource(std::make_unique<ShaderGPUResource_GL>(shader, programInfo));
        }
        LOG_INFO(CONTEXT_RENDERER, "Device_GL::uploadShader: renderer failed to upload binary shader for effect " << shader.getName() << ". Error was: " << debugErrorLog);
        return DeviceResourceHandle::Invalid();
    }

    bool Device_GL::getBinaryShader(DeviceResourceHandle handle, std::vector<std::byte>& binaryShader, BinaryShaderFormatID& binaryShaderFormat)
    {
        binaryShader.clear();

        const auto& shaderProgramGL = m_resourceMapper.getResourceAs<ShaderGPUResource_GL>(handle);
        LOG_TRACE(CONTEXT_RENDERER, "Device_GL::getBinaryShader:  retrieving shader binary for effect with handle " << handle.asMemoryHandle());
        return shaderProgramGL.getBinaryInfo(binaryShader, binaryShaderFormat);
    }

    void Device_GL::deleteShader(DeviceResourceHandle handle)
    {
        const auto& shaderProgramGL = m_resourceMapper.getResourceAs<ShaderGPUResource_GL>(handle);
        if (m_activeShader == &shaderProgramGL)
        {
            m_activeShader = nullptr;
        }

        m_resourceMapper.deleteResource(handle);
    }

    void Device_GL::activateShader(DeviceResourceHandle handle)
    {
        const auto& shaderProgramGL = m_resourceMapper.getResourceAs<ShaderGPUResource_GL>(handle);
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
        const auto uniformLocation = m_activeShader->getUniformLocation(field);
        if (uniformLocation.isValid())
        {
            const TextureSlotInfo textureSlot = m_activeShader->getTextureSlot(field);

            assert(static_cast<uint32_t>(textureSlot.slot) < m_limits.getMaximumTextureUnits());
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

    uint32_t Device_GL::getTextureAddress(DeviceResourceHandle handle) const
    {
        return m_resourceMapper.getResource(handle).getGPUAddress();
    }

    DeviceResourceHandle Device_GL::getFramebufferRenderTarget() const
    {
        return m_framebufferRenderTarget;
    }

    void Device_GL::blitRenderTargets(DeviceResourceHandle rtSrc, DeviceResourceHandle rtDst, const PixelRectangle& srcRect, const PixelRectangle& dstRect, bool colorOnly)
    {
        const GPUResource& rtSrcResource = m_resourceMapper.getResource(rtSrc);
        const GPUResource& rtDstResource = m_resourceMapper.getResource(rtDst);

        const GLuint blittingSourceFrameBuffer = rtSrcResource.getGPUAddress();
        const GLuint blittingDestinationFrameBuffer = rtDstResource.getGPUAddress();

        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        const GLenum blittingMask = (colorOnly ? GL_COLOR_BUFFER_BIT : (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));
        glBindFramebuffer(GL_READ_FRAMEBUFFER, blittingSourceFrameBuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, blittingDestinationFrameBuffer);
        glBlitFramebuffer(static_cast<GLint>(srcRect.x),
            static_cast<GLint>(srcRect.y),
            static_cast<GLint>(srcRect.x + srcRect.width),
            static_cast<GLint>(srcRect.y + srcRect.height),

            static_cast<GLint>(dstRect.x),
            static_cast<GLint>(dstRect.y),
            static_cast<GLint>(dstRect.x + dstRect.width),
            static_cast<GLint>(dstRect.y + dstRect.height),

            blittingMask,
            GL_NEAREST);
    }

    bool Device_GL::isApiExtensionAvailable(const std::string& extensionName) const
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
                const auto tmp = reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, i));
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
                const EPixelStorageFormat textureFormat = TypesConversion_GL::GetTextureFormatFromCompressedGLTextureFormat(compressedGLTextureFormat);
                if (EPixelStorageFormat::Invalid != textureFormat)
                {
                    m_limits.addTextureFormat(textureFormat);
                }
            }
        }

        GLint maxMSAASamples{ 0 };
        glGetIntegerv(GL_MAX_SAMPLES, &maxMSAASamples);
        m_limits.setMaximumSamples(maxMSAASamples);

        std::array<GLint, 2> maxViewport{};
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
            LOG_WARN(CONTEXT_RENDERER, "Device_GL::queryDeviceDependentFeatures: anisotropic filtering not available on this device");
        }

        GLint maxDrawBuffers{ 0 };
        glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxDrawBuffers);
        m_limits.setMaximumDrawBuffers(maxDrawBuffers);

        // There are 2 extensions for external texture, one for using external texture sampler in ES2 shader and one for using it in ES3+ shader.
        // Either of them can be used on client side and therefore we require both.
        const bool externalTexturesSupported = isApiExtensionAvailable("GL_OES_EGL_image_external") && isApiExtensionAvailable("GL_OES_EGL_image_external_essl3");
        LOG_INFO(CONTEXT_RENDERER, fmt::format("Device_GL::queryDeviceDependentFeatures: External textures support = {}", externalTexturesSupported));

        m_limits.setExternalTextureExtensionSupported(externalTexturesSupported);
    }

    void Device_GL::readPixels(uint8_t* buffer, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
    {
        glReadPixels(static_cast<GLint>(x), static_cast<GLint>(y), static_cast<GLsizei>(width), static_cast<GLsizei>(height), GL_RGBA, GL_UNSIGNED_BYTE, static_cast<void*>(buffer));
    }

    uint32_t Device_GL::getTotalGpuMemoryUsageInKB() const
    {
        return m_resourceMapper.getTotalGpuMemoryUsageInKB();
    }

    bool Device_GL::isDeviceStatusHealthy() const
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

    bool Device_GL::isExternalTextureExtensionSupported() const
    {
        return m_limits.isExternalTextureExtensionSupported();
    }

    void Device_GL::flush()
    {
        glFlush();
    }
}
