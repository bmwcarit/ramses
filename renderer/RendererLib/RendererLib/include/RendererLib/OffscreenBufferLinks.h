//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_OFFSCREENBUFFERLINKS_H
#define RAMSES_OFFSCREENBUFFERLINKS_H

#include "SceneAPI/SceneId.h"
#include "SceneAPI/Handles.h"
#include "RendererAPI/Types.h"
#include "Collections/Vector.h"

namespace ramses_internal
{
    struct OffscreenBufferLink
    {
        OffscreenBufferHandle providerBuffer;
        SceneId               consumerSceneId;
        DataSlotHandle        consumerSlot;
    };
    typedef std::vector<OffscreenBufferLink> OffscreenBufferLinkVector;

    class OffscreenBufferLinks
    {
    public:
        void                       addLink(OffscreenBufferHandle providerBuffer, SceneId consumerSceneId, DataSlotHandle consumerSlotHandle);
        void                       removeLink(SceneId consumerSceneId, DataSlotHandle consumerSlotHandle);

        Bool                       hasAnyLinksToProvider(SceneId consumerSceneId) const;
        Bool                       hasAnyLinksToConsumer(OffscreenBufferHandle providerBuffer) const;

        Bool                       hasLinkedProvider(SceneId consumerSceneId, DataSlotHandle consumerSlotHandle) const;
        void                       getLinkedProviders(SceneId consumerSceneId, OffscreenBufferLinkVector& links) const;
        const OffscreenBufferLink& getLinkedProvider(SceneId consumerSceneId, DataSlotHandle consumerSlotHandle) const;

        void                       getLinkedConsumers(OffscreenBufferHandle providerBuffer, OffscreenBufferLinkVector& links) const;

    private:
        OffscreenBufferLinkVector m_links;
    };
}

#endif
