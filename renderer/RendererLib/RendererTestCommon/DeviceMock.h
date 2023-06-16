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
#include "SceneAPI/TextureSamplerStates.h"

namespace ramses_internal
{
    class DeviceResourceMapper;

    class DeviceMock : public IDevice
    {
    public:
        DeviceMock();
        ~DeviceMock() override;

        MOCK_METHOD(bool, init, ());

        MOCK_METHOD(void, clear, (uint32_t), (override));

        MOCK_METHOD(void, setConstant, (DataFieldHandle, uint32_t, const float*), (override));
        MOCK_METHOD(void, setConstant, (DataFieldHandle, uint32_t, const glm::vec2*), (override));
        MOCK_METHOD(void, setConstant, (DataFieldHandle, uint32_t, const glm::vec3*), (override));
        MOCK_METHOD(void, setConstant, (DataFieldHandle, uint32_t, const glm::vec4*), (override));
        MOCK_METHOD(void, setConstant, (DataFieldHandle, uint32_t, const int32_t*), (override));
        MOCK_METHOD(void, setConstant, (DataFieldHandle, uint32_t, const glm::ivec2*), (override));
        MOCK_METHOD(void, setConstant, (DataFieldHandle, uint32_t, const glm::ivec3*), (override));
        MOCK_METHOD(void, setConstant, (DataFieldHandle, uint32_t, const glm::ivec4*), (override));
        MOCK_METHOD(void, setConstant, (DataFieldHandle, uint32_t, const glm::mat2*), (override));
        MOCK_METHOD(void, setConstant, (DataFieldHandle, uint32_t, const glm::mat3*), (override));
        MOCK_METHOD(void, setConstant, (DataFieldHandle, uint32_t, const glm::mat4*), (override));

        MOCK_METHOD(void, drawIndexedTriangles, (int32_t, int32_t, uint32_t), (override));
        MOCK_METHOD(void, drawTriangles, (int32_t, int32_t, uint32_t), (override));

        MOCK_METHOD(void, colorMask, (bool, bool, bool, bool), (override));
        MOCK_METHOD(void, clearColor, (const glm::vec4&), (override));
        MOCK_METHOD(void, clearDepth, (float), (override));
        MOCK_METHOD(void, clearStencil, (int32_t), (override));
        MOCK_METHOD(void, blendFactors, (EBlendFactor, EBlendFactor, EBlendFactor, EBlendFactor), (override));
        MOCK_METHOD(void, blendOperations, (EBlendOperation, EBlendOperation), (override));
        MOCK_METHOD(void, blendColor, (const glm::vec4&), (override));
        MOCK_METHOD(void, cullMode, (ECullMode), (override));
        MOCK_METHOD(void, depthFunc, (EDepthFunc), (override));
        MOCK_METHOD(void, depthWrite, (EDepthWrite), (override));
        MOCK_METHOD(void, stencilFunc, (EStencilFunc, uint8_t, uint8_t), (override));
        MOCK_METHOD(void, scissorTest, (EScissorTest, const RenderState::ScissorRegion&), (override));
        MOCK_METHOD(void, stencilOp, (EStencilOp, EStencilOp, EStencilOp), (override));
        MOCK_METHOD(void, drawMode, (EDrawMode), (override));
        MOCK_METHOD(void, setViewport, (int32_t, int32_t, uint32_t, uint32_t), (override));

        MOCK_METHOD(DeviceResourceHandle, allocateVertexBuffer, (uint32_t), (override));
        MOCK_METHOD(void, uploadVertexBufferData, (DeviceResourceHandle, const Byte*, uint32_t), (override));
        MOCK_METHOD(void, deleteVertexBuffer, (DeviceResourceHandle), (override));
        MOCK_METHOD(DeviceResourceHandle, allocateVertexArray, (const VertexArrayInfo&), (override));
        MOCK_METHOD(void, activateVertexArray, (DeviceResourceHandle handle), (override));
        MOCK_METHOD(void, deleteVertexArray, (DeviceResourceHandle handle), (override));
        MOCK_METHOD(DeviceResourceHandle, allocateIndexBuffer, (EDataType, uint32_t), (override));
        MOCK_METHOD(void, uploadIndexBufferData, (DeviceResourceHandle, const Byte*, uint32_t), (override));
        MOCK_METHOD(void, deleteIndexBuffer, (DeviceResourceHandle), (override));

        MOCK_METHOD(std::unique_ptr<const GPUResource>, uploadShader, (const EffectResource&), (override));
        MOCK_METHOD(DeviceResourceHandle, registerShader, (std::unique_ptr<const GPUResource>), (override));
        MOCK_METHOD(DeviceResourceHandle, uploadBinaryShader, (const EffectResource&, const uint8_t* binaryShaderData, uint32_t binaryShaderDataSize, BinaryShaderFormatID binaryShaderFormat), (override));
        MOCK_METHOD(bool, getBinaryShader, (DeviceResourceHandle, UInt8Vector&, BinaryShaderFormatID&), (override));
        MOCK_METHOD(void, deleteShader, (DeviceResourceHandle), (override));
        MOCK_METHOD(void, activateShader, (DeviceResourceHandle), (override));

        MOCK_METHOD(DeviceResourceHandle, allocateTexture2D, (uint32_t width, uint32_t height, ETextureFormat textureFormat, const TextureSwizzleArray& swizzle, uint32_t mipLevelCount, uint32_t totalSizeInBytes), (override));
        MOCK_METHOD(DeviceResourceHandle, allocateTexture3D, (uint32_t width, uint32_t height, uint32_t depth, ETextureFormat textureFormat, uint32_t mipLevelCount, uint32_t totalSizeInBytes), (override));
        MOCK_METHOD(DeviceResourceHandle, allocateTextureCube, (uint32_t faceSize, ETextureFormat textureFormat, const TextureSwizzleArray& swizzle, uint32_t mipLevelCount, uint32_t totalSizeInBytes), (override));
        MOCK_METHOD(DeviceResourceHandle, allocateExternalTexture, (), (override));
        MOCK_METHOD(DeviceResourceHandle, getEmptyExternalTexture, (), (const, override));
        MOCK_METHOD(void, bindTexture, (DeviceResourceHandle handle), (override));
        MOCK_METHOD(void, generateMipmaps, (DeviceResourceHandle handle), (override));
        MOCK_METHOD(void, uploadTextureData, (DeviceResourceHandle handle, uint32_t mipLevel, uint32_t x, uint32_t y, uint32_t z, uint32_t width, uint32_t height, uint32_t depth, const Byte* data, uint32_t dataSize), (override));
        MOCK_METHOD(DeviceResourceHandle, uploadStreamTexture2D, (DeviceResourceHandle handle, uint32_t width, uint32_t height, ETextureFormat format, const uint8_t* data, const TextureSwizzleArray& swizzle), (override));
        MOCK_METHOD(void, deleteTexture, (DeviceResourceHandle), (override));
        MOCK_METHOD(void, activateTexture, (DeviceResourceHandle, DataFieldHandle), (override));
        MOCK_METHOD(DeviceResourceHandle, uploadRenderBuffer, (uint32_t, uint32_t, ERenderBufferType, ETextureFormat, ERenderBufferAccessMode, uint32_t), (override));
        MOCK_METHOD(void, deleteRenderBuffer, (DeviceResourceHandle), (override));
        MOCK_METHOD(DeviceResourceHandle, uploadDmaRenderBuffer, (uint32_t, uint32_t, DmaBufferFourccFormat, DmaBufferUsageFlags, DmaBufferModifiers), (override));
        MOCK_METHOD(int, getDmaRenderBufferFD, (DeviceResourceHandle handle), (override));
        MOCK_METHOD(uint32_t, getDmaRenderBufferStride, (DeviceResourceHandle handle), (override));
        MOCK_METHOD(void, destroyDmaRenderBuffer, (DeviceResourceHandle handle), (override));
        MOCK_METHOD(void, activateTextureSamplerObject, (const TextureSamplerStates&, DataFieldHandle), (override));
        MOCK_METHOD(DeviceResourceHandle, getFramebufferRenderTarget, (), (const, override));
        MOCK_METHOD(DeviceResourceHandle, uploadRenderTarget, (const DeviceHandleVector&), (override));
        MOCK_METHOD(void, activateRenderTarget, (DeviceResourceHandle), (override));
        MOCK_METHOD(void, deleteRenderTarget, (DeviceResourceHandle), (override));
        MOCK_METHOD(void, discardDepthStencil, (), (override));
        MOCK_METHOD(void, blitRenderTargets, (DeviceResourceHandle, DeviceResourceHandle, const PixelRectangle&, const PixelRectangle&, bool), (override));

        MOCK_METHOD(void, pairRenderTargetsForDoubleBuffering, ((const std::array<DeviceResourceHandle, 2>&), (const std::array<DeviceResourceHandle, 2>&)), (override));
        MOCK_METHOD(void, unpairRenderTargets, (DeviceResourceHandle), (override));
        MOCK_METHOD(void, swapDoubleBufferedRenderTarget, (DeviceResourceHandle), (override));

        MOCK_METHOD(void, readPixels, (uint8_t*, uint32_t, uint32_t, uint32_t, uint32_t), (override));

        MOCK_METHOD(uint32_t, getTotalGpuMemoryUsageInKB, (), (const, override));
        MOCK_METHOD(uint32_t, getAndResetDrawCallCount, (), (override));

        MOCK_METHOD(void, validateDeviceStatusHealthy, (), (const, override));
        MOCK_METHOD(bool, isDeviceStatusHealthy, (), (const, override));
        MOCK_METHOD(void, getSupportedBinaryProgramFormats, (std::vector<BinaryShaderFormatID>&), (const, override));
        MOCK_METHOD(bool, isExternalTextureExtensionSupported, (), (const, override));

        MOCK_METHOD(void, flush, (), (override));

        MOCK_METHOD(int, getTextureAddress, (DeviceResourceHandle), (const, override));
        MOCK_METHOD(uint32_t, getGPUHandle, (DeviceResourceHandle), (const, override));

        static const DeviceResourceHandle FakeShaderDeviceHandle                 ;
        static const DeviceResourceHandle FakeVertexBufferDeviceHandle           ;
        static const DeviceResourceHandle FakeIndexBufferDeviceHandle            ;
        static const DeviceResourceHandle FakeVertexArrayDeviceHandle            ;
        static const DeviceResourceHandle FakeTextureDeviceHandle                ;
        static const DeviceResourceHandle FakeExternalTextureDeviceHandle        ;
        static const DeviceResourceHandle FakeEmptyExternalTextureDeviceHandle   ;
        static const DeviceResourceHandle FakeFrameBufferRenderTargetDeviceHandle;
        static const DeviceResourceHandle FakeRenderTargetDeviceHandle           ;
        static const DeviceResourceHandle FakeRenderBufferDeviceHandle           ;
        static const DeviceResourceHandle FakeDmaRenderBufferDeviceHandle        ;
        static const DeviceResourceHandle FakeTextureSamplerDeviceHandle         ;
        static const DeviceResourceHandle FakeBlitPassRenderTargetDeviceHandle   ;
        static constexpr BinaryShaderFormatID FakeSupportedBinaryShaderFormat{ 63666u };
        static constexpr uint32_t FakeExternalTextureGlId{ 162023u };

    private:
        void createDefaultMockCalls();
    };
}


#endif
