//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DYNAMICQUAD_RESOURCES_H
#define RAMSES_DYNAMICQUAD_RESOURCES_H

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

    class DynamicQuad_Resources : public DynamicQuad_Base
    {
    public:
        DynamicQuad_Resources(ramses::Scene& scene, const ScreenspaceQuad& screenspaceQuad);
        ~DynamicQuad_Resources() override;

        void recreate() final override;

        // Needed for proper clean-up upon destruction (destroy scene -> mark objects destroyed -> destroy resources)
        void markSceneObjectsDestroyed();

        void createTextureDataConsumer(ramses::dataConsumerId_t textureSlotId);

    private:
        void                    destroyTexture(const TextureResources& texture);
        TextureResources        createRandomizedTexture();

        TextureResources m_textureResources;
    };
}

#endif
