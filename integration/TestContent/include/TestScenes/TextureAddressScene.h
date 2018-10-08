//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TEXTUREADDRESSSCENE_H
#define RAMSES_TEXTUREADDRESSSCENE_H

#include "IntegrationScene.h"
#include "ramses-client-api/TextureEnums.h"

namespace ramses
{
    class GroupNode;
    class UInt16Array;
    class Effect;
    class Vector3fArray;
    class Vector2fArray;
    class Texture2D;
}

namespace ramses_internal
{
    class TextureAddressScene : public IntegrationScene
    {
    public:
        TextureAddressScene(ramses::RamsesClient& ramsesClient, ramses::Scene& scene, UInt32 state, const Vector3& cameraPosition);

        enum
        {
            ADDRESS_MODE_STATE = 0
        };

    private:
        void createQuad(
            Float x,
            Float y,
            ramses::ETextureAddressMode addressMethodU,
            ramses::ETextureAddressMode addressMethodV);

        ramses::GroupNode* m_groupNode;
        const ramses::UInt16Array* m_indices;
        ramses::Effect* m_effect;
        const ramses::Vector3fArray* m_vertexPositions;
        const ramses::Vector2fArray* m_textureCoords;
        ramses::Texture2D* m_texture;
    };
}

#endif
