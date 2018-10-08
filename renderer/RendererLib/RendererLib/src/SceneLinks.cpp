//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/SceneLinks.h"
#include "Common/Cpp11Macros.h"

namespace ramses_internal
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

    Bool SceneLinks::hasAnyLinksToProvider(SceneId consumerSceneId) const
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

    Bool SceneLinks::hasAnyLinksToConsumer(SceneId providerSceneId) const
    {
        ramses_foreach(m_links, linkIt)
        {
            if (linkIt->providerSceneId == providerSceneId)
            {
                return true;
            }
        }

        return false;
    }

    Bool SceneLinks::hasLinkedProvider(SceneId consumerSceneId, DataSlotHandle consumerSlotHandle) const
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

    void SceneLinks::getLinkedProviders(SceneId consumerSceneId, SceneLinkVector& links) const
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

    const SceneLink& SceneLinks::getLinkedProvider(SceneId consumerSceneId, DataSlotHandle consumerSlotHandle) const
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

    Bool SceneLinks::hasLinkedConsumers(SceneId providerSceneId, DataSlotHandle providerSlotHandle) const
    {
        ramses_foreach(m_links, linkIt)
        {
            if (linkIt->providerSceneId == providerSceneId && linkIt->providerSlot == providerSlotHandle)
            {
                return true;
            }
        }

        return false;
    }

    void SceneLinks::getLinkedConsumers(SceneId providerSceneId, SceneLinkVector& links) const
    {
        assert(links.empty());
        ramses_foreach(m_links, linkIt)
        {
            if (linkIt->providerSceneId == providerSceneId)
            {
                links.push_back(*linkIt);
            }
        }
    }

    void SceneLinks::getLinkedConsumers(SceneId providerSceneId, DataSlotHandle providerSlotHandle, SceneLinkVector& links) const
    {
        assert(links.empty());
        ramses_foreach(m_links, linkIt)
        {
            if (linkIt->providerSceneId == providerSceneId && linkIt->providerSlot == providerSlotHandle)
            {
                links.push_back(*linkIt);
            }
        }
    }
}
