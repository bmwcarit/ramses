//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TEXTURELINKCACHEDSCENE_H
#define RAMSES_TEXTURELINKCACHEDSCENE_H

#include "RendererLib/DataReferenceLinkCachedScene.h"
#include "RendererAPI/Types.h"

namespace ramses_internal
{
    class TextureLinkCachedScene : public DataReferenceLinkCachedScene
    {
    public:
        explicit TextureLinkCachedScene(SceneLinksManager& sceneLinksManager, const SceneInfo& sceneInfo = SceneInfo());

        // From IScene
        DataSlotHandle      allocateDataSlot(const DataSlot& dataSlot, DataSlotHandle handle = DataSlotHandle::Invalid()) override;
        void                releaseDataSlot(DataSlotHandle handle) override;

        void setDataSlotTexture(DataSlotHandle handle, const ResourceContentHash& texture) override;

        void setTextureSamplerContentSource(TextureSamplerHandle sampler, const ResourceContentHash& hash);
        void setTextureSamplerContentSource(TextureSamplerHandle sampler, OffscreenBufferHandle offscreenBuffer);
        void setTextureSamplerContentSource(TextureSamplerHandle sampler, StreamBufferHandle streamBuffer);
        void setTextureSamplerContentSource(TextureSamplerHandle sampler, ExternalBufferHandle externalTexture);
        void restoreTextureSamplerFallbackValue(TextureSamplerHandle sampler);
        [[nodiscard]] const TextureSampler& getFallbackTextureSampler(TextureSamplerHandle sampler) const;

    protected:
        HashMap<TextureSamplerHandle, TextureSampler> m_fallbackTextureSamplers;
    };
}

#endif
