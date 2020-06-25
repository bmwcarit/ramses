//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERRESOURCEMANAGER_H
#define RAMSES_RENDERERRESOURCEMANAGER_H

#include "IRendererResourceManager.h"
#include "RendererLib/RendererClientResourceRegistry.h"
#include "RendererLib/RendererSceneResourceRegistry.h"
#include "RendererLib/ClientResourceUploadingManager.h"
#include "RendererResourceManagerUtils.h"
#include "Collections/HashMap.h"
#include "Collections/Vector.h"
#include "Utils/MemoryPool.h"
#include "Components/ResourceRequesterID.h"

namespace ramses_internal
{
    class IResourceProvider;
    class IRenderBackend;
    class IEmbeddedCompositingManager;
    class IResourceUploader;
    class IRendererResourceCache;
    class FrameTimer;
    class RendererStatistics;

    class RendererResourceManager : public IRendererResourceManager
    {
    public:
        RendererResourceManager(
            IResourceProvider& resourceProvider,
            IResourceUploader& uploader,
            IRenderBackend& renderBackend,
            IEmbeddedCompositingManager& embeddedCompositingManager,
            ResourceRequesterID requesterId,
            Bool keepEffects,
            const FrameTimer& frameTimer,
            RendererStatistics& stats,
            UInt64 clientResourceCacheSize = 0u);
        virtual ~RendererResourceManager();

        // Client resources
        virtual void                 referenceClientResourcesForScene     (SceneId sceneId, const ResourceContentHashVector& resources) override;
        virtual void                 unreferenceClientResourcesForScene   (SceneId sceneId, const ResourceContentHashVector& resources) override;

        virtual void                 getRequestedResourcesAlreadyInCache(const IRendererResourceCache* cache) override;
        virtual void                 requestAndUnrequestPendingClientResources() override;
        virtual void                 processArrivedClientResources(IRendererResourceCache* cache) override;
        virtual Bool                 hasClientResourcesToBeUploaded() const override;
        virtual void                 uploadAndUnloadPendingClientResources() override;

        virtual DeviceResourceHandle getClientResourceDeviceHandle(const ResourceContentHash& hash) const override;
        virtual EResourceStatus      getClientResourceStatus(const ResourceContentHash& hash) const override;
        virtual EResourceType        getClientResourceType(const ResourceContentHash& hash) const override;

        // Scene resources
        virtual DeviceResourceHandle getRenderTargetDeviceHandle(RenderTargetHandle, SceneId sceneId) const override;
        virtual DeviceResourceHandle getRenderTargetBufferDeviceHandle(RenderBufferHandle bufferHandle, SceneId sceneId) const override;
        virtual void                 getBlitPassRenderTargetsDeviceHandle(BlitPassHandle blitPassHandle, SceneId sceneId, DeviceResourceHandle& srcRT, DeviceResourceHandle& dstRT) const override;

        virtual void                 uploadRenderTargetBuffer(RenderBufferHandle renderBufferHandle, SceneId sceneId, const RenderBuffer& renderBuffer) override;
        virtual void                 unloadRenderTargetBuffer(RenderBufferHandle renderBufferHandle, SceneId sceneId) override;

        virtual void                 uploadRenderTarget(RenderTargetHandle renderTarget, const RenderBufferHandleVector& rtBufferHandles, SceneId sceneId) override;
        virtual void                 unloadRenderTarget(RenderTargetHandle renderTarget, SceneId sceneId) override;

        virtual void                 uploadOffscreenBuffer(OffscreenBufferHandle bufferHandle, UInt32 width, UInt32 height, Bool isDoubleBuffered) override;
        virtual void                 unloadOffscreenBuffer(OffscreenBufferHandle bufferHandle) override;

        virtual void                 uploadTextureSampler(TextureSamplerHandle handle, SceneId sceneId, const TextureSamplerStates& states) override;
        virtual void                 unloadTextureSampler(TextureSamplerHandle handle, SceneId sceneId) override;
        virtual DeviceResourceHandle getTextureSamplerDeviceHandle(TextureSamplerHandle textureBufferHandle, SceneId sceneId) const override;

        virtual void                 uploadStreamTexture(StreamTextureHandle handle, StreamTextureSourceId source, SceneId sceneId) override;
        virtual void                 unloadStreamTexture(StreamTextureHandle handle, SceneId sceneId) override;

        virtual void                 uploadBlitPassRenderTargets(BlitPassHandle blitPass, RenderBufferHandle sourceRenderBuffer, RenderBufferHandle destinationRenderBuffer, SceneId sceneId) override;
        virtual void                 unloadBlitPassRenderTargets(BlitPassHandle blitPass, SceneId sceneId) override;

        virtual void                 uploadDataBuffer(DataBufferHandle dataBufferHandle, EDataBufferType dataBufferType, EDataType dataType, UInt32 dataSizeInBytes, SceneId sceneId) override;
        virtual void                 unloadDataBuffer(DataBufferHandle dataBufferHandle, SceneId sceneId) override;
        virtual void                 updateDataBuffer(DataBufferHandle handle, UInt32 dataSizeInBytes, const Byte* data, SceneId sceneId) override;
        virtual DeviceResourceHandle getDataBufferDeviceHandle(DataBufferHandle dataBufferHandle, SceneId sceneId) const override;

        virtual void                 uploadTextureBuffer(TextureBufferHandle textureBufferHandle, UInt32 width, UInt32 height, ETextureFormat textureFormat, UInt32 mipLevelCount, SceneId sceneId) override;
        virtual void                 unloadTextureBuffer(TextureBufferHandle textureBufferHandle, SceneId sceneId) override;
        virtual void                 updateTextureBuffer(TextureBufferHandle textureBufferHandle, UInt32 mipLevel, UInt32 x, UInt32 y, UInt32 width, UInt32 height, const Byte* data, SceneId sceneId) override;
        virtual DeviceResourceHandle getTextureBufferDeviceHandle(TextureBufferHandle textureBufferHandle, SceneId sceneId) const override;

        virtual void                 unloadAllSceneResourcesForScene(SceneId sceneId) override;
        virtual void                 unreferenceAllClientResourcesForScene(SceneId sceneId) override;

        // Renderer resources
        virtual DeviceResourceHandle getOffscreenBufferDeviceHandle(OffscreenBufferHandle bufferHandle) const override;
        virtual DeviceResourceHandle getOffscreenBufferColorBufferDeviceHandle(OffscreenBufferHandle bufferHandle) const override;
        virtual OffscreenBufferHandle getOffscreenBufferHandle(DeviceResourceHandle bufferDeviceHandle) const override;

        const ResourceRequesterID& getRequesterID() const;

    private:
        typedef HashMap<SceneId, ResourceContentHashVector> ResourcesPerSceneMap;
        typedef HashMap<SceneId, RendererSceneResourceRegistry> SceneResourceRegistryMap;

        void groupResourcesBySceneId(const ResourceContentHashVector& resources, ResourcesPerSceneMap& resourcesPerScene) const;
        void requestResourcesFromProvider(const ResourceContentHashVector& resources);
        RendererSceneResourceRegistry& getSceneResourceRegistry(SceneId sceneId);

        struct OffscreenBufferDescriptor
        {
            // Second render target and color buffer are only used for double-buffered offscreen buffers, otherwise just the first
            DeviceResourceHandle m_renderTargetHandle[2];
            DeviceResourceHandle m_colorBufferHandle[2];
            DeviceResourceHandle m_depthBufferHandle;
            UInt32 m_estimatedVRAMUsage;
        };
        using OffscreenBufferMap = MemoryPool<OffscreenBufferDescriptor, OffscreenBufferHandle>;

        const ResourceRequesterID m_id;

        IResourceProvider&             m_resourceProvider;
        IRenderBackend&                m_renderBackend;
        IEmbeddedCompositingManager&   m_embeddedCompositingManager;

        OffscreenBufferMap             m_offscreenBuffers;
        RendererClientResourceRegistry m_clientResourceRegistry;
        SceneResourceRegistryMap       m_sceneResourceRegistryMap;
        ClientResourceUploadingManager m_resourceUploadingManager;
        RendererStatistics&            m_stats;

        const UInt64 m_numberOfFramesToRerequestResource = 90u;  // 1.5s at 60fps, less than 2s force apply for remote content (assuming 30 flushes/s)
        UInt64 m_frameCounter = 0u;
        UInt64 m_numberOfArrivedResourcesInWrongStatus = 0u;
        UInt64 m_sizeOfArrivedResourcesInWrongStatus = 0u;

        friend class RendererLogger;
        // TODO Violin remove this after KPI monitor is reworked
        friend class GpuMemorySample;
    };
}

#endif
