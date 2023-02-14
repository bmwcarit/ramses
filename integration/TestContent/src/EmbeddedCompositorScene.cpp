//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/EmbeddedCompositorScene.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/ArrayResource.h"
#include "ramses-client-api/GeometryBinding.h"
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/Effect.h"
#include "ramses-client-api/AttributeInput.h"
#include "ramses-client-api/UniformInput.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/EffectDescription.h"
#include "ramses-client-api/StreamTexture.h"
#include "ramses-client-api/Texture2D.h"
#include "ramses-utils.h"
#include "RamsesObjectTypeUtils.h"

namespace ramses_internal
{
    EmbeddedCompositorScene::EmbeddedCompositorScene(ramses::Scene& scene, UInt32 state, const Vector3& cameraPosition, uint32_t vpWidth, uint32_t vpHeight)
        : IntegrationScene(scene, cameraPosition, vpWidth, vpHeight)
        , m_effect(createTestEffect(state))
        , m_textureCoords(createTextureCoordinates(state))
    {
        const ramses::waylandIviSurfaceId_t streamSource1(GetStreamTextureSourceId().getValue());
        const ramses::waylandIviSurfaceId_t streamSource2(GetSecondStreamTextureSourceId().getValue());
        const ramses::Texture2D* fallbackTexture1 = ramses::RamsesUtils::CreateTextureResourceFromPng("res/ramses-test-client-embedded-compositing-1.png", m_scene);
        const ramses::Texture2D* fallbackTexture2 = ramses::RamsesUtils::CreateTextureResourceFromPng("res/ramses-test-client-embedded-compositing-2.png", m_scene);

        switch (state)
        {
        case SINGLE_STREAM_TEXTURE:
        case SINGLE_STREAM_TEXTURE_WITH_TEXCOORDS_OFFSET:
        case SINGLE_STREAM_TEXTURE_WITH_TEXEL_FETCH:
        {
            createQuadWithStreamTexture(-1.0f, -1.0f, 2.0f, 2.0f, streamSource1, SamplerConsumerId1, *fallbackTexture1);
            break;
        }
        case TWO_STREAM_TEXTURES_WITH_SAME_SOURCE_ID_AND_DIFFERENT_FALLBACK_TEXTURES:
        {
            createQuadWithStreamTexture(-2.0f, -1.0f, 2.0f, 2.0f, streamSource1, SamplerConsumerId1, *fallbackTexture1);
            createQuadWithStreamTexture(0.0f, -1.0f, 2.0f, 2.0f, streamSource1, SamplerConsumerId2, *fallbackTexture2);
            break;
        }
        case TWO_STREAM_TEXTURES_WITH_DIFFERENT_SOURCE_ID_AND_SAME_FALLBACK_TEXTURE:
        {
            createQuadWithStreamTexture(-2.0f, -1.0f, 2.0f, 2.0f, streamSource1, SamplerConsumerId1, *fallbackTexture1);
            createQuadWithStreamTexture(0.0f, -1.0f, 2.0f, 2.0f, streamSource2, SamplerConsumerId2, *fallbackTexture1);
            break;
        }
        case TWO_STREAM_TEXTURES_WITH_DIFFERENT_SOURCE_ID_AND_SWIZZLED_FALLBACK_TEXTURES:
        {
            //use same png with different swizzle for fallbacks
            const ramses::TextureSwizzle leftTextureSwizzle { ramses::ETextureChannelColor::Green, ramses::ETextureChannelColor::One, ramses::ETextureChannelColor::Red, ramses::ETextureChannelColor::Alpha };
            const ramses::TextureSwizzle rightTextureSwizzle { ramses::ETextureChannelColor::Green, ramses::ETextureChannelColor::Blue, ramses::ETextureChannelColor::Red, ramses::ETextureChannelColor::Alpha };
            ramses::Texture2D* leftSceneFallbackTexture = ramses::RamsesUtils::CreateTextureResourceFromPng("res/ramses-test-client-embedded-compositing-1.png", m_scene, leftTextureSwizzle, "leftSceneFallbackTexture");
            ramses::Texture2D* rightSceneFallbackTexture = ramses::RamsesUtils::CreateTextureResourceFromPng("res/ramses-test-client-embedded-compositing-1.png", m_scene, rightTextureSwizzle, "leftSceneFallbackTexture");

            createQuadWithStreamTexture(-2.0f, -1.0f, 2.0f, 2.0f, streamSource1, SamplerConsumerId1, *leftSceneFallbackTexture);
            createQuadWithStreamTexture(0.0f, -1.0f, 2.0f, 2.0f, streamSource2, SamplerConsumerId2, *rightSceneFallbackTexture);
            break;
        }
        case SINGLE_STREAM_TEXTURE_ON_THE_LEFT:
        {
            ramses::Texture2D* leftSceneFallbackTexture = ramses::RamsesUtils::CreateTextureResourceFromPng("res/ramses-test-client-embedded-compositing-1.png", m_scene, {}, "leftSceneFallbackTexture");
            createQuadWithStreamTexture(-2.0f, -1.0f, 2.0f, 2.0f, streamSource1, SamplerConsumerId1, *leftSceneFallbackTexture);
            break;
        }
        case SINGLE_STREAM_TEXTURE_ON_THE_RIGHT_WITH_FALLBACK_FROM_LEFT_SCENE:
        {
            ramses::Texture2D* leftSceneFallbackTexture = ramses::RamsesUtils::CreateTextureResourceFromPng("res/ramses-test-client-embedded-compositing-1.png", m_scene, {}, "leftSceneFallbackTexture");
            createQuadWithStreamTexture(0.0f, -1.0f, 2.0f, 2.0f, streamSource1, SamplerConsumerId1, *leftSceneFallbackTexture);
            break;
        }
        case SINGLE_STREAM_TEXTURE_ON_THE_RIGHT:
        {
            createQuadWithStreamTexture(0.0f, -1.0f, 2.0f, 2.0f, streamSource1, SamplerConsumerId1, *fallbackTexture2);
            break;
        }
        case SINGLE_STREAM_TEXTURE_ON_THE_RIGHT_WITH_SECOND_SOURCE_ID_AND_FALLBACK_FROM_LEFT_SCENE:
        {
            ramses::Texture2D* leftSceneFallbackTexture = ramses::RamsesUtils::CreateTextureResourceFromPng("res/ramses-test-client-embedded-compositing-1.png", m_scene, {}, "leftSceneFallbackTexture");
            createQuadWithStreamTexture(0.0f, -1.0f, 2.0f, 2.0f, streamSource2, SamplerConsumerId1, *leftSceneFallbackTexture);
            break;
        }
        }
    }

    WaylandIviSurfaceId EmbeddedCompositorScene::GetStreamTextureSourceId()
    {
        return EmbeddedSurfaceStreamTextureSourceId;
    }

    WaylandIviSurfaceId EmbeddedCompositorScene::GetSecondStreamTextureSourceId()
    {
        return WaylandIviSurfaceId(EmbeddedSurfaceStreamTextureSourceId.getValue() + 1u);
    }

    WaylandIviSurfaceId EmbeddedCompositorScene::GetThirdStreamTextureSourceId()
    {
        return WaylandIviSurfaceId(EmbeddedSurfaceStreamTextureSourceId.getValue() + 2u);
    }

    void EmbeddedCompositorScene::createQuad(float x, float y, float w, float h, ramses::Appearance& appearance)
    {
        const float vertexPositionsArray[] =
        {
            x, y, -8.0f,
            x, y + h, -8.0f,
            x + w, y + h, -8.0f,
            x + w, y, -8.0f
        };
        const ramses::ArrayResource* vertexPositions = m_scene.createArrayResource(ramses::EDataType::Vector3F, 4, vertexPositionsArray);

        const uint16_t indicesArray[] =
        {
            0, 2, 1, //ABC
            0, 3, 2  //ACD
        };
        const ramses::ArrayResource* indices = m_scene.createArrayResource(ramses::EDataType::UInt16, 6, indicesArray);

        // set vertex positions directly in geometry
        ramses::GeometryBinding* geometry = m_scene.createGeometryBinding(appearance.getEffect(), "triangle geometry");
        geometry->setIndices(*indices);
        ramses::AttributeInput positionsInput;
        ramses::AttributeInput texcoordsInput;
        appearance.getEffect().findAttributeInput("a_position", positionsInput);
        appearance.getEffect().findAttributeInput("a_texcoord", texcoordsInput);
        geometry->setInputBuffer(positionsInput, *vertexPositions);
        geometry->setInputBuffer(texcoordsInput, m_textureCoords);

        // create a mesh node to define the triangle with chosen appearance
        ramses::MeshNode* meshNode = m_scene.createMeshNode("textured triangle mesh node");
        meshNode->setAppearance(appearance);
        meshNode->setGeometryBinding(*geometry);
        // mesh needs to be added to a render group that belongs to a render pass with camera in order to be rendered
        addMeshNodeToDefaultRenderGroup(*meshNode);
    }

    ramses::Appearance& EmbeddedCompositorScene::createAppearanceWithStreamTexture(ramses::Scene& scene, ramses::waylandIviSurfaceId_t sourceId, ramses::dataConsumerId_t consumerId, const ramses::Texture2D& fallbackTexture)
    {
        ramses::StreamTexture* streamTexture = scene.createStreamTexture(fallbackTexture, sourceId);
        const ramses::TextureSampler* sampler = scene.createTextureSampler(
                    ramses::ETextureAddressMode_Repeat,
                    ramses::ETextureAddressMode_Repeat,
                    ramses::ETextureSamplingMethod_Nearest,
                    ramses::ETextureSamplingMethod_Nearest,
                    *streamTexture);
        ramses::Appearance* appearance = scene.createAppearance(m_effect, "triangle appearance");
        scene.createTextureConsumer(*sampler, consumerId);

        ramses::UniformInput textureInput;
        m_effect.findUniformInput("u_texture", textureInput);
        appearance->setInputTexture(textureInput, *sampler);

        return *appearance;
    }

    const ramses::Effect& EmbeddedCompositorScene::createTestEffect(UInt32 state)
    {
        ramses::EffectDescription effectDesc;

        switch (state)
        {
        case SINGLE_STREAM_TEXTURE:
        case SINGLE_STREAM_TEXTURE_WITH_TEXCOORDS_OFFSET:
        case TWO_STREAM_TEXTURES_WITH_SAME_SOURCE_ID_AND_DIFFERENT_FALLBACK_TEXTURES:
        case TWO_STREAM_TEXTURES_WITH_DIFFERENT_SOURCE_ID_AND_SAME_FALLBACK_TEXTURE:
        case TWO_STREAM_TEXTURES_WITH_DIFFERENT_SOURCE_ID_AND_SWIZZLED_FALLBACK_TEXTURES:
        case SINGLE_STREAM_TEXTURE_ON_THE_LEFT:
        case SINGLE_STREAM_TEXTURE_ON_THE_RIGHT_WITH_FALLBACK_FROM_LEFT_SCENE:
        case SINGLE_STREAM_TEXTURE_ON_THE_RIGHT:
        case SINGLE_STREAM_TEXTURE_ON_THE_RIGHT_WITH_SECOND_SOURCE_ID_AND_FALLBACK_FROM_LEFT_SCENE:
            effectDesc.setVertexShaderFromFile("res/ramses-test-client-textured.vert");
            effectDesc.setFragmentShaderFromFile("res/ramses-test-client-textured.frag");
            break;
        case SINGLE_STREAM_TEXTURE_WITH_TEXEL_FETCH:
            effectDesc.setVertexShaderFromFile("res/ramses-test-client-textured-with-texel-fetch.vert");
            effectDesc.setFragmentShaderFromFile("res/ramses-test-client-textured-with-texel-fetch.frag");
            break;
        }

        effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic::ModelViewProjectionMatrix);

        const ramses::Effect* effect = m_scene.createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "glsl shader");
        assert(nullptr != effect);

        return *effect;
    }

    const ramses::ArrayResource& EmbeddedCompositorScene::createTextureCoordinates(UInt32 state)
    {
        switch(state)
        {
        case SINGLE_STREAM_TEXTURE_WITH_TEXCOORDS_OFFSET:
        {
            const float textureCoordsArray[] =
            {
                0.25f, 2.f, //A   A-----D
                0.25f, -1.f, //B   |     |
                .75f,  -1.f, //C   |     |
                .75f,  2.f  //D   B-----C
            };
            return *m_scene.createArrayResource(ramses::EDataType::Vector2F, 4, textureCoordsArray);
        }
        default:
        {
            const float textureCoordsArray[] =
            {
                0.f, 1.f, //A   A-----D
                0.f, 0.f, //B   |     |
                1.f, 0.f, //C   |     |
                1.f, 1.f  //D   B-----C
            };
            return *m_scene.createArrayResource(ramses::EDataType::Vector2F, 4, textureCoordsArray);
        }
        }
    }

    void EmbeddedCompositorScene::createQuadWithStreamTexture(float xPos, float yPos, float width, float height, ramses::waylandIviSurfaceId_t sourceId, ramses::dataConsumerId_t consumerId, const ramses::Texture2D& fallbackTexture)
    {
        ramses::Appearance& appearance = createAppearanceWithStreamTexture(m_scene, sourceId, consumerId, fallbackTexture);
        createQuad(xPos, yPos, width, height, appearance);
    }
}
