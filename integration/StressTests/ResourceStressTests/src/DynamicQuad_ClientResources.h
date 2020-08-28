//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DYNAMICQUAD_CLIENTRESOURCES_H
#define RAMSES_DYNAMICQUAD_CLIENTRESOURCES_H

#include "DynamicQuad_Base.h"

namespace ramses
{
    class Texture2D;
    class TextureSampler;
}

namespace ramses_internal
{
    struct TextureResources
    {
        ramses::Texture2D* texture = nullptr;
        ramses::TextureSampler* textureSampler = nullptr;
    };

    class DynamicQuad_ClientResources : public DynamicQuad_Base
    {
    public:
        DynamicQuad_ClientResources(ramses::Scene& scene, const ScreenspaceQuad& screenspaceQuad);
        ~DynamicQuad_ClientResources();

        virtual void recreate() override final;

        // Needed for proper clean-up upon destruction (destroy scene -> mark objects destroyed -> destroy client resources)
        void markSceneObjectsDestroyed();

        void createTextureDataConsumer(ramses::dataConsumerId_t textureSlotId);

    private:
        void                    destroyTexture(const TextureResources& texture);
        TextureResources        createRandomizedTexture();

        TextureResources m_textureResources;
    };
}

#endif
