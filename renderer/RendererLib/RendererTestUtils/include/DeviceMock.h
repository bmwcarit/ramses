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

namespace ramses_internal
{
    class DeviceResourceMapper;

    class DeviceMock : public IDevice
    {
    public:
        DeviceMock();
        DeviceMock(IContext&);
        ~DeviceMock() override;

        MOCK_CONST_METHOD0(getDeviceTypeId, EDeviceTypeId());
        MOCK_METHOD0(init, Bool());

        MOCK_METHOD1(clear, void(UInt32));

        MOCK_METHOD3(setConstant, void(DataFieldHandle, UInt32, const Float*));
        MOCK_METHOD3(setConstant, void(DataFieldHandle, UInt32, const Vector2*));
        MOCK_METHOD3(setConstant, void(DataFieldHandle, UInt32, const Vector3*));
        MOCK_METHOD3(setConstant, void(DataFieldHandle, UInt32, const Vector4*));
        MOCK_METHOD3(setConstant, void(DataFieldHandle, UInt32, const Int32*));
        MOCK_METHOD3(setConstant, void(DataFieldHandle, UInt32, const Vector2i*));
        MOCK_METHOD3(setConstant, void(DataFieldHandle, UInt32, const Vector3i*));
        MOCK_METHOD3(setConstant, void(DataFieldHandle, UInt32, const Vector4i*));
        MOCK_METHOD3(setConstant, void(DataFieldHandle, UInt32, const Matrix22f*));
        MOCK_METHOD3(setConstant, void(DataFieldHandle, UInt32, const Matrix33f*));
        MOCK_METHOD3(setConstant, void(DataFieldHandle, UInt32, const Matrix44f*));

        MOCK_METHOD3(drawIndexedTriangles, void(Int32, Int32, UInt32));
        MOCK_METHOD3(drawTriangles, void(Int32, Int32, UInt32));

        MOCK_METHOD4(colorMask, void(Bool, Bool, Bool, Bool));
        MOCK_METHOD1(clearColor, void(const Vector4&));
        MOCK_METHOD1(clearDepth, void(Float));
        MOCK_METHOD1(clearStencil, void(Int32));
        MOCK_METHOD4(blendFactors, void(EBlendFactor, EBlendFactor, EBlendFactor, EBlendFactor));
        MOCK_METHOD2(blendOperations, void(EBlendOperation, EBlendOperation));
        MOCK_METHOD1(cullMode, void(ECullMode));
        MOCK_METHOD1(depthFunc, void(EDepthFunc));
        MOCK_METHOD1(depthWrite, void(EDepthWrite));
        MOCK_METHOD3(stencilFunc, void(EStencilFunc, UInt8, UInt8));
        MOCK_METHOD2(scissorTest, void(EScissorTest, const RenderState::ScissorRegion&));
        MOCK_METHOD3(stencilOp, void(EStencilOp, EStencilOp, EStencilOp));
        MOCK_METHOD1(drawMode, void(EDrawMode));
        MOCK_METHOD4(setViewport, void(UInt32, UInt32, UInt32, UInt32));
        MOCK_METHOD7(setTextureSampling, void(DataFieldHandle, EWrapMethod, EWrapMethod, EWrapMethod, ESamplingMethod, ESamplingMethod, UInt32));

        MOCK_METHOD2(allocateVertexBuffer, DeviceResourceHandle(EDataType, UInt32));
        MOCK_METHOD3(uploadVertexBufferData, void(DeviceResourceHandle, const Byte*, UInt32));
        MOCK_METHOD1(deleteVertexBuffer, void(DeviceResourceHandle));
        MOCK_METHOD3(activateVertexBuffer, void(DeviceResourceHandle, DataFieldHandle, UInt32));
        MOCK_METHOD2(allocateIndexBuffer, DeviceResourceHandle(EDataType, UInt32));
        MOCK_METHOD3(uploadIndexBufferData, void(DeviceResourceHandle, const Byte*, UInt32));
        MOCK_METHOD1(deleteIndexBuffer, void(DeviceResourceHandle));
        MOCK_METHOD1(activateIndexBuffer, void(DeviceResourceHandle));

        MOCK_METHOD1(uploadShader, DeviceResourceHandle(const EffectResource&));
        MOCK_METHOD4(uploadBinaryShader, DeviceResourceHandle(const EffectResource&, const UInt8* binaryShaderData, UInt32 binaryShaderDataSize, UInt32 binaryShaderFormat));
        MOCK_METHOD3(getBinaryShader, Bool(DeviceResourceHandle, UInt8Vector&, UInt32&));
        MOCK_METHOD1(deleteShader, void(DeviceResourceHandle));
        MOCK_METHOD1(activateShader, void(DeviceResourceHandle));

        MOCK_METHOD5(allocateTexture2D, DeviceResourceHandle(UInt32 width, UInt32 height, ETextureFormat textureFormat, UInt32 mipLevelCount, UInt32 totalSizeInBytes));
        MOCK_METHOD6(allocateTexture3D, DeviceResourceHandle(UInt32 width, UInt32 height, UInt32 depth, ETextureFormat textureFormat, UInt32 mipLevelCount, UInt32 totalSizeInBytes));
        MOCK_METHOD4(allocateTextureCube, DeviceResourceHandle(UInt32 faceSize, ETextureFormat textureFormat, UInt32 mipLevelCount, UInt32 totalSizeInBytes));
        MOCK_METHOD1(bindTexture, void(DeviceResourceHandle handle));
        MOCK_METHOD1(generateMipmaps, void(DeviceResourceHandle handle));
        MOCK_METHOD10(uploadTextureData, void(DeviceResourceHandle handle, UInt32 mipLevel, UInt32 x, UInt32 y, UInt32 z, UInt32 width, UInt32 height, UInt32 depth, const Byte* data, UInt32 dataSize));
        MOCK_METHOD5(uploadStreamTexture2D, DeviceResourceHandle(DeviceResourceHandle handle, UInt32 width, UInt32 height, ETextureFormat format, const UInt8* data));
        MOCK_METHOD1(deleteTexture, void(DeviceResourceHandle));
        MOCK_METHOD2(activateTexture, void(DeviceResourceHandle, DataFieldHandle));

        MOCK_METHOD1(uploadRenderBuffer, DeviceResourceHandle(const RenderBuffer&));
        MOCK_METHOD1(deleteRenderBuffer, void(DeviceResourceHandle));
        MOCK_METHOD6(uploadTextureSampler, DeviceResourceHandle(EWrapMethod, EWrapMethod, EWrapMethod, ESamplingMethod, ESamplingMethod, UInt32));
        MOCK_METHOD1(deleteTextureSampler, void(DeviceResourceHandle));
        MOCK_METHOD2(activateTextureSampler, void(DeviceResourceHandle, DataFieldHandle));
        MOCK_CONST_METHOD0(getFramebufferRenderTarget, DeviceResourceHandle());
        MOCK_METHOD1(uploadRenderTarget, DeviceResourceHandle(const DeviceHandleVector&));
        MOCK_METHOD1(activateRenderTarget, void(DeviceResourceHandle));
        MOCK_METHOD1(deleteRenderTarget, void(DeviceResourceHandle));
        MOCK_METHOD5(blitRenderTargets, void(DeviceResourceHandle, DeviceResourceHandle, const PixelRectangle&, const PixelRectangle&, Bool));

        MOCK_METHOD2(pairRenderTargetsForDoubleBuffering, void(DeviceResourceHandle[2], DeviceResourceHandle[2]));
        MOCK_METHOD1(unpairRenderTargets, void(DeviceResourceHandle));
        MOCK_METHOD1(swapDoubleBufferedRenderTarget, void(DeviceResourceHandle));

        MOCK_METHOD2(uploadBlitPassRenderTargets, DeviceResourceHandle(DeviceResourceHandle, DeviceResourceHandle));
        MOCK_METHOD1(deleteBlitPassRenderTargets, void(DeviceResourceHandle));

        MOCK_METHOD5(readPixels, void(UInt8*, UInt32, UInt32, UInt32, UInt32));

        MOCK_CONST_METHOD0(getTotalGpuMemoryUsageInKB, UInt32());
        MOCK_CONST_METHOD0(getDrawCallCount, UInt32());
        MOCK_METHOD0(resetDrawCallCount, void());

        MOCK_CONST_METHOD0(validateDeviceStatusHealthy, void());
        MOCK_CONST_METHOD0(isDeviceStatusHealthy, Bool());

        MOCK_METHOD0(finish, void());

        MOCK_CONST_METHOD1(getTextureAddress, int(DeviceResourceHandle));

        static const DeviceResourceHandle FakeShaderDeviceHandle                 ;
        static const DeviceResourceHandle FakeVertexBufferDeviceHandle           ;
        static const DeviceResourceHandle FakeIndexBufferDeviceHandle            ;
        static const DeviceResourceHandle FakeTextureDeviceHandle                ;
        static const DeviceResourceHandle FakeFrameBufferRenderTargetDeviceHandle;
        static const DeviceResourceHandle FakeRenderTargetDeviceHandle           ;
        static const DeviceResourceHandle FakeRenderBufferDeviceHandle           ;
        static const DeviceResourceHandle FakeTextureSamplerDeviceHandle         ;
        static const DeviceResourceHandle FakeBlitPassRenderTargetDeviceHandle   ;

    private:
        void createDefaultMockCalls();
    };

    class DeviceMockWithDestructor : public DeviceMock
    {
    public:
        DeviceMockWithDestructor();
        ~DeviceMockWithDestructor();

        MOCK_METHOD0(Die, void());
    };
}


#endif
