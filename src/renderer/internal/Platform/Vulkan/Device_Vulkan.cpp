//  -------------------------------------------------------------------------
//  Copyright (C) 2024 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Device_Vulkan.h"


namespace ramses::internal
{
    Device_Vulkan::Device_Vulkan(IContext& context, VkInstance instance, VkSurfaceKHR surface)
        : Device_Base(context)
        , m_instance(instance)
        , m_surface(surface)
    {
    }

    bool Device_Vulkan::init()
    {
        (void) m_instance;
        (void) m_surface;

        return true;
    }

    void Device_Vulkan::drawIndexedTriangles([[maybe_unused]] int32_t startOffset, [[maybe_unused]] int32_t elementCount, [[maybe_unused]] uint32_t instanceCount)
    {

    }

    void Device_Vulkan::drawTriangles([[maybe_unused]] int32_t startOffset, [[maybe_unused]] int32_t elementCount, [[maybe_unused]] uint32_t instanceCount)
    {

    }

    void Device_Vulkan::clear([[maybe_unused]] ClearFlags clearFlags)
    {

    }

    void Device_Vulkan::colorMask([[maybe_unused]] bool r, [[maybe_unused]] bool g, [[maybe_unused]] bool b, [[maybe_unused]] bool a)
    {

    }

    void Device_Vulkan::clearColor([[maybe_unused]] const glm::vec4& clearColor)
    {

    }

    void Device_Vulkan::clearDepth([[maybe_unused]] float d)
    {

    }

    void Device_Vulkan::clearStencil([[maybe_unused]] int32_t s)
    {

    }

    void Device_Vulkan::depthFunc([[maybe_unused]] EDepthFunc func)
    {

    }

    void Device_Vulkan::depthWrite([[maybe_unused]] EDepthWrite flag)
    {

    }

    void Device_Vulkan::scissorTest([[maybe_unused]] EScissorTest state, [[maybe_unused]] const RenderState::ScissorRegion& region)
    {

    }

    void Device_Vulkan::blendFactors([[maybe_unused]] EBlendFactor sourceColor, [[maybe_unused]] EBlendFactor destinationColor, [[maybe_unused]] EBlendFactor sourceAlpha, [[maybe_unused]] EBlendFactor destinationAlpha)
    {

    }

    void Device_Vulkan::blendColor([[maybe_unused]] const glm::vec4& color)
    {

    }

    void Device_Vulkan::blendOperations([[maybe_unused]] EBlendOperation operationColor, [[maybe_unused]] EBlendOperation operationAlpha)
    {

    }

    void Device_Vulkan::cullMode([[maybe_unused]] ECullMode mode)
    {

    }

    void Device_Vulkan::stencilFunc([[maybe_unused]] EStencilFunc func, [[maybe_unused]] uint8_t ref, [[maybe_unused]] uint8_t mask)
    {

    }

    void Device_Vulkan::stencilOp([[maybe_unused]] EStencilOp sfail, [[maybe_unused]] EStencilOp dpfail, [[maybe_unused]] EStencilOp dppass)
    {

    }

    void Device_Vulkan::drawMode([[maybe_unused]] EDrawMode mode)
    {

    }

    void Device_Vulkan::setViewport([[maybe_unused]] int32_t x, [[maybe_unused]] int32_t y, [[maybe_unused]] uint32_t width, [[maybe_unused]] uint32_t height)
    {

    }

    bool Device_Vulkan::setConstant([[maybe_unused]] DataFieldHandle field, [[maybe_unused]] uint32_t count, [[maybe_unused]] const float* value)
    {
        return {};
    }

    bool Device_Vulkan::setConstant([[maybe_unused]] DataFieldHandle field, [[maybe_unused]] uint32_t count, [[maybe_unused]] const glm::vec2* value)
    {
        return {};
    }

    bool Device_Vulkan::setConstant([[maybe_unused]] DataFieldHandle field, [[maybe_unused]] uint32_t count, [[maybe_unused]] const glm::vec3* value)
    {
        return {};
    }

    bool Device_Vulkan::setConstant([[maybe_unused]] DataFieldHandle field, [[maybe_unused]] uint32_t count, [[maybe_unused]] const glm::vec4* value)
    {
        return {};
    }

    bool Device_Vulkan::setConstant([[maybe_unused]] DataFieldHandle field, [[maybe_unused]] uint32_t count, [[maybe_unused]] const bool* value)
    {
        return {};
    }

    bool Device_Vulkan::setConstant([[maybe_unused]] DataFieldHandle field, [[maybe_unused]] uint32_t count, [[maybe_unused]] const int32_t* value)
    {
        return {};
    }

    bool Device_Vulkan::setConstant([[maybe_unused]] DataFieldHandle field, [[maybe_unused]] uint32_t count, [[maybe_unused]] const glm::ivec2* value)
    {
        return {};
    }

    bool Device_Vulkan::setConstant([[maybe_unused]] DataFieldHandle field, [[maybe_unused]] uint32_t count, [[maybe_unused]] const glm::ivec3* value)
    {
        return {};
    }

    bool Device_Vulkan::setConstant([[maybe_unused]] DataFieldHandle field, [[maybe_unused]] uint32_t count, [[maybe_unused]] const glm::ivec4* value)
    {
        return {};
    }

    bool Device_Vulkan::setConstant([[maybe_unused]] DataFieldHandle field, [[maybe_unused]] uint32_t count, [[maybe_unused]] const glm::mat2* value)
    {
        return {};
    }

    bool Device_Vulkan::setConstant([[maybe_unused]] DataFieldHandle field, [[maybe_unused]] uint32_t count, [[maybe_unused]] const glm::mat3* value)
    {
        return {};
    }

    bool Device_Vulkan::setConstant([[maybe_unused]] DataFieldHandle field, [[maybe_unused]] uint32_t count, [[maybe_unused]] const glm::mat4* value)
    {
        return {};
    }

    void Device_Vulkan::readPixels([[maybe_unused]] uint8_t* buffer, [[maybe_unused]] uint32_t x, [[maybe_unused]] uint32_t y, [[maybe_unused]] uint32_t width, [[maybe_unused]] uint32_t height)
    {

    }

    DeviceResourceHandle Device_Vulkan::allocateUniformBuffer([[maybe_unused]] uint32_t totalSizeInBytes)
    {
        return {};
    }

    void Device_Vulkan::uploadUniformBufferData([[maybe_unused]] DeviceResourceHandle handle, [[maybe_unused]] const std::byte* data, [[maybe_unused]] uint32_t dataSize)
    {

    }

    void Device_Vulkan::activateUniformBuffer([[maybe_unused]] DeviceResourceHandle handle, [[maybe_unused]] DataFieldHandle field)
    {

    }

    void Device_Vulkan::deleteUniformBuffer([[maybe_unused]] DeviceResourceHandle handle)
    {

    }

    DeviceResourceHandle Device_Vulkan::allocateVertexBuffer([[maybe_unused]] uint32_t totalSizeInBytes)
    {
        return {};
    }

    void Device_Vulkan::uploadVertexBufferData([[maybe_unused]] DeviceResourceHandle handle, [[maybe_unused]] const std::byte* data, [[maybe_unused]] uint32_t dataSize)
    {

    }

    void Device_Vulkan::deleteVertexBuffer([[maybe_unused]] DeviceResourceHandle handle)
    {

    }

    DeviceResourceHandle Device_Vulkan::allocateVertexArray([[maybe_unused]] const VertexArrayInfo& vertexArrayInfo)
    {
        return {};
    }

    void Device_Vulkan::activateVertexArray([[maybe_unused]] DeviceResourceHandle handle)
    {

    }

    void Device_Vulkan::deleteVertexArray([[maybe_unused]] DeviceResourceHandle handle)
    {

    }

    DeviceResourceHandle Device_Vulkan::allocateIndexBuffer([[maybe_unused]] EDataType dataType, [[maybe_unused]] uint32_t sizeInBytes)
    {
        return {};
    }

    void Device_Vulkan::uploadIndexBufferData([[maybe_unused]] DeviceResourceHandle handle, [[maybe_unused]] const std::byte* data, [[maybe_unused]] uint32_t dataSize)
    {

    }

    void Device_Vulkan::deleteIndexBuffer([[maybe_unused]] DeviceResourceHandle handle)
    {

    }

    std::unique_ptr<const GPUResource> Device_Vulkan::uploadShader([[maybe_unused]] const EffectResource& shader)
    {
        return {};
    }

    DeviceResourceHandle Device_Vulkan::registerShader([[maybe_unused]] std::unique_ptr<const GPUResource> shaderResource)
    {
        return {};
    }

    DeviceResourceHandle Device_Vulkan::uploadBinaryShader([[maybe_unused]] const EffectResource& shader, [[maybe_unused]] const std::byte* binaryShaderData, [[maybe_unused]] uint32_t binaryShaderDataSize, [[maybe_unused]] BinaryShaderFormatID binaryShaderFormat)
    {
        return {};
    }

    bool Device_Vulkan::getBinaryShader([[maybe_unused]] DeviceResourceHandle handleconst, [[maybe_unused]] std::vector<std::byte>& binaryShader, [[maybe_unused]] BinaryShaderFormatID& binaryShaderFormat)
    {
        return {};
    }

    void Device_Vulkan::deleteShader([[maybe_unused]] DeviceResourceHandle handle)
    {

    }

    void Device_Vulkan::activateShader([[maybe_unused]] DeviceResourceHandle handle)
    {

    }

    DeviceResourceHandle Device_Vulkan::allocateTexture2D([[maybe_unused]] uint32_t width, [[maybe_unused]] uint32_t height, [[maybe_unused]] EPixelStorageFormat textureFormat, [[maybe_unused]] const TextureSwizzleArray& swizzle, [[maybe_unused]] uint32_t mipLevelCount, [[maybe_unused]] uint32_t totalSizeInBytes)
    {
        return {};
    }

    DeviceResourceHandle Device_Vulkan::allocateTexture3D([[maybe_unused]] uint32_t width, [[maybe_unused]] uint32_t height, [[maybe_unused]] uint32_t depth, [[maybe_unused]] EPixelStorageFormat textureFormat, [[maybe_unused]] uint32_t mipLevelCount, [[maybe_unused]] uint32_t totalSizeInBytes)
    {
        return {};
    }

    DeviceResourceHandle Device_Vulkan::allocateTextureCube([[maybe_unused]] uint32_t faceSize, [[maybe_unused]] EPixelStorageFormat textureFormat, [[maybe_unused]] const TextureSwizzleArray& swizzle, [[maybe_unused]] uint32_t mipLevelCount, [[maybe_unused]] uint32_t totalSizeInBytes)
    {
        return {};
    }

    DeviceResourceHandle Device_Vulkan::allocateExternalTexture()
    {
        return {};
    }

    DeviceResourceHandle Device_Vulkan::getEmptyExternalTexture() const
    {
        return {};
    }

    void Device_Vulkan::bindTexture([[maybe_unused]] DeviceResourceHandle handle)
    {

    }

    void Device_Vulkan::generateMipmaps([[maybe_unused]] DeviceResourceHandle handle)
    {

    }

    void Device_Vulkan::uploadTextureData([[maybe_unused]] DeviceResourceHandle handle,
        [[maybe_unused]] uint32_t mipLevel,
        [[maybe_unused]] uint32_t x, [[maybe_unused]] uint32_t y, [[maybe_unused]] uint32_t z,
        [[maybe_unused]] uint32_t width, [[maybe_unused]] uint32_t height, [[maybe_unused]] uint32_t depth,
        [[maybe_unused]] const std::byte* data, [[maybe_unused]] uint32_t dataSize, [[maybe_unused]] uint32_t stride)
    {

    }

    DeviceResourceHandle Device_Vulkan::uploadStreamTexture2D([[maybe_unused]] DeviceResourceHandle handle, [[maybe_unused]] uint32_t width, [[maybe_unused]] uint32_t height, [[maybe_unused]] EPixelStorageFormat format, [[maybe_unused]] const std::byte* data, [[maybe_unused]] const TextureSwizzleArray& swizzle)
    {
        return {};
    }

    void Device_Vulkan::deleteTexture([[maybe_unused]] DeviceResourceHandle handle)
    {

    }

    void Device_Vulkan::activateTexture([[maybe_unused]] DeviceResourceHandle handle, [[maybe_unused]] DataFieldHandle field)
    {

    }

    uint32_t Device_Vulkan::getTextureAddress([[maybe_unused]] DeviceResourceHandle handle) const
    {
        return {};
    }

    DeviceResourceHandle Device_Vulkan::uploadRenderBuffer([[maybe_unused]] uint32_t width, [[maybe_unused]] uint32_t height, [[maybe_unused]] EPixelStorageFormat format, [[maybe_unused]] ERenderBufferAccessMode accessMode, [[maybe_unused]] uint32_t sampleCount)
    {
        return {};
    }

    void Device_Vulkan::deleteRenderBuffer([[maybe_unused]] DeviceResourceHandle handle)
    {

    }

    DeviceResourceHandle Device_Vulkan::uploadDmaRenderBuffer([[maybe_unused]] uint32_t width, [[maybe_unused]] uint32_t height, [[maybe_unused]] DmaBufferFourccFormat fourccFormat, [[maybe_unused]] DmaBufferUsageFlags usageFlags, [[maybe_unused]] DmaBufferModifiers modifiers)
    {
        return {};
    }

    int Device_Vulkan::getDmaRenderBufferFD([[maybe_unused]] DeviceResourceHandle handle)
    {
        return -1;
    }

    uint32_t Device_Vulkan::getDmaRenderBufferStride([[maybe_unused]] DeviceResourceHandle handle)
    {
        return std::numeric_limits<uint32_t>::max();
    }

    void Device_Vulkan::destroyDmaRenderBuffer([[maybe_unused]] DeviceResourceHandle handle)
    {

    }

    void Device_Vulkan::activateTextureSamplerObject([[maybe_unused]] const TextureSamplerStates& samplerStates, [[maybe_unused]] DataFieldHandle field)
    {

    }

    DeviceResourceHandle Device_Vulkan::getFramebufferRenderTarget() const
    {
        return {};
    }

    DeviceResourceHandle Device_Vulkan::uploadRenderTarget([[maybe_unused]] const DeviceHandleVector& renderBuffers)
    {
        return {};
    }

    void Device_Vulkan::activateRenderTarget([[maybe_unused]] DeviceResourceHandle handle)
    {

    }

    void Device_Vulkan::deleteRenderTarget([[maybe_unused]] DeviceResourceHandle handle)
    {

    }

    void Device_Vulkan::discardDepthStencil()
    {

    }

    void Device_Vulkan::pairRenderTargetsForDoubleBuffering([[maybe_unused]] const std::array<DeviceResourceHandle, 2>& renderTargets, [[maybe_unused]] const std::array<DeviceResourceHandle, 2>& colorBuffers)
    {

    }

    void Device_Vulkan::unpairRenderTargets([[maybe_unused]] DeviceResourceHandle renderTarget)
    {

    }

    void Device_Vulkan::swapDoubleBufferedRenderTarget([[maybe_unused]] DeviceResourceHandle renderTarget)
    {

    }

    void Device_Vulkan::blitRenderTargets([[maybe_unused]] DeviceResourceHandle rtSrc, [[maybe_unused]] DeviceResourceHandle rtDst, [[maybe_unused]] const PixelRectangle& srcRect, [[maybe_unused]] const PixelRectangle& dstRect, [[maybe_unused]] bool colorOnly)
    {

    }

    void Device_Vulkan::validateDeviceStatusHealthy() const
    {

    }

    bool Device_Vulkan::isDeviceStatusHealthy() const
    {
        return true;
    }

    void Device_Vulkan::getSupportedBinaryProgramFormats([[maybe_unused]] std::vector<BinaryShaderFormatID>& formats) const
    {

    }

    bool Device_Vulkan::isExternalTextureExtensionSupported() const
    {
        return {};
    }

    uint32_t Device_Vulkan::getTotalGpuMemoryUsageInKB() const
    {
        return 0u;
    }

    void Device_Vulkan::flush()
    {

    }
}
