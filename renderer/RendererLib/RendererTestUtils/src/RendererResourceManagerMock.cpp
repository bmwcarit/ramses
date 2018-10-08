//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererResourceManagerMock.h"
#include "DeviceMock.h"
#include "ResourceProviderMock.h"

namespace ramses_internal {
using namespace testing;

RendererResourceManagerMock::RendererResourceManagerMock()
{
    ON_CALL(*this, getClientResourceDeviceHandle(ResourceProviderMock::FakeEffectHash)).WillByDefault(Return(DeviceMock::FakeShaderDeviceHandle));
    ON_CALL(*this, getClientResourceDeviceHandle(ResourceProviderMock::FakeVertArrayHash)).WillByDefault(Return(DeviceMock::FakeVertexBufferDeviceHandle));
    ON_CALL(*this, getClientResourceDeviceHandle(ResourceProviderMock::FakeIndexArrayHash)).WillByDefault(Return(DeviceMock::FakeIndexBufferDeviceHandle));
    ON_CALL(*this, getClientResourceDeviceHandle(ResourceProviderMock::FakeTextureHash)).WillByDefault(Return(DeviceMock::FakeTextureDeviceHandle));
    ON_CALL(*this, getClientResourceDeviceHandle(ResourceProviderMock::FakeTextureHash2)).WillByDefault(Return(DeviceMock::FakeTextureDeviceHandle));

    ON_CALL(*this, getRenderTargetDeviceHandle(_, _)).WillByDefault(Return(DeviceMock::FakeRenderTargetDeviceHandle));
    ON_CALL(*this, getRenderTargetBufferDeviceHandle(_, _)).WillByDefault(Return(DeviceMock::FakeRenderBufferDeviceHandle));
    ON_CALL(*this, getOffscreenBufferDeviceHandle(_)).WillByDefault(Return(DeviceMock::FakeRenderTargetDeviceHandle));
    ON_CALL(*this, getOffscreenBufferColorBufferDeviceHandle(_)).WillByDefault(Return(DeviceMock::FakeRenderBufferDeviceHandle));

    ON_CALL(*this, getBlitPassRenderTargetsDeviceHandle(_, _, _, _)).WillByDefault(DoAll(SetArgReferee<2>(DeviceMock::FakeBlitPassRenderTargetDeviceHandle), SetArgReferee<3>(DeviceMock::FakeBlitPassRenderTargetDeviceHandle)));

    // no need to strictly test getters
    EXPECT_CALL(*this, getClientResourceDeviceHandle(_)).Times(AnyNumber());
    EXPECT_CALL(*this, getDataBufferDeviceHandle(_, _)).Times(AnyNumber());
    EXPECT_CALL(*this, getTextureBufferDeviceHandle(_, _)).Times(AnyNumber());
    EXPECT_CALL(*this, getRenderTargetDeviceHandle(_, _)).Times(AnyNumber());
    EXPECT_CALL(*this, getRenderTargetBufferDeviceHandle(_, _)).Times(AnyNumber());
    EXPECT_CALL(*this, getBlitPassRenderTargetsDeviceHandle(_, _, _, _)).Times(AnyNumber());
    EXPECT_CALL(*this, getOffscreenBufferColorBufferDeviceHandle(_)).Times(AnyNumber());
}
}
