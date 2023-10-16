//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/RendererLib/LoggingDevice.h"
#include "internal/RendererLib/ConstantLogger.h"
#include "internal/SceneGraph/SceneAPI/RenderBuffer.h"
#include "internal/SceneGraph/Resource/EffectResource.h"
#include "internal/SceneGraph/SceneAPI/TextureSamplerStates.h"
#include "impl/AppearanceEnumsImpl.h"
#include "impl/TextureEnumsImpl.h"

namespace ramses::internal
{
    LoggingDevice::LoggingDevice(const IDevice& deviceDelegate, RendererLogContext& context)
        : m_deviceDelegate(deviceDelegate)
        , m_logContext(context)
    {
    }

    bool LoggingDevice::setConstant(DataFieldHandle field, uint32_t count, const glm::mat2* value)
    {
        ConstantLogger::LogValueArray(field, value, count, m_logContext);
        return true;
    }

    bool LoggingDevice::setConstant(DataFieldHandle field, uint32_t count, const glm::mat3* value)
    {
        ConstantLogger::LogValueArray(field, value, count, m_logContext);
        return true;
    }

    bool LoggingDevice::setConstant(DataFieldHandle field, uint32_t count, const glm::mat4* value)
    {
        ConstantLogger::LogValueArray(field, value, count, m_logContext);
        return true;
    }

    bool LoggingDevice::setConstant(DataFieldHandle field, uint32_t count, const glm::ivec4* value)
    {
        ConstantLogger::LogValueArray(field, value, count, m_logContext);
        return true;
    }

    bool LoggingDevice::setConstant(DataFieldHandle field, uint32_t count, const glm::ivec3* value)
    {
        ConstantLogger::LogValueArray(field, value, count, m_logContext);
        return true;
    }

    bool LoggingDevice::setConstant(DataFieldHandle field, uint32_t count, const glm::ivec2* value)
    {
        ConstantLogger::LogValueArray(field, value, count, m_logContext);
        return true;
    }

    bool LoggingDevice::setConstant(DataFieldHandle field, uint32_t count, const bool* value)
    {
        ConstantLogger::LogValueArray(field, value, count, m_logContext);
        return true;
    }

    bool LoggingDevice::setConstant(DataFieldHandle field, uint32_t count, const int32_t* value)
    {
        ConstantLogger::LogValueArray(field, value, count, m_logContext);
        return true;
    }

    bool LoggingDevice::setConstant(DataFieldHandle field, uint32_t count, const glm::vec4* value)
    {
        ConstantLogger::LogValueArray(field, value, count, m_logContext);
        return true;
    }

    bool LoggingDevice::setConstant(DataFieldHandle field, uint32_t count, const glm::vec3* value)
    {
        ConstantLogger::LogValueArray(field, value, count, m_logContext);
        return true;
    }

    bool LoggingDevice::setConstant(DataFieldHandle field, uint32_t count, const glm::vec2* value)
    {
        ConstantLogger::LogValueArray(field, value, count, m_logContext);
        return true;
    }

    bool LoggingDevice::setConstant(DataFieldHandle field, uint32_t count, const float* value)
    {
        ConstantLogger::LogValueArray(field, value, count, m_logContext);
        return true;
    }

    void LoggingDevice::colorMask(bool r, bool g, bool b, bool a)
    {
        if (m_logContext.isLogLevelFlagEnabled(ERendererLogLevelFlag_Details))
        {
            m_logContext << "set color write mask: [" << r << "; " << g << "; " << b << "; " << a << "]" << RendererLogContext::NewLine;
        }
    }

    void LoggingDevice::clearColor(const glm::vec4& clearColor)
    {
        m_logContext << "clear color to [" << clearColor.x << "; " << clearColor.y << "; " << clearColor.z << "; " << clearColor.w << "]" << RendererLogContext::NewLine;
    }

    void LoggingDevice::blendOperations(EBlendOperation colorOperation, EBlendOperation alphaOperation)
    {
        if (m_logContext.isLogLevelFlagEnabled(ERendererLogLevelFlag_Details))
        {
            m_logContext << "set blend ops: [color: " << EnumToString(colorOperation) << "; alpha: " << EnumToString(alphaOperation) << "]" << RendererLogContext::NewLine;
        }
    }

    void LoggingDevice::blendFactors(EBlendFactor sourceColor, EBlendFactor destinationColor, EBlendFactor sourceAlpha, EBlendFactor destinationAlpha)
    {
        if (m_logContext.isLogLevelFlagEnabled(ERendererLogLevelFlag_Details))
        {
            m_logContext << "set blend factors to ["
                << EnumToString(sourceColor) << "; "
                << EnumToString(destinationColor) << "; "
                << EnumToString(sourceAlpha) << "; "
                << EnumToString(destinationAlpha) << " ]" << RendererLogContext::NewLine;
        }
    }

    void LoggingDevice::blendColor(const glm::vec4& color)
    {
        if (m_logContext.isLogLevelFlagEnabled(ERendererLogLevelFlag_Details))
        {
            m_logContext << "set blend color to [" << color.r << "; " << color.g << "; " << color.b << "; " << color.a << " ]" << RendererLogContext::NewLine;
        }
    }

    void LoggingDevice::cullMode(ECullMode mode)
    {
        if (m_logContext.isLogLevelFlagEnabled(ERendererLogLevelFlag_Details))
        {
            m_logContext << "set cull mode: [mode: " << EnumToString(mode) << "]" << RendererLogContext::NewLine;
        }
    }

    void LoggingDevice::depthFunc(EDepthFunc func)
    {
        if (m_logContext.isLogLevelFlagEnabled(ERendererLogLevelFlag_Details))
        {
            m_logContext << "set depth func: [func: " << EnumToString(func) << "]" << RendererLogContext::NewLine;
        }
    }

    void LoggingDevice::depthWrite(EDepthWrite flag)
    {
        if (m_logContext.isLogLevelFlagEnabled(ERendererLogLevelFlag_Details))
        {
            m_logContext << "set depth write: [flag: " << EnumToString(flag) << "]" << RendererLogContext::NewLine;
        }
    }

    void LoggingDevice::scissorTest(EScissorTest flag, const RenderState::ScissorRegion& region)
    {
        if (m_logContext.isLogLevelFlagEnabled(ERendererLogLevelFlag_Details))
        {
            m_logContext << "set scissor test: [flag: " << EnumToString(flag) << "; x: " << region.x << "; y: " << region.y << "; width: " << region.width << "; height: " << region.height << "]" << RendererLogContext::NewLine;
        }
    }

    void LoggingDevice::stencilFunc(EStencilFunc func, uint8_t ref, uint8_t mask)
    {
        if (m_logContext.isLogLevelFlagEnabled(ERendererLogLevelFlag_Details))
        {
            m_logContext << "set stencil func: [func: " << EnumToString(func) << "; ref: " << ref << "; mask: " << mask << " ]" << RendererLogContext::NewLine;
        }
    }

    void LoggingDevice::stencilOp(EStencilOp sfail, EStencilOp dpfail, EStencilOp dppass)
    {
        if (m_logContext.isLogLevelFlagEnabled(ERendererLogLevelFlag_Details))
        {
            m_logContext << "set stencil operation: [sfail: " << EnumToString(sfail) << "; dpfail: " << EnumToString(dpfail) << "; dppass: " << EnumToString(dppass) << " ]" << RendererLogContext::NewLine;
        }
    }

    void LoggingDevice::setViewport(int32_t x, int32_t y, uint32_t width, uint32_t height)
    {
        m_logContext << "set viewport: [x: " << x << "; y: " << y << "; w: " << width << "; h: " << height << " ]" <<RendererLogContext::NewLine;
    }

    void LoggingDevice::drawMode(EDrawMode mode)
    {
        m_logContext << "set draw mode: " << EnumToString(mode) << RendererLogContext::NewLine;
    }

    DeviceResourceHandle LoggingDevice::allocateVertexBuffer(uint32_t totalSizeInBytes)
    {
        m_logContext << "allocate vertex buffer [total size: " << totalSizeInBytes << "]" << RendererLogContext::NewLine;
        return DeviceResourceHandle::Invalid();
    }

    void LoggingDevice::uploadVertexBufferData(DeviceResourceHandle handle, const std::byte* /*data*/, uint32_t dataSize)
    {
        m_logContext << "upload vertex buffer data [device handle: " << handle << " size: " << dataSize << "]" << RendererLogContext::NewLine;
    }

    void LoggingDevice::deleteVertexBuffer(DeviceResourceHandle handle)
    {
        m_logContext << "delete vertex buffer [handle: " << handle << "]" << RendererLogContext::NewLine;
    }

    DeviceResourceHandle LoggingDevice::allocateVertexArray(const VertexArrayInfo& /*vertexArrayInfo*/)
    {
        m_logContext << "allocate vertex array" << RendererLogContext::NewLine;
        return {};
    }

    void LoggingDevice::activateVertexArray(DeviceResourceHandle handle)
    {
        m_logContext << "activate vertex array [handle: " << handle << "]" << RendererLogContext::NewLine;
    }

    void LoggingDevice::deleteVertexArray(DeviceResourceHandle handle)
    {
        m_logContext << "delete vertex array [handle: " << handle << "]" << RendererLogContext::NewLine;
    }

    DeviceResourceHandle LoggingDevice::allocateIndexBuffer(EDataType dataType, uint32_t sizeInBytes)
    {
        m_logContext << "allocate index buffer [type: " << EnumToString(dataType) << " size: " << sizeInBytes << "]" << RendererLogContext::NewLine;
        return DeviceResourceHandle::Invalid();
    }

    void LoggingDevice::uploadIndexBufferData(DeviceResourceHandle handle, const std::byte* /*data*/, uint32_t dataSize)
    {
        m_logContext << "upload index buffer data [device handle: " << handle << " size: " << dataSize << "]" << RendererLogContext::NewLine;
    }

    void LoggingDevice::deleteIndexBuffer(DeviceResourceHandle handle)
    {
        m_logContext << "delete index buffer [handle: " << handle << "]" << RendererLogContext::NewLine;
    }

    std::unique_ptr<const GPUResource> LoggingDevice::uploadShader(const EffectResource& effect)
    {
        m_logContext << "upload shader " << effect.getName() << RendererLogContext::NewLine;
        return nullptr;
    }

    DeviceResourceHandle LoggingDevice::registerShader(std::unique_ptr<const GPUResource> shaderResource)
    {
        m_logContext << "register shader " << shaderResource->getGPUAddress() << RendererLogContext::NewLine;
        return {};
    }

    DeviceResourceHandle LoggingDevice::uploadBinaryShader(const EffectResource&             effect,
                                                           [[maybe_unused]] const std::byte* binaryShaderData,
                                                           uint32_t                          binaryShaderDataSize,
                                                           BinaryShaderFormatID              binaryShaderFormat)
    {
        m_logContext << "upload binary shader " << effect.getName() << " size: " << binaryShaderDataSize << " format: "  << binaryShaderFormat << RendererLogContext::NewLine;
        return DeviceResourceHandle::Invalid();
    }

    bool
    LoggingDevice::getBinaryShader(DeviceResourceHandle handle, [[maybe_unused]] std::vector<std::byte>& binaryShader, [[maybe_unused]] BinaryShaderFormatID& binaryShaderFormat)
    {
        m_logContext << "get shader binary for shader [handle: " << handle << "]" << RendererLogContext::NewLine;
        return false;
    }

    void LoggingDevice::deleteShader(DeviceResourceHandle handle)
    {
        m_logContext << "delete shader [handle: " << handle << "]" << RendererLogContext::NewLine;
    }

    void LoggingDevice::activateShader(DeviceResourceHandle handle)
    {
        m_logContext << "activate shader [handle: " << handle << "]" << RendererLogContext::NewLine;
    }

    DeviceResourceHandle LoggingDevice::allocateTexture2D(uint32_t width, uint32_t height, EPixelStorageFormat format, const TextureSwizzleArray& swizzle, uint32_t mipLevelCount, uint32_t totalSizeInBytes)
    {
        m_logContext << "allocate texture2d [ (w,h):(" << width << "," << height << ") mipLevelCount:" << mipLevelCount << " format:" << EnumToString(format)
                     << "textureSwizzle:"<< EnumToString(swizzle[0]) << ";" << EnumToString(swizzle[1]) << ";" << EnumToString(swizzle[2]) << ";" << EnumToString(swizzle[3])
                     << ";" << " totalSizeInBytes:" << totalSizeInBytes << "]" << RendererLogContext::NewLine;
        return DeviceResourceHandle::Invalid();
    }

    DeviceResourceHandle LoggingDevice::allocateTexture3D(uint32_t width, uint32_t height, uint32_t depth, EPixelStorageFormat format, uint32_t mipLevelCount, uint32_t /*totalSizeInBytes*/)
    {
        m_logContext << "allocate texture3d [ (w,h,d):(" << width << "," << height << "," << depth << ") mipLevelCount:" << mipLevelCount << " format:" << EnumToString(format) << "]" << RendererLogContext::NewLine;
        return DeviceResourceHandle::Invalid();
    }

    DeviceResourceHandle LoggingDevice::allocateTextureCube(uint32_t faceSize, EPixelStorageFormat format, const TextureSwizzleArray& swizzle, uint32_t mipLevelCount, uint32_t /*totalSizeInBytes*/)
    {
        m_logContext << "allocate textureCube [ faceSize:" << faceSize << " mipLevelCount:" << mipLevelCount << " format:" << EnumToString(format)
                     << "textureSwizzle:"<< EnumToString(swizzle[0]) << ";" << EnumToString(swizzle[1]) << ";" << EnumToString(swizzle[2]) << ";" << EnumToString(swizzle[3]) <<  "]" << RendererLogContext::NewLine;
        return DeviceResourceHandle::Invalid();
    }

    DeviceResourceHandle LoggingDevice::allocateExternalTexture()
    {
        m_logContext << "allocate external texture" << RendererLogContext::NewLine;
        return {};
    }

    DeviceResourceHandle LoggingDevice::getEmptyExternalTexture() const
    {
        m_logContext << "get empty external texture" << RendererLogContext::NewLine;
        return {};
    }

    void LoggingDevice::bindTexture(DeviceResourceHandle handle)
    {
        m_logContext << "bind texture [handle:" << handle << "]" << RendererLogContext::NewLine;
    }

    void LoggingDevice::generateMipmaps(DeviceResourceHandle handle)
    {
        m_logContext << "generate mipmaps for texture [handle:" << handle << "]" << RendererLogContext::NewLine;
    }

    void LoggingDevice::uploadTextureData(DeviceResourceHandle handle, uint32_t mipLevel, uint32_t x, uint32_t y, uint32_t z, uint32_t width, uint32_t height, uint32_t depth, const std::byte* /*data*/, uint32_t dataSize, uint32_t /*stride*/)
    {
        m_logContext << "update texture data [handle:" << handle << " mipLevel:" << mipLevel << " (x,y,z):(" << x << "," << y << "," << z << ") (w,h,d):(" << width << "," << height << "," << depth << ") dataSize:" << dataSize << "]" << RendererLogContext::NewLine;
    }

    DeviceResourceHandle LoggingDevice::uploadStreamTexture2D(DeviceResourceHandle handle, uint32_t width, uint32_t height, EPixelStorageFormat /*format*/, const std::byte* /*data*/, const TextureSwizzleArray& swizzle)
    {
        m_logContext << "upload stream texture2d [textureHandle: " << handle << " (w,h):(" << width << "," << height << ") " << "textureSwizzle: " << EnumToString(swizzle[0]) << "," << EnumToString(swizzle[1]) << "," << EnumToString(swizzle[2]) << "," << EnumToString(swizzle[3])    << "]" << RendererLogContext::NewLine;
        return DeviceResourceHandle::Invalid();
    }

    void LoggingDevice::deleteTexture(DeviceResourceHandle handle)
    {
        m_logContext << "delete texture [handle: " << handle << "]" << RendererLogContext::NewLine;
    }

    void LoggingDevice::activateTexture(DeviceResourceHandle handle, DataFieldHandle field)
    {
        logResourceActivation("texture", handle, field);
    }

    DeviceResourceHandle LoggingDevice::uploadRenderBuffer(uint32_t /*width*/, uint32_t /*height*/, EPixelStorageFormat format, ERenderBufferAccessMode /*accessMode*/, uint32_t /*sampleCount*/)
    {
        m_logContext << "upload render buffer [type: " << EnumToString(format) << "]" << RendererLogContext::NewLine;
        return DeviceResourceHandle::Invalid();
    }

    void LoggingDevice::deleteRenderBuffer(DeviceResourceHandle handle)
    {
        m_logContext << "delete render buffer [handle: " << handle << "]" << RendererLogContext::NewLine;
    }

    void LoggingDevice::activateTextureSamplerObject(const TextureSamplerStates& samplerState, DataFieldHandle field)
    {
        m_logContext << "activate texture sampler object [hash: " << samplerState.hash()
            << "; field: " << field
            << "; wrapU: " << EnumToString(samplerState.m_addressModeU)
            << "; wrapV: " << EnumToString(samplerState.m_addressModeV)
            << "; wrapR: " << EnumToString(samplerState.m_addressModeR)
            << "; min sampling: " << EnumToString(samplerState.m_minSamplingMode)
            << "; mag sampling: " << EnumToString(samplerState.m_magSamplingMode)
            << "; anisotropyLevel: " << samplerState.m_anisotropyLevel
            << RendererLogContext::NewLine;
    }

    DeviceResourceHandle LoggingDevice::uploadDmaRenderBuffer(uint32_t width, uint32_t height, DmaBufferFourccFormat format, DmaBufferUsageFlags bufferUsage, DmaBufferModifiers bufferModifiers)
    {
        m_logContext << "upload dma render buffer [width: " << width << " height: " << height << "format: " << format.getValue()  << " usage: " << bufferUsage.getValue() << " modifiers: " << bufferModifiers.getValue() << "]" << RendererLogContext::NewLine;
        return {};
    }

    int LoggingDevice::getDmaRenderBufferFD(DeviceResourceHandle handle)
    {
        m_logContext << "get dma render buffer FD [handle: " << handle << "]" << RendererLogContext::NewLine;
        return -1;
    }

    uint32_t LoggingDevice::getDmaRenderBufferStride(DeviceResourceHandle handle)
    {
        m_logContext << "get dma render buffer stride [handle: " << handle << "]" << RendererLogContext::NewLine;
        return 0;
    }

    void LoggingDevice::destroyDmaRenderBuffer(DeviceResourceHandle handle)
    {
        m_logContext << "destroy dma render buffer [handle: " << handle << "]" << RendererLogContext::NewLine;
    }

    DeviceResourceHandle LoggingDevice::getFramebufferRenderTarget() const
    {
        return DeviceResourceHandle::Invalid();
    }

    DeviceResourceHandle    LoggingDevice::uploadRenderTarget(const DeviceHandleVector& renderBuffers)
    {
        m_logContext << "creating render target with " << renderBuffers.size() << " renderbuffers'" << RendererLogContext::NewLine;
        return DeviceResourceHandle::Invalid();
    }

    void LoggingDevice::activateRenderTarget(DeviceResourceHandle handle)
    {
        m_logContext << "activate rendertarget [Handle: " << handle << "]" << RendererLogContext::NewLine;
    }

    void LoggingDevice::deleteRenderTarget(DeviceResourceHandle handle)
    {
        m_logContext << "delete render target [handle: " << handle << "]" << RendererLogContext::NewLine;
    }

    void LoggingDevice::discardDepthStencil()
    {
        m_logContext << "discard depthstencil buffer" << RendererLogContext::NewLine;
    }

    void LoggingDevice::blitRenderTargets(DeviceResourceHandle rtSrc, DeviceResourceHandle rtDst, const PixelRectangle& /*srcRect*/, const PixelRectangle& /*dstRect*/, bool /*colorOnly*/)
    {
        m_logContext << "blit render pass [RT src device handle:  " << rtSrc << ", RT dst device handle: " << rtDst << "]" << RendererLogContext::NewLine;
    }

    void LoggingDevice::drawIndexedTriangles(int32_t startOffset, int32_t elementCount, uint32_t instanceCount)
    {
        m_logContext << "draw " << instanceCount << " instances with " << elementCount << " indexed vertices starting from " << startOffset << RendererLogContext::NewLine;
    }

    void LoggingDevice::drawTriangles(int32_t startOffset, int32_t elementCount, uint32_t instanceCount)
    {
        m_logContext << "draw " << instanceCount << " instances with " << elementCount << " vertices starting from " << startOffset << RendererLogContext::NewLine;
    }

    void LoggingDevice::clear(ClearFlags clearFlags)
    {
        m_logContext << "clear buffer [flags: " << clearFlags.value() << "]" << RendererLogContext::NewLine;
    }

    void LoggingDevice::pairRenderTargetsForDoubleBuffering(const std::array<DeviceResourceHandle, 2>& renderTargets, const std::array<DeviceResourceHandle, 2>& colorBuffers)
    {
        m_logContext << "pair render targets [render targets: (" << renderTargets[0] << ", " << renderTargets[1] << "), render buffers: (" << colorBuffers[0] << ", " << colorBuffers[1] << ")]" << RendererLogContext::NewLine;
    }

    void LoggingDevice::unpairRenderTargets(DeviceResourceHandle renderTarget)
    {
        m_logContext << "unpair render targets [render target: " << renderTarget << "]" << RendererLogContext::NewLine;
    }

    void LoggingDevice::swapDoubleBufferedRenderTarget(DeviceResourceHandle renderTarget)
    {
        m_logContext << "swap render targets [render target: " << renderTarget << "]" << RendererLogContext::NewLine;
    }

    void LoggingDevice::readPixels(uint8_t* /*buffer*/, uint32_t /*x*/, uint32_t /*y*/, uint32_t /*width*/, uint32_t /*height*/)
    {
    }

    uint32_t LoggingDevice::getTotalGpuMemoryUsageInKB() const
    {
        return m_deviceDelegate.getTotalGpuMemoryUsageInKB();
    }

    void LoggingDevice::logResourceActivation(std::string_view resourceTypeName, DeviceResourceHandle handle, DataFieldHandle field)
    {
        if (m_logContext.isLogLevelFlagEnabled(ERendererLogLevelFlag_Details))
        {
            m_logContext << "activate " << resourceTypeName << " [handle: " << handle << "]";

            if (field.isValid())
            {
                m_logContext << " for input [" << field << "]";
            }

            m_logContext << RendererLogContext::NewLine;
        }
    }

    void LoggingDevice::clearDepth(float d)
    {
        m_logContext << "clear depth to [ " << d << "]" << RendererLogContext::NewLine;
    }

    void LoggingDevice::clearStencil(int32_t s)
    {
        m_logContext << "clear stencil to [ " << s << "]" << RendererLogContext::NewLine;
    }

    uint32_t LoggingDevice::getTextureAddress(DeviceResourceHandle /*handle*/) const
    {
        return 0;
    }

    void LoggingDevice::validateDeviceStatusHealthy() const
    {
    }

    bool LoggingDevice::isDeviceStatusHealthy() const
    {
        return true;
    }

    void LoggingDevice::getSupportedBinaryProgramFormats(std::vector<BinaryShaderFormatID>& /*unused*/) const
    {
    }

    bool LoggingDevice::isExternalTextureExtensionSupported() const
    {
        return false;
    }

    uint32_t LoggingDevice::getAndResetDrawCallCount()
    {
        return 0;
    }

    void LoggingDevice::flush()
    {
    }

    uint32_t LoggingDevice::getGPUHandle(DeviceResourceHandle /*deviceHandle*/) const
    {
        return 0u;
    }
}
