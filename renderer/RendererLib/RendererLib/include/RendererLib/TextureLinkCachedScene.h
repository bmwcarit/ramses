//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TEXTURELINKCACHEDSCENE_H
#define RAMSES_TEXTURELINKCACHEDSCENE_H

#include "RendererLib/ResourceCachedScene.h"

namespace ramses_internal
{
    class TextureLinkCachedScene : public ResourceCachedScene
    {
    public:
        explicit TextureLinkCachedScene(SceneLinksManager& sceneLinksManager, const SceneInfo& sceneInfo = SceneInfo());

        // From IScene
        virtual DataSlotHandle      allocateDataSlot(const DataSlot& dataSlot, DataSlotHandle handle = DataSlotHandle::Invalid()) override;
        virtual void                releaseDataSlot(DataSlotHandle handle) override;

        virtual void setDataSlotTexture(DataSlotHandle handle, const ResourceContentHash& texture) override;

        void setTextureSamplerContentSource(TextureSamplerHandle sampler, const ResourceContentHash& hash);
        void setTextureSamplerContentSource(TextureSamplerHandle sampler, OffscreenBufferHandle offscreenBuffer);
        void restoreTextureSamplerFallbackValue(TextureSamplerHandle sampler);

    protected:
        HashMap<TextureSamplerHandle, TextureSampler> m_fallbackTextureSamplers;
    };
}

#endif
