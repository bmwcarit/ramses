//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/Types.h"
#include "internal/SceneGraph/SceneAPI/TextureEnums.h"
#include "internal/SceneGraph/SceneAPI/RenderState.h"
#include "internal/SceneGraph/SceneAPI/EDataType.h"
#include "internal/SceneGraph/Resource/TextureMetaInfo.h"
#include "internal/RendererLib/PlatformBase/GpuResource.h"
#include <memory>
#include <array>

namespace ramses::internal
{
    class EffectResource;

    struct PixelRectangle;
    struct TextureSamplerStates;

    class IDevice
    {
    public:
        virtual ~IDevice() = default;

        // data
        virtual bool setConstant(DataFieldHandle field, uint32_t count, const float*      value) = 0;
        virtual bool setConstant(DataFieldHandle field, uint32_t count, const glm::vec2*    value) = 0;
        virtual bool setConstant(DataFieldHandle field, uint32_t count, const glm::vec3*    value) = 0;
        virtual bool setConstant(DataFieldHandle field, uint32_t count, const glm::vec4*    value) = 0;
        virtual bool setConstant(DataFieldHandle field, uint32_t count, const bool*         value) = 0;
        virtual bool setConstant(DataFieldHandle field, uint32_t count, const int32_t*      value) = 0;
        virtual bool setConstant(DataFieldHandle field, uint32_t count, const glm::ivec2*   value) = 0;
        virtual bool setConstant(DataFieldHandle field, uint32_t count, const glm::ivec3*   value) = 0;
        virtual bool setConstant(DataFieldHandle field, uint32_t count, const glm::ivec4*   value) = 0;
        virtual bool setConstant(DataFieldHandle field, uint32_t count, const glm::mat2*  value) = 0;
        virtual bool setConstant(DataFieldHandle field, uint32_t count, const glm::mat3*  value) = 0;
        virtual bool setConstant(DataFieldHandle field, uint32_t count, const glm::mat4*  value) = 0;

        //draw calls
        virtual void clear               (ClearFlags clearFlags) = 0;
        virtual void drawIndexedTriangles(int32_t startOffset, int32_t elementCount, uint32_t instanceCount) = 0;
        virtual void drawTriangles       (int32_t startOffset, int32_t elementCount, uint32_t instanceCount) = 0;
        virtual void flush              () = 0;

        //states
        virtual void colorMask           (bool r, bool g, bool b, bool a) = 0;
        virtual void clearColor          (const glm::vec4& clearColor) = 0;
        virtual void clearDepth          (float d) = 0;
        virtual void clearStencil        (int32_t s) = 0;
        virtual void blendFactors        (EBlendFactor sourceColor, EBlendFactor destinationColor, EBlendFactor sourceAlpha, EBlendFactor destinationAlpha) = 0;
        virtual void blendOperations     (EBlendOperation operationColor, EBlendOperation operationAlpha) = 0;
        virtual void blendColor          (const glm::vec4& color) = 0;
        virtual void cullMode            (ECullMode mode) = 0;
        virtual void depthFunc           (EDepthFunc func) = 0;
        virtual void depthWrite          (EDepthWrite flag) = 0;
        virtual void scissorTest         (EScissorTest flag, const RenderState::ScissorRegion& region) = 0;
        virtual void stencilFunc         (EStencilFunc func, uint8_t ref, uint8_t mask) = 0;
        virtual void stencilOp           (EStencilOp sfail, EStencilOp dpfail, EStencilOp dppass) = 0;
        virtual void drawMode            (EDrawMode mode) = 0;
        virtual void setViewport         (int32_t x, int32_t y, uint32_t width, uint32_t height) = 0;

        // resources
        virtual DeviceResourceHandle    allocateVertexBuffer        (uint32_t totalSizeInBytes) = 0;
        virtual void                    uploadVertexBufferData      (DeviceResourceHandle handle, const std::byte* data, uint32_t dataSize) = 0;
        virtual void                    deleteVertexBuffer          (DeviceResourceHandle handle) = 0;

        virtual DeviceResourceHandle    allocateIndexBuffer         (EDataType dataType, uint32_t sizeInBytes) = 0;
        virtual void                    uploadIndexBufferData       (DeviceResourceHandle handle, const std::byte* data, uint32_t dataSize) = 0;
        virtual void                    deleteIndexBuffer           (DeviceResourceHandle handle) = 0;

        virtual DeviceResourceHandle    allocateVertexArray         (const VertexArrayInfo& vertexArrayInfo) = 0;
        virtual void                    activateVertexArray         (DeviceResourceHandle handle) = 0;
        virtual void                    deleteVertexArray           (DeviceResourceHandle handle) = 0;

        virtual std::unique_ptr<const GPUResource> uploadShader     (const EffectResource& effect) = 0;
        virtual DeviceResourceHandle    registerShader              (std::unique_ptr<const GPUResource> shaderResource) = 0;
        virtual DeviceResourceHandle    uploadBinaryShader          (const EffectResource& effect, const std::byte* binaryShaderData, uint32_t binaryShaderDataSize, BinaryShaderFormatID binaryShaderFormat) = 0;
        virtual bool                    getBinaryShader             (DeviceResourceHandle handle, std::vector<std::byte>& binaryShader, BinaryShaderFormatID& binaryShaderFormat) = 0;
        virtual void                    deleteShader                (DeviceResourceHandle handle) = 0;
        virtual void                    activateShader              (DeviceResourceHandle handle) = 0;

        virtual DeviceResourceHandle    allocateTexture2D           (uint32_t width, uint32_t height, EPixelStorageFormat textureFormat, const TextureSwizzleArray& swizzle, uint32_t mipLevelCount, uint32_t totalSizeInBytes) = 0;
        virtual DeviceResourceHandle    allocateTexture3D           (uint32_t width, uint32_t height, uint32_t depth, EPixelStorageFormat textureFormat, uint32_t mipLevelCount, uint32_t totalSizeInBytes) = 0;
        virtual DeviceResourceHandle    allocateTextureCube         (uint32_t faceSize, EPixelStorageFormat textureFormat, const TextureSwizzleArray& swizzle, uint32_t mipLevelCount, uint32_t totalSizeInBytes) = 0;
        virtual DeviceResourceHandle    allocateExternalTexture     () = 0;
        [[nodiscard]] virtual DeviceResourceHandle    getEmptyExternalTexture     () const = 0;

        virtual void                    bindTexture                 (DeviceResourceHandle handle) = 0;
        virtual void                    generateMipmaps             (DeviceResourceHandle handle) = 0;
        virtual void                    uploadTextureData           (DeviceResourceHandle handle, uint32_t mipLevel, uint32_t x, uint32_t y, uint32_t z, uint32_t width, uint32_t height, uint32_t depth, const std::byte* data, uint32_t dataSize, uint32_t stride) = 0;
        virtual DeviceResourceHandle    uploadStreamTexture2D       (DeviceResourceHandle handle, uint32_t width, uint32_t height, EPixelStorageFormat format, const std::byte* data, const TextureSwizzleArray& swizzle) = 0;
        virtual void                    deleteTexture               (DeviceResourceHandle handle) = 0;
        virtual void                    activateTexture             (DeviceResourceHandle handle, DataFieldHandle field) = 0;
        [[nodiscard]] virtual uint32_t  getTextureAddress           (DeviceResourceHandle handle) const = 0;

        // Render buffers/targets
        virtual DeviceResourceHandle    uploadRenderBuffer          (uint32_t width, uint32_t height, EPixelStorageFormat format, ERenderBufferAccessMode accessMode, uint32_t sampleCount) = 0;
        virtual void                    deleteRenderBuffer          (DeviceResourceHandle handle) = 0;

        virtual DeviceResourceHandle    uploadDmaRenderBuffer       (uint32_t width, uint32_t height, DmaBufferFourccFormat fourccFormat, DmaBufferUsageFlags usageFlags, DmaBufferModifiers modifiers) = 0;
        virtual int                     getDmaRenderBufferFD        (DeviceResourceHandle handle) = 0;
        virtual uint32_t                getDmaRenderBufferStride    (DeviceResourceHandle handle) = 0;
        virtual void                    destroyDmaRenderBuffer      (DeviceResourceHandle handle) = 0;

        virtual void                    activateTextureSamplerObject(const TextureSamplerStates& samplerStates, DataFieldHandle field) = 0;

        [[nodiscard]] virtual DeviceResourceHandle    getFramebufferRenderTarget  () const = 0;
        virtual DeviceResourceHandle    uploadRenderTarget          (const DeviceHandleVector& renderBuffers) = 0;
        virtual void                    activateRenderTarget        (DeviceResourceHandle handle) = 0;
        virtual void                    deleteRenderTarget          (DeviceResourceHandle handle) = 0;
        virtual void                    discardDepthStencil         () = 0;

        virtual void                    pairRenderTargetsForDoubleBuffering (const std::array<DeviceResourceHandle, 2>& renderTargets, const std::array<DeviceResourceHandle, 2>& colorBuffers) = 0;
        virtual void                    unpairRenderTargets               (DeviceResourceHandle renderTarget) = 0;
        virtual void                    swapDoubleBufferedRenderTarget    (DeviceResourceHandle renderTarget) = 0;

        // Blitting
        virtual void                    blitRenderTargets           (DeviceResourceHandle rtSrc, DeviceResourceHandle rtDst, const PixelRectangle& srcRect, const PixelRectangle& dstRect, bool colorOnly) = 0;

        // read back data, statistics, info
        virtual void readPixels(uint8_t* buffer, uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;

        [[nodiscard]] virtual uint32_t getTotalGpuMemoryUsageInKB() const = 0;
        virtual uint32_t getAndResetDrawCallCount() = 0;

        virtual void    validateDeviceStatusHealthy() const = 0;
        [[nodiscard]] virtual bool    isDeviceStatusHealthy() const = 0;
        virtual void    getSupportedBinaryProgramFormats(std::vector<BinaryShaderFormatID>& formats) const = 0;
        [[nodiscard]] virtual bool    isExternalTextureExtensionSupported() const = 0;

        [[nodiscard]] virtual uint32_t getGPUHandle(DeviceResourceHandle deviceHandle) const = 0;
    };
}
