//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/TextureLinkManager.h"
#include "RendererLib/RendererScenes.h"
#include "RendererLib/DataLinkUtils.h"
#include "Utils/LogMacros.h"

namespace ramses_internal
{
    TextureLinkManager::TextureLinkManager(RendererScenes& rendererScenes)
        : LinkManagerBase(rendererScenes)
    {
    }

    void TextureLinkManager::removeSceneLinks(SceneId sceneId)
    {
        SceneLinkVector links;
        getSceneLinks().getLinkedConsumers(sceneId, links);
        for (const auto& link : links)
        {
            assert(link.providerSceneId == sceneId);
            removeDataLink(link.consumerSceneId, link.consumerSlot);
        }

        links.clear();
        getSceneLinks().getLinkedProviders(sceneId, links);
        for (const auto& link : links)
        {
            assert(link.consumerSceneId == sceneId);
            removeDataLink(link.consumerSceneId, link.consumerSlot);
        }

        OffscreenBufferLinkVector bufferLinks;
        m_offscreenBufferLinks.getLinkedProviders(sceneId, bufferLinks);
        for (const auto& link : bufferLinks)
        {
            assert(link.consumerSceneId == sceneId);
            removeDataLink(link.consumerSceneId, link.consumerSlot);
        }

        m_samplersToDataSlots.remove(sceneId);
        LinkManagerBase::removeSceneLinks(sceneId);
    }

    Bool TextureLinkManager::createDataLink(SceneId providerSceneId, DataSlotHandle providerSlotHandle, SceneId consumerSceneId, DataSlotHandle consumerSlotHandle)
    {
        assert(EDataSlotType_TextureProvider == DataLinkUtils::GetDataSlot(providerSceneId, providerSlotHandle, m_scenes).type);
        assert(EDataSlotType_TextureConsumer == DataLinkUtils::GetDataSlot(consumerSceneId, consumerSlotHandle, m_scenes).type);

        if (m_offscreenBufferLinks.hasLinkedProvider(consumerSceneId, consumerSlotHandle))
        {
            LOG_ERROR(CONTEXT_RENDERER, "Renderer::createDataLink failed: consumer slot " << consumerSlotHandle << " already has an offscreen buffer link assigned! (consumer scene: " << consumerSceneId.getValue() << ")");
            return false;
        }

        if (!LinkManagerBase::createDataLink(providerSceneId, providerSlotHandle, consumerSceneId, consumerSlotHandle))
        {
            return false;
        }

        const TextureSamplerHandle consumerSampler = storeConsumerSlot(consumerSceneId, consumerSlotHandle);
        m_scenes.getScene(consumerSceneId).setTextureSamplerContentSource(consumerSampler, getLinkedTexture(consumerSceneId, consumerSampler));

        return true;
    }

    Bool TextureLinkManager::createBufferLink(OffscreenBufferHandle providerBuffer, SceneId consumerSceneId, DataSlotHandle consumerSlotHandle)
    {
        assert(EDataSlotType_TextureConsumer == DataLinkUtils::GetDataSlot(consumerSceneId, consumerSlotHandle, m_scenes).type);

        if (getSceneLinks().hasLinkedProvider(consumerSceneId, consumerSlotHandle))
        {
            LOG_ERROR(CONTEXT_RENDERER, "Renderer::createDataLink failed: consumer slot already has a data link assigned! (consumer scene: " << consumerSceneId.getValue() << ")");
            return false;
        }

        if (m_offscreenBufferLinks.hasLinkedProvider(consumerSceneId, consumerSlotHandle))
        {
            LOG_ERROR(CONTEXT_RENDERER, "Renderer::createDataLink failed: consumer slot already has a buffer link assigned! (consumer scene: " << consumerSceneId.getValue() << ")");
            return false;
        }

        m_offscreenBufferLinks.addLink(providerBuffer, consumerSceneId, consumerSlotHandle);

        const TextureSamplerHandle consumerSampler = storeConsumerSlot(consumerSceneId, consumerSlotHandle);
        m_scenes.getScene(consumerSceneId).setTextureSamplerContentSource(consumerSampler, providerBuffer);

        return true;
    }

    Bool TextureLinkManager::removeDataLink(SceneId consumerSceneId, DataSlotHandle consumerSlotHandle)
    {
        Bool removed = false;
        if (m_offscreenBufferLinks.hasLinkedProvider(consumerSceneId, consumerSlotHandle))
        {
            m_offscreenBufferLinks.removeLink(consumerSceneId, consumerSlotHandle);
            removed = true;
        }
        else
        {
            removed = LinkManagerBase::removeDataLink(consumerSceneId, consumerSlotHandle);
        }

        if (removed)
        {
            const TextureSamplerHandle consumerSampler = DataLinkUtils::GetDataSlot(consumerSceneId, consumerSlotHandle, m_scenes).attachedTextureSampler;
            assert(consumerSampler != TextureSamplerHandle::Invalid());
            m_scenes.getScene(consumerSceneId).restoreTextureSamplerFallbackValue(consumerSampler);

            const TextureSamplerHandle sampler = DataLinkUtils::GetDataSlot(consumerSceneId, consumerSlotHandle, m_scenes).attachedTextureSampler;
            assert(m_samplersToDataSlots.contains(consumerSceneId));
            m_samplersToDataSlots.get(consumerSceneId)->remove(sampler);
        }

        return removed;
    }

    Bool TextureLinkManager::hasLinkedTexture(SceneId consumerSceneId, TextureSamplerHandle sampler) const
    {
        const DataSlotHandle consumerSlotHandle = getDataSlotForSampler(consumerSceneId, sampler);
        if (consumerSlotHandle.isValid())
        {
            return getSceneLinks().hasLinkedProvider(consumerSceneId, consumerSlotHandle);
        }

        return false;
    }

    ResourceContentHash TextureLinkManager::getLinkedTexture(SceneId consumerSceneId, TextureSamplerHandle sampler) const
    {
        const DataSlotHandle consumerSlotHandle = getDataSlotForSampler(consumerSceneId, sampler);
        assert(consumerSlotHandle.isValid());

        const SceneLink& link = getSceneLinks().getLinkedProvider(consumerSceneId, consumerSlotHandle);
        assert(link.consumerSceneId == consumerSceneId);
        assert(link.consumerSlot == consumerSlotHandle);

        const IScene& providerScene = m_scenes.getScene(link.providerSceneId);
        assert(providerScene.isDataSlotAllocated(link.providerSlot));
        return providerScene.getDataSlot(link.providerSlot).attachedTexture;
    }

    Bool TextureLinkManager::hasLinkedOffscreenBuffer(SceneId consumerSceneId, TextureSamplerHandle sampler) const
    {
        const DataSlotHandle consumerSlotHandle = getDataSlotForSampler(consumerSceneId, sampler);
        if (consumerSlotHandle.isValid())
        {
            return m_offscreenBufferLinks.hasLinkedProvider(consumerSceneId, consumerSlotHandle);
        }

        return false;
    }

    OffscreenBufferHandle TextureLinkManager::getLinkedOffscreenBuffer(SceneId consumerSceneId, TextureSamplerHandle sampler) const
    {
        const DataSlotHandle consumerSlotHandle = getDataSlotForSampler(consumerSceneId, sampler);
        assert(consumerSlotHandle.isValid());

        const OffscreenBufferLink& link = m_offscreenBufferLinks.getLinkedProvider(consumerSceneId, consumerSlotHandle);
        assert(link.consumerSceneId == consumerSceneId);
        assert(link.consumerSlot == consumerSlotHandle);

        return link.providerBuffer;
    }

    const OffscreenBufferLinks& TextureLinkManager::getOffscreenBufferLinks() const
    {
        return m_offscreenBufferLinks;
    }

    TextureSamplerHandle TextureLinkManager::storeConsumerSlot(SceneId consumerSceneId, DataSlotHandle consumerSlotHandle)
    {
        const TextureSamplerHandle consumerSampler = DataLinkUtils::GetDataSlot(consumerSceneId, consumerSlotHandle, m_scenes).attachedTextureSampler;
        assert(consumerSampler.isValid());

        if (!m_samplersToDataSlots.contains(consumerSceneId))
        {
            m_samplersToDataSlots.put(consumerSceneId, SamplerToSlotMap());
        }
        m_samplersToDataSlots.get(consumerSceneId)->put(consumerSampler, consumerSlotHandle);

        return consumerSampler;
    }

    DataSlotHandle TextureLinkManager::getDataSlotForSampler(SceneId sceneId, TextureSamplerHandle sampler) const
    {
        const SamplerToSlotMap* samplerToSlotMap = m_samplersToDataSlots.get(sceneId);
        if (samplerToSlotMap == nullptr)
        {
            return DataSlotHandle::Invalid();
        }

        DataSlotHandle* slotEntry = samplerToSlotMap->get(sampler);
        if (slotEntry == nullptr)
        {
            return DataSlotHandle::Invalid();
        }

        return *slotEntry;
    }

    void TextureLinkManager::setTextureToConsumers(SceneId providerSceneId, DataSlotHandle providerSlotHandle, ResourceContentHash texture) const
    {
        SceneLinkVector consumers;
        getSceneLinks().getLinkedConsumers(providerSceneId, providerSlotHandle, consumers);
        for (const auto& consumer : consumers)
        {
            TextureLinkCachedScene& consumerScene = m_scenes.getScene(consumer.consumerSceneId);
            consumerScene.setTextureSamplerContentSource(consumerScene.getDataSlot(consumer.consumerSlot).attachedTextureSampler, texture);
        }
    }
}
