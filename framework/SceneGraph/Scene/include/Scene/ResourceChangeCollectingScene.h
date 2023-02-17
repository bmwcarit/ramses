//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RESOURCECHANGECOLLECTINGSCENE_H
#define RAMSES_RESOURCECHANGECOLLECTINGSCENE_H

#include "Scene/TransformationCachedScene.h"
#include "Scene/ResourceChanges.h"

namespace ramses_internal
{
    class ResourceChangeCollectingScene : public TransformationCachedScene
    {
    public:
        explicit ResourceChangeCollectingScene(const SceneInfo& sceneInfo = SceneInfo());

        [[nodiscard]] const SceneResourceActionVector&    getSceneResourceActions() const;
        [[nodiscard]] bool                                haveResourcesChanged() const;
        void                                resetResourceChanges();

        // functions which affect client resources
        virtual void                        releaseRenderable(RenderableHandle renderableHandle) override;
        virtual void                        setRenderableDataInstance(RenderableHandle renderableHandle, ERenderableDataSlotType slot, DataInstanceHandle newDataInstance) override;
        virtual void                        setRenderableVisibility(RenderableHandle renderableHandle, EVisibilityMode visibility) override;

        virtual void                        setDataResource(DataInstanceHandle dataInstanceHandle, DataFieldHandle field, const ResourceContentHash& hash, DataBufferHandle dataBuffer, UInt32 instancingDivisor, UInt16 offsetWithinElementInBytes, UInt16 stride) override;
        virtual void                        setDataTextureSamplerHandle(DataInstanceHandle containerHandle, DataFieldHandle field, TextureSamplerHandle samplerHandle) override;

        virtual TextureSamplerHandle        allocateTextureSampler(const TextureSampler& sampler, TextureSamplerHandle handle = TextureSamplerHandle::Invalid()) override;
        virtual void                        releaseTextureSampler(TextureSamplerHandle handle) override;

        virtual DataSlotHandle              allocateDataSlot(const DataSlot& dataSlot, DataSlotHandle handle = DataSlotHandle::Invalid()) override;
        virtual void                        setDataSlotTexture(DataSlotHandle providerHandle, const ResourceContentHash& texture) override;
        virtual void                        releaseDataSlot(DataSlotHandle handle) override;

        // functions which both affect client and scene resources
        virtual StreamTextureHandle         allocateStreamTexture(WaylandIviSurfaceId streamSource, const ResourceContentHash& fallbackTextureHash, StreamTextureHandle streamTextureHandle = StreamTextureHandle::Invalid()) override;
        virtual void                        releaseStreamTexture(StreamTextureHandle handle) override;

        // functions which affect scene resources
        virtual RenderTargetHandle          allocateRenderTarget(RenderTargetHandle targetHandle = RenderTargetHandle::Invalid()) override;
        virtual void                        releaseRenderTarget(RenderTargetHandle handle) override;

        virtual RenderBufferHandle          allocateRenderBuffer(const RenderBuffer& renderBuffer, RenderBufferHandle handle = RenderBufferHandle::Invalid()) override;
        virtual void                        releaseRenderBuffer(RenderBufferHandle handle) override;

        virtual BlitPassHandle              allocateBlitPass(RenderBufferHandle sourceRenderBufferHandle, RenderBufferHandle destinationRenderBufferHandle, BlitPassHandle passHandle = BlitPassHandle::Invalid()) override;
        virtual void                        releaseBlitPass(BlitPassHandle handle) override;

        virtual DataBufferHandle            allocateDataBuffer(EDataBufferType dataBufferType, EDataType dataType, UInt32 maximumSizeInBytes, DataBufferHandle handle = DataBufferHandle::Invalid()) override;
        virtual void                        releaseDataBuffer(DataBufferHandle handle) override;
        virtual void                        updateDataBuffer(DataBufferHandle handle, UInt32 offsetInBytes, UInt32 dataSizeInBytes, const Byte* data) override;

        virtual TextureBufferHandle         allocateTextureBuffer(ETextureFormat textureFormat, const MipMapDimensions& mipMapDimensions, TextureBufferHandle handle = TextureBufferHandle::Invalid()) override;
        virtual void                        releaseTextureBuffer(TextureBufferHandle handle) override;
        virtual void                        updateTextureBuffer(TextureBufferHandle handle, UInt32 mipLevel, UInt32 x, UInt32 y, UInt32 width, UInt32 height, const Byte* data) override;

    private:

        SceneResourceActionVector   m_sceneResourceActions;
        bool                        m_resourcesChanged = false;
    };
}

#endif
