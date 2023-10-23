//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "IntegrationScene.h"
#include "ramses/client/UniformInput.h"
#include "ramses/framework/TextureEnums.h"
#include "ramses/client/MipLevelData.h"
#include "internal/PlatformAbstraction/Collections/Vector.h"

namespace ramses
{
    class Appearance;
    class Effect;
    class ArrayResource;
}

namespace ramses::internal
{
    class Texture2DAnisotropicTextureFilteringScene : public IntegrationScene
    {
    public:
        Texture2DAnisotropicTextureFilteringScene(ramses::Scene& scene, uint32_t state, const glm::vec3& cameraPosition);

        enum EState
        {
            EState_Anisotropic = 0
        };

    protected:
        void createOrthoCamera();

        ramses::Appearance* createQuad(
            ramses::Effect* effect,
            const ramses::ArrayResource* vertexPositions,
            const ramses::ArrayResource* textureCoords,
            const ramses::ArrayResource* indices,
            float x,
            float y);
    };
}
