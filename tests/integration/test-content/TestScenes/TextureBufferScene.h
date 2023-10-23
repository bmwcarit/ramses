//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "IntegrationScene.h"

namespace ramses
{
    class Effect;
    class Appearance;
    class Texture2D;
    class Texture2DBuffer;
    class Geometry;
}

namespace ramses::internal
{
    class TextureBufferScene : public IntegrationScene
    {
    public:
        TextureBufferScene(ramses::Scene& scene, uint32_t state, const glm::vec3& cameraPosition, uint32_t vpWidth = IntegrationScene::DefaultViewportWidth, uint32_t vpHeight = IntegrationScene::DefaultViewportHeight);

        enum EState
        {
            EState_RGBA8_OneMip = 0,
            EState_RGBA8_OneMip_ScaledDown,
            EState_RGBA8_ThreeMips,
            EState_PartialUpdate,
            EState_PartialUpdate1,
            EState_PartialUpdate2,
            EState_PartialUpdateMipMap,
            EState_PartialUpdateMipMap_RG8,
            EState_ClientTextureResource_RGBA8,
            EState_SwitchBackToClientTexture,
            EState_SwitchBackToExistingTextureBufferAndUpdate
        };

        void setState(uint32_t state);

    private:
        void setOrthoCamera(const glm::vec3& cameraPosition);

        ramses::MeshNode& m_quadMesh;
        ramses::Effect& m_effectSingleMip;
        ramses::Effect& m_effectAllMips;
        ramses::Appearance& m_appearanceSingleMip;
        ramses::Appearance& m_appearanceAllMips;
        ramses::Geometry& m_geometrySingleMip;
        ramses::Geometry& m_geometryAllMips;
        ramses::Texture2D* m_clientTexture = nullptr;
        ramses::Texture2DBuffer* m_textureBuffer = nullptr;
    };
}
