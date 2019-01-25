//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/LinkManagerBase.h"
#include "RendererLib/RendererScenes.h"
#include "RendererLib/DataLinkUtils.h"
#include "Utils/LogMacros.h"

namespace ramses_internal
{
    LinkManagerBase::LinkManagerBase(RendererScenes& rendererScenes)
        : m_scenes(rendererScenes)
    {
    }

    void LinkManagerBase::removeSceneLinks(SceneId sceneId)
    {
        SceneLinkVector links;
        m_sceneLinks.getLinkedConsumers(sceneId, links);
        for(const auto& link : links)
        {
            assert(link.providerSceneId == sceneId);
            m_sceneLinks.removeLink(link.consumerSceneId, link.consumerSlot);
        }

        links.clear();
        m_sceneLinks.getLinkedProviders(sceneId, links);
        for (const auto& link : links)
        {
            assert(link.consumerSceneId == sceneId);
            m_sceneLinks.removeLink(link.consumerSceneId, link.consumerSlot);
        }

        m_dependencyChecker.removeScene(sceneId);
    }

    Bool LinkManagerBase::createDataLink(SceneId providerSceneId, DataSlotHandle providerSlotHandle, SceneId consumerSceneId, DataSlotHandle consumerSlotHandle)
    {
        assert(EDataSlotType_DataProvider == DataLinkUtils::GetDataSlot(providerSceneId, providerSlotHandle, m_scenes).type
            || EDataSlotType_TransformationProvider == DataLinkUtils::GetDataSlot(providerSceneId, providerSlotHandle, m_scenes).type
            || EDataSlotType_TextureProvider == DataLinkUtils::GetDataSlot(providerSceneId, providerSlotHandle, m_scenes).type);
        assert(EDataSlotType_DataConsumer == DataLinkUtils::GetDataSlot(consumerSceneId, consumerSlotHandle, m_scenes).type
            || EDataSlotType_TransformationConsumer == DataLinkUtils::GetDataSlot(consumerSceneId, consumerSlotHandle, m_scenes).type
            || EDataSlotType_TextureConsumer == DataLinkUtils::GetDataSlot(consumerSceneId, consumerSlotHandle, m_scenes).type);

        if (m_sceneLinks.hasLinkedProvider(consumerSceneId, consumerSlotHandle))
        {
            LOG_ERROR(CONTEXT_RENDERER, "Renderer::createDataLink failed: consumer slot (Scene: " << consumerSceneId.getValue() << ") already has a data link assigned!");
            return false;
        }

        if (!m_dependencyChecker.addDependency(providerSceneId, consumerSceneId))
        {
            LOG_ERROR(CONTEXT_RENDERER, "Renderer::createDataLink failed: cyclic link dependency detected! (Provider scene: " << providerSceneId.getValue() << ", consumer scene: " << consumerSceneId.getValue() << ")");
            return false;
        }

        m_sceneLinks.addLink(providerSceneId, providerSlotHandle, consumerSceneId, consumerSlotHandle);

        return true;
    }

    Bool LinkManagerBase::removeDataLink(SceneId consumerSceneId, DataSlotHandle consumerSlotHandle)
    {
        assert(EDataSlotType_DataConsumer == DataLinkUtils::GetDataSlot(consumerSceneId, consumerSlotHandle, m_scenes).type
            || EDataSlotType_TransformationConsumer == DataLinkUtils::GetDataSlot(consumerSceneId, consumerSlotHandle, m_scenes).type
            || EDataSlotType_TextureConsumer == DataLinkUtils::GetDataSlot(consumerSceneId, consumerSlotHandle, m_scenes).type);

        if (!m_sceneLinks.hasLinkedProvider(consumerSceneId, consumerSlotHandle))
        {
            LOG_ERROR(CONTEXT_RENDERER, "Renderer::removeDataLink failed: consumer slot is not linked!  (Consumer scene: " << consumerSceneId.getValue() << ")");
            return false;
        }

        const SceneLink& link = m_sceneLinks.getLinkedProvider(consumerSceneId, consumerSlotHandle);
        assert(link.consumerSceneId == consumerSceneId && link.consumerSlot == consumerSlotHandle);
        m_dependencyChecker.removeDependency(link.providerSceneId, consumerSceneId);

        m_sceneLinks.removeLink(consumerSceneId, consumerSlotHandle);

        return true;
    }

    const SceneDependencyChecker& LinkManagerBase::getDependencyChecker() const
    {
        return m_dependencyChecker;
    }

    const SceneLinks& LinkManagerBase::getSceneLinks() const
    {
        return m_sceneLinks;
    }
}
