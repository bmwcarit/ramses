//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/BufferLinks.h"

namespace ramses_internal
{
    template <typename BUFFERHANDLE>
    void BufferLinks<BUFFERHANDLE>::addLink(BUFFERHANDLE providerBuffer, SceneId consumerSceneId, DataSlotHandle consumerSlotHandle)
    {
        assert(!hasLinkedProvider(consumerSceneId, consumerSlotHandle));

        BufferLink<BUFFERHANDLE> link;
        link.providerBuffer = providerBuffer;
        link.consumerSceneId = consumerSceneId;
        link.consumerSlot = consumerSlotHandle;

        m_links.push_back(link);
    }

    template <typename BUFFERHANDLE>
    void BufferLinks<BUFFERHANDLE>::removeLink(SceneId consumerSceneId, DataSlotHandle consumerSlotHandle)
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

    template <typename BUFFERHANDLE>
    Bool BufferLinks<BUFFERHANDLE>::hasAnyLinksToProvider(SceneId consumerSceneId) const
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

    template <typename BUFFERHANDLE>
    Bool BufferLinks<BUFFERHANDLE>::hasAnyLinksToConsumer(BUFFERHANDLE providerBuffer) const
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

    template <typename BUFFERHANDLE>
    Bool BufferLinks<BUFFERHANDLE>::hasLinkedProvider(SceneId consumerSceneId, DataSlotHandle consumerSlotHandle) const
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

    template <typename BUFFERHANDLE>
    void BufferLinks<BUFFERHANDLE>::getLinkedProviders(SceneId consumerSceneId, BufferLinkVector<BUFFERHANDLE>& links) const
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

    template <typename BUFFERHANDLE>
    const BufferLink<BUFFERHANDLE>& BufferLinks<BUFFERHANDLE>::getLinkedProvider(SceneId consumerSceneId, DataSlotHandle consumerSlotHandle) const
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

    template <typename BUFFERHANDLE>
    void BufferLinks<BUFFERHANDLE>::getLinkedConsumers(BUFFERHANDLE providerBuffer, BufferLinkVector<BUFFERHANDLE>& links) const
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

    template class BufferLinks<OffscreenBufferHandle>;
    template class BufferLinks<StreamBufferHandle>;
    template class BufferLinks<ExternalBufferHandle>;
}

