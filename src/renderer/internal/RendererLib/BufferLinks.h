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
#include "internal/RendererLib/Types.h"
#include "internal/PlatformAbstraction/Collections/Vector.h"

namespace ramses::internal
{
    template <typename BUFFERHANDLE>
    struct BufferLink
    {
        BUFFERHANDLE   providerBuffer;
        SceneId        consumerSceneId;
        DataSlotHandle consumerSlot;
    };
    template <typename BUFFERHANDLE>
    using BufferLinkVector = std::vector<BufferLink<BUFFERHANDLE>>;

    template <typename BUFFERHANDLE>
    class BufferLinks
    {
    public:
        void                       addLink(BUFFERHANDLE providerBuffer, SceneId consumerSceneId, DataSlotHandle consumerSlotHandle);
        void                       removeLink(SceneId consumerSceneId, DataSlotHandle consumerSlotHandle);

        [[nodiscard]] bool                       hasAnyLinksToProvider(SceneId consumerSceneId) const;
        [[nodiscard]] bool                       hasAnyLinksToConsumer(BUFFERHANDLE providerBuffer) const;

        [[nodiscard]] bool                       hasLinkedProvider(SceneId consumerSceneId, DataSlotHandle consumerSlotHandle) const;
        void                       getLinkedProviders(SceneId consumerSceneId, BufferLinkVector<BUFFERHANDLE>& links) const;
        [[nodiscard]] const BufferLink<BUFFERHANDLE>& getLinkedProvider(SceneId consumerSceneId, DataSlotHandle consumerSlotHandle) const;

        void                       getLinkedConsumers(BUFFERHANDLE providerBuffer, BufferLinkVector<BUFFERHANDLE>& links) const;

    private:
        BufferLinkVector<BUFFERHANDLE> m_links;
    };

    using OffscreenBufferLink = BufferLink<OffscreenBufferHandle>;
    using OffscreenBufferLinkVector = BufferLinkVector<OffscreenBufferHandle>;
    using OffscreenBufferLinks = BufferLinks<OffscreenBufferHandle>;

    using StreamBufferLink = BufferLink<StreamBufferHandle>;
    using StreamBufferLinkVector = BufferLinkVector<StreamBufferHandle>;
    using StreamBufferLinks = BufferLinks<StreamBufferHandle>;

    using ExternalBufferLinks = BufferLinks<ExternalBufferHandle>;
    using ExternalBufferLinkVector = BufferLinkVector<ExternalBufferHandle>;
}
