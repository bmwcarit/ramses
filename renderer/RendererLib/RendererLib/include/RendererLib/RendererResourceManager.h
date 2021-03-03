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
#include "RendererLib/RendererResourceRegistry.h"
#include "RendererLib/RendererSceneResourceRegistry.h"
#include "RendererLib/ResourceUploadingManager.h"
#include "RendererResourceManagerUtils.h"
#include "Collections/HashMap.h"
#include "Collections/Vector.h"
#include "Utils/MemoryPool.h"

namespace ramses_internal
{
    class IRenderBackend;
    class AsyncEffectUploader;
    class IEmbeddedCompositingManager;
    class IRendererResourceCache;
    class FrameTimer;
    class RendererStatistics;
    class IBinaryShaderCache;
    class IResourceUploader;

    class RendererResourceManager : public IRendererResourceManager
    {
    public:
        RendererResourceManager(
            IRenderBackend& renderBackend,
            std::unique_ptr<IResourceUploader> resourceUploader,
            AsyncEffectUploader& asyncEffectUploader,
            IEmbeddedCompositingManager& embeddedCompositingManager,
            Bool keepEffects,
            const FrameTimer& frameTimer,
            RendererStatistics& stats,
            UInt64 gpuCacheSize = 0u);
        virtual ~RendererResourceManager() override;

        // Immutable resources
        virtual void                 referenceResourcesForScene     (SceneId sceneId, const ResourceContentHashVector& resources) override;
        virtual void                 unreferenceResourcesForScene   (SceneId sceneId, const ResourceContentHashVector& resources) override;

        virtual void                 provideResourceData(const ManagedResource& mr) override;
        virtual Bool                 hasResourcesToBeUploaded() const override;
        virtual void                 uploadAndUnloadPendingResources() override;

        virtual DeviceResourceHandle getResourceDeviceHandle(const ResourceContentHash& hash) const override;
        virtual EResourceStatus      getResourceStatus(const ResourceContentHash& hash) const override;
        virtual EResourceType        getResourceType(const ResourceContentHash& hash) const override;

        // Scene resources
        virtual DeviceResourceHandle getRenderTargetDeviceHandle(RenderTargetHandle, SceneId sceneId) const override;
        virtual DeviceResourceHandle getRenderTargetBufferDeviceHandle(RenderBufferHandle bufferHandle, SceneId sceneId) const override;
        virtual void                 getBlitPassRenderTargetsDeviceHandle(BlitPassHandle blitPassHandle, SceneId sceneId, DeviceResourceHandle& srcRT, DeviceResourceHandle& dstRT) const override;

        virtual void                 uploadRenderTargetBuffer(RenderBufferHandle renderBufferHandle, SceneId sceneId, const RenderBuffer& renderBuffer) override;
        virtual void                 unloadRenderTargetBuffer(RenderBufferHandle renderBufferHandle, SceneId sceneId) override;

        virtual void                 uploadRenderTarget(RenderTargetHandle renderTarget, const RenderBufferHandleVector& rtBufferHandles, SceneId sceneId) override;
        virtual void                 unloadRenderTarget(RenderTargetHandle renderTarget, SceneId sceneId) override;

        virtual void                 uploadOffscreenBuffer(OffscreenBufferHandle bufferHandle, UInt32 width, UInt32 height, UInt32 sampleCount, Bool isDoubleBuffered, ERenderBufferType depthStencilBufferType) override;
        virtual void                 unloadOffscreenBuffer(OffscreenBufferHandle bufferHandle) override;

        virtual void                 uploadStreamBuffer(StreamBufferHandle bufferHandle, WaylandIviSurfaceId source) override;
        virtual void                 unloadStreamBuffer(StreamBufferHandle bufferHandle) override;

        virtual void                 uploadStreamTexture(StreamTextureHandle handle, WaylandIviSurfaceId source, SceneId sceneId) override;
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
        virtual void                 unreferenceAllResourcesForScene(SceneId sceneId) override;
        virtual const ResourceContentHashVector* getResourcesInUseByScene(SceneId sceneId) const override;

        // Renderer resources
        virtual DeviceResourceHandle getOffscreenBufferDeviceHandle(OffscreenBufferHandle bufferHandle) const override;
        virtual DeviceResourceHandle getOffscreenBufferColorBufferDeviceHandle(OffscreenBufferHandle bufferHandle) const override;
        virtual OffscreenBufferHandle getOffscreenBufferHandle(DeviceResourceHandle bufferDeviceHandle) const override;
        virtual DeviceResourceHandle getStreamBufferDeviceHandle(StreamBufferHandle bufferHandle) const override;

        virtual const StreamUsage& getStreamUsage(WaylandIviSurfaceId source) const override;

        const RendererResourceRegistry& getRendererResourceRegistry() const;

    private:
        using SceneResourceRegistryMap = HashMap<SceneId, RendererSceneResourceRegistry>;

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
        using StreamBufferMap = MemoryPool<WaylandIviSurfaceId, StreamBufferHandle>;

        IRenderBackend&                m_renderBackend;
        IEmbeddedCompositingManager&   m_embeddedCompositingManager;

        OffscreenBufferMap             m_offscreenBuffers;
        StreamBufferMap                m_streamBuffers;
        RendererResourceRegistry       m_resourceRegistry;
        SceneResourceRegistryMap       m_sceneResourceRegistryMap;
        ResourceUploadingManager       m_resourceUploadingManager;
        RendererStatistics&            m_stats;

        std::unordered_map<WaylandIviSurfaceId, StreamUsage> m_streamUsages;

        friend class RendererLogger;
        // TODO Violin remove this after KPI monitor is reworked
        friend class GpuMemorySample;
    };
}

#endif
