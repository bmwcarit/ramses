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

namespace ramses_internal
{
    class ShaderGPUResource_GL;
    class RenderBufferGPUResource;
    struct GLTextureInfo;

    // TODO Violin fix this
    using GLenum = unsigned int;

    class Device_GL final : public Device_Base
    {
    public:
        explicit Device_GL(IContext& context, UInt8 majorApiVersion, UInt8 minorApiVersion, bool isEmbedded);
        virtual ~Device_GL() override;

        Bool init();

        virtual EDeviceTypeId getDeviceTypeId() const override;

        virtual void drawIndexedTriangles(Int32 startOffset, Int32 elementCount, UInt32 instanceCount) override;
        virtual void drawTriangles         (Int32 startOffset, Int32 elementCount, UInt32 instanceCount) override;

        virtual void clear                 (UInt32 clearFlags) override;
        virtual void colorMask             (Bool r, Bool g, Bool b, Bool a) override;
        virtual void clearColor            (const Vector4& clearColor) override;
        virtual void clearDepth            (Float d) override;
        virtual void clearStencil          (Int32 s) override;
        virtual void depthFunc             (EDepthFunc func) override;
        virtual void depthWrite            (EDepthWrite flag) override;
        virtual void scissorTest           (EScissorTest state, const RenderState::ScissorRegion& region) override;
        virtual void blendFactors          (EBlendFactor sourceColor, EBlendFactor destinationColor, EBlendFactor sourceAlpha, EBlendFactor destinationAlpha) override;
        virtual void blendOperations       (EBlendOperation operationColor, EBlendOperation operationAlpha) override;
        virtual void cullMode              (ECullMode mode) override;
        virtual void stencilFunc           (EStencilFunc func, UInt8 ref, UInt8 mask) override;
        virtual void stencilOp             (EStencilOp sfail, EStencilOp dpfail, EStencilOp dppass) override;
        virtual void drawMode              (EDrawMode mode) override;
        virtual void setViewport           (UInt32 start, UInt32 end, UInt32 width, UInt32 height) override;
        virtual void setTextureSampling    (DataFieldHandle field, EWrapMethod wrapU, EWrapMethod wrapV, EWrapMethod wrapR, ESamplingMethod minSampling, ESamplingMethod magSampling, UInt32 anisotropyLevel) override;

        virtual void setConstant(DataFieldHandle field, UInt32 count, const Float*      value) override;
        virtual void setConstant(DataFieldHandle field, UInt32 count, const Vector2*    value) override;
        virtual void setConstant(DataFieldHandle field, UInt32 count, const Vector3*    value) override;
        virtual void setConstant(DataFieldHandle field, UInt32 count, const Vector4*    value) override;
        virtual void setConstant(DataFieldHandle field, UInt32 count, const Int32*      value) override;
        virtual void setConstant(DataFieldHandle field, UInt32 count, const Vector2i*   value) override;
        virtual void setConstant(DataFieldHandle field, UInt32 count, const Vector3i*   value) override;
        virtual void setConstant(DataFieldHandle field, UInt32 count, const Vector4i*   value) override;
        virtual void setConstant(DataFieldHandle field, UInt32 count, const Matrix22f*  value) override;
        virtual void setConstant(DataFieldHandle field, UInt32 count, const Matrix33f*  value) override;
        virtual void setConstant(DataFieldHandle field, UInt32 count, const Matrix44f*  value) override;

        virtual void readPixels(UInt8* buffer, UInt32 x, UInt32 y, UInt32 width, UInt32 height) override;

        virtual DeviceResourceHandle    allocateVertexBuffer  (EDataType dataType, UInt32 sizeInBytes) override;
        virtual void                    uploadVertexBufferData(DeviceResourceHandle handle, const Byte* data, UInt32 dataSize) override;
        virtual void                    deleteVertexBuffer    (DeviceResourceHandle handle) override;
        virtual void                    activateVertexBuffer  (DeviceResourceHandle handle, DataFieldHandle field, UInt32 instancingDivisor) override;

        virtual DeviceResourceHandle    allocateIndexBuffer   (EDataType dataType, UInt32 sizeInBytes) override;
        virtual void                    uploadIndexBufferData (DeviceResourceHandle handle, const Byte* data, UInt32 dataSize) override;
        virtual void                    deleteIndexBuffer     (DeviceResourceHandle handle) override;
        virtual void                    activateIndexBuffer   (DeviceResourceHandle handle) override;

        virtual DeviceResourceHandle    uploadShader        (const EffectResource& shader) override;
        virtual DeviceResourceHandle    uploadBinaryShader  (const EffectResource& shader, const UInt8* binaryShaderData, UInt32 binaryShaderDataSize, UInt32 binaryShaderFormat) override;
        virtual Bool                    getBinaryShader     (DeviceResourceHandle handleconst, UInt8Vector& binaryShader, UInt32& binaryShaderFormat) override;
        virtual void                    deleteShader        (DeviceResourceHandle handle) override;
        virtual void                    activateShader      (DeviceResourceHandle handle) override;

        virtual DeviceResourceHandle    allocateTexture2D   (UInt32 width, UInt32 height, ETextureFormat textureFormat, UInt32 mipLevelCount, UInt32 totalSizeInBytes) override;
        virtual DeviceResourceHandle    allocateTexture3D   (UInt32 width, UInt32 height, UInt32 depth, ETextureFormat textureFormat, UInt32 mipLevelCount, UInt32 totalSizeInBytes) override;
        virtual DeviceResourceHandle    allocateTextureCube (UInt32 faceSize, ETextureFormat textureFormat, UInt32 mipLevelCount, UInt32 totalSizeInBytes) override;
        virtual void                    bindTexture         (DeviceResourceHandle handle) override;
        virtual void                    generateMipmaps     (DeviceResourceHandle handle) override;
        virtual void                    uploadTextureData   (DeviceResourceHandle handle, UInt32 mipLevel, UInt32 x, UInt32 y, UInt32 z, UInt32 width, UInt32 height, UInt32 depth, const Byte* data, UInt32 dataSize) override;
        virtual DeviceResourceHandle    uploadStreamTexture2D(DeviceResourceHandle handle, UInt32 width, UInt32 height, ETextureFormat format, const UInt8* data) override;
        virtual void                    deleteTexture       (DeviceResourceHandle handle) override;
        virtual void                    activateTexture     (DeviceResourceHandle handle, DataFieldHandle field) override;
        virtual int                     getTextureAddress   (DeviceResourceHandle handle) const override;

        virtual DeviceResourceHandle    uploadRenderBuffer  (const RenderBuffer& renderBuffer) override;
        virtual void                    deleteRenderBuffer  (DeviceResourceHandle handle) override;

        virtual DeviceResourceHandle    uploadTextureSampler(EWrapMethod wrapU, EWrapMethod wrapV, EWrapMethod wrapR, ESamplingMethod minSampling, ESamplingMethod magSampling, UInt32 anisotropyLevel) override;
        virtual void                    deleteTextureSampler(DeviceResourceHandle handle) override;
        virtual void                    activateTextureSampler(DeviceResourceHandle handle, DataFieldHandle field) override;

        virtual DeviceResourceHandle    getFramebufferRenderTarget() const override;
        virtual DeviceResourceHandle    uploadRenderTarget  (const DeviceHandleVector& renderBuffers) override;
        virtual void                    activateRenderTarget(DeviceResourceHandle handle) override;
        virtual void                    deleteRenderTarget(DeviceResourceHandle handle) override;

        virtual void                    pairRenderTargetsForDoubleBuffering(DeviceResourceHandle renderTargets[2], DeviceResourceHandle colorBuffers[2]) override;
        virtual void                    unpairRenderTargets(DeviceResourceHandle renderTarget) override;
        virtual void                    swapDoubleBufferedRenderTarget(DeviceResourceHandle renderTarget) override;

        virtual void                    blitRenderTargets   (DeviceResourceHandle rtSrc, DeviceResourceHandle rtDst, const PixelRectangle& srcRect, const PixelRectangle& dstRect, Bool colorOnly) override;

        virtual void                    validateDeviceStatusHealthy() const override;
        virtual Bool                    isDeviceStatusHealthy() const override;

        virtual UInt32                  getTotalGpuMemoryUsageInKB() const override;

        virtual void                    finish() override;

    private:
        const IContext&             m_context;
        DeviceResourceMapper&       m_resourceMapper;
        DeviceResourceHandle        m_framebufferRenderTarget;

        struct RenderTargetPair
        {
            DeviceResourceHandle renderTargets[2];
            DeviceResourceHandle colorBuffers[2];
            UInt8 readingIndex;
        };

        std::vector<RenderTargetPair> m_pairedRenderTargets;

        // Active states for upcoming draw call(s)
        const ShaderGPUResource_GL* m_activeShader;
        EDrawMode                   m_activePrimitiveDrawMode;
        UInt32                      m_activeIndexArrayElementSizeBytes;

        const UInt8                 m_majorApiVersion;
        const UInt8                 m_minorApiVersion;
        const bool                  m_isEmbedded;
        DebugOutput                 m_debugOutput;
        StringSet                   m_apiExtensions;

        Bool getUniformLocation(DataFieldHandle field, GLInputLocation& location) const;
        Bool getAttributeLocation(DataFieldHandle field, GLInputLocation& location) const;

        Bool allBuffersHaveTheSameSize(const DeviceHandleVector& renderBuffers) const;
        void bindRenderBufferToRenderTarget(const RenderBufferGPUResource& renderBufferGpuResource, const UInt32 colorBufferSlot);
        void bindReadWriteRenderBufferToRenderTarget(const ERenderBufferType bufferType, const UInt32 colorBufferSlot, const GLHandle bufferGLHandle);
        void bindWriteOnlyRenderBufferToRenderTarget(const ERenderBufferType bufferType, const UInt32 colorBufferSlot, const GLHandle bufferGLHandle);
        GLHandle createTexture(UInt32 width, UInt32 height, ETextureFormat storageFormat) const;
        GLHandle createRenderBuffer(UInt32 width, UInt32 height, ETextureFormat format, UInt32 sampleCount);
        void setDrawBuffers(const RenderTarget& renderTarget);

        GLHandle generateAndBindTexture(GLenum target) const;
        void setTextureFiltering(GLenum target, EWrapMethod wrapU, EWrapMethod wrapV, EWrapMethod wrapR, ESamplingMethod minSampling, ESamplingMethod magSampling, UInt32 anisotropyLevel);

        void fillGLInternalTextureInfo(GLenum target, UInt32 width, UInt32 height, UInt32 depth, ETextureFormat textureFormat, GLTextureInfo& texInfoOut) const;

        void allocateTextureStorage(const GLTextureInfo& texInfo, UInt32 mipLevels) const;
        void uploadTextureMipMapData(UInt32 mipLevel, UInt32 x, UInt32 y, UInt32 z, UInt32 width, UInt32 height, UInt32 depth, const GLTextureInfo& texInfo, const UInt8 *pData, UInt32 dataSize) const;

        Bool isApiExtensionAvailable(const String& extensionName) const;
        void loadExtensionDependentFeatures();
        void loadOpenGLExtensions();
    };
}

#endif
