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
#include "Components/ManagedResource.h"
#include <unordered_map>
#include <vector>

namespace ramses_internal
{
    struct RenderTarget;
    class IRendererResourceCache;
    enum class EDataBufferType : uint8_t;

    using StreamUsage = std::vector<StreamBufferHandle>;

    class IRendererResourceManager : public IResourceDeviceHandleAccessor
    {
    public:
        // Immutable resources
        [[nodiscard]] virtual EResourceStatus  getResourceStatus(const ResourceContentHash& hash) const = 0;
        [[nodiscard]] virtual EResourceType    getResourceType(const ResourceContentHash& hash) const = 0;

        virtual void             referenceResourcesForScene     (SceneId sceneId, const ResourceContentHashVector& resources) = 0;
        virtual void             unreferenceResourcesForScene   (SceneId sceneId, const ResourceContentHashVector& resources) = 0;

        virtual void             provideResourceData(const ManagedResource& mr) = 0;
        [[nodiscard]] virtual bool             hasResourcesToBeUploaded() const = 0;
        virtual void             uploadAndUnloadPendingResources() = 0;

        // Scene resources
        virtual void             uploadRenderTargetBuffer(RenderBufferHandle renderBufferHandle, SceneId sceneId, const RenderBuffer& renderBuffer) = 0;
        virtual void             unloadRenderTargetBuffer(RenderBufferHandle renderBufferHandle, SceneId sceneId) = 0;
        virtual void             uploadRenderTarget(RenderTargetHandle renderTarget, const RenderBufferHandleVector& rtBufferHandles, SceneId sceneId) = 0;
        virtual void             unloadRenderTarget(RenderTargetHandle renderTarget, SceneId sceneId) = 0;

        virtual void             uploadBlitPassRenderTargets(BlitPassHandle blitPass, RenderBufferHandle sourceRenderBuffer, RenderBufferHandle destinationRenderBuffer, SceneId sceneId) = 0;
        virtual void             unloadBlitPassRenderTargets(BlitPassHandle blitPass, SceneId sceneId) = 0;

        virtual void             uploadDataBuffer(DataBufferHandle dataBufferHandle, EDataBufferType dataBufferType, EDataType dataType, uint32_t dataSizeInBytes, SceneId sceneId) = 0;
        virtual void             unloadDataBuffer(DataBufferHandle dataBufferHandle, SceneId sceneId) = 0;
        virtual void             updateDataBuffer(DataBufferHandle handle, uint32_t dataSizeInBytes, const Byte* data, SceneId sceneId) = 0;

        virtual void             uploadTextureBuffer(TextureBufferHandle textureBufferHandle, uint32_t width, uint32_t height, ETextureFormat textureFormat, uint32_t mipLevelCount,  SceneId sceneId) = 0;
        virtual void             unloadTextureBuffer(TextureBufferHandle textureBufferHandle, SceneId sceneId) = 0;
        virtual void             updateTextureBuffer(TextureBufferHandle textureBufferHandle, uint32_t mipLevel, uint32_t x, uint32_t y, uint32_t width, uint32_t height, const Byte* data, SceneId sceneId) = 0;

        virtual void             uploadVertexArray(RenderableHandle renderableHandle, const VertexArrayInfo& vertexArrayInfo, SceneId sceneId) = 0;
        virtual void             unloadVertexArray(RenderableHandle renderableHandle, SceneId sceneId) = 0;

        virtual void             unloadAllSceneResourcesForScene(SceneId sceneId) = 0;
        virtual void             unreferenceAllResourcesForScene(SceneId sceneId) = 0;
        [[nodiscard]] virtual const ResourceContentHashVector* getResourcesInUseByScene(SceneId sceneId) const = 0;

        // Renderer resources
        virtual void             uploadOffscreenBuffer(OffscreenBufferHandle bufferHandle, uint32_t width, uint32_t height, uint32_t sampleCount, bool isDoubleBuffered, ERenderBufferType depthStencilBufferType) = 0;
        virtual void             uploadDmaOffscreenBuffer(OffscreenBufferHandle bufferHandle, uint32_t width, uint32_t height, DmaBufferFourccFormat dmaBufferFourccFormat, DmaBufferUsageFlags dmaBufferUsageFlags, DmaBufferModifiers dmaBufferModifiers) = 0;
        virtual void             unloadOffscreenBuffer(OffscreenBufferHandle bufferHandle) = 0;

        virtual void             uploadStreamBuffer(StreamBufferHandle bufferHandle, WaylandIviSurfaceId surfaceId) = 0;
        virtual void             unloadStreamBuffer(StreamBufferHandle bufferHandle) = 0;

        virtual void             uploadExternalBuffer(ExternalBufferHandle bufferHandle) = 0;
        virtual void             unloadExternalBuffer(ExternalBufferHandle bufferHandle) = 0;

        [[nodiscard]] virtual const StreamUsage& getStreamUsage(WaylandIviSurfaceId source) const = 0;
    };
}
#endif
