//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_LOGGINGDEVICE_H
#define RAMSES_LOGGINGDEVICE_H

#include "RendererAPI/IDevice.h"

#include <string_view>

namespace ramses_internal
{
    class RendererLogContext;

    class LoggingDevice : public IDevice
    {
    public:
        LoggingDevice(const IDevice& deviceDelegate, RendererLogContext& context);

        void setConstant(DataFieldHandle field, UInt32 count, const float* value) override;
        void setConstant(DataFieldHandle field, UInt32 count, const glm::vec2* value) override;
        void setConstant(DataFieldHandle field, UInt32 count, const glm::vec3* value) override;
        void setConstant(DataFieldHandle field, UInt32 count, const glm::vec4* value) override;
        void setConstant(DataFieldHandle field, UInt32 count, const Int32* value) override;
        void setConstant(DataFieldHandle field, UInt32 count, const glm::ivec2* value) override;
        void setConstant(DataFieldHandle field, UInt32 count, const glm::ivec3* value) override;
        void setConstant(DataFieldHandle field, UInt32 count, const glm::ivec4* value) override;
        void setConstant(DataFieldHandle field, UInt32 count, const glm::mat2* value) override;
        void setConstant(DataFieldHandle field, UInt32 count, const glm::mat3* value) override;
        void setConstant(DataFieldHandle field, UInt32 count, const glm::mat4* value) override;

        void colorMask(bool r, bool g, bool b, bool a) override;
        void clearColor(const glm::vec4& clearColor) override;
        void blendOperations(EBlendOperation colorOperation, EBlendOperation alphaOperation) override;
        void blendFactors(EBlendFactor sourceColor, EBlendFactor destinationColor, EBlendFactor sourceAlpha, EBlendFactor destinationAlpha) override;
        void blendColor(const glm::vec4& color) override;
        void cullMode(ECullMode mode) override;
        void depthFunc(EDepthFunc func) override;
        void depthWrite(EDepthWrite flag) override;
        void scissorTest(EScissorTest flag, const RenderState::ScissorRegion& region) override;
        void stencilFunc(EStencilFunc func, uint8_t ref, uint8_t mask) override;
        void stencilOp(EStencilOp sfail, EStencilOp dpfail, EStencilOp dppass) override;
        void drawMode(EDrawMode mode) override;
        void setViewport(int32_t x, int32_t y, uint32_t width, uint32_t height) override;

        DeviceResourceHandle allocateVertexBuffer(UInt32 totalSizeInBytes) override;
        void uploadVertexBufferData(DeviceResourceHandle handle, const Byte* data, UInt32 dataSize) override;
        void deleteVertexBuffer(DeviceResourceHandle handle) override;
        DeviceResourceHandle allocateVertexArray(const VertexArrayInfo& vertexArrayInfo) override;
        void activateVertexArray(DeviceResourceHandle handle) override;
        void deleteVertexArray(DeviceResourceHandle handle) override;
        DeviceResourceHandle allocateIndexBuffer(EDataType dataType, UInt32 sizeInBytes) override;
        void uploadIndexBufferData(DeviceResourceHandle handle, const Byte* data, UInt32 dataSize) override;
        void deleteIndexBuffer(DeviceResourceHandle handle) override;
        std::unique_ptr<const GPUResource> uploadShader(const EffectResource& effect) override;
        DeviceResourceHandle registerShader(std::unique_ptr<const GPUResource> shaderResource) override;
        DeviceResourceHandle uploadBinaryShader(const EffectResource& effect, const uint8_t* binaryShaderData = nullptr, UInt32 binaryShaderDataSize = 0, BinaryShaderFormatID binaryShaderFormat = {}) override;
        bool getBinaryShader(DeviceResourceHandle handle, UInt8Vector& binaryShader, BinaryShaderFormatID& binaryShaderFormat) override;
        void deleteShader(DeviceResourceHandle handle) override;
        void activateShader(DeviceResourceHandle handle) override;
        DeviceResourceHandle allocateTexture2D(UInt32 width, UInt32 height, ETextureFormat textureFormat, const TextureSwizzleArray& swizzle, UInt32 mipLevelCount, UInt32 totalSizeInBytes) override;
        DeviceResourceHandle allocateTexture3D(UInt32 width, UInt32 height, UInt32 depth, ETextureFormat textureFormat, UInt32 mipLevelCount, UInt32 dataSize) override;
        DeviceResourceHandle allocateTextureCube(UInt32 faceSize, ETextureFormat textureFormat, const TextureSwizzleArray& swizzle, UInt32 mipLevelCount, UInt32 dataSize) override;
        DeviceResourceHandle allocateExternalTexture() override;
        [[nodiscard]] DeviceResourceHandle getEmptyExternalTexture() const override;
        void                 bindTexture(DeviceResourceHandle handle) override;
        void                 generateMipmaps(DeviceResourceHandle handle) override;
        void                 uploadTextureData(DeviceResourceHandle handle, UInt32 mipLevel, UInt32 x, UInt32 y, UInt32 z, UInt32 width, UInt32 height, UInt32 depth, const Byte* data, UInt32 dataSize) override;
        DeviceResourceHandle uploadStreamTexture2D(DeviceResourceHandle handle, UInt32 width, UInt32 height, ETextureFormat format, const uint8_t* data, const TextureSwizzleArray& swizzle) override;
        void deleteTexture(DeviceResourceHandle handle) override;
        void activateTexture(DeviceResourceHandle handle, DataFieldHandle field) override;
        DeviceResourceHandle    uploadRenderBuffer(uint32_t width, uint32_t height, ERenderBufferType type, ETextureFormat format, ERenderBufferAccessMode accessMode, uint32_t sampleCount) override;
        void                    deleteRenderBuffer(DeviceResourceHandle handle) override;
        void                    activateTextureSamplerObject(const TextureSamplerStates& samplerStates, DataFieldHandle field) override;

        DeviceResourceHandle    uploadDmaRenderBuffer(UInt32 width, UInt32 height, DmaBufferFourccFormat format, DmaBufferUsageFlags bufferUsage, DmaBufferModifiers bufferModifiers) override;
        int                     getDmaRenderBufferFD(DeviceResourceHandle handle) override;
        uint32_t                getDmaRenderBufferStride(DeviceResourceHandle handle) override;
        void                    destroyDmaRenderBuffer(DeviceResourceHandle handle) override;

        [[nodiscard]] DeviceResourceHandle    getFramebufferRenderTarget() const override;
        DeviceResourceHandle    uploadRenderTarget(const DeviceHandleVector& renderBuffers) override;
        void                    activateRenderTarget(DeviceResourceHandle handle) override;
        void                    deleteRenderTarget(DeviceResourceHandle handle) override;
        void                    discardDepthStencil() override;
        void                    blitRenderTargets(DeviceResourceHandle rtSrc, DeviceResourceHandle rtDst, const PixelRectangle& srcRect, const PixelRectangle& dstRect, bool colorOnly) override;

        void drawIndexedTriangles(Int32 startOffset, Int32 elementCount, UInt32 instanceCount) override;
        void drawTriangles(Int32 startOffset, Int32 elementCount, UInt32 instanceCount) override;
        void clear(UInt32 clearFlags) override;

        void                    pairRenderTargetsForDoubleBuffering(const std::array<DeviceResourceHandle, 2>& renderTargets, const std::array<DeviceResourceHandle, 2>& colorBuffers) override;
        void                    unpairRenderTargets(DeviceResourceHandle renderTarget) override;
        void                    swapDoubleBufferedRenderTarget(DeviceResourceHandle renderTarget) override;

        void readPixels(uint8_t* buffer, UInt32 x, UInt32 y, UInt32 width, UInt32 height) override;

        [[nodiscard]] uint32_t getTotalGpuMemoryUsageInKB() const override;
        uint32_t getAndResetDrawCallCount() override;

        void clearDepth(float d) override;
        void clearStencil(Int32 s) override;

        [[nodiscard]] Int32 getTextureAddress(DeviceResourceHandle handle) const override;

        void validateDeviceStatusHealthy() const override;
        [[nodiscard]] bool isDeviceStatusHealthy() const override;
        void getSupportedBinaryProgramFormats(std::vector<BinaryShaderFormatID>& formats) const override;
        [[nodiscard]] bool isExternalTextureExtensionSupported() const override;

        void flush() override;

        [[nodiscard]] uint32_t getGPUHandle(DeviceResourceHandle deviceHandle) const override;

    private:
        // Used only to delegate getters for components
        const IDevice& m_deviceDelegate;
        RendererLogContext& m_logContext;

        void logResourceActivation(std::string_view resourceTypeName, DeviceResourceHandle handle, DataFieldHandle field);

    };
}

#endif
