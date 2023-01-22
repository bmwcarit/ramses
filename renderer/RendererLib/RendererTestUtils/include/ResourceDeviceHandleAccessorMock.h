//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RESOURCEDEVICEHANDLEACCESSORMOCK_H
#define RAMSES_RESOURCEDEVICEHANDLEACCESSORMOCK_H

#include "renderer_common_gmock_header.h"
#include "gmock/gmock.h"
#include "RendererLib/IResourceDeviceHandleAccessor.h"

namespace ramses_internal
{

class ResourceDeviceHandleAccessorMock : public IResourceDeviceHandleAccessor
{
public:
    MOCK_METHOD(DeviceResourceHandle, getResourceDeviceHandle, (const ResourceContentHash& resourceHash), (const, override));
    MOCK_METHOD(DeviceResourceHandle, getRenderTargetDeviceHandle, (RenderTargetHandle targetHandle, SceneId sceneId), (const, override));
    MOCK_METHOD(DeviceResourceHandle, getRenderTargetBufferDeviceHandle, (RenderBufferHandle bufferHandle, SceneId sceneId), (const, override));
    MOCK_METHOD(void, getBlitPassRenderTargetsDeviceHandle, (BlitPassHandle blitPassHandle, SceneId sceneId, DeviceResourceHandle&, DeviceResourceHandle&), (const, override));
    MOCK_METHOD(DeviceResourceHandle, getOffscreenBufferDeviceHandle, (OffscreenBufferHandle bufferHandle), (const, override));
    MOCK_METHOD(DeviceResourceHandle, getOffscreenBufferColorBufferDeviceHandle, (OffscreenBufferHandle bufferHandle), (const, override));
    MOCK_METHOD(int, getDmaOffscreenBufferFD, (OffscreenBufferHandle), (const, override));
    MOCK_METHOD(UInt32, getDmaOffscreenBufferStride, (OffscreenBufferHandle), (const, override));
    MOCK_METHOD(OffscreenBufferHandle, getOffscreenBufferHandle, (DeviceResourceHandle bufferDeviceHandle), (const, override));
    MOCK_METHOD(DeviceResourceHandle, getStreamBufferDeviceHandle, (StreamBufferHandle bufferHandle), (const, override));
    MOCK_METHOD(DeviceResourceHandle, getDataBufferDeviceHandle, (DataBufferHandle dataBufferHandle, SceneId sceneId), (const, override));
    MOCK_METHOD(DeviceResourceHandle, getTextureBufferDeviceHandle, (TextureBufferHandle textureBufferHandle, SceneId sceneId), (const, override));
    MOCK_METHOD(DeviceResourceHandle, getVertexArrayDeviceHandle, (RenderableHandle renderableHandle, SceneId sceneId), (const, override));
    MOCK_METHOD(DeviceResourceHandle, getExternalBufferDeviceHandle, (ExternalBufferHandle), (const, override));
    MOCK_METHOD(DeviceResourceHandle, getEmptyExternalBufferDeviceHandle, (), (const, override));
    MOCK_METHOD(uint32_t, getExternalBufferGlId, (ExternalBufferHandle), (const, override));
};

}
#endif
