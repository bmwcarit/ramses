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


namespace ramses_internal
{
    class RendererLogContext;

    class LoggingDevice : public IDevice
    {
    public:
        LoggingDevice(const IDevice& deviceDelegate, RendererLogContext& context);

        virtual EDeviceTypeId getDeviceTypeId() const override final;

        virtual void setConstant(DataFieldHandle field, UInt32 count, const Float* value) override;
        virtual void setConstant(DataFieldHandle field, UInt32 count, const Vector2* value) override;
        virtual void setConstant(DataFieldHandle field, UInt32 count, const Vector3* value) override;
        virtual void setConstant(DataFieldHandle field, UInt32 count, const Vector4* value) override;
        virtual void setConstant(DataFieldHandle field, UInt32 count, const Int32* value) override;
        virtual void setConstant(DataFieldHandle field, UInt32 count, const Vector2i* value) override;
        virtual void setConstant(DataFieldHandle field, UInt32 count, const Vector3i* value) override;
        virtual void setConstant(DataFieldHandle field, UInt32 count, const Vector4i* value) override;
        virtual void setConstant(DataFieldHandle field, UInt32 count, const Matrix22f* value) override;
        virtual void setConstant(DataFieldHandle field, UInt32 count, const Matrix33f* value) override;
        virtual void setConstant(DataFieldHandle field, UInt32 count, const Matrix44f* value) override;

        virtual void colorMask(Bool r, Bool g, Bool b, Bool a) override;
        virtual void clearColor(const Vector4& clearColor) override;
        virtual void blendOperations(EBlendOperation colorOperation, EBlendOperation alphaOperation) override;
        virtual void blendFactors(EBlendFactor sourceColor, EBlendFactor destinationColor, EBlendFactor sourceAlpha, EBlendFactor destinationAlpha) override;
        virtual void cullMode(ECullMode mode) override;
        virtual void depthFunc(EDepthFunc func) override;
        virtual void depthWrite(EDepthWrite flag) override;
        virtual void scissorTest(EScissorTest flag, const RenderState::ScissorRegion& region) override;
        virtual void stencilFunc(EStencilFunc func, UInt8 ref, UInt8 mask) override;
        virtual void stencilOp(EStencilOp sfail, EStencilOp dpfail, EStencilOp dppass) override;
        virtual void drawMode(EDrawMode mode) override;
        virtual void setViewport(UInt32 x, UInt32 y, UInt32 width, UInt32 height) override;
        virtual void setTextureSampling(DataFieldHandle field, EWrapMethod wrapU, EWrapMethod wrapV, EWrapMethod wrapR, ESamplingMethod minSampling, ESamplingMethod magSampling, UInt32 anisotropyLevel) override;

        virtual DeviceResourceHandle allocateVertexBuffer(EDataType dataType, UInt32 sizeInBytes) override;
        virtual void uploadVertexBufferData(DeviceResourceHandle handle, const Byte* data, UInt32 dataSize) override;
        virtual void deleteVertexBuffer(DeviceResourceHandle handle) override;
        virtual void activateVertexBuffer(DeviceResourceHandle handle, DataFieldHandle field, UInt32 instancingDivisor) override;
        virtual DeviceResourceHandle allocateIndexBuffer(EDataType dataType, UInt32 sizeInBytes) override;
        virtual void uploadIndexBufferData(DeviceResourceHandle handle, const Byte* data, UInt32 dataSize) override;
        virtual void deleteIndexBuffer(DeviceResourceHandle handle) override;
        virtual void activateIndexBuffer(DeviceResourceHandle handle) override;
        virtual DeviceResourceHandle uploadShader(const EffectResource& effect) override;
        virtual DeviceResourceHandle uploadBinaryShader(const EffectResource& effect, const UInt8* binaryShaderData = NULL, UInt32 binaryShaderDataSize = 0, UInt32 binaryShaderFormat = 0) override;
        virtual Bool getBinaryShader(DeviceResourceHandle handle, UInt8Vector& binaryShader, UInt32& binaryShaderFormat) override;
        virtual void deleteShader(DeviceResourceHandle handle) override;
        virtual void activateShader(DeviceResourceHandle handle) override;
        virtual DeviceResourceHandle allocateTexture2D(UInt32 width, UInt32 height, ETextureFormat textureFormat, UInt32 mipLevelCount, UInt32 totalSizeInBytes) override;
        virtual DeviceResourceHandle allocateTexture3D(UInt32 width, UInt32 height, UInt32 depth, ETextureFormat textureFormat, UInt32 mipLevelCount, UInt32 dataSize) override;
        virtual DeviceResourceHandle allocateTextureCube(UInt32 faceSize, ETextureFormat textureFormat, UInt32 mipLevelCount, UInt32 dataSize) override;
        virtual void                 bindTexture(DeviceResourceHandle handle) override;
        virtual void                 generateMipmaps(DeviceResourceHandle handle) override;
        virtual void                 uploadTextureData(DeviceResourceHandle handle, UInt32 mipLevel, UInt32 x, UInt32 y, UInt32 z, UInt32 width, UInt32 height, UInt32 depth, const Byte* data, UInt32 dataSize) override;
        virtual DeviceResourceHandle uploadStreamTexture2D(DeviceResourceHandle handle, UInt32 width, UInt32 height, ETextureFormat format, const UInt8* data) override;
        virtual void deleteTexture(DeviceResourceHandle handle) override;
        virtual void activateTexture(DeviceResourceHandle handle, DataFieldHandle field) override;
        virtual DeviceResourceHandle    uploadRenderBuffer(const RenderBuffer& renderBuffer) override;
        virtual void                    deleteRenderBuffer(DeviceResourceHandle handle) override;
        virtual DeviceResourceHandle    uploadTextureSampler(EWrapMethod wrapU, EWrapMethod wrapV, EWrapMethod wrapR, ESamplingMethod minSampling, ESamplingMethod magSampling, UInt32 anisotropyLevel) override;
        virtual void                    deleteTextureSampler(DeviceResourceHandle handle) override;
        virtual void                    activateTextureSampler(DeviceResourceHandle handle, DataFieldHandle field) override;

        virtual DeviceResourceHandle    getFramebufferRenderTarget() const override;
        virtual DeviceResourceHandle    uploadRenderTarget(const DeviceHandleVector& renderBuffers) override;
        virtual void                    activateRenderTarget(DeviceResourceHandle handle) override;
        virtual void                    deleteRenderTarget(DeviceResourceHandle handle) override;
        virtual void                    blitRenderTargets(DeviceResourceHandle rtSrc, DeviceResourceHandle rtDst, const PixelRectangle& srcRect, const PixelRectangle& dstRect, Bool colorOnly) override;

        virtual void drawIndexedTriangles(Int32 startOffset, Int32 elementCount, UInt32 instanceCount) override;
        virtual void drawTriangles(Int32 startOffset, Int32 elementCount, UInt32 instanceCount) override;
        virtual void clear(UInt32 clearFlags) override;

        virtual void                    pairRenderTargetsForDoubleBuffering(DeviceResourceHandle renderTargets[2], DeviceResourceHandle colorBuffers[2]) override;
        virtual void                    unpairRenderTargets(DeviceResourceHandle renderTarget) override;
        virtual void                    swapDoubleBufferedRenderTarget(DeviceResourceHandle renderTarget) override;

        virtual void readPixels(UInt8* buffer, UInt32 x, UInt32 y, UInt32 width, UInt32 height) override;

        virtual UInt32 getTotalGpuMemoryUsageInKB() const override;
        virtual UInt32 getDrawCallCount() const override;
        virtual void resetDrawCallCount() override;

        virtual void clearDepth(Float d) override;
        virtual void clearStencil(Int32 s) override;

        virtual Int32 getTextureAddress(DeviceResourceHandle handle) const override;

        virtual void validateDeviceStatusHealthy() const override;
        virtual Bool isDeviceStatusHealthy() const override;

        virtual void finish() override;
    private:
        // Used only to delegate getters for components
        const IDevice& m_deviceDelegate;
        RendererLogContext& m_logContext;

        void logResourceActivation(const String& resourceTypeName, DeviceResourceHandle handle, DataFieldHandle field);

    };
}

#endif
