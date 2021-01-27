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
#include "SceneAPI/WaylandIviSurfaceId.h"

namespace ramses
{
    class Appearance;
    class ArrayResource;
}

namespace ramses_internal
{
    class EmbeddedCompositorScene : public IntegrationScene
    {
    public:
        EmbeddedCompositorScene(ramses::Scene& scene, UInt32 state, const Vector3& cameraPosition, uint32_t vpWidth = IntegrationScene::DefaultViewportWidth, uint32_t vpHeight = IntegrationScene::DefaultViewportHeight);

        enum
        {
            SINGLE_STREAM_TEXTURE = 0,
            SINGLE_STREAM_TEXTURE_WITH_TEXCOORDS_OFFSET,
            SINGLE_STREAM_TEXTURE_WITH_TEXEL_FETCH,
            TWO_STREAM_TEXTURES_WITH_SAME_SOURCE_ID_AND_DIFFERENT_FALLBACK_TEXTURES,
            TWO_STREAM_TEXTURES_WITH_DIFFERENT_SOURCE_ID_AND_SAME_FALLBACK_TEXTURE,
            TWO_STREAM_TEXTURES_WITH_DIFFERENT_SOURCE_ID_AND_SWIZZLED_FALLBACK_TEXTURES,
            SINGLE_STREAM_TEXTURE_ON_THE_LEFT,
            SINGLE_STREAM_TEXTURE_ON_THE_RIGHT_WITH_FALLBACK_FROM_LEFT_SCENE,
            SINGLE_STREAM_TEXTURE_ON_THE_RIGHT,
            SINGLE_STREAM_TEXTURE_ON_THE_RIGHT_WITH_SECOND_SOURCE_ID_AND_FALLBACK_FROM_LEFT_SCENE
        };

        static WaylandIviSurfaceId GetStreamTextureSourceId();
        static WaylandIviSurfaceId GetSecondStreamTextureSourceId();
        static WaylandIviSurfaceId GetThirdStreamTextureSourceId();

        static constexpr ramses::dataConsumerId_t SamplerConsumerId1{ 1u };
        static constexpr ramses::dataConsumerId_t SamplerConsumerId2{ 2u };

    private:
        void createQuad(float x, float y, float w, float h, ramses::Appearance& appearance);
        ramses::Appearance& createAppearanceWithStreamTexture(ramses::Scene& scene, ramses::waylandIviSurfaceId_t sourceId, ramses::dataConsumerId_t consumerId, const ramses::Texture2D& fallbackTexture);
        const ramses::Effect& createTestEffect(UInt32 state);
        const ramses::ArrayResource& createTextureCoordinates(UInt32 state);
        void createQuadWithStreamTexture(float xPos, float yPos, float width, float height, ramses::waylandIviSurfaceId_t sourceId, ramses::dataConsumerId_t consumerId, const ramses::Texture2D& fallbackTexture);

        static const WaylandIviSurfaceId EmbeddedSurfaceStreamTextureSourceId;
        const ramses::Effect& m_effect;
        const ramses::ArrayResource& m_textureCoords;
    };
}

#endif
