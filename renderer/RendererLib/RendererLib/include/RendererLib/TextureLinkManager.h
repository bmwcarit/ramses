//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TEXTURELINKMANAGER_H
#define RAMSES_TEXTURELINKMANAGER_H

#include "RendererLib/LinkManagerBase.h"
#include "RendererLib/BufferLinks.h"

namespace ramses_internal
{
    class RendererScenes;

    class TextureLinkManager : private LinkManagerBase
    {
    public:
        explicit TextureLinkManager(RendererScenes& rendererScenes);

        void removeSceneLinks(SceneId sceneId);

        bool createDataLink(SceneId providerSceneId, DataSlotHandle providerSlotHandle, SceneId consumerSceneId, DataSlotHandle consumerSlotHandle);
        void createBufferLink(OffscreenBufferHandle providerBuffer, SceneId consumerSceneId, DataSlotHandle consumerSlotHandle);
        void createBufferLink(StreamBufferHandle providerBuffer, SceneId consumerSceneId, DataSlotHandle consumerSlotHandle);
        bool removeDataLink(SceneId consumerSceneId, DataSlotHandle consumerSlotHandle, SceneId* providerSceneIdOut = nullptr);

        bool                hasLinkedTexture(SceneId consumerSceneId, TextureSamplerHandle sampler) const;
        ResourceContentHash getLinkedTexture(SceneId consumerSceneId, TextureSamplerHandle sampler) const;

        bool                  hasLinkedOffscreenBuffer(SceneId consumerSceneId, TextureSamplerHandle sampler) const;
        OffscreenBufferHandle getLinkedOffscreenBuffer(SceneId consumerSceneId, TextureSamplerHandle sampler) const;
        bool                  hasLinkedStreamBuffer(SceneId consumerSceneId, TextureSamplerHandle sampler) const;
        StreamBufferHandle    getLinkedStreamBuffer(SceneId consumerSceneId, TextureSamplerHandle sampler) const;

        using LinkManagerBase::getDependencyChecker;
        using LinkManagerBase::getSceneLinks;

        const OffscreenBufferLinks& getOffscreenBufferLinks() const;
        const StreamBufferLinks& getStreamBufferLinks() const;

        void setTextureToConsumers(SceneId providerSceneId, DataSlotHandle providerSlotHandle, ResourceContentHash texture) const;

    private:
        template <typename BUFFERHANDLE>
        void createBufferLinkInternal(BUFFERHANDLE providerBuffer, SceneId consumerSceneId, DataSlotHandle consumerSlotHandle, BufferLinks<BUFFERHANDLE>& links);
        TextureSamplerHandle storeConsumerSlot(SceneId consumerSceneId, DataSlotHandle consumerSlotHandle);
        DataSlotHandle getDataSlotForSampler(SceneId sceneId, TextureSamplerHandle sampler) const;

        using SamplerToSlotMap = HashMap<TextureSamplerHandle, DataSlotHandle>;
        using SceneToSamplerSlotMap = HashMap<SceneId, SamplerToSlotMap>;
        SceneToSamplerSlotMap m_samplersToDataSlots;

        OffscreenBufferLinks m_offscreenBufferLinks;
        StreamBufferLinks m_streamBufferLinks;
    };
}

#endif
