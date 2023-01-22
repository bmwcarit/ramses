//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/TextureLinkCachedScene.h"
#include "RendererLib/SceneLinksManager.h"
#include "RendererLib/IResourceDeviceHandleAccessor.h"
#include "SceneAPI/TextureSampler.h"

namespace ramses_internal
{
    TextureLinkCachedScene::TextureLinkCachedScene(SceneLinksManager& sceneLinksManager, const SceneInfo& sceneInfo)
        : ResourceCachedScene(sceneLinksManager, sceneInfo)
    {
    }

    DataSlotHandle TextureLinkCachedScene::allocateDataSlot(const DataSlot& dataSlot, DataSlotHandle handle)
    {
        const DataSlotHandle actualHandle = ResourceCachedScene::allocateDataSlot(dataSlot, handle);

        if (dataSlot.type == EDataSlotType_TextureConsumer)
        {
            const auto sampler = dataSlot.attachedTextureSampler;
            assert(sampler.isValid() && isTextureSamplerAllocated(sampler));
            m_fallbackTextureSamplers[sampler] = ResourceCachedScene::getTextureSampler(sampler);
        }

        return actualHandle;
    }

    void TextureLinkCachedScene::releaseDataSlot(DataSlotHandle handle)
    {
        const TextureSamplerHandle sampler = getDataSlot(handle).attachedTextureSampler;
        ResourceCachedScene::releaseDataSlot(handle);

        auto it = m_fallbackTextureSamplers.find(sampler);
        if (it != m_fallbackTextureSamplers.end())
            m_fallbackTextureSamplers.remove(it);
    }

    void TextureLinkCachedScene::setDataSlotTexture(DataSlotHandle handle, const ResourceContentHash& texture)
    {
        ResourceCachedScene::setDataSlotTexture(handle, texture);
        m_sceneLinksManager.getTextureLinkManager().setTextureToConsumers(getSceneId(), handle, texture);
    }

    void TextureLinkCachedScene::setTextureSamplerContentSource(TextureSamplerHandle sampler, const ResourceContentHash& hash)
    {
        assert(sampler.isValid() && isTextureSamplerAllocated(sampler));
        assert(m_fallbackTextureSamplers.contains(sampler));
        TextureSampler& samplerData = getTextureSamplerInternal(sampler);
        samplerData.contentType = TextureSampler::ContentType::ClientTexture;
        samplerData.textureResource = hash;
        samplerData.contentHandle = {};
        setRenderableResourcesDirtyByTextureSampler(sampler);
    }

    void TextureLinkCachedScene::setTextureSamplerContentSource(TextureSamplerHandle sampler, OffscreenBufferHandle offscreenBuffer)
    {
        assert(sampler.isValid() && isTextureSamplerAllocated(sampler));
        assert(m_fallbackTextureSamplers.contains(sampler));
        TextureSampler& samplerData = getTextureSamplerInternal(sampler);
        samplerData.contentType = TextureSampler::ContentType::OffscreenBuffer;
        samplerData.textureResource = ResourceContentHash::Invalid();
        samplerData.contentHandle = offscreenBuffer.asMemoryHandle();
        setRenderableResourcesDirtyByTextureSampler(sampler);
    }

    void TextureLinkCachedScene::setTextureSamplerContentSource(TextureSamplerHandle sampler, StreamBufferHandle streamBuffer)
    {
        assert(sampler.isValid() && isTextureSamplerAllocated(sampler));
        assert(m_fallbackTextureSamplers.contains(sampler));
        TextureSampler& samplerData = getTextureSamplerInternal(sampler);
        samplerData.contentType = TextureSampler::ContentType::StreamBuffer;
        samplerData.textureResource = ResourceContentHash::Invalid();
        samplerData.contentHandle = streamBuffer.asMemoryHandle();
        setRenderableResourcesDirtyByTextureSampler(sampler);
    }

    void TextureLinkCachedScene::setTextureSamplerContentSource(TextureSamplerHandle sampler, ExternalBufferHandle externalTexture)
    {
        assert(sampler.isValid() && isTextureSamplerAllocated(sampler));
        assert(m_fallbackTextureSamplers.contains(sampler));
        TextureSampler& samplerData = getTextureSamplerInternal(sampler);
        samplerData.contentType = TextureSampler::ContentType::ExternalTexture;
        samplerData.textureResource = ResourceContentHash::Invalid();
        samplerData.contentHandle = externalTexture.asMemoryHandle();
        setRenderableResourcesDirtyByTextureSampler(sampler);
    }

    void TextureLinkCachedScene::restoreTextureSamplerFallbackValue(TextureSamplerHandle sampler)
    {
        assert(sampler.isValid() && isTextureSamplerAllocated(sampler));
        assert(m_fallbackTextureSamplers.contains(sampler));
        getTextureSamplerInternal(sampler) = m_fallbackTextureSamplers.find(sampler)->value;
        setRenderableResourcesDirtyByTextureSampler(sampler);
    }
}
