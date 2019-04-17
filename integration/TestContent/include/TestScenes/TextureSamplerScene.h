//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TEXTURESAMPLERSCENE_H
#define RAMSES_TEXTURESAMPLERSCENE_H

#include "IntegrationScene.h"

namespace ramses
{
    class TextureSampler;
}

namespace ramses_internal
{
    class TextureSamplerScene : public IntegrationScene
    {
    public:
        TextureSamplerScene(ramses::RamsesClient& ramsesClient, ramses::Scene& scene, uint32_t state, const Vector3& cameraPosition);

        void setState(UInt32 state);

        enum EState
        {
            EState_ClientTexture = 0,
            EState_TextureBuffer,
            EState_SetClientTexture,
            EState_SetTextureBuffer,
            EState_SetRenderBuffer,
            EState_SetStreamTexture
        };

    protected:
        ramses::TextureSampler* m_sampler = nullptr;
    };
}

#endif
