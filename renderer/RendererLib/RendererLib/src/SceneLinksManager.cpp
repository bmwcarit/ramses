//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/SceneLinksManager.h"
#include "RendererLib/RendererScenes.h"
#include "RendererLib/DataLinkUtils.h"
#include "RendererEventCollector.h"
#include "Utils/LogMacros.h"
#include "Common/Cpp11Macros.h"

namespace ramses_internal
{
    SceneLinksManager::SceneLinksManager(RendererScenes& rendererScenes, RendererEventCollector& rendererEventCollector)
        : m_rendererScenes(rendererScenes)
        , m_rendererEventCollector(rendererEventCollector)
        , m_transformationLinkManager(rendererScenes)
        , m_dataReferenceLinkManager(rendererScenes)
        , m_textureLinkManager(rendererScenes)
    {
    }

    void SceneLinksManager::createDataLink(SceneId providerSceneId, DataSlotId providerId, SceneId consumerSceneId, DataSlotId consumerId)
    {
        if (!m_rendererScenes.hasScene(providerSceneId))
        {
            LOG_ERROR(CONTEXT_RENDERER, "Renderer::createDataLink failed: provider scene " << providerSceneId.getValue() << " is not known to the renderer!");
            m_rendererEventCollector.addEvent(ERendererEventType_SceneDataLinkFailed, providerSceneId, consumerSceneId, providerId, consumerId);
            return;
        }

        if (!m_rendererScenes.hasScene(consumerSceneId))
        {
            LOG_ERROR(CONTEXT_RENDERER, "Renderer::createDataLink failed: consumer scene " << consumerSceneId << " is not known to the renderer!");
            m_rendererEventCollector.addEvent(ERendererEventType_SceneDataLinkFailed, providerSceneId, consumerSceneId, providerId, consumerId);
            return;
        }

        const DataSlotHandle providerSlotHandle = DataLinkUtils::GetDataSlotHandle(providerSceneId, providerId, m_rendererScenes);
        const DataSlotHandle consumerSlotHandle = DataLinkUtils::GetDataSlotHandle(consumerSceneId, consumerId, m_rendererScenes);

        if (!providerSlotHandle.isValid())
        {
            LOG_ERROR(CONTEXT_RENDERER, "Renderer::createDataLink failed: provider data id " << providerId << " is invalid! (Provider scene: " << providerSceneId.getValue() << ")");
            m_rendererEventCollector.addEvent(ERendererEventType_SceneDataLinkFailed, providerSceneId, consumerSceneId, providerId, consumerId);
            return;
        }

        if (!consumerSlotHandle.isValid())
        {
            LOG_ERROR(CONTEXT_RENDERER, "Renderer::createDataLink failed: consumer data id " << consumerId << " is invalid! (Consumer scene: " << consumerSceneId << ")");
            m_rendererEventCollector.addEvent(ERendererEventType_SceneDataLinkFailed, providerSceneId, consumerSceneId, providerId, consumerId);
            return;
        }

        const EDataSlotType providerSlotType = DataLinkUtils::GetDataSlot(providerSceneId, providerSlotHandle, m_rendererScenes).type;
        const EDataSlotType consumerSlotType = DataLinkUtils::GetDataSlot(consumerSceneId, consumerSlotHandle, m_rendererScenes).type;
        Bool linkSuccess = false;

        if (providerSlotType == EDataSlotType_TransformationProvider && consumerSlotType == EDataSlotType_TransformationConsumer)
        {
            linkSuccess = m_transformationLinkManager.createDataLink(providerSceneId, providerSlotHandle, consumerSceneId, consumerSlotHandle);
        }
        else if (providerSlotType == EDataSlotType_DataProvider && consumerSlotType == EDataSlotType_DataConsumer)
        {
            linkSuccess = m_dataReferenceLinkManager.createDataLink(providerSceneId, providerSlotHandle, consumerSceneId, consumerSlotHandle);
        }
        else if (providerSlotType == EDataSlotType_TextureProvider && consumerSlotType == EDataSlotType_TextureConsumer)
        {
            linkSuccess = m_textureLinkManager.createDataLink(providerSceneId, providerSlotHandle, consumerSceneId, consumerSlotHandle);
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "Renderer::createDataLink failed: data slot type mismatch, provider and consumer slot types must match! (Provider scene: " << providerSceneId.getValue() << ", consumer scene: " << consumerSceneId << ")");
        }

        m_rendererEventCollector.addEvent((linkSuccess ? ERendererEventType_SceneDataLinked : ERendererEventType_SceneDataLinkFailed), providerSceneId, consumerSceneId, providerId, consumerId);
    }

    void SceneLinksManager::createBufferLink(OffscreenBufferHandle providerBuffer, SceneId consumerSceneId, DataSlotId consumerId)
    {
        if (!m_rendererScenes.hasScene(consumerSceneId))
        {
            LOG_ERROR(CONTEXT_RENDERER, "Renderer::createBufferLink failed: consumer scene " << consumerSceneId << " is not known to the renderer!");
            m_rendererEventCollector.addEvent(ERendererEventType_SceneDataBufferLinkFailed, providerBuffer, consumerSceneId, consumerId);
            return;
        }

        const DataSlotHandle consumerSlotHandle = DataLinkUtils::GetDataSlotHandle(consumerSceneId, consumerId, m_rendererScenes);
        if (!consumerSlotHandle.isValid())
        {
            LOG_ERROR(CONTEXT_RENDERER, "Renderer::createBufferLink failed: consumer data id is invalid! (consumer scene: " << consumerSceneId << ")");
            m_rendererEventCollector.addEvent(ERendererEventType_SceneDataBufferLinkFailed, providerBuffer, consumerSceneId, consumerId);
            return;
        }

        const EDataSlotType consumerSlotType = DataLinkUtils::GetDataSlot(consumerSceneId, consumerSlotHandle, m_rendererScenes).type;
        if (consumerSlotType != EDataSlotType_TextureConsumer)
        {
            LOG_ERROR(CONTEXT_RENDERER, "Renderer::createBufferLink failed: consumer data " << consumerId << " refers to a slot that is not of texture type! (consumer scene: " << consumerSceneId << ")");
            m_rendererEventCollector.addEvent(ERendererEventType_SceneDataBufferLinkFailed, providerBuffer, consumerSceneId, consumerId);
            return;
        }

        const Bool linkSuccess = m_textureLinkManager.createBufferLink(providerBuffer, consumerSceneId, consumerSlotHandle);

        m_rendererEventCollector.addEvent((linkSuccess ? ERendererEventType_SceneDataBufferLinked : ERendererEventType_SceneDataBufferLinkFailed), providerBuffer, consumerSceneId, consumerId);
    }

    void SceneLinksManager::removeDataLink(SceneId consumerSceneId, DataSlotId consumerId)
    {
        if (!m_rendererScenes.hasScene(consumerSceneId))
        {
            LOG_ERROR(CONTEXT_RENDERER, "Renderer::createDataLink failed: consumer scene " << consumerSceneId << " is not known to the renderer!");
            m_rendererEventCollector.addEvent(ERendererEventType_SceneDataUnlinkFailed, SceneId(0u), consumerSceneId, DataSlotId(0u), consumerId);
            return;
        }

        const DataSlotHandle consumerSlotHandle = DataLinkUtils::GetDataSlotHandle(consumerSceneId, consumerId, m_rendererScenes);

        if (!consumerSlotHandle.isValid())
        {
            LOG_ERROR(CONTEXT_RENDERER, "Renderer::removeDataLink failed: consumer data id " << consumerId << " is invalid!");
            m_rendererEventCollector.addEvent(ERendererEventType_SceneDataUnlinkFailed, SceneId(0u), consumerSceneId, DataSlotId(0u), consumerId);
            return;
        }

        const EDataSlotType consumerSlotType = DataLinkUtils::GetDataSlot(consumerSceneId, consumerSlotHandle, m_rendererScenes).type;
        Bool unlinkSuccess = false;

        if (consumerSlotType == EDataSlotType_TransformationConsumer)
        {
            unlinkSuccess = m_transformationLinkManager.removeDataLink(consumerSceneId, consumerSlotHandle);
        }
        else if (consumerSlotType == EDataSlotType_DataConsumer)
        {
            unlinkSuccess = m_dataReferenceLinkManager.removeDataLink(consumerSceneId, consumerSlotHandle);
        }
        else if (consumerSlotType == EDataSlotType_TextureConsumer)
        {
            unlinkSuccess = m_textureLinkManager.removeDataLink(consumerSceneId, consumerSlotHandle);
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "Renderer::removeDataLink failed: given slot " << consumerSlotHandle << " is not a consumer slot or there was no link!");
            m_rendererEventCollector.addEvent(ERendererEventType_SceneDataUnlinkFailed, SceneId(0u), consumerSceneId, DataSlotId(0u), consumerId);
            return;
        }

        m_rendererEventCollector.addEvent((unlinkSuccess ? ERendererEventType_SceneDataUnlinked : ERendererEventType_SceneDataUnlinkFailed), SceneId(0u), consumerSceneId, DataSlotId(0u), consumerId);
    }

    void SceneLinksManager::handleSceneRemoved(SceneId sceneId)
    {
        m_transformationLinkManager.removeSceneLinks(sceneId);
        m_dataReferenceLinkManager.removeSceneLinks(sceneId);
        m_textureLinkManager.removeSceneLinks(sceneId);
    }

    void SceneLinksManager::handleSceneUnmapped(SceneId sceneId)
    {
        m_textureLinkManager.removeSceneLinks(sceneId);
    }

    void SceneLinksManager::handleDataSlotCreated(SceneId sceneId, DataSlotHandle dataSlotHandle)
    {
        const IScene& scene = m_rendererScenes.getScene(sceneId);
        const DataSlot& dataSlot = scene.getDataSlot(dataSlotHandle);

        switch (dataSlot.type)
        {
        case EDataSlotType_DataProvider:
        case EDataSlotType_TransformationProvider:
        case EDataSlotType_TextureProvider:
            m_rendererEventCollector.addEvent(ERendererEventType_SceneDataSlotProviderCreated, sceneId, SceneId(0), dataSlot.id, DataSlotId(0));
            break;
        case EDataSlotType_DataConsumer:
        case EDataSlotType_TransformationConsumer:
        case EDataSlotType_TextureConsumer:
            m_rendererEventCollector.addEvent(ERendererEventType_SceneDataSlotConsumerCreated, SceneId(0), sceneId, DataSlotId(0), dataSlot.id);
            break;
        default:
            assert(false);
            break;
        }
    }

    void SceneLinksManager::handleDataSlotDestroyed(SceneId sceneId, DataSlotHandle dataSlotHandle)
    {
        const IScene& scene = m_rendererScenes.getScene(sceneId);
        const DataSlot& dataSlot = scene.getDataSlot(dataSlotHandle);

        switch (dataSlot.type)
        {
        case EDataSlotType_TransformationProvider:
            removeLinksToProvider(sceneId, dataSlotHandle, m_transformationLinkManager);
            m_rendererEventCollector.addEvent(ERendererEventType_SceneDataSlotProviderDestroyed, sceneId, SceneId(0), dataSlot.id, DataSlotId(0));
            break;

        case EDataSlotType_TransformationConsumer:
            if (m_transformationLinkManager.getSceneLinks().hasLinkedProvider(sceneId, dataSlotHandle))
            {
                if (m_transformationLinkManager.removeDataLink(sceneId, dataSlotHandle))
                {
                    m_rendererEventCollector.addEvent(ERendererEventType_SceneDataUnlinkedAsResultOfClientSceneChange, SceneId(0u), sceneId, DataSlotId(0u), dataSlot.id);
                }
            }
            m_rendererEventCollector.addEvent(ERendererEventType_SceneDataSlotConsumerDestroyed, SceneId(0), sceneId, DataSlotId(0), dataSlot.id);
            break;

        case EDataSlotType_DataProvider:
            removeLinksToProvider(sceneId, dataSlotHandle, m_dataReferenceLinkManager);
            m_rendererEventCollector.addEvent(ERendererEventType_SceneDataSlotProviderDestroyed, sceneId, SceneId(0), dataSlot.id, DataSlotId(0));
            break;

        case EDataSlotType_DataConsumer:
            if (m_dataReferenceLinkManager.getSceneLinks().hasLinkedProvider(sceneId, dataSlotHandle))
            {
                if (m_dataReferenceLinkManager.removeDataLink(sceneId, dataSlotHandle))
                {
                    m_rendererEventCollector.addEvent(ERendererEventType_SceneDataUnlinkedAsResultOfClientSceneChange, SceneId(0u), sceneId, DataSlotId(0u), dataSlot.id);
                }
            }
            m_rendererEventCollector.addEvent(ERendererEventType_SceneDataSlotConsumerDestroyed, SceneId(0), sceneId, DataSlotId(0), dataSlot.id);
            break;

        case EDataSlotType_TextureProvider:
            removeLinksToProvider(sceneId, dataSlotHandle, m_textureLinkManager);
            m_rendererEventCollector.addEvent(ERendererEventType_SceneDataSlotProviderDestroyed, sceneId, SceneId(0), dataSlot.id, DataSlotId(0));
            break;

        case EDataSlotType_TextureConsumer:
            if (m_textureLinkManager.getSceneLinks().hasLinkedProvider(sceneId, dataSlotHandle) ||
                m_textureLinkManager.getOffscreenBufferLinks().hasLinkedProvider(sceneId, dataSlotHandle))
            {
                if (m_textureLinkManager.removeDataLink(sceneId, dataSlotHandle))
                {
                    m_rendererEventCollector.addEvent(ERendererEventType_SceneDataUnlinkedAsResultOfClientSceneChange, SceneId(0u), sceneId, DataSlotId(0u), dataSlot.id);
                }
            }
            m_rendererEventCollector.addEvent(ERendererEventType_SceneDataSlotConsumerDestroyed, SceneId(0), sceneId, DataSlotId(0), dataSlot.id);
            break;

        default:
            assert(false);
            break;
        }
    }

    void SceneLinksManager::handleBufferDestroyed(OffscreenBufferHandle providerBuffer)
    {
        OffscreenBufferLinkVector links;
        m_textureLinkManager.getOffscreenBufferLinks().getLinkedConsumers(providerBuffer, links);
        ramses_foreach(links, linkIt)
        {
            assert(linkIt->providerBuffer == providerBuffer);
            m_textureLinkManager.removeDataLink(linkIt->consumerSceneId, linkIt->consumerSlot);
        }
    }

    template <typename LINKMANAGER>
    void SceneLinksManager::removeLinksToProvider(SceneId sceneId, DataSlotHandle providerSlotHandle, LINKMANAGER& manager) const
    {
        const SceneLinks& sceneLinks = manager.getSceneLinks();
        SceneLinkVector links;
        sceneLinks.getLinkedConsumers(sceneId, providerSlotHandle, links);

        ramses_foreach(links, linkIt)
        {
            assert(linkIt->providerSceneId == sceneId);
            assert(linkIt->providerSlot == providerSlotHandle);
            if (manager.removeDataLink(linkIt->consumerSceneId, linkIt->consumerSlot))
            {
                const IScene& consumerScene = m_rendererScenes.getScene(linkIt->consumerSceneId);
                const DataSlotId consumerId = consumerScene.getDataSlot(linkIt->consumerSlot).id;
                m_rendererEventCollector.addEvent(ERendererEventType_SceneDataUnlinkedAsResultOfClientSceneChange, SceneId(0u), linkIt->consumerSceneId, DataSlotId(0u), consumerId);
            }
        }
    }

    const TransformationLinkManager& SceneLinksManager::getTransformationLinkManager() const
    {
        return m_transformationLinkManager;
    }

    const DataReferenceLinkManager& SceneLinksManager::getDataReferenceLinkManager() const
    {
        return m_dataReferenceLinkManager;
    }

    const TextureLinkManager& SceneLinksManager::getTextureLinkManager() const
    {
        return m_textureLinkManager;
    }
}
