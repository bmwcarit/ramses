//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/IResourceDeviceHandleAccessor.h"
#include "gmock/gmock.h"

namespace ramses::internal
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
        MOCK_METHOD(uint32_t, getDmaOffscreenBufferStride, (OffscreenBufferHandle), (const, override));
        MOCK_METHOD(OffscreenBufferHandle, getOffscreenBufferHandle, (DeviceResourceHandle bufferDeviceHandle), (const, override));
        MOCK_METHOD(DeviceResourceHandle, getStreamBufferDeviceHandle, (StreamBufferHandle bufferHandle), (const, override));
        MOCK_METHOD(DeviceResourceHandle, getDataBufferDeviceHandle, (DataBufferHandle dataBufferHandle, SceneId sceneId), (const, override));
        MOCK_METHOD(DeviceResourceHandle, getTextureBufferDeviceHandle, (TextureBufferHandle textureBufferHandle, SceneId sceneId), (const, override));
        MOCK_METHOD(DeviceResourceHandle, getVertexArrayDeviceHandle, (RenderableHandle renderableHandle, SceneId sceneId), (const, override));
        MOCK_METHOD(DeviceResourceHandle, getExternalBufferDeviceHandle, (ExternalBufferHandle), (const, override));
        MOCK_METHOD(DeviceResourceHandle, getEmptyExternalBufferDeviceHandle, (), (const, override));
        MOCK_METHOD(uint32_t, getExternalBufferGlId, (ExternalBufferHandle), (const, override));
        MOCK_METHOD(DeviceResourceHandle, getUniformBufferDeviceHandle, (UniformBufferHandle uniformBufferHandle, SceneId sceneId), (const, override));
        MOCK_METHOD(DeviceResourceHandle, getUniformBufferDeviceHandle, (SemanticUniformBufferHandle handle, SceneId sceneId), (const, override));
    };
}

