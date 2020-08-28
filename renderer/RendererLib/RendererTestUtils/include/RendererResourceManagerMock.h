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
#include "Components/ManagedResource.h"

namespace ramses_internal{

class RendererResourceManagerMock : public IRendererResourceManager
{
public:
    RendererResourceManagerMock();

    // IResourceDeviceHandleAccessor
    MOCK_METHOD(DeviceResourceHandle, getClientResourceDeviceHandle, (const ResourceContentHash&), (const, override));
    MOCK_METHOD(DeviceResourceHandle, getRenderTargetDeviceHandle, (RenderTargetHandle, SceneId), (const, override));
    MOCK_METHOD(DeviceResourceHandle, getRenderTargetBufferDeviceHandle, (RenderBufferHandle, SceneId), (const, override));
    MOCK_METHOD(DeviceResourceHandle, getOffscreenBufferDeviceHandle, (OffscreenBufferHandle), (const, override));
    MOCK_METHOD(OffscreenBufferHandle, getOffscreenBufferHandle, (DeviceResourceHandle), (const, override));
    MOCK_METHOD(DeviceResourceHandle, getOffscreenBufferColorBufferDeviceHandle, (OffscreenBufferHandle), (const, override));
    MOCK_METHOD(void, getBlitPassRenderTargetsDeviceHandle, (BlitPassHandle, SceneId, DeviceResourceHandle&, DeviceResourceHandle&), (const, override));
    MOCK_METHOD(DeviceResourceHandle, getDataBufferDeviceHandle, (DataBufferHandle, SceneId), (const, override));
    MOCK_METHOD(DeviceResourceHandle, getTextureBufferDeviceHandle, (TextureBufferHandle, SceneId), (const, override));
    MOCK_METHOD(DeviceResourceHandle, getTextureSamplerDeviceHandle, (TextureSamplerHandle, SceneId), (const, override));
    // IRendererResourceManager
    MOCK_METHOD(EResourceStatus, getClientResourceStatus, (const ResourceContentHash& hash), (const, override));
    MOCK_METHOD(EResourceType, getClientResourceType, (const ResourceContentHash& hash), (const, override));
    MOCK_METHOD(void, referenceClientResourcesForScene, (SceneId sceneId, const ResourceContentHashVector& resources), (override));
    MOCK_METHOD(void, unreferenceClientResourcesForScene, (SceneId sceneId, const ResourceContentHashVector& resources), (override));
    MOCK_METHOD(void, getRequestedResourcesAlreadyInCache, (const IRendererResourceCache* cache), (override));
    MOCK_METHOD(void, requestAndUnrequestPendingClientResources, (), (override));
    MOCK_METHOD(void, unloadAllSceneResourcesForScene, (SceneId sceneId), (override));
    MOCK_METHOD(void, unreferenceAllClientResourcesForScene, (SceneId sceneId), (override));
    MOCK_METHOD(void, processArrivedClientResources, (IRendererResourceCache* cache), (override));
    MOCK_METHOD(bool, hasClientResourcesToBeUploaded, (), (const, override));
    MOCK_METHOD(void, uploadAndUnloadPendingClientResources, (), (override));
    MOCK_METHOD(void, uploadRenderTargetBuffer, (RenderBufferHandle renderBufferHandle, SceneId sceneId, const RenderBuffer& renderBuffer), (override));
    MOCK_METHOD(void, unloadRenderTargetBuffer, (RenderBufferHandle renderBufferHandle, SceneId sceneId), (override));
    MOCK_METHOD(void, uploadRenderTarget, (RenderTargetHandle renderTarget, const RenderBufferHandleVector& rtBufferHandles, SceneId sceneId), (override));
    MOCK_METHOD(void, unloadRenderTarget, (RenderTargetHandle renderTarget, SceneId sceneId), (override));
    MOCK_METHOD(void, uploadOffscreenBuffer, (OffscreenBufferHandle bufferHandle, UInt32 width, UInt32 height, bool isDoubleBuffered), (override));
    MOCK_METHOD(void, unloadOffscreenBuffer, (OffscreenBufferHandle bufferHandle), (override));
    MOCK_METHOD(void, uploadTextureSampler, (TextureSamplerHandle bufferHandle, SceneId sceneId, const TextureSamplerStates& states), (override));
    MOCK_METHOD(void, unloadTextureSampler, (TextureSamplerHandle bufferHandle, SceneId sceneId), (override));
    MOCK_METHOD(void, uploadStreamTexture, (StreamTextureHandle bufferHandle, StreamTextureSourceId source, SceneId sceneId), (override));
    MOCK_METHOD(void, unloadStreamTexture, (StreamTextureHandle bufferHandle, SceneId sceneId), (override));
    MOCK_METHOD(void, uploadBlitPassRenderTargets, (BlitPassHandle, RenderBufferHandle, RenderBufferHandle, SceneId), (override));
    MOCK_METHOD(void, unloadBlitPassRenderTargets, (BlitPassHandle, SceneId), (override));
    MOCK_METHOD(void, uploadDataBuffer, (DataBufferHandle dataBufferHandle, EDataBufferType dataBufferType, EDataType dataType, UInt32 elementCount, SceneId sceneId), (override));
    MOCK_METHOD(void, unloadDataBuffer, (DataBufferHandle dataBufferHandle, SceneId sceneId), (override));
    MOCK_METHOD(void, updateDataBuffer, (DataBufferHandle handle, UInt32 dataSizeInBytes, const Byte* data, SceneId sceneId), (override));

    MOCK_METHOD(void, uploadTextureBuffer, (TextureBufferHandle textureBufferHandle, UInt32 width, UInt32 height, ETextureFormat textureFormat, UInt32 mipLevelCount, SceneId sceneId), (override));
    MOCK_METHOD(void, unloadTextureBuffer, (TextureBufferHandle textureBufferHandle, SceneId sceneId), (override));
    MOCK_METHOD(void, updateTextureBuffer, (TextureBufferHandle textureBufferHandle, UInt32 mipLevel, UInt32 x, UInt32 y, UInt32 width, UInt32 height, const Byte* data, SceneId sceneId), (override));
};
}
#endif
