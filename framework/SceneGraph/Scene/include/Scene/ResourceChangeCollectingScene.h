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
        void                        releaseRenderable(RenderableHandle renderableHandle) override;
        void                        setRenderableDataInstance(RenderableHandle renderableHandle, ERenderableDataSlotType slot, DataInstanceHandle newDataInstance) override;
        void                        setRenderableVisibility(RenderableHandle renderableHandle, EVisibilityMode visibility) override;

        void                        setDataResource(DataInstanceHandle dataInstanceHandle, DataFieldHandle field, const ResourceContentHash& hash, DataBufferHandle dataBuffer, UInt32 instancingDivisor, UInt16 offsetWithinElementInBytes, UInt16 stride) override;
        void                        setDataTextureSamplerHandle(DataInstanceHandle containerHandle, DataFieldHandle field, TextureSamplerHandle samplerHandle) override;

        TextureSamplerHandle        allocateTextureSampler(const TextureSampler& sampler, TextureSamplerHandle handle = TextureSamplerHandle::Invalid()) override;
        void                        releaseTextureSampler(TextureSamplerHandle handle) override;

        DataSlotHandle              allocateDataSlot(const DataSlot& dataSlot, DataSlotHandle handle = DataSlotHandle::Invalid()) override;
        void                        setDataSlotTexture(DataSlotHandle providerHandle, const ResourceContentHash& texture) override;
        void                        releaseDataSlot(DataSlotHandle handle) override;

        // functions which affect scene resources
        RenderTargetHandle          allocateRenderTarget(RenderTargetHandle targetHandle = RenderTargetHandle::Invalid()) override;
        void                        releaseRenderTarget(RenderTargetHandle handle) override;

        RenderBufferHandle          allocateRenderBuffer(const RenderBuffer& renderBuffer, RenderBufferHandle handle = RenderBufferHandle::Invalid()) override;
        void                        releaseRenderBuffer(RenderBufferHandle handle) override;

        BlitPassHandle              allocateBlitPass(RenderBufferHandle sourceRenderBufferHandle, RenderBufferHandle destinationRenderBufferHandle, BlitPassHandle passHandle = BlitPassHandle::Invalid()) override;
        void                        releaseBlitPass(BlitPassHandle handle) override;

        DataBufferHandle            allocateDataBuffer(EDataBufferType dataBufferType, EDataType dataType, UInt32 maximumSizeInBytes, DataBufferHandle handle = DataBufferHandle::Invalid()) override;
        void                        releaseDataBuffer(DataBufferHandle handle) override;
        void                        updateDataBuffer(DataBufferHandle handle, UInt32 offsetInBytes, UInt32 dataSizeInBytes, const Byte* data) override;

        TextureBufferHandle         allocateTextureBuffer(ETextureFormat textureFormat, const MipMapDimensions& mipMapDimensions, TextureBufferHandle handle = TextureBufferHandle::Invalid()) override;
        void                        releaseTextureBuffer(TextureBufferHandle handle) override;
        void                        updateTextureBuffer(TextureBufferHandle handle, UInt32 mipLevel, UInt32 x, UInt32 y, UInt32 width, UInt32 height, const Byte* data) override;

    private:

        SceneResourceActionVector   m_sceneResourceActions;
        bool                        m_resourcesChanged = false;
    };
}

#endif
