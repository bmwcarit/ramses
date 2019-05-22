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
    const DeviceResourceHandle DeviceMock::FakeTextureDeviceHandle(4444u);
    const DeviceResourceHandle DeviceMock::FakeFrameBufferRenderTargetDeviceHandle(5555u);
    const DeviceResourceHandle DeviceMock::FakeRenderTargetDeviceHandle(6666u);
    const DeviceResourceHandle DeviceMock::FakeRenderBufferDeviceHandle(7777u);
    const DeviceResourceHandle DeviceMock::FakeTextureSamplerDeviceHandle(8888u);
    const DeviceResourceHandle DeviceMock::FakeBlitPassRenderTargetDeviceHandle(9999u);

    DeviceMock::DeviceMock()
    {
        createDefaultMockCalls();
    }

    DeviceMock::DeviceMock(IContext&)
    {
        createDefaultMockCalls();
    }

    DeviceMock::~DeviceMock()
    {
    }

    void DeviceMock::createDefaultMockCalls()
    {

        ON_CALL(*this, init()).WillByDefault(Return(true));
        ON_CALL(*this, getBinaryShader(_, _, _)).WillByDefault(Return(true));

        ON_CALL(*this, isDeviceStatusHealthy()).WillByDefault(Return(true));

        EXPECT_CALL(*this, getFramebufferRenderTarget()).Times(AnyNumber());
        EXPECT_CALL(*this, getTextureAddress(_)).Times(AnyNumber());

        // fake uploads
        ON_CALL(*this, allocateVertexBuffer(_, _)).WillByDefault(Return(FakeVertexBufferDeviceHandle));
        ON_CALL(*this, allocateIndexBuffer(_, _)).WillByDefault(Return(FakeIndexBufferDeviceHandle));
        ON_CALL(*this, uploadShader(_)).WillByDefault(Return(FakeShaderDeviceHandle));
        ON_CALL(*this, uploadBinaryShader(_, _, _, _)).WillByDefault(Return(FakeShaderDeviceHandle));
        ON_CALL(*this, allocateTexture2D(_, _, _, _, _)).WillByDefault(Return(FakeTextureDeviceHandle));
        ON_CALL(*this, uploadRenderBuffer(_)).WillByDefault(Return(FakeRenderBufferDeviceHandle));
        ON_CALL(*this, uploadTextureSampler(_,_,_,_,_,_)).WillByDefault(Return(FakeTextureSamplerDeviceHandle));
        ON_CALL(*this, uploadRenderTarget(_)).WillByDefault(Return(FakeRenderTargetDeviceHandle));
        ON_CALL(*this, getFramebufferRenderTarget()).WillByDefault(Return(FakeFrameBufferRenderTargetDeviceHandle));
    }

    DeviceMockWithDestructor::DeviceMockWithDestructor()
    {
    }

    DeviceMockWithDestructor::~DeviceMockWithDestructor()
    {
        Die();
    }
}
