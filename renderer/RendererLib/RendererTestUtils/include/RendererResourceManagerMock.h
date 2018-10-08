//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERRESOURCEMANAGERMOCK_H
#define RAMSES_RENDERERRESOURCEMANAGERMOCK_H

#include "renderer_common_gmock_header.h"
#include "gmock/gmock.h"
#include "SceneAPI/SceneId.h"
#include "SceneAPI/ResourceContentHash.h"
#include "RendererLib/IRendererResourceManager.h"
#include "RendererLib/RendererLogContext.h"
#include "RendererLib/EResourceStatus.h"

namespace ramses_internal{

class RendererResourceManagerMock : public IRendererResourceManager
{
public:
    RendererResourceManagerMock();

    // IResourceDeviceHandleAccessor
    MOCK_CONST_METHOD1(getClientResourceDeviceHandle, DeviceResourceHandle(const ResourceContentHash&));
    MOCK_CONST_METHOD2(getRenderTargetDeviceHandle, DeviceResourceHandle(RenderTargetHandle, SceneId));
    MOCK_CONST_METHOD2(getRenderTargetBufferDeviceHandle, DeviceResourceHandle(RenderBufferHandle, SceneId));
    MOCK_CONST_METHOD1(getOffscreenBufferDeviceHandle, DeviceResourceHandle(OffscreenBufferHandle));
    MOCK_CONST_METHOD1(getOffscreenBufferHandle, OffscreenBufferHandle(DeviceResourceHandle));
    MOCK_CONST_METHOD1(getOffscreenBufferColorBufferDeviceHandle, DeviceResourceHandle(OffscreenBufferHandle));
    MOCK_CONST_METHOD4(getBlitPassRenderTargetsDeviceHandle, void(BlitPassHandle, SceneId, DeviceResourceHandle&, DeviceResourceHandle&));
    MOCK_CONST_METHOD2(getDataBufferDeviceHandle, DeviceResourceHandle(DataBufferHandle, SceneId));
    MOCK_CONST_METHOD2(getTextureBufferDeviceHandle, DeviceResourceHandle(TextureBufferHandle, SceneId));
    MOCK_CONST_METHOD2(getTextureSamplerDeviceHandle, DeviceResourceHandle(TextureSamplerHandle, SceneId));

    // IRendererResourceManager
    MOCK_CONST_METHOD1(getClientResourceStatus, EResourceStatus(const ResourceContentHash& hash));
    MOCK_CONST_METHOD1(getClientResourceType, EResourceType(const ResourceContentHash& hash));
    MOCK_METHOD2(referenceClientResourcesForScene, void(SceneId sceneId, const ResourceContentHashVector& resources));
    MOCK_METHOD2(unreferenceClientResourcesForScene, void(SceneId sceneId, const ResourceContentHashVector& resources));
    MOCK_METHOD1(getRequestedResourcesAlreadyInCache, void(const IRendererResourceCache* cache));
    MOCK_METHOD0(requestAndUnrequestPendingClientResources, void());
    MOCK_METHOD1(unloadAllSceneResourcesForScene, void(SceneId sceneId));
    MOCK_METHOD1(unreferenceAllClientResourcesForScene, void(SceneId sceneId));
    MOCK_METHOD1(processArrivedClientResources, void(IRendererResourceCache* cache));
    MOCK_CONST_METHOD0(hasClientResourcesToBeUploaded, bool());
    MOCK_METHOD0(uploadAndUnloadPendingClientResources, void());
    MOCK_CONST_METHOD1(logResources, void(RendererLogContext& context));
    MOCK_METHOD3(uploadRenderTargetBuffer, void(RenderBufferHandle renderBufferHandle, SceneId sceneId, const RenderBuffer& renderBuffer));
    MOCK_METHOD2(unloadRenderTargetBuffer, void(RenderBufferHandle renderBufferHandle, SceneId sceneId));
    MOCK_METHOD3(uploadRenderTarget, void(RenderTargetHandle renderTarget, const RenderBufferHandleVector& rtBufferHandles, SceneId sceneId));
    MOCK_METHOD2(unloadRenderTarget, void(RenderTargetHandle renderTarget, SceneId sceneId));
    MOCK_METHOD4(uploadOffscreenBuffer, void(OffscreenBufferHandle bufferHandle, UInt32 width, UInt32 height, bool isDoubleBuffered));
    MOCK_METHOD1(unloadOffscreenBuffer, void(OffscreenBufferHandle bufferHandle));
    MOCK_METHOD3(uploadTextureSampler, void(TextureSamplerHandle bufferHandle, SceneId sceneId, const TextureSamplerStates& states));
    MOCK_METHOD2(unloadTextureSampler, void(TextureSamplerHandle bufferHandle, SceneId sceneId));
    MOCK_METHOD3(uploadStreamTexture, void(StreamTextureHandle bufferHandle, StreamTextureSourceId source, SceneId sceneId));
    MOCK_METHOD2(unloadStreamTexture, void(StreamTextureHandle bufferHandle, SceneId sceneId));
    MOCK_METHOD4(uploadBlitPassRenderTargets, void(BlitPassHandle, RenderBufferHandle, RenderBufferHandle, SceneId));
    MOCK_METHOD2(unloadBlitPassRenderTargets, void(BlitPassHandle, SceneId));
    MOCK_METHOD5(uploadDataBuffer, void(DataBufferHandle dataBufferHandle, EDataBufferType dataBufferType, EDataType dataType, UInt32 elementCount, SceneId sceneId));
    MOCK_METHOD2(unloadDataBuffer, void(DataBufferHandle dataBufferHandle, SceneId sceneId));
    MOCK_METHOD4(updateDataBuffer, void(DataBufferHandle handle, UInt32 dataSizeInBytes, const Byte* data, SceneId sceneId));

    MOCK_METHOD6(uploadTextureBuffer, void(TextureBufferHandle textureBufferHandle, UInt32 width, UInt32 height, ETextureFormat textureFormat, UInt32 mipLevelCount, SceneId sceneId));
    MOCK_METHOD2(unloadTextureBuffer, void(TextureBufferHandle textureBufferHandle, SceneId sceneId));
    MOCK_METHOD8(updateTextureBuffer, void(TextureBufferHandle textureBufferHandle, UInt32 mipLevel, UInt32 x, UInt32 y, UInt32 width, UInt32 height, const Byte* data, SceneId sceneId));
};
}
#endif
