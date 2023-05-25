//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_IDEVICE_H
#define RAMSES_IDEVICE_H

#include "RendererAPI/Types.h"
#include "SceneAPI/TextureEnums.h"
#include "SceneAPI/RenderState.h"
#include "SceneAPI/EDataType.h"
#include "Resource/TextureMetaInfo.h"
#include "Platform_Base/GpuResource.h"
#include <memory>
#include <array>

namespace ramses_internal
{
    class EffectResource;

    struct PixelRectangle;
    struct TextureSamplerStates;

    class IDevice
    {
    public:
        virtual ~IDevice() {}

        // data
        virtual void setConstant(DataFieldHandle field, UInt32 count, const float*      value) = 0;
        virtual void setConstant(DataFieldHandle field, UInt32 count, const glm::vec2*    value) = 0;
        virtual void setConstant(DataFieldHandle field, UInt32 count, const glm::vec3*    value) = 0;
        virtual void setConstant(DataFieldHandle field, UInt32 count, const glm::vec4*    value) = 0;
        virtual void setConstant(DataFieldHandle field, UInt32 count, const Int32*      value) = 0;
        virtual void setConstant(DataFieldHandle field, UInt32 count, const glm::ivec2*   value) = 0;
        virtual void setConstant(DataFieldHandle field, UInt32 count, const glm::ivec3*   value) = 0;
        virtual void setConstant(DataFieldHandle field, UInt32 count, const glm::ivec4*   value) = 0;
        virtual void setConstant(DataFieldHandle field, UInt32 count, const glm::mat2*  value) = 0;
        virtual void setConstant(DataFieldHandle field, UInt32 count, const glm::mat3*  value) = 0;
        virtual void setConstant(DataFieldHandle field, UInt32 count, const glm::mat4*  value) = 0;

        //draw calls
        virtual void clear               (UInt32 clearFlags) = 0;
        virtual void drawIndexedTriangles(Int32 startOffset, Int32 elementCount, UInt32 instanceCount) = 0;
        virtual void drawTriangles       (Int32 startOffset, Int32 elementCount, UInt32 instanceCount) = 0;
        virtual void flush              () = 0;

        //states
        virtual void colorMask           (bool r, bool g, bool b, bool a) = 0;
        virtual void clearColor          (const glm::vec4& clearColor) = 0;
        virtual void clearDepth          (float d) = 0;
        virtual void clearStencil        (Int32 s) = 0;
        virtual void blendFactors        (EBlendFactor sourceColor, EBlendFactor destinationColor, EBlendFactor sourceAlpha, EBlendFactor destinationAlpha) = 0;
        virtual void blendOperations     (EBlendOperation operationColor, EBlendOperation operationAlpha) = 0;
        virtual void blendColor          (const glm::vec4& color) = 0;
        virtual void cullMode            (ECullMode mode) = 0;
        virtual void depthFunc           (EDepthFunc func) = 0;
        virtual void depthWrite          (EDepthWrite flag) = 0;
        virtual void scissorTest         (EScissorTest flag, const RenderState::ScissorRegion& region) = 0;
        virtual void stencilFunc         (EStencilFunc func, UInt8 ref, UInt8 mask) = 0;
        virtual void stencilOp           (EStencilOp sfail, EStencilOp dpfail, EStencilOp dppass) = 0;
        virtual void drawMode            (EDrawMode mode) = 0;
        virtual void setViewport         (int32_t x, int32_t y, uint32_t width, uint32_t height) = 0;

        // resources
        virtual DeviceResourceHandle    allocateVertexBuffer        (UInt32 totalSizeInBytes) = 0;
        virtual void                    uploadVertexBufferData      (DeviceResourceHandle handle, const Byte* data, UInt32 dataSize) = 0;
        virtual void                    deleteVertexBuffer          (DeviceResourceHandle handle) = 0;

        virtual DeviceResourceHandle    allocateIndexBuffer         (EDataType dataType, UInt32 sizeInBytes) = 0;
        virtual void                    uploadIndexBufferData       (DeviceResourceHandle handle, const Byte* data, UInt32 dataSize) = 0;
        virtual void                    deleteIndexBuffer           (DeviceResourceHandle handle) = 0;

        virtual DeviceResourceHandle    allocateVertexArray         (const VertexArrayInfo& vertexArrayInfo) = 0;
        virtual void                    activateVertexArray         (DeviceResourceHandle handle) = 0;
        virtual void                    deleteVertexArray           (DeviceResourceHandle handle) = 0;

        virtual std::unique_ptr<const GPUResource> uploadShader     (const EffectResource& effect) = 0;
        virtual DeviceResourceHandle    registerShader              (std::unique_ptr<const GPUResource> shaderResource) = 0;
        virtual DeviceResourceHandle    uploadBinaryShader          (const EffectResource& effect, const UInt8* binaryShaderData, UInt32 binaryShaderDataSize, BinaryShaderFormatID binaryShaderFormat) = 0;
        virtual bool                    getBinaryShader             (DeviceResourceHandle handle, UInt8Vector& binaryShader, BinaryShaderFormatID& binaryShaderFormat) = 0;
        virtual void                    deleteShader                (DeviceResourceHandle handle) = 0;
        virtual void                    activateShader              (DeviceResourceHandle handle) = 0;

        virtual DeviceResourceHandle    allocateTexture2D           (UInt32 width, UInt32 height, ETextureFormat textureFormat, const TextureSwizzleArray& swizzle, UInt32 mipLevelCount, UInt32 totalSizeInBytes) = 0;
        virtual DeviceResourceHandle    allocateTexture3D           (UInt32 width, UInt32 height, UInt32 depth, ETextureFormat textureFormat, UInt32 mipLevelCount, UInt32 totalSizeInBytes) = 0;
        virtual DeviceResourceHandle    allocateTextureCube         (UInt32 faceSize, ETextureFormat textureFormat, const TextureSwizzleArray& swizzle, UInt32 mipLevelCount, UInt32 totalSizeInBytes) = 0;
        virtual DeviceResourceHandle    allocateExternalTexture     () = 0;
        [[nodiscard]] virtual DeviceResourceHandle    getEmptyExternalTexture     () const = 0;

        virtual void                    bindTexture                 (DeviceResourceHandle handle) = 0;
        virtual void                    generateMipmaps             (DeviceResourceHandle handle) = 0;
        virtual void                    uploadTextureData           (DeviceResourceHandle handle, UInt32 mipLevel, UInt32 x, UInt32 y, UInt32 z, UInt32 width, UInt32 height, UInt32 depth, const Byte* data, UInt32 dataSize) = 0;
        virtual DeviceResourceHandle    uploadStreamTexture2D       (DeviceResourceHandle handle, UInt32 width, UInt32 height, ETextureFormat format, const UInt8* data, const TextureSwizzleArray& swizzle) = 0;
        virtual void                    deleteTexture               (DeviceResourceHandle handle) = 0;
        virtual void                    activateTexture             (DeviceResourceHandle handle, DataFieldHandle field) = 0;
        [[nodiscard]] virtual int                     getTextureAddress           (DeviceResourceHandle handle) const = 0;

        // Render buffers/targets
        virtual DeviceResourceHandle    uploadRenderBuffer          (uint32_t width, uint32_t height, ERenderBufferType type, ETextureFormat format, ERenderBufferAccessMode accessMode, uint32_t sampleCount) = 0;
        virtual void                    deleteRenderBuffer          (DeviceResourceHandle handle) = 0;

        virtual DeviceResourceHandle    uploadDmaRenderBuffer       (UInt32 width, UInt32 height, DmaBufferFourccFormat fourccFormat, DmaBufferUsageFlags usageFlags, DmaBufferModifiers modifiers) = 0;
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
        virtual void readPixels(UInt8* buffer, UInt32 x, UInt32 y, UInt32 width, UInt32 height) = 0;

        [[nodiscard]] virtual uint32_t getTotalGpuMemoryUsageInKB() const = 0;
        virtual uint32_t getAndResetDrawCallCount() = 0;

        virtual void    validateDeviceStatusHealthy() const = 0;
        [[nodiscard]] virtual bool    isDeviceStatusHealthy() const = 0;
        virtual void    getSupportedBinaryProgramFormats(std::vector<BinaryShaderFormatID>& formats) const = 0;
        [[nodiscard]] virtual bool    isExternalTextureExtensionSupported() const = 0;

        [[nodiscard]] virtual uint32_t getGPUHandle(DeviceResourceHandle deviceHandle) const = 0;
    };
}

#endif
