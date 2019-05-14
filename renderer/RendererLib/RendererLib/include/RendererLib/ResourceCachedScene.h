//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RESOURCECACHEDSCENE_H
#define RAMSES_RESOURCECACHEDSCENE_H

#include "RendererAPI/Types.h"
#include "RendererLib/DataReferenceLinkCachedScene.h"

namespace ramses_internal
{
    class IResourceDeviceHandleAccessor;
    class IEmbeddedCompositingManager;

    typedef std::vector<DeviceHandleVector> DeviceHandleCache;

    class ResourceCachedScene : public DataReferenceLinkCachedScene
    {
    public:
        ResourceCachedScene(SceneLinksManager& sceneLinksManager, const SceneInfo& creationInfo = SceneInfo());

        virtual void                        preallocateSceneSize(const SceneSizeInformation& sizeInfo) override;
        // Renderable allocation
        virtual RenderableHandle            allocateRenderable          (NodeHandle nodeHandle, RenderableHandle handle = RenderableHandle::Invalid()) override;
        virtual void                        releaseRenderable           (RenderableHandle renderableHandle) override;
        virtual DataInstanceHandle          allocateDataInstance        (DataLayoutHandle handle, DataInstanceHandle instanceHandle = DataInstanceHandle::Invalid()) override;
        virtual void                        releaseDataInstance         (DataInstanceHandle dataInstanceHandle) override;
        virtual TextureSamplerHandle        allocateTextureSampler      (const TextureSampler& sampler, TextureSamplerHandle handle) override;
        virtual void                        releaseTextureSampler       (TextureSamplerHandle handle) override;
        virtual void                        releaseStreamTexture        (StreamTextureHandle handle) override;

        // Renderable data (stuff required for rendering)
        virtual void                        setRenderableEffect         (RenderableHandle renderableHandle, const ResourceContentHash& effectHash) override;

        virtual void                        setRenderableDataInstance   (RenderableHandle renderableHandle, ERenderableDataSlotType slot, DataInstanceHandle newDataInstance) override;
        virtual void                        setDataResource             (DataInstanceHandle dataInstanceHandle, DataFieldHandle field, const ResourceContentHash& hash, DataBufferHandle dataBuffer, UInt32 instancingDivisor) override;
        virtual void                        setDataTextureSamplerHandle (DataInstanceHandle containerHandle, DataFieldHandle field, TextureSamplerHandle samplerHandle) override;

        virtual void                        setForceFallbackImage       (StreamTextureHandle streamTextureHandle, Bool forceFallbackImage) override;

        virtual RenderTargetHandle          allocateRenderTarget        (RenderTargetHandle targetHandle = RenderTargetHandle::Invalid()) override;
        virtual BlitPassHandle              allocateBlitPass            (RenderBufferHandle sourceRenderBufferHandle, RenderBufferHandle destinationRenderBufferHandle, BlitPassHandle passHandle = BlitPassHandle::Invalid()) override;

        void                                resetResourceCache();

        Bool                                renderableResourcesDirty    (RenderableHandle handle) const;
        Bool                                renderableResourcesDirty    (const RenderableVector& handles) const;

        DeviceResourceHandle                getRenderableEffectDeviceHandle(RenderableHandle renderable) const;
        const DeviceHandleCache&            getCachedHandlesForVertexAttributes() const;
        const DeviceHandleVector&           getCachedHandlesForTextureSamplers() const;
        const DeviceHandleVector&           getCachedHandlesForRenderTargets() const;
        const DeviceHandleVector&           getCachedHandlesForBlitPassRenderTargets() const;

        void updateRenderableResources(const IResourceDeviceHandleAccessor& resourceAccessor, const IEmbeddedCompositingManager& embeddedCompositingManager);
        void updateRenderablesResourcesDirtiness();
        void setRenderableResourcesDirtyByTextureSampler(TextureSamplerHandle textureSamplerHandle) const;
        void setRenderableResourcesDirtyByStreamTexture(StreamTextureHandle streamTextureHandle) const;

    protected:
        Bool resolveTextureSamplerResourceDeviceHandle(const IResourceDeviceHandleAccessor& resourceAccessor, TextureSamplerHandle sampler, DeviceResourceHandle& deviceHandleInOut);

    private:
        void setRenderableResourcesDirtyFlag(RenderableHandle handle, Bool dirty) const;
        void setDataInstanceDirtyFlag(DataInstanceHandle handle, Bool dirty) const;
        void setTextureSamplerDirtyFlag(TextureSamplerHandle handle, Bool dirty) const;
        Bool doesRenderableReferToDirtyDataInstance(RenderableHandle handle) const;
        Bool doesDataInstanceReferToDirtyTextureSampler(DataInstanceHandle handle) const;
        Bool isDataInstanceDirty(DataInstanceHandle handle) const;
        Bool isTextureSamplerDirty(TextureSamplerHandle handle) const;
        Bool isGeometryDataLayout(const DataLayout& layout) const;
        static Bool CheckAndUpdateDeviceHandle(const IResourceDeviceHandleAccessor& resourceAccessor, DeviceResourceHandle& deviceHandleInOut, const ResourceContentHash& resourceHash);
        static Bool CheckAndUpdateBufferDeviceHandle(const IResourceDeviceHandleAccessor& resourceAccessor, DeviceResourceHandle& deviceHandleInOut, const ResourceContentHash& resourceHash, SceneId sceneId, DataBufferHandle dataBufferHandle);

        Bool checkAndUpdateRenderableResources(const IResourceDeviceHandleAccessor& resourceAccessor, RenderableHandle renderable);
        Bool checkAndUpdateTextureResources(const IResourceDeviceHandleAccessor& resourceAccessor, const IEmbeddedCompositingManager& embeddedCompositingManager, RenderableHandle renderable);
        Bool checkAndUpdateGeometryResources(const IResourceDeviceHandleAccessor& resourceAccessor, RenderableHandle renderable);
        void checkAndUpdateRenderTargetResources(const IResourceDeviceHandleAccessor& resourceAccessor);
        void checkAndUpdateBlitPassResources(const IResourceDeviceHandleAccessor& resourceAccessor);

        Bool updateTextureSamplerResource(const IResourceDeviceHandleAccessor& resourceAccessor, const IEmbeddedCompositingManager& embeddedCompositingManager, TextureSamplerHandle sampler);
        Bool updateTextureSamplerResourceAsRenderBuffer(const IResourceDeviceHandleAccessor& resourceAccessor, const RenderBufferHandle bufferHandle, DeviceResourceHandle& deviceHandleOut);
        Bool updateTextureSamplerResourceAsTextureBuffer(const IResourceDeviceHandleAccessor& resourceAccessor, const TextureBufferHandle bufferHandle, DeviceResourceHandle& deviceHandleOut);
        Bool updateTextureSamplerResourceAsStreamTexture(const IResourceDeviceHandleAccessor& resourceAccessor, const IEmbeddedCompositingManager& embeddedCompositingManager, const StreamTextureHandle streamTextureHandle, DeviceResourceHandle& deviceHandleInOut);

        DeviceHandleVector         m_effectDeviceHandleCache;
        DeviceHandleCache          m_deviceHandleCacheForVertexAttributes;
        mutable DeviceHandleVector m_deviceHandleCacheForTextures;
        DeviceHandleVector         m_renderTargetCache;
        DeviceHandleVector         m_blitPassCache;

        mutable Bool       m_renderableResourcesDirtinessNeedsUpdate;
        mutable BoolVector m_renderableResourcesDirty;
        mutable BoolVector m_dataInstancesDirty;
        mutable BoolVector m_textureSamplersDirty;

        Bool m_renderTargetsDirty;
        Bool m_blitPassesDirty;
    };
}

#endif
