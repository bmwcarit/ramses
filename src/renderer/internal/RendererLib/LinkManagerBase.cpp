//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/RendererLib/LinkManagerBase.h"
#include "internal/RendererLib/RendererScenes.h"
#include "internal/RendererLib/DataLinkUtils.h"
#include "internal/Core/Utils/ThreadLocalLogForced.h"

namespace ramses::internal
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

    bool LinkManagerBase::createDataLink(SceneId providerSceneId, DataSlotHandle providerSlotHandle, SceneId consumerSceneId, DataSlotHandle consumerSlotHandle)
    {
        assert(EDataSlotType::DataProvider == DataLinkUtils::GetDataSlot(providerSceneId, providerSlotHandle, m_scenes).type
            || EDataSlotType::TransformationProvider == DataLinkUtils::GetDataSlot(providerSceneId, providerSlotHandle, m_scenes).type
            || EDataSlotType::TextureProvider == DataLinkUtils::GetDataSlot(providerSceneId, providerSlotHandle, m_scenes).type);
        assert(EDataSlotType::DataConsumer == DataLinkUtils::GetDataSlot(consumerSceneId, consumerSlotHandle, m_scenes).type
            || EDataSlotType::TransformationConsumer == DataLinkUtils::GetDataSlot(consumerSceneId, consumerSlotHandle, m_scenes).type
            || EDataSlotType::TextureConsumer == DataLinkUtils::GetDataSlot(consumerSceneId, consumerSlotHandle, m_scenes).type);

        if (m_sceneLinks.hasLinkedProvider(consumerSceneId, consumerSlotHandle))
        {
            LOG_ERROR(CONTEXT_RENDERER, "LinkManagerBase::createDataLink failed: consumer slot (Scene: " << consumerSceneId << ") already has a data link assigned!");
            return false;
        }

        if (!m_dependencyChecker.addDependency(providerSceneId, consumerSceneId))
        {
            LOG_ERROR(CONTEXT_RENDERER, "LinkManagerBase::createDataLink failed: cyclic link dependency detected! (Provider scene: " << providerSceneId << ", consumer scene: " << consumerSceneId << ")");
            return false;
        }

        m_sceneLinks.addLink(providerSceneId, providerSlotHandle, consumerSceneId, consumerSlotHandle);

        return true;
    }

    bool LinkManagerBase::removeDataLink(SceneId consumerSceneId, DataSlotHandle consumerSlotHandle)
    {
        assert(EDataSlotType::DataConsumer == DataLinkUtils::GetDataSlot(consumerSceneId, consumerSlotHandle, m_scenes).type
            || EDataSlotType::TransformationConsumer == DataLinkUtils::GetDataSlot(consumerSceneId, consumerSlotHandle, m_scenes).type
            || EDataSlotType::TextureConsumer == DataLinkUtils::GetDataSlot(consumerSceneId, consumerSlotHandle, m_scenes).type);

        if (!m_sceneLinks.hasLinkedProvider(consumerSceneId, consumerSlotHandle))
        {
            LOG_ERROR(CONTEXT_RENDERER, "LinkManagerBase::removeDataLink failed: consumer slot is not linked!  (Consumer scene: " << consumerSceneId << ")");
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
