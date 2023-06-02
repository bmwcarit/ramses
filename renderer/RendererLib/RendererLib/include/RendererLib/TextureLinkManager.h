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
#include "SceneAPI/ResourceContentHash.h"

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
        void createBufferLink(ExternalBufferHandle providerBuffer, SceneId consumerSceneId, DataSlotHandle consumerSlotHandle);

        bool removeDataLink(SceneId consumerSceneId, DataSlotHandle consumerSlotHandle, SceneId* providerSceneIdOut = nullptr);

        [[nodiscard]] bool                hasLinkedTexture(SceneId consumerSceneId, TextureSamplerHandle sampler) const;
        [[nodiscard]] ResourceContentHash getLinkedTexture(SceneId consumerSceneId, TextureSamplerHandle sampler) const;

        [[nodiscard]] bool                  hasLinkedOffscreenBuffer(SceneId consumerSceneId, TextureSamplerHandle sampler) const;
        [[nodiscard]] OffscreenBufferHandle getLinkedOffscreenBuffer(SceneId consumerSceneId, TextureSamplerHandle sampler) const;
        [[nodiscard]] bool                  hasLinkedStreamBuffer(SceneId consumerSceneId, TextureSamplerHandle sampler) const;
        [[nodiscard]] StreamBufferHandle    getLinkedStreamBuffer(SceneId consumerSceneId, TextureSamplerHandle sampler) const;
        [[nodiscard]] bool                  hasLinkedExternalBuffer(SceneId consumerSceneId, TextureSamplerHandle sampler) const;
        [[nodiscard]] ExternalBufferHandle  getLinkedExternalBuffer(SceneId consumerSceneId, TextureSamplerHandle sampler) const;

        using LinkManagerBase::getDependencyChecker;
        using LinkManagerBase::getSceneLinks;

        [[nodiscard]] const OffscreenBufferLinks& getOffscreenBufferLinks() const;
        [[nodiscard]] const StreamBufferLinks& getStreamBufferLinks() const;
        [[nodiscard]] const ExternalBufferLinks& getExternalBufferLinks() const;

        void setTextureToConsumers(SceneId providerSceneId, DataSlotHandle providerSlotHandle, ResourceContentHash texture) const;

    private:
        template <typename BUFFERHANDLE>
        void createBufferLinkInternal(BUFFERHANDLE providerBuffer, SceneId consumerSceneId, DataSlotHandle consumerSlotHandle, BufferLinks<BUFFERHANDLE>& links);
        TextureSamplerHandle storeConsumerSlot(SceneId consumerSceneId, DataSlotHandle consumerSlotHandle);
        [[nodiscard]] DataSlotHandle getDataSlotForSampler(SceneId sceneId, TextureSamplerHandle sampler) const;

        using SamplerToSlotMap = HashMap<TextureSamplerHandle, DataSlotHandle>;
        using SceneToSamplerSlotMap = HashMap<SceneId, SamplerToSlotMap>;
        SceneToSamplerSlotMap m_samplersToDataSlots;

        OffscreenBufferLinks m_offscreenBufferLinks;
        StreamBufferLinks m_streamBufferLinks;
        ExternalBufferLinks m_externalBufferLinks;
    };
}

#endif
