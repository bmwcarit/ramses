//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DEVICEMOCK_H
#define RAMSES_DEVICEMOCK_H

#include "renderer_common_gmock_header.h"
#include "gmock/gmock.h"
#include "RendererAPI/IDevice.h"
#include "RendererAPI/IContext.h"
#include "Resource/EffectResource.h"
#include "SceneAPI/RenderBuffer.h"
#include "SceneAPI/PixelRectangle.h"
#include "Math3d/Vector4.h"
#include "SceneAPI/TextureSamplerStates.h"

namespace ramses_internal
{
    class DeviceResourceMapper;

    class DeviceMock : public IDevice
    {
    public:
        DeviceMock();
        ~DeviceMock() override;

        MOCK_METHOD(EDeviceTypeId, getDeviceTypeId, (), (const, override));
        MOCK_METHOD(Bool, init, ());

        MOCK_METHOD(void, clear, (UInt32), (override));

        MOCK_METHOD(void, setConstant, (DataFieldHandle, UInt32, const Float*), (override));
        MOCK_METHOD(void, setConstant, (DataFieldHandle, UInt32, const Vector2*), (override));
        MOCK_METHOD(void, setConstant, (DataFieldHandle, UInt32, const Vector3*), (override));
        MOCK_METHOD(void, setConstant, (DataFieldHandle, UInt32, const Vector4*), (override));
        MOCK_METHOD(void, setConstant, (DataFieldHandle, UInt32, const Int32*), (override));
        MOCK_METHOD(void, setConstant, (DataFieldHandle, UInt32, const Vector2i*), (override));
        MOCK_METHOD(void, setConstant, (DataFieldHandle, UInt32, const Vector3i*), (override));
        MOCK_METHOD(void, setConstant, (DataFieldHandle, UInt32, const Vector4i*), (override));
        MOCK_METHOD(void, setConstant, (DataFieldHandle, UInt32, const Matrix22f*), (override));
        MOCK_METHOD(void, setConstant, (DataFieldHandle, UInt32, const Matrix33f*), (override));
        MOCK_METHOD(void, setConstant, (DataFieldHandle, UInt32, const Matrix44f*), (override));

        MOCK_METHOD(void, drawIndexedTriangles, (Int32, Int32, UInt32), (override));
        MOCK_METHOD(void, drawTriangles, (Int32, Int32, UInt32), (override));

        MOCK_METHOD(void, colorMask, (Bool, Bool, Bool, Bool), (override));
        MOCK_METHOD(void, clearColor, (const Vector4&), (override));
        MOCK_METHOD(void, clearDepth, (Float), (override));
        MOCK_METHOD(void, clearStencil, (Int32), (override));
        MOCK_METHOD(void, blendFactors, (EBlendFactor, EBlendFactor, EBlendFactor, EBlendFactor), (override));
        MOCK_METHOD(void, blendOperations, (EBlendOperation, EBlendOperation), (override));
        MOCK_METHOD(void, blendColor, (const Vector4&), (override));
        MOCK_METHOD(void, cullMode, (ECullMode), (override));
        MOCK_METHOD(void, depthFunc, (EDepthFunc), (override));
        MOCK_METHOD(void, depthWrite, (EDepthWrite), (override));
        MOCK_METHOD(void, stencilFunc, (EStencilFunc, UInt8, UInt8), (override));
        MOCK_METHOD(void, scissorTest, (EScissorTest, const RenderState::ScissorRegion&), (override));
        MOCK_METHOD(void, stencilOp, (EStencilOp, EStencilOp, EStencilOp), (override));
        MOCK_METHOD(void, drawMode, (EDrawMode), (override));
        MOCK_METHOD(void, setViewport, (int32_t, int32_t, uint32_t, uint32_t), (override));

        MOCK_METHOD(DeviceResourceHandle, allocateVertexBuffer, (UInt32), (override));
        MOCK_METHOD(void, uploadVertexBufferData, (DeviceResourceHandle, const Byte*, UInt32), (override));
        MOCK_METHOD(void, deleteVertexBuffer, (DeviceResourceHandle), (override));
        MOCK_METHOD(DeviceResourceHandle, allocateVertexArray, (const VertexArrayInfo&), (override));
        MOCK_METHOD(void, activateVertexArray, (DeviceResourceHandle handle), (override));
        MOCK_METHOD(void, deleteVertexArray, (DeviceResourceHandle handle), (override));
        MOCK_METHOD(DeviceResourceHandle, allocateIndexBuffer, (EDataType, UInt32), (override));
        MOCK_METHOD(void, uploadIndexBufferData, (DeviceResourceHandle, const Byte*, UInt32), (override));
        MOCK_METHOD(void, deleteIndexBuffer, (DeviceResourceHandle), (override));

        MOCK_METHOD(std::unique_ptr<const GPUResource>, uploadShader, (const EffectResource&), (override));
        MOCK_METHOD(DeviceResourceHandle, registerShader, (std::unique_ptr<const GPUResource>), (override));
        MOCK_METHOD(DeviceResourceHandle, uploadBinaryShader, (const EffectResource&, const UInt8* binaryShaderData, UInt32 binaryShaderDataSize, BinaryShaderFormatID binaryShaderFormat), (override));
        MOCK_METHOD(Bool, getBinaryShader, (DeviceResourceHandle, UInt8Vector&, BinaryShaderFormatID&), (override));
        MOCK_METHOD(void, deleteShader, (DeviceResourceHandle), (override));
        MOCK_METHOD(void, activateShader, (DeviceResourceHandle), (override));

        MOCK_METHOD(DeviceResourceHandle, allocateTexture2D, (UInt32 width, UInt32 height, ETextureFormat textureFormat, const TextureSwizzleArray& swizzle, UInt32 mipLevelCount, UInt32 totalSizeInBytes), (override));
        MOCK_METHOD(DeviceResourceHandle, allocateTexture3D, (UInt32 width, UInt32 height, UInt32 depth, ETextureFormat textureFormat, UInt32 mipLevelCount, UInt32 totalSizeInBytes), (override));
        MOCK_METHOD(DeviceResourceHandle, allocateTextureCube, (UInt32 faceSize, ETextureFormat textureFormat, const TextureSwizzleArray& swizzle, UInt32 mipLevelCount, UInt32 totalSizeInBytes), (override));
        MOCK_METHOD(void, bindTexture, (DeviceResourceHandle handle), (override));
        MOCK_METHOD(void, generateMipmaps, (DeviceResourceHandle handle), (override));
        MOCK_METHOD(void, uploadTextureData, (DeviceResourceHandle handle, UInt32 mipLevel, UInt32 x, UInt32 y, UInt32 z, UInt32 width, UInt32 height, UInt32 depth, const Byte* data, UInt32 dataSize), (override));
        MOCK_METHOD(DeviceResourceHandle, uploadStreamTexture2D, (DeviceResourceHandle handle, UInt32 width, UInt32 height, ETextureFormat format, const UInt8* data, const TextureSwizzleArray& swizzle), (override));
        MOCK_METHOD(void, deleteTexture, (DeviceResourceHandle), (override));
        MOCK_METHOD(void, activateTexture, (DeviceResourceHandle, DataFieldHandle), (override));
        MOCK_METHOD(DeviceResourceHandle, uploadRenderBuffer, (uint32_t, uint32_t, ERenderBufferType, ETextureFormat, ERenderBufferAccessMode, uint32_t), (override));
        MOCK_METHOD(void, deleteRenderBuffer, (DeviceResourceHandle), (override));
        MOCK_METHOD(DeviceResourceHandle, uploadDmaRenderBuffer, (UInt32, UInt32, DmaBufferFourccFormat, DmaBufferUsageFlags, DmaBufferModifiers), (override));
        MOCK_METHOD(int, getDmaRenderBufferFD, (DeviceResourceHandle handle), (override));
        MOCK_METHOD(UInt32, getDmaRenderBufferStride, (DeviceResourceHandle handle), (override));
        MOCK_METHOD(void, destroyDmaRenderBuffer, (DeviceResourceHandle handle), (override));
        MOCK_METHOD(void, activateTextureSamplerObject, (const TextureSamplerStates&, DataFieldHandle), (override));
        MOCK_METHOD(DeviceResourceHandle, getFramebufferRenderTarget, (), (const, override));
        MOCK_METHOD(DeviceResourceHandle, uploadRenderTarget, (const DeviceHandleVector&), (override));
        MOCK_METHOD(void, activateRenderTarget, (DeviceResourceHandle), (override));
        MOCK_METHOD(void, deleteRenderTarget, (DeviceResourceHandle), (override));
        MOCK_METHOD(void, discardDepthStencil, (), (override));
        MOCK_METHOD(void, blitRenderTargets, (DeviceResourceHandle, DeviceResourceHandle, const PixelRectangle&, const PixelRectangle&, Bool), (override));

        MOCK_METHOD(void, pairRenderTargetsForDoubleBuffering, (DeviceResourceHandle[2], DeviceResourceHandle[2]), (override));
        MOCK_METHOD(void, unpairRenderTargets, (DeviceResourceHandle), (override));
        MOCK_METHOD(void, swapDoubleBufferedRenderTarget, (DeviceResourceHandle), (override));

        MOCK_METHOD(void, readPixels, (UInt8*, UInt32, UInt32, UInt32, UInt32), (override));

        MOCK_METHOD(UInt32, getTotalGpuMemoryUsageInKB, (), (const, override));
        MOCK_METHOD(UInt32, getAndResetDrawCallCount, (), (override));

        MOCK_METHOD(void, validateDeviceStatusHealthy, (), (const, override));
        MOCK_METHOD(Bool, isDeviceStatusHealthy, (), (const, override));
        MOCK_METHOD(void, getSupportedBinaryProgramFormats, (std::vector<BinaryShaderFormatID>&), (const, override));

        MOCK_METHOD(void, finish, (), (override));

        MOCK_METHOD(int, getTextureAddress, (DeviceResourceHandle), (const, override));
        MOCK_METHOD(uint32_t, getGPUHandle, (DeviceResourceHandle), (const, override));

        static const DeviceResourceHandle FakeShaderDeviceHandle                 ;
        static const DeviceResourceHandle FakeVertexBufferDeviceHandle           ;
        static const DeviceResourceHandle FakeIndexBufferDeviceHandle            ;
        static const DeviceResourceHandle FakeVertexArrayDeviceHandle            ;
        static const DeviceResourceHandle FakeTextureDeviceHandle                ;
        static const DeviceResourceHandle FakeFrameBufferRenderTargetDeviceHandle;
        static const DeviceResourceHandle FakeRenderTargetDeviceHandle           ;
        static const DeviceResourceHandle FakeRenderBufferDeviceHandle           ;
        static const DeviceResourceHandle FakeDmaRenderBufferDeviceHandle        ;
        static const DeviceResourceHandle FakeTextureSamplerDeviceHandle         ;
        static const DeviceResourceHandle FakeBlitPassRenderTargetDeviceHandle   ;
        static constexpr BinaryShaderFormatID FakeSupportedBinaryShaderFormat{ 63666u };

    private:
        void createDefaultMockCalls();
    };
}


#endif
