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

        Bool init();

        void drawIndexedTriangles(Int32 startOffset, Int32 elementCount, UInt32 instanceCount) override;
        void drawTriangles         (Int32 startOffset, Int32 elementCount, UInt32 instanceCount) override;

        void clear                 (UInt32 clearFlags) override;
        void colorMask             (Bool r, Bool g, Bool b, Bool a) override;
        void clearColor            (const Vector4& clearColor) override;
        void clearDepth            (Float d) override;
        void clearStencil          (Int32 s) override;
        void depthFunc             (EDepthFunc func) override;
        void depthWrite            (EDepthWrite flag) override;
        void scissorTest           (EScissorTest state, const RenderState::ScissorRegion& region) override;
        void blendFactors          (EBlendFactor sourceColor, EBlendFactor destinationColor, EBlendFactor sourceAlpha, EBlendFactor destinationAlpha) override;
        void blendColor            (const Vector4& color) override;
        void blendOperations       (EBlendOperation operationColor, EBlendOperation operationAlpha) override;
        void cullMode              (ECullMode mode) override;
        void stencilFunc           (EStencilFunc func, UInt8 ref, UInt8 mask) override;
        void stencilOp             (EStencilOp sfail, EStencilOp dpfail, EStencilOp dppass) override;
        void drawMode              (EDrawMode mode) override;
        void setViewport           (int32_t x, int32_t y, uint32_t width, uint32_t height) override;

        void setConstant(DataFieldHandle field, UInt32 count, const Float*      value) override;
        void setConstant(DataFieldHandle field, UInt32 count, const Vector2*    value) override;
        void setConstant(DataFieldHandle field, UInt32 count, const Vector3*    value) override;
        void setConstant(DataFieldHandle field, UInt32 count, const Vector4*    value) override;
        void setConstant(DataFieldHandle field, UInt32 count, const Int32*      value) override;
        void setConstant(DataFieldHandle field, UInt32 count, const Vector2i*   value) override;
        void setConstant(DataFieldHandle field, UInt32 count, const Vector3i*   value) override;
        void setConstant(DataFieldHandle field, UInt32 count, const Vector4i*   value) override;
        void setConstant(DataFieldHandle field, UInt32 count, const Matrix22f*  value) override;
        void setConstant(DataFieldHandle field, UInt32 count, const Matrix33f*  value) override;
        void setConstant(DataFieldHandle field, UInt32 count, const Matrix44f*  value) override;

        void readPixels(UInt8* buffer, UInt32 x, UInt32 y, UInt32 width, UInt32 height) override;

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
        DeviceResourceHandle    uploadBinaryShader  (const EffectResource& shader, const UInt8* binaryShaderData, UInt32 binaryShaderDataSize, BinaryShaderFormatID binaryShaderFormat) override;
        Bool                    getBinaryShader     (DeviceResourceHandle handleconst, UInt8Vector& binaryShader, BinaryShaderFormatID& binaryShaderFormat) override;
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
        DeviceResourceHandle    uploadStreamTexture2D(DeviceResourceHandle handle, UInt32 width, UInt32 height, ETextureFormat format, const UInt8* data, const TextureSwizzleArray& swizzle) override;
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

        void                    pairRenderTargetsForDoubleBuffering(DeviceResourceHandle renderTargets[2], DeviceResourceHandle colorBuffers[2]) override;
        void                    unpairRenderTargets(DeviceResourceHandle renderTarget) override;
        void                    swapDoubleBufferedRenderTarget(DeviceResourceHandle renderTarget) override;

        void                    blitRenderTargets   (DeviceResourceHandle rtSrc, DeviceResourceHandle rtDst, const PixelRectangle& srcRect, const PixelRectangle& dstRect, Bool colorOnly) override;

        void                    validateDeviceStatusHealthy() const override;
        Bool                    isDeviceStatusHealthy() const override;
        void                    getSupportedBinaryProgramFormats(std::vector<BinaryShaderFormatID>& formats) const override;
        bool                    isExternalTextureExtensionSupported() const override;


        UInt32                  getTotalGpuMemoryUsageInKB() const override;

        void                    flush() override;

    private:
        DeviceResourceHandle        m_framebufferRenderTarget;

        struct RenderTargetPair
        {
            DeviceResourceHandle renderTargets[2];
            DeviceResourceHandle colorBuffers[2];
            UInt8 readingIndex;
        };

        std::vector<RenderTargetPair> m_pairedRenderTargets;

        // Active states for upcoming draw call(s)
        const ShaderGPUResource_GL* m_activeShader = nullptr;
        EDrawMode                   m_activePrimitiveDrawMode = EDrawMode::Points;
        uint32_t                    m_activeIndexArrayElementSizeBytes = 0u;
        uint32_t                    m_activeIndexArraySizeBytes = 0u;

        DebugOutput                 m_debugOutput;
        HashSet<String>             m_apiExtensions;
        std::vector<GLint>          m_supportedBinaryProgramFormats;
        IDeviceExtension*           m_deviceExtension = nullptr;
        const DeviceResourceHandle  m_emptyExternalTextureResource;

        std::unordered_map<uint64_t, DeviceResourceHandle> m_textureSamplerObjectsCache;

        Bool getUniformLocation(DataFieldHandle field, GLInputLocation& location) const;
        Bool getAttributeLocation(DataFieldHandle field, GLInputLocation& location) const;

        Bool allBuffersHaveTheSameSize(const DeviceHandleVector& renderBuffers) const;
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
        void uploadTextureMipMapData(UInt32 mipLevel, UInt32 x, UInt32 y, UInt32 z, UInt32 width, UInt32 height, UInt32 depth, const GLTextureInfo& texInfo, const UInt8 *pData, UInt32 dataSize) const;

        Bool isApiExtensionAvailable(const String& extensionName) const;
        void queryDeviceDependentFeatures();
        void loadOpenGLExtensions();
    };
}

#endif
