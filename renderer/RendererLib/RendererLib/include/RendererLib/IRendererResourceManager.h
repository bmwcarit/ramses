//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_IRENDERERRESOURCEMANAGER_H
#define RAMSES_IRENDERERRESOURCEMANAGER_H

#include "IResourceDeviceHandleAccessor.h"
#include "RendererLib/EResourceStatus.h"
#include "SceneAPI/RenderBuffer.h"
#include "SceneAPI/SceneTypes.h"
#include "SceneAPI/TextureSamplerStates.h"
#include "SceneAPI/EDataType.h"
#include "Resource/ResourceTypes.h"

namespace ramses_internal
{
    struct RenderTarget;
    class IRendererResourceCache;
    enum class EDataBufferType : UInt8;

    class IRendererResourceManager : public IResourceDeviceHandleAccessor
    {
    public:
        // Client resources
        virtual EResourceStatus  getClientResourceStatus(const ResourceContentHash& hash) const = 0;
        virtual EResourceType    getClientResourceType(const ResourceContentHash& hash) const = 0;

        virtual void             referenceClientResourcesForScene     (SceneId sceneId, const ResourceContentHashVector& resources) = 0;
        virtual void             unreferenceClientResourcesForScene   (SceneId sceneId, const ResourceContentHashVector& resources) = 0;

        virtual void             getRequestedResourcesAlreadyInCache(const IRendererResourceCache* cache) = 0;
        virtual void             requestAndUnrequestPendingClientResources() = 0;
        virtual void             processArrivedClientResources(IRendererResourceCache* cache) = 0;
        virtual Bool             hasClientResourcesToBeUploaded() const = 0;
        virtual void             uploadAndUnloadPendingClientResources() = 0;

        // Scene resources
        virtual void             uploadRenderTargetBuffer(RenderBufferHandle renderBufferHandle, SceneId sceneId, const RenderBuffer& renderBuffer) = 0;
        virtual void             unloadRenderTargetBuffer(RenderBufferHandle renderBufferHandle, SceneId sceneId) = 0;
        virtual void             uploadRenderTarget(RenderTargetHandle renderTarget, const RenderBufferHandleVector& rtBufferHandles, SceneId sceneId) = 0;
        virtual void             unloadRenderTarget(RenderTargetHandle renderTarget, SceneId sceneId) = 0;

        virtual void             uploadTextureSampler(TextureSamplerHandle handle, SceneId sceneId, const TextureSamplerStates& states) = 0;
        virtual void             unloadTextureSampler(TextureSamplerHandle handle, SceneId sceneId) = 0;

        virtual void             uploadStreamTexture(StreamTextureHandle handle, StreamTextureSourceId source, SceneId sceneId) = 0;
        virtual void             unloadStreamTexture(StreamTextureHandle handle, SceneId sceneId) = 0;

        virtual void             uploadBlitPassRenderTargets(BlitPassHandle blitPass, RenderBufferHandle sourceRenderBuffer, RenderBufferHandle destinationRenderBuffer, SceneId sceneId) = 0;
        virtual void             unloadBlitPassRenderTargets(BlitPassHandle blitPass, SceneId sceneId) = 0;

        virtual void             uploadDataBuffer(DataBufferHandle dataBufferHandle, EDataBufferType dataBufferType, EDataType dataType, UInt32 dataSizeInBytes, SceneId sceneId) = 0;
        virtual void             unloadDataBuffer(DataBufferHandle dataBufferHandle, SceneId sceneId) = 0;
        virtual void             updateDataBuffer(DataBufferHandle handle, UInt32 dataSizeInBytes, const Byte* data, SceneId sceneId) = 0;

        virtual void             uploadTextureBuffer(TextureBufferHandle textureBufferHandle, UInt32 width, UInt32 height, ETextureFormat textureFormat, UInt32 mipLevelCount,  SceneId sceneId) = 0;
        virtual void             unloadTextureBuffer(TextureBufferHandle textureBufferHandle, SceneId sceneId) = 0;
        virtual void             updateTextureBuffer(TextureBufferHandle textureBufferHandle, UInt32 mipLevel, UInt32 x, UInt32 y, UInt32 width, UInt32 height, const Byte* data, SceneId sceneId) = 0;

        virtual void             unloadAllSceneResourcesForScene(SceneId sceneId) = 0;
        virtual void             unreferenceAllClientResourcesForScene(SceneId sceneId) = 0;

        // Renderer resources
        virtual void             uploadOffscreenBuffer(OffscreenBufferHandle bufferHandle, UInt32 width, UInt32 height, Bool isDoubleBuffered) = 0;
        virtual void             unloadOffscreenBuffer(OffscreenBufferHandle bufferHandle) = 0;
    };
}
#endif
