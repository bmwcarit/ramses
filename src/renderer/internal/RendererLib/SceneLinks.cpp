//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/RendererLib/SceneLinks.h"

namespace ramses::internal
{
    void SceneLinks::addLink(SceneId providerSceneId, DataSlotHandle providerSlotHandle, SceneId consumerSceneId, DataSlotHandle consumerSlotHandle)
    {
        assert(!hasLinkedProvider(consumerSceneId, consumerSlotHandle));

        SceneLink link;
        link.providerSceneId = providerSceneId;
        link.providerSlot = providerSlotHandle;
        link.consumerSceneId = consumerSceneId;
        link.consumerSlot = consumerSlotHandle;

        m_links.push_back(link);
    }

    void SceneLinks::removeLink(SceneId consumerSceneId, DataSlotHandle consumerSlotHandle)
    {
        for (auto link = m_links.begin(); link != m_links.end(); ++link)
        {
            if (link->consumerSceneId == consumerSceneId && link->consumerSlot == consumerSlotHandle)
            {
                m_links.erase(link);
                return;
            }
        }

        assert(false && "tried to remove non-existent scene link");
    }

    bool SceneLinks::hasAnyLinksToProvider(SceneId consumerSceneId) const
    {
        for(const auto& link : m_links)
        {
            if (link.consumerSceneId == consumerSceneId)
            {
                return true;
            }
        }

        return false;
    }

    bool SceneLinks::hasAnyLinksToConsumer(SceneId providerSceneId) const
    {
        for(const auto& link : m_links)
        {
            if (link.providerSceneId == providerSceneId)
            {
                return true;
            }
        }

        return false;
    }

    bool SceneLinks::hasLinkedProvider(SceneId consumerSceneId, DataSlotHandle consumerSlotHandle) const
    {
        for(const auto& link : m_links)
        {
            if (link.consumerSceneId == consumerSceneId && link.consumerSlot == consumerSlotHandle)
            {
                return true;
            }
        }

        return false;
    }

    void SceneLinks::getLinkedProviders(SceneId consumerSceneId, SceneLinkVector& links) const
    {
        assert(links.empty());
        for(const auto& link : m_links)
        {
            if (link.consumerSceneId == consumerSceneId)
            {
                links.push_back(link);
            }
        }
    }

    const SceneLink& SceneLinks::getLinkedProvider(SceneId consumerSceneId, DataSlotHandle consumerSlotHandle) const
    {
        for(const auto& link : m_links)
        {
            if (link.consumerSceneId == consumerSceneId && link.consumerSlot == consumerSlotHandle)
            {
                return link;
            }
        }

        assert(false && "tried to get non-existent linked provider");
        return m_links.front();
    }

    bool SceneLinks::hasLinkedConsumers(SceneId providerSceneId, DataSlotHandle providerSlotHandle) const
    {
        for(const auto& link : m_links)
        {
            if (link.providerSceneId == providerSceneId && link.providerSlot == providerSlotHandle)
            {
                return true;
            }
        }

        return false;
    }

    void SceneLinks::getLinkedConsumers(SceneId providerSceneId, SceneLinkVector& links) const
    {
        assert(links.empty());
        for(const auto& link : m_links)
        {
            if (link.providerSceneId == providerSceneId)
            {
                links.push_back(link);
            }
        }
    }

    void SceneLinks::getLinkedConsumers(SceneId providerSceneId, DataSlotHandle providerSlotHandle, SceneLinkVector& links) const
    {
        assert(links.empty());
        for(const auto& link : m_links)
        {
            if (link.providerSceneId == providerSceneId && link.providerSlot == providerSlotHandle)
            {
                links.push_back(link);
            }
        }
    }
}
