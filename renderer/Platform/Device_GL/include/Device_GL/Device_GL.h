//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DEVICE_GL_H
#define RAMSES_DEVICE_GL_H

#include "Platform_Base/Device_Base.h"
#include "Platform_Base/DeviceResourceMapper.h"
#include "Types_GL.h"
#include "DebugOutput.h"
#include "SceneAPI/TextureSamplerStates.h"

#include <unordered_map>
#include <string>

namespace ramses_internal
{
    class ShaderGPUResource_GL;
    class RenderBufferGPUResource;
    class IDeviceExtension;
    struct GLTextureInfo;

    class Device_GL : public Device_Base
    {
    public:
        explicit Device_GL(IContext& context, IDeviceExtension* deviceExtension);
        ~Device_GL() override;

        bool init();

        void drawIndexedTriangles(Int32 startOffset, Int32 elementCount, UInt32 instanceCount) override;
        void drawTriangles         (Int32 startOffset, Int32 elementCount, UInt32 instanceCount) override;

        void clear                 (UInt32 clearFlags) override;
        void colorMask             (bool r, bool g, bool b, bool a) override;
        void clearColor            (const glm::vec4& clearColor) override;
        void clearDepth            (float d) override;
        void clearStencil          (Int32 s) override;
        void depthFunc             (EDepthFunc func) override;
        void depthWrite            (EDepthWrite flag) override;
        void scissorTest           (EScissorTest state, const RenderState::ScissorRegion& region) override;
        void blendFactors          (EBlendFactor sourceColor, EBlendFactor destinationColor, EBlendFactor sourceAlpha, EBlendFactor destinationAlpha) override;
        void blendColor            (const glm::vec4& color) override;
        void blendOperations       (EBlendOperation operationColor, EBlendOperation operationAlpha) override;
        void cullMode              (ECullMode mode) override;
        void stencilFunc           (EStencilFunc func, uint8_t ref, uint8_t mask) override;
        void stencilOp             (EStencilOp sfail, EStencilOp dpfail, EStencilOp dppass) override;
        void drawMode              (EDrawMode mode) override;
        void setViewport           (int32_t x, int32_t y, uint32_t width, uint32_t height) override;

        void setConstant(DataFieldHandle field, UInt32 count, const float*      value) override;
        void setConstant(DataFieldHandle field, UInt32 count, const glm::vec2*    value) override;
        void setConstant(DataFieldHandle field, UInt32 count, const glm::vec3*    value) override;
        void setConstant(DataFieldHandle field, UInt32 count, const glm::vec4*    value) override;
        void setConstant(DataFieldHandle field, UInt32 count, const Int32*      value) override;
        void setConstant(DataFieldHandle field, UInt32 count, const glm::ivec2*   value) override;
        void setConstant(DataFieldHandle field, UInt32 count, const glm::ivec3*   value) override;
        void setConstant(DataFieldHandle field, UInt32 count, const glm::ivec4*   value) override;
        void setConstant(DataFieldHandle field, UInt32 count, const glm::mat2*  value) override;
        void setConstant(DataFieldHandle field, UInt32 count, const glm::mat3*  value) override;
        void setConstant(DataFieldHandle field, UInt32 count, const glm::mat4*  value) override;

        void readPixels(uint8_t* buffer, UInt32 x, UInt32 y, UInt32 width, UInt32 height) override;

        DeviceResourceHandle    allocateVertexBuffer  (UInt32 totalSizeInBytes) override;
        void                    uploadVertexBufferData(DeviceResourceHandle handle, const Byte* data, UInt32 dataSize) override;
        void                    deleteVertexBuffer    (DeviceResourceHandle handle) override;

        DeviceResourceHandle    allocateVertexArray   (const VertexArrayInfo& vertexArrayInfo) override;
        void                    activateVertexArray   (DeviceResourceHandle handle) override;
        void                    deleteVertexArray     (DeviceResourceHandle handle) override;

        DeviceResourceHandle    allocateIndexBuffer   (EDataType dataType, UInt32 sizeInBytes) override;
        void                    uploadIndexBufferData (DeviceResourceHandle handle, const Byte* data, UInt32 dataSize) override;
        void                    deleteIndexBuffer     (DeviceResourceHandle handle) override;

        std::unique_ptr<const GPUResource> uploadShader(const EffectResource& shader) override;
        DeviceResourceHandle    registerShader      (std::unique_ptr<const GPUResource> shaderResource) override;
        DeviceResourceHandle    uploadBinaryShader  (const EffectResource& shader, const uint8_t* binaryShaderData, UInt32 binaryShaderDataSize, BinaryShaderFormatID binaryShaderFormat) override;
        bool                    getBinaryShader     (DeviceResourceHandle handleconst, UInt8Vector& binaryShader, BinaryShaderFormatID& binaryShaderFormat) override;
        void                    deleteShader        (DeviceResourceHandle handle) override;
        void                    activateShader      (DeviceResourceHandle handle) override;

        DeviceResourceHandle    allocateTexture2D   (UInt32 width, UInt32 height, ETextureFormat textureFormat, const TextureSwizzleArray& swizzle, UInt32 mipLevelCount, UInt32 totalSizeInBytes) override;
        DeviceResourceHandle    allocateTexture3D   (UInt32 width, UInt32 height, UInt32 depth, ETextureFormat textureFormat, UInt32 mipLevelCount, UInt32 totalSizeInBytes) override;
        DeviceResourceHandle    allocateTextureCube (UInt32 faceSize, ETextureFormat textureFormat, const TextureSwizzleArray& swizzle, UInt32 mipLevelCount, UInt32 totalSizeInBytes) override;
        DeviceResourceHandle    allocateExternalTexture() override;
        DeviceResourceHandle    getEmptyExternalTexture() const override;

        void                    bindTexture         (DeviceResourceHandle handle) override;
        void                    generateMipmaps     (DeviceResourceHandle handle) override;
        void                    uploadTextureData   (DeviceResourceHandle handle, UInt32 mipLevel, UInt32 x, UInt32 y, UInt32 z, UInt32 width, UInt32 height, UInt32 depth, const Byte* data, UInt32 dataSize) override;
        DeviceResourceHandle    uploadStreamTexture2D(DeviceResourceHandle handle, UInt32 width, UInt32 height, ETextureFormat format, const uint8_t* data, const TextureSwizzleArray& swizzle) override;
        void                    deleteTexture       (DeviceResourceHandle handle) override;
        void                    activateTexture     (DeviceResourceHandle handle, DataFieldHandle field) override;
        int                     getTextureAddress   (DeviceResourceHandle handle) const override;

        DeviceResourceHandle    uploadRenderBuffer  (uint32_t width, uint32_t height, ERenderBufferType type, ETextureFormat format, ERenderBufferAccessMode accessMode, uint32_t sampleCount) override;
        void                    deleteRenderBuffer  (DeviceResourceHandle handle) override;

        DeviceResourceHandle    uploadDmaRenderBuffer   (UInt32 width, UInt32 height, DmaBufferFourccFormat fourccFormat, DmaBufferUsageFlags usageFlags, DmaBufferModifiers modifiers) override;
        int                     getDmaRenderBufferFD    (DeviceResourceHandle handle) override;
        uint32_t                getDmaRenderBufferStride(DeviceResourceHandle handle) override;
        void                    destroyDmaRenderBuffer  (DeviceResourceHandle handle) override;

        void                    activateTextureSamplerObject(const TextureSamplerStates& samplerStates, DataFieldHandle field) override;

        DeviceResourceHandle    getFramebufferRenderTarget() const override;
        DeviceResourceHandle    uploadRenderTarget  (const DeviceHandleVector& renderBuffers) override;
        void                    activateRenderTarget(DeviceResourceHandle handle) override;
        void                    deleteRenderTarget(DeviceResourceHandle handle) override;
        void                    discardDepthStencil() override;

        void                    pairRenderTargetsForDoubleBuffering(const std::array<DeviceResourceHandle, 2>& renderTargets, const std::array<DeviceResourceHandle, 2>& colorBuffers) override;
        void                    unpairRenderTargets(DeviceResourceHandle renderTarget) override;
        void                    swapDoubleBufferedRenderTarget(DeviceResourceHandle renderTarget) override;

        void                    blitRenderTargets   (DeviceResourceHandle rtSrc, DeviceResourceHandle rtDst, const PixelRectangle& srcRect, const PixelRectangle& dstRect, bool colorOnly) override;

        void                    validateDeviceStatusHealthy() const override;
        bool                    isDeviceStatusHealthy() const override;
        void                    getSupportedBinaryProgramFormats(std::vector<BinaryShaderFormatID>& formats) const override;
        bool                    isExternalTextureExtensionSupported() const override;


        UInt32                  getTotalGpuMemoryUsageInKB() const override;

        void                    flush() override;

    private:
        DeviceResourceHandle        m_framebufferRenderTarget;

        struct RenderTargetPair
        {
            std::array<DeviceResourceHandle, 2> renderTargets;
            std::array<DeviceResourceHandle, 2> colorBuffers;
            uint8_t readingIndex;
        };

        std::vector<RenderTargetPair> m_pairedRenderTargets;

        // Active states for upcoming draw call(s)
        const ShaderGPUResource_GL* m_activeShader = nullptr;
        EDrawMode                   m_activePrimitiveDrawMode = EDrawMode::Points;
        uint32_t                    m_activeIndexArrayElementSizeBytes = 0u;
        uint32_t                    m_activeIndexArraySizeBytes = 0u;

        DebugOutput                 m_debugOutput;
        HashSet<std::string>        m_apiExtensions;
        std::vector<GLint>          m_supportedBinaryProgramFormats;
        IDeviceExtension*           m_deviceExtension = nullptr;
        const DeviceResourceHandle  m_emptyExternalTextureResource;

        std::unordered_map<uint64_t, DeviceResourceHandle> m_textureSamplerObjectsCache;

        bool getUniformLocation(DataFieldHandle field, GLInputLocation& location) const;
        bool getAttributeLocation(DataFieldHandle field, GLInputLocation& location) const;

        bool allBuffersHaveTheSameSize(const DeviceHandleVector& renderBuffers) const;
        void bindRenderBufferToRenderTarget(const RenderBufferGPUResource& renderBufferGpuResource, size_t colorBufferSlot);
        void bindReadWriteRenderBufferToRenderTarget(ERenderBufferType bufferType, size_t colorBufferSlot, GLHandle bufferGLHandle, bool multiSample);
        void bindWriteOnlyRenderBufferToRenderTarget(ERenderBufferType bufferType, size_t colorBufferSlot, GLHandle bufferGLHandle);
        GLHandle createTexture(UInt32 width, UInt32 height, ETextureFormat storageFormat, UInt32 sampleCount) const;
        GLHandle createRenderBuffer(UInt32 width, UInt32 height, ETextureFormat format, UInt32 sampleCount);

        DeviceResourceHandle    uploadTextureSampler(const TextureSamplerStates& samplerStates);
        void                    deleteTextureSampler(DeviceResourceHandle handle);
        void                    activateTextureSampler(DeviceResourceHandle handle, DataFieldHandle field);

        GLHandle generateAndBindTexture(GLenum target) const;

        void fillGLInternalTextureInfo(GLenum target, UInt32 width, UInt32 height, UInt32 depth, ETextureFormat textureFormat, const TextureSwizzleArray& swizzle, GLTextureInfo& texInfoOut) const;
        uint32_t checkAndClampNumberOfSamples(GLenum internalFormat, uint32_t numSamples) const;

        void allocateTextureStorage(const GLTextureInfo& texInfo, UInt32 mipLevels, UInt32 sampleCount = 0) const;
        void uploadTextureMipMapData(UInt32 mipLevel, UInt32 x, UInt32 y, UInt32 z, UInt32 width, UInt32 height, UInt32 depth, const GLTextureInfo& texInfo, const uint8_t *pData, UInt32 dataSize) const;

        bool isApiExtensionAvailable(const std::string& extensionName) const;
        void queryDeviceDependentFeatures();
        void loadOpenGLExtensions();
    };
}

#endif
