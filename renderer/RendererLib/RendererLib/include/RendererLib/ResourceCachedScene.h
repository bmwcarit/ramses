//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RESOURCECACHEDSCENE_H
#define RAMSES_RESOURCECACHEDSCENE_H

#include "RendererLib/TextureLinkCachedScene.h"

namespace ramses_internal
{
    class IResourceDeviceHandleAccessor;

    struct VertexArrayCacheEntry
    {
        DeviceResourceHandle deviceHandle;
        bool usesIndexArray = false;
    };
    using VertexArrayCache = std::vector<VertexArrayCacheEntry>;

    class ResourceCachedScene : public TextureLinkCachedScene
    {
    public:
        explicit ResourceCachedScene(SceneLinksManager& sceneLinksManager, const SceneInfo& sceneInfo = SceneInfo());

        void                        preallocateSceneSize(const SceneSizeInformation& sizeInfo) override;
        // Renderable allocation
        RenderableHandle            allocateRenderable          (NodeHandle nodeHandle, RenderableHandle handle = RenderableHandle::Invalid()) override;
        void                        releaseRenderable           (RenderableHandle renderableHandle) override;
        void                        setRenderableVisibility     (RenderableHandle renderableHandle, EVisibilityMode visibility) override;
        void                        setRenderableStartVertex    (RenderableHandle renderableHandle, uint32_t startVertex) override;
        DataInstanceHandle          allocateDataInstance        (DataLayoutHandle handle, DataInstanceHandle instanceHandle = DataInstanceHandle::Invalid()) override;
        void                        releaseDataInstance         (DataInstanceHandle dataInstanceHandle) override;
        TextureSamplerHandle        allocateTextureSampler      (const TextureSampler& sampler, TextureSamplerHandle handle) override;
        void                        releaseTextureSampler       (TextureSamplerHandle handle) override;

        // Renderable data (stuff required for rendering)
        void                        setRenderableDataInstance   (RenderableHandle renderableHandle, ERenderableDataSlotType slot, DataInstanceHandle newDataInstance) override;
        void                        setDataResource             (DataInstanceHandle dataInstanceHandle, DataFieldHandle field, const ResourceContentHash& hash, DataBufferHandle dataBuffer, uint32_t instancingDivisor, uint16_t offsetWithinElementInBytes, uint16_t stride) override;
        void                        setDataTextureSamplerHandle (DataInstanceHandle dataInstanceHandle, DataFieldHandle field, TextureSamplerHandle samplerHandle) override;

        RenderTargetHandle          allocateRenderTarget        (RenderTargetHandle targetHandle = RenderTargetHandle::Invalid()) override;
        BlitPassHandle              allocateBlitPass            (RenderBufferHandle sourceRenderBufferHandle, RenderBufferHandle destinationRenderBufferHandle, BlitPassHandle passHandle = BlitPassHandle::Invalid()) override;

        void                                resetResourceCache();

        bool                                renderableResourcesDirty    (RenderableHandle handle) const;
        bool                                renderableResourcesDirty    (const RenderableVector& handles) const;

        DeviceResourceHandle                getRenderableEffectDeviceHandle(RenderableHandle renderable) const;
        const VertexArrayCache&             getCachedHandlesForVertexArrays() const;
        const DeviceHandleVector&           getCachedHandlesForTextureSamplers() const;
        const DeviceHandleVector&           getCachedHandlesForRenderTargets() const;
        const DeviceHandleVector&           getCachedHandlesForBlitPassRenderTargets() const;
        const BoolVector&                   getVertexArraysDirtinessFlags() const;

        void updateRenderableResources(const IResourceDeviceHandleAccessor& resourceAccessor);
        void updateRenderablesResourcesDirtiness();
        void setRenderableResourcesDirtyByTextureSampler(TextureSamplerHandle textureSamplerHandle) const;

        bool hasDirtyVertexArrays() const;
        void markVertexArraysClean();
        bool isRenderableVertexArrayDirty(RenderableHandle renderable) const;
        void updateRenderableVertexArrays(const IResourceDeviceHandleAccessor& resourceAccessor, const RenderableVector& renderablesWithUpdatedVertexArrays);

    protected:
        bool resolveTextureSamplerResourceDeviceHandle(const IResourceDeviceHandleAccessor& resourceAccessor, TextureSamplerHandle sampler, DeviceResourceHandle& deviceHandleInOut);

    private:
        void setRenderableResourcesDirtyFlag(RenderableHandle handle, bool dirty) const;
        void setRenderableVertexArrayDirtyFlag(RenderableHandle handle, bool dirty) const;
        void setDataInstanceDirtyFlag(DataInstanceHandle handle, bool dirty) const;
        void setTextureSamplerDirtyFlag(TextureSamplerHandle handle, bool dirty) const;
        bool doesRenderableReferToDirtyUniforms(RenderableHandle handle) const;
        bool doesRenderableReferToDirtyGeometry(RenderableHandle handle) const;
        bool doesDataInstanceReferToDirtyTextureSampler(DataInstanceHandle handle) const;
        bool isDataInstanceDirty(DataInstanceHandle handle) const;
        bool isTextureSamplerDirty(TextureSamplerHandle handle) const;
        bool isGeometryDataLayout(const DataLayout& layout) const;
        static bool CheckAndUpdateDeviceHandle(const IResourceDeviceHandleAccessor& resourceAccessor, DeviceResourceHandle& deviceHandleInOut, const ResourceContentHash& resourceHash);

        bool checkAndUpdateEffectResource(const IResourceDeviceHandleAccessor& resourceAccessor, RenderableHandle renderable);
        bool checkAndUpdateTextureResources(const IResourceDeviceHandleAccessor& resourceAccessor, RenderableHandle renderable);
        bool checkGeometryResources(const IResourceDeviceHandleAccessor& resourceAccessor, RenderableHandle renderable);
        void checkAndUpdateRenderTargetResources(const IResourceDeviceHandleAccessor& resourceAccessor);
        void checkAndUpdateBlitPassResources(const IResourceDeviceHandleAccessor& resourceAccessor);

        bool updateTextureSamplerResource(const IResourceDeviceHandleAccessor& resourceAccessor, TextureSamplerHandle sampler);
        bool updateTextureSamplerResourceAsRenderBuffer(const IResourceDeviceHandleAccessor& resourceAccessor, const RenderBufferHandle bufferHandle, DeviceResourceHandle& deviceHandleOut);
        bool updateTextureSamplerResourceAsTextureBuffer(const IResourceDeviceHandleAccessor& resourceAccessor, const TextureBufferHandle bufferHandle, DeviceResourceHandle& deviceHandleOut);
        bool updateTextureSamplerResourceAsStreamBuffer(const IResourceDeviceHandleAccessor& resourceAccessor, const StreamBufferHandle streamBuffer, const TextureSampler& fallbackSamplerData, DeviceResourceHandle& deviceHandleInOut);

        DeviceHandleVector         m_effectDeviceHandleCache;
        VertexArrayCache           m_vertexArrayCache;
        mutable DeviceHandleVector m_deviceHandleCacheForTextures;
        DeviceHandleVector         m_renderTargetCache;
        DeviceHandleVector         m_blitPassCache;

        mutable bool       m_renderableResourcesDirtinessNeedsUpdate = false;
        mutable bool       m_renderableVertexArraysDirty = false;
        mutable BoolVector m_renderableResourcesDirty;
        mutable BoolVector m_dataInstancesDirty;
        mutable BoolVector m_textureSamplersDirty;
        mutable BoolVector m_renderableVertexArrayDirty;

        bool m_renderTargetsDirty = false;
        bool m_blitPassesDirty = false;
    };
}

#endif
