//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/SceneAPI/SceneId.h"
#include "internal/SceneGraph/SceneAPI/Handles.h"
#include "internal/PlatformAbstraction/Collections/Vector.h"

namespace ramses::internal
{
    struct SceneLink
    {
        SceneId        providerSceneId;
        DataSlotHandle providerSlot;
        SceneId        consumerSceneId;
        DataSlotHandle consumerSlot;
    };
    using SceneLinkVector = std::vector<SceneLink>;

    class SceneLinks
    {
    public:
        void                       addLink(SceneId providerSceneId, DataSlotHandle providerSlotHandle, SceneId consumerSceneId, DataSlotHandle consumerSlotHandle);
        void                       removeLink(SceneId consumerSceneId, DataSlotHandle consumerSlotHandle);

        [[nodiscard]] bool                       hasAnyLinksToProvider(SceneId consumerSceneId) const;
        [[nodiscard]] bool                       hasAnyLinksToConsumer(SceneId providerSceneId) const;

        [[nodiscard]] bool                       hasLinkedProvider(SceneId consumerSceneId, DataSlotHandle consumerSlotHandle) const;
        void                       getLinkedProviders(SceneId consumerSceneId, SceneLinkVector& links) const;
        [[nodiscard]] const SceneLink&           getLinkedProvider(SceneId consumerSceneId, DataSlotHandle consumerSlotHandle) const;

        [[nodiscard]] bool                       hasLinkedConsumers(SceneId providerSceneId, DataSlotHandle providerSlotHandle) const;
        void                       getLinkedConsumers(SceneId providerSceneId, SceneLinkVector& links) const;
        void                       getLinkedConsumers(SceneId providerSceneId, DataSlotHandle providerSlotHandle, SceneLinkVector& links) const;

    private:
        // querying links via vector search should be better for small number of links (expected typical case)
        // in case of larger amount of links, storing of links in hashmaps in both directions might be better
        SceneLinkVector m_links;
    };
}
