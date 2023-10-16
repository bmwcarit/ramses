//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "IntegrationScene.h"

namespace ramses
{
    class TextureSampler;
    class Effect;
    class Appearance;
}

namespace ramses::internal
{
    class TextureSamplerScene : public IntegrationScene
    {
    public:
        TextureSamplerScene(ramses::Scene& scene, uint32_t state, const glm::vec3& cameraPosition);

        void setState(uint32_t state);

        enum EState
        {
            EState_ClientTexture = 0,
            EState_TextureBuffer,
            EState_SetClientTexture,
            EState_SetTextureBuffer,
            EState_SetRenderBuffer,
            EState_NoTextureSampler,
            EState_SetTextureSampler,
        };

    protected:
        ramses::TextureSampler* m_sampler = nullptr;
        const ramses::Effect* m_effect = nullptr;
        ramses::Appearance* m_appearance = nullptr;
    };
}
