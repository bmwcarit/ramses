//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_EMBEDDEDCOMPOSITORSCENE_H
#define RAMSES_EMBEDDEDCOMPOSITORSCENE_H

#include "IntegrationScene.h"
#include "RendererAPI/Types.h"
#include "ramses-client-api/Scene.h"

namespace ramses
{
    class Appearance;
    class Vector2fArray;
}

namespace ramses_internal
{
    class EmbeddedCompositorScene : public IntegrationScene
    {
    public:
        EmbeddedCompositorScene(ramses::RamsesClient& ramsesClient, ramses::Scene& scene, UInt32 state, const Vector3& cameraPosition);

        enum
        {
            SINGLE_STREAM_TEXTURE = 0,
            SINGLE_STREAM_TEXTURE_WITH_TEXCOORDS_OFFSET,
            SINGLE_STREAM_TEXTURE_WITH_TEXEL_FETCH,
            TWO_STREAM_TEXTURES_WITH_SAME_SOURCE_ID_AND_DIFFERENT_FALLBACK_TEXTURES,
            TWO_STREAM_TEXTURES_WITH_DIFFERENT_SOURCE_ID_AND_SAME_FALLBACK_TEXTURE,
            SINGLE_STREAM_TEXTURE_ON_THE_LEFT,
            SINGLE_STREAM_TEXTURE_ON_THE_RIGHT_WITH_FALLBACK_FROM_LEFT_SCENE,
            SINGLE_STREAM_TEXTURE_ON_THE_RIGHT,
            SINGLE_STREAM_TEXTURE_ON_THE_RIGHT_WITH_SECOND_SOURCE_ID_AND_FALLBACK_FROM_LEFT_SCENE
        };

        static StreamTextureSourceId GetStreamTextureSourceId();
        static StreamTextureSourceId GetSecondStreamTextureSourceId();
        static StreamTextureSourceId GetThirdStreamTextureSourceId();

    private:
        void createQuad(float x, float y, float w, float h, ramses::Appearance& appearance);
        ramses::Appearance& createAppearanceWithStreamTexture(ramses::Scene& scene, ramses::streamSource_t sourceId, const ramses::Texture2D& fallbackTexture);
        const ramses::Effect& createTestEffect(UInt32 state);
        const ramses::Vector2fArray& createTextureCoordinates(UInt32 state);
        void createQuadWithStreamTexture(float xPos, float yPos, float width, float height, ramses::streamSource_t sourceId, const ramses::Texture2D& fallbackTexture);

        static const StreamTextureSourceId EmbeddedSurfaceStreamTextureSourceId;
        const ramses::Effect& m_effect;
        const ramses::Vector2fArray& m_textureCoords;
    };
}

#endif
