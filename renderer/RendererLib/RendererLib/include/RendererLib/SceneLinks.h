//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENELINKS_H
#define RAMSES_SCENELINKS_H

#include "SceneAPI/SceneId.h"
#include "SceneAPI/Handles.h"
#include "Collections/Vector.h"

namespace ramses_internal
{
    struct SceneLink
    {
        SceneId        providerSceneId;
        DataSlotHandle providerSlot;
        SceneId        consumerSceneId;
        DataSlotHandle consumerSlot;
    };
    typedef std::vector<SceneLink> SceneLinkVector;

    class SceneLinks
    {
    public:
        void                       addLink(SceneId providerSceneId, DataSlotHandle providerSlotHandle, SceneId consumerSceneId, DataSlotHandle consumerSlotHandle);
        void                       removeLink(SceneId consumerSceneId, DataSlotHandle consumerSlotHandle);

        Bool                       hasAnyLinksToProvider(SceneId consumerSceneId) const;
        Bool                       hasAnyLinksToConsumer(SceneId providerSceneId) const;

        Bool                       hasLinkedProvider(SceneId consumerSceneId, DataSlotHandle consumerSlotHandle) const;
        void                       getLinkedProviders(SceneId consumerSceneId, SceneLinkVector& links) const;
        const SceneLink&           getLinkedProvider(SceneId consumerSceneId, DataSlotHandle consumerSlotHandle) const;

        Bool                       hasLinkedConsumers(SceneId providerSceneId, DataSlotHandle providerSlotHandle) const;
        void                       getLinkedConsumers(SceneId providerSceneId, SceneLinkVector& links) const;
        void                       getLinkedConsumers(SceneId providerSceneId, DataSlotHandle providerSlotHandle, SceneLinkVector& links) const;

    private:
        // querying links via vector search should be better for small number of links (expected typical case)
        // in case of larger amount of links, storing of links in hashmaps in both directions might be better
        SceneLinkVector m_links;
    };
}

#endif
