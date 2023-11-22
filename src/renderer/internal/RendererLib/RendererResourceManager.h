//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "IRendererResourceManager.h"
#include "internal/RendererLib/RendererResourceRegistry.h"
#include "internal/RendererLib/RendererSceneResourceRegistry.h"
#include "internal/RendererLib/ResourceUploadingManager.h"
#include "internal/PlatformAbstraction/Collections/HashMap.h"
#include "internal/PlatformAbstraction/Collections/Vector.h"
#include "internal/Core/Utils/MemoryPool.h"
#include <array>

namespace ramses::internal
{
    class IRenderBackend;
    class AsyncEffectUploader;
    class IEmbeddedCompositingManager;
    class FrameTimer;
    class RendererStatistics;
    class IBinaryShaderCache;
    class IResourceUploader;
    class DisplayConfig;

    class RendererResourceManager final : public IRendererResourceManager
    {
    public:
        RendererResourceManager(
            IRenderBackend& renderBackend,
            std::unique_ptr<IResourceUploader> resourceUploader,
            AsyncEffectUploader& asyncEffectUploader,
            IEmbeddedCompositingManager& embeddedCompositingManager,
            const DisplayConfig& displayConfig,
            const FrameTimer& frameTimer,
            RendererStatistics& stats);
        ~RendererResourceManager() override;

        // Immutable resources
        void                 referenceResourcesForScene     (SceneId sceneId, const ResourceContentHashVector& resources) override;
        void                 unreferenceResourcesForScene   (SceneId sceneId, const ResourceContentHashVector& resources) override;

        void                 provideResourceData(const ManagedResource& mr) override;
        [[nodiscard]] bool   hasResourcesToBeUploaded() const override;
        void                 uploadAndUnloadPendingResources() override;

        [[nodiscard]] DeviceResourceHandle getResourceDeviceHandle(const ResourceContentHash& hash) const override;
        [[nodiscard]] EResourceStatus      getResourceStatus(const ResourceContentHash& hash) const override;
        [[nodiscard]] EResourceType        getResourceType(const ResourceContentHash& hash) const override;

        // Scene resources
        [[nodiscard]] DeviceResourceHandle getRenderTargetDeviceHandle(RenderTargetHandle targetHandle, SceneId sceneId) const override;
        [[nodiscard]] DeviceResourceHandle getRenderTargetBufferDeviceHandle(RenderBufferHandle bufferHandle, SceneId sceneId) const override;
        void                 getBlitPassRenderTargetsDeviceHandle(BlitPassHandle blitPassHandle, SceneId sceneId, DeviceResourceHandle& srcRT, DeviceResourceHandle& dstRT) const override;

        void                 uploadRenderTargetBuffer(RenderBufferHandle renderBufferHandle, SceneId sceneId, const RenderBuffer& renderBuffer) override;
        void                 unloadRenderTargetBuffer(RenderBufferHandle renderBufferHandle, SceneId sceneId) override;
        void                 updateRenderTargetBufferProperties(RenderBufferHandle renderBufferHandle, SceneId sceneId, const RenderBuffer& renderBuffer) override;

        void                 uploadRenderTarget(RenderTargetHandle renderTarget, const RenderBufferHandleVector& rtBufferHandles, SceneId sceneId) override;
        void                 unloadRenderTarget(RenderTargetHandle renderTarget, SceneId sceneId) override;

        void                 uploadOffscreenBuffer(OffscreenBufferHandle bufferHandle, uint32_t width, uint32_t height, uint32_t sampleCount, bool isDoubleBuffered, EDepthBufferType depthStencilBufferType) override;
        void                 uploadDmaOffscreenBuffer(OffscreenBufferHandle bufferHandle, uint32_t width, uint32_t height, DmaBufferFourccFormat dmaBufferFourccFormat, DmaBufferUsageFlags dmaBufferUsageFlags, DmaBufferModifiers dmaBufferModifiers) override;
        void                 unloadOffscreenBuffer(OffscreenBufferHandle bufferHandle) override;

        void                 uploadStreamBuffer(StreamBufferHandle bufferHandle, WaylandIviSurfaceId source) override;
        void                 unloadStreamBuffer(StreamBufferHandle bufferHandle) override;

        void                 uploadExternalBuffer(ExternalBufferHandle bufferHandle) override;
        void                 unloadExternalBuffer(ExternalBufferHandle bufferHandle) override;

        void                 uploadBlitPassRenderTargets(BlitPassHandle blitPass, RenderBufferHandle sourceRenderBuffer, RenderBufferHandle destinationRenderBuffer, SceneId sceneId) override;
        void                 unloadBlitPassRenderTargets(BlitPassHandle blitPass, SceneId sceneId) override;

        void                 uploadDataBuffer(DataBufferHandle dataBufferHandle, EDataBufferType dataBufferType, EDataType dataType, uint32_t dataSizeInBytes, SceneId sceneId) override;
        void                 unloadDataBuffer(DataBufferHandle dataBufferHandle, SceneId sceneId) override;
        void                 updateDataBuffer(DataBufferHandle handle, uint32_t dataSizeInBytes, const std::byte* data, SceneId sceneId) override;
        [[nodiscard]] DeviceResourceHandle getDataBufferDeviceHandle(DataBufferHandle dataBufferHandle, SceneId sceneId) const override;

        void                 uploadTextureBuffer(TextureBufferHandle textureBufferHandle, uint32_t width, uint32_t height, EPixelStorageFormat textureFormat, uint32_t mipLevelCount, SceneId sceneId) override;
        void                 unloadTextureBuffer(TextureBufferHandle textureBufferHandle, SceneId sceneId) override;
        void                 updateTextureBuffer(TextureBufferHandle textureBufferHandle, uint32_t mipLevel, const Quad& area, uint32_t stride, const std::byte* data, SceneId sceneId) override;
        [[nodiscard]] DeviceResourceHandle getTextureBufferDeviceHandle(TextureBufferHandle textureBufferHandle, SceneId sceneId) const override;

        void                 uploadVertexArray(RenderableHandle renderableHandle, const VertexArrayInfo& vertexArrayInfo, SceneId sceneId) override;
        void                 unloadVertexArray(RenderableHandle renderableHandle, SceneId sceneId) override;
        [[nodiscard]] DeviceResourceHandle getVertexArrayDeviceHandle(RenderableHandle renderableHandle, SceneId sceneId) const override;

        void                 unloadAllSceneResourcesForScene(SceneId sceneId) override;
        void                 unreferenceAllResourcesForScene(SceneId sceneId) override;
        [[nodiscard]] const ResourceContentHashVector* getResourcesInUseByScene(SceneId sceneId) const override;

        // Renderer resources
        [[nodiscard]] DeviceResourceHandle getOffscreenBufferDeviceHandle(OffscreenBufferHandle bufferHandle) const override;
        [[nodiscard]] DeviceResourceHandle getOffscreenBufferColorBufferDeviceHandle(OffscreenBufferHandle bufferHandle) const override;
        [[nodiscard]] int getDmaOffscreenBufferFD(OffscreenBufferHandle bufferHandle) const override;
        [[nodiscard]] uint32_t getDmaOffscreenBufferStride(OffscreenBufferHandle bufferHandle) const override;
        [[nodiscard]] OffscreenBufferHandle getOffscreenBufferHandle(DeviceResourceHandle bufferDeviceHandle) const override;
        [[nodiscard]] DeviceResourceHandle getStreamBufferDeviceHandle(StreamBufferHandle bufferHandle) const override;
        [[nodiscard]] DeviceResourceHandle getExternalBufferDeviceHandle(ExternalBufferHandle bufferHandle) const override;
        [[nodiscard]] DeviceResourceHandle getEmptyExternalBufferDeviceHandle() const override;
        [[nodiscard]] uint32_t getExternalBufferGlId(ExternalBufferHandle bufferHandle) const override;

        [[nodiscard]] const StreamUsage& getStreamUsage(WaylandIviSurfaceId source) const override;

        [[nodiscard]] const RendererResourceRegistry& getRendererResourceRegistry() const;

    private:
        using SceneResourceRegistryMap = HashMap<SceneId, RendererSceneResourceRegistry>;

        RendererSceneResourceRegistry& getSceneResourceRegistry(SceneId sceneId);
        void clearRenderTarget(DeviceResourceHandle handle);

        struct OffscreenBufferDescriptor
        {
            // Second render target and color buffer are only used for double-buffered offscreen buffers, otherwise just the first
            std::array<DeviceResourceHandle, 2> m_renderTargetHandle{};
            std::array<DeviceResourceHandle, 2> m_colorBufferHandle{};
            DeviceResourceHandle m_depthBufferHandle;
            uint32_t m_estimatedVRAMUsage = 0;
            bool isDmaBuffer = false;
        };
        using OffscreenBufferMap = MemoryPool<OffscreenBufferDescriptor, OffscreenBufferHandle>;
        using StreamBufferMap = MemoryPool<WaylandIviSurfaceId, StreamBufferHandle>;
        using ExternalBufferMap = MemoryPool<DeviceResourceHandle, ExternalBufferHandle>;

        IRenderBackend&                m_renderBackend;
        IEmbeddedCompositingManager&   m_embeddedCompositingManager;

        OffscreenBufferMap             m_offscreenBuffers;
        StreamBufferMap                m_streamBuffers;
        ExternalBufferMap              m_externalBuffers;
        RendererResourceRegistry       m_resourceRegistry;
        SceneResourceRegistryMap       m_sceneResourceRegistryMap;
        ResourceUploadingManager       m_resourceUploadingManager;
        RendererStatistics&            m_stats;

        std::unordered_map<WaylandIviSurfaceId, StreamUsage> m_streamUsages;

        friend class RendererLogger;
    };
}
