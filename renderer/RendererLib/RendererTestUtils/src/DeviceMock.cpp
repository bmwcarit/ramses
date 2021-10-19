//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "DeviceMock.h"

using namespace testing;

namespace ramses_internal
{
    const DeviceResourceHandle DeviceMock::FakeShaderDeviceHandle(1111u);
    const DeviceResourceHandle DeviceMock::FakeVertexBufferDeviceHandle(2222u);
    const DeviceResourceHandle DeviceMock::FakeIndexBufferDeviceHandle(3333u);
    const DeviceResourceHandle DeviceMock::FakeVertexArrayDeviceHandle(3334u);
    const DeviceResourceHandle DeviceMock::FakeTextureDeviceHandle(4444u);
    const DeviceResourceHandle DeviceMock::FakeFrameBufferRenderTargetDeviceHandle(5555u);
    const DeviceResourceHandle DeviceMock::FakeRenderTargetDeviceHandle(6666u);
    const DeviceResourceHandle DeviceMock::FakeRenderBufferDeviceHandle(7777u);
    const DeviceResourceHandle DeviceMock::FakeDmaRenderBufferDeviceHandle(7778u);
    const DeviceResourceHandle DeviceMock::FakeTextureSamplerDeviceHandle(8888u);
    const DeviceResourceHandle DeviceMock::FakeBlitPassRenderTargetDeviceHandle(9999u);
    constexpr BinaryShaderFormatID DeviceMock::FakeSupportedBinaryShaderFormat;

    DeviceMock::DeviceMock()
    {
        createDefaultMockCalls();
    }

    DeviceMock::~DeviceMock() = default;

    void DeviceMock::createDefaultMockCalls()
    {

        ON_CALL(*this, init()).WillByDefault(Return(true));
        ON_CALL(*this, getBinaryShader(_, _, _)).WillByDefault(Return(true));

        ON_CALL(*this, isDeviceStatusHealthy()).WillByDefault(Return(true));

        EXPECT_CALL(*this, getFramebufferRenderTarget()).Times(AnyNumber());
        EXPECT_CALL(*this, getTextureAddress(_)).Times(AnyNumber());

        // fake uploads
        ON_CALL(*this, allocateVertexBuffer(_)).WillByDefault(Return(FakeVertexBufferDeviceHandle));
        ON_CALL(*this, allocateIndexBuffer(_, _)).WillByDefault(Return(FakeIndexBufferDeviceHandle));
        ON_CALL(*this, allocateVertexArray(_)).WillByDefault(Return(FakeVertexArrayDeviceHandle));
        ON_CALL(*this, uploadShader(_)).WillByDefault(Invoke([](const auto&){return std::make_unique<const GPUResource>(1u, 2u);}));
        ON_CALL(*this, registerShader(_)).WillByDefault(Return(FakeShaderDeviceHandle));
        ON_CALL(*this, uploadBinaryShader(_, _, _, _)).WillByDefault(Return(FakeShaderDeviceHandle));
        ON_CALL(*this, allocateTexture2D(_, _, _, _, _, _)).WillByDefault(Return(FakeTextureDeviceHandle));
        ON_CALL(*this, uploadRenderBuffer(_, _, _, _, _, _)).WillByDefault(Return(FakeRenderBufferDeviceHandle));
        ON_CALL(*this, uploadDmaRenderBuffer(_, _, _, _, _)).WillByDefault(Return(FakeDmaRenderBufferDeviceHandle));
        ON_CALL(*this, uploadRenderTarget(_)).WillByDefault(Return(FakeRenderTargetDeviceHandle));
        ON_CALL(*this, getFramebufferRenderTarget()).WillByDefault(Return(FakeFrameBufferRenderTargetDeviceHandle));

        EXPECT_CALL(*this, getSupportedBinaryProgramFormats(_)).Times(AnyNumber());
        ON_CALL(*this, getSupportedBinaryProgramFormats(_)).WillByDefault(Invoke([](auto& formats) { formats = { FakeSupportedBinaryShaderFormat }; }));
    }
}
