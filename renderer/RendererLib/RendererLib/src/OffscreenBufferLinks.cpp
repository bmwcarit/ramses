//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/OffscreenBufferLinks.h"

namespace ramses_internal
{
    void OffscreenBufferLinks::addLink(OffscreenBufferHandle providerBuffer, SceneId consumerSceneId, DataSlotHandle consumerSlotHandle)
    {
        assert(!hasLinkedProvider(consumerSceneId, consumerSlotHandle));

        OffscreenBufferLink link;
        link.providerBuffer = providerBuffer;
        link.consumerSceneId = consumerSceneId;
        link.consumerSlot = consumerSlotHandle;

        m_links.push_back(link);
    }

    void OffscreenBufferLinks::removeLink(SceneId consumerSceneId, DataSlotHandle consumerSlotHandle)
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

    Bool OffscreenBufferLinks::hasAnyLinksToProvider(SceneId consumerSceneId) const
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

    Bool OffscreenBufferLinks::hasAnyLinksToConsumer(OffscreenBufferHandle providerBuffer) const
    {
        for(const auto& link : m_links)
        {
            if (link.providerBuffer == providerBuffer)
            {
                return true;
            }
        }

        return false;
    }

    Bool OffscreenBufferLinks::hasLinkedProvider(SceneId consumerSceneId, DataSlotHandle consumerSlotHandle) const
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

    void OffscreenBufferLinks::getLinkedProviders(SceneId consumerSceneId, OffscreenBufferLinkVector& links) const
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

    const OffscreenBufferLink& OffscreenBufferLinks::getLinkedProvider(SceneId consumerSceneId, DataSlotHandle consumerSlotHandle) const
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

    void OffscreenBufferLinks::getLinkedConsumers(OffscreenBufferHandle providerBuffer, OffscreenBufferLinkVector& links) const
    {
        assert(links.empty());
        for(const auto& link : m_links)
        {
            if (link.providerBuffer == providerBuffer)
            {
                links.push_back(link);
            }
        }
    }
}
