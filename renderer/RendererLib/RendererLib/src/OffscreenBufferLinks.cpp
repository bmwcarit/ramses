//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/OffscreenBufferLinks.h"
#include "Common/Cpp11Macros.h"

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
        ramses_foreach(m_links, linkIt)
        {
            if (linkIt->consumerSceneId == consumerSceneId && linkIt->consumerSlot == consumerSlotHandle)
            {
                m_links.erase(linkIt);
                return;
            }
        }

        assert(false && "tried to remove non-existent scene link");
    }

    Bool OffscreenBufferLinks::hasAnyLinksToProvider(SceneId consumerSceneId) const
    {
        ramses_foreach(m_links, linkIt)
        {
            if (linkIt->consumerSceneId == consumerSceneId)
            {
                return true;
            }
        }

        return false;
    }

    Bool OffscreenBufferLinks::hasAnyLinksToConsumer(OffscreenBufferHandle providerBuffer) const
    {
        ramses_foreach(m_links, linkIt)
        {
            if (linkIt->providerBuffer == providerBuffer)
            {
                return true;
            }
        }

        return false;
    }

    Bool OffscreenBufferLinks::hasLinkedProvider(SceneId consumerSceneId, DataSlotHandle consumerSlotHandle) const
    {
        ramses_foreach(m_links, linkIt)
        {
            if (linkIt->consumerSceneId == consumerSceneId && linkIt->consumerSlot == consumerSlotHandle)
            {
                return true;
            }
        }

        return false;
    }

    void OffscreenBufferLinks::getLinkedProviders(SceneId consumerSceneId, OffscreenBufferLinkVector& links) const
    {
        assert(links.empty());
        ramses_foreach(m_links, linkIt)
        {
            if (linkIt->consumerSceneId == consumerSceneId)
            {
                links.push_back(*linkIt);
            }
        }
    }

    const OffscreenBufferLink& OffscreenBufferLinks::getLinkedProvider(SceneId consumerSceneId, DataSlotHandle consumerSlotHandle) const
    {
        ramses_foreach(m_links, linkIt)
        {
            if (linkIt->consumerSceneId == consumerSceneId && linkIt->consumerSlot == consumerSlotHandle)
            {
                return *linkIt;
            }
        }

        assert(false && "tried to get non-existent linked provider");
        return m_links.front();
    }

    void OffscreenBufferLinks::getLinkedConsumers(OffscreenBufferHandle providerBuffer, OffscreenBufferLinkVector& links) const
    {
        assert(links.empty());
        ramses_foreach(m_links, linkIt)
        {
            if (linkIt->providerBuffer == providerBuffer)
            {
                links.push_back(*linkIt);
            }
        }
    }
}
