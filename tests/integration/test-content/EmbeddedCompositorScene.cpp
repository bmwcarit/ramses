//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/EmbeddedCompositorScene.h"
#include "ramses/client/Scene.h"
#include "ramses/client/ArrayResource.h"
#include "ramses/client/Geometry.h"
#include "ramses/client/Appearance.h"
#include "ramses/client/Effect.h"
#include "ramses/client/AttributeInput.h"
#include "ramses/client/UniformInput.h"
#include "ramses/client/MeshNode.h"
#include "ramses/client/EffectDescription.h"
#include "ramses/client/Texture2D.h"
#include "ramses/client/ramses-utils.h"
#include "impl/RamsesObjectTypeUtils.h"

namespace ramses::internal
{
    EmbeddedCompositorScene::EmbeddedCompositorScene(ramses::Scene& scene, uint32_t state, const glm::vec3& cameraPosition, uint32_t vpWidth, uint32_t vpHeight)
        : IntegrationScene(scene, cameraPosition, vpWidth, vpHeight)
        , m_effect(createTestEffect(state))
        , m_textureCoords(createTextureCoordinates(state))
    {
        const ramses::Texture2D* fallbackTexture1 = ramses::RamsesUtils::CreateTextureResourceFromPng("res/ramses-test-client-embedded-compositing-1.png", m_scene);
        const ramses::Texture2D* fallbackTexture2 = ramses::RamsesUtils::CreateTextureResourceFromPng("res/ramses-test-client-embedded-compositing-2.png", m_scene);

        switch (state)
        {
        case SINGLE_STREAM_TEXTURE:
        case SINGLE_STREAM_TEXTURE_WITH_TEXCOORDS_OFFSET:
        case SINGLE_STREAM_TEXTURE_WITH_TEXEL_FETCH:
        {
            createQuadWithTextureConsumer(-1.0f, -1.0f, 2.0f, 2.0f, SamplerConsumerId1, *fallbackTexture1);
            break;
        }
        case TWO_STREAM_TEXTURES_WITH_SAME_SOURCE_ID_AND_DIFFERENT_FALLBACK_TEXTURES:
        {
            createQuadWithTextureConsumer(-2.0f, -1.0f, 2.0f, 2.0f, SamplerConsumerId1, *fallbackTexture1);
            createQuadWithTextureConsumer(0.0f, -1.0f, 2.0f, 2.0f, SamplerConsumerId2, *fallbackTexture2);
            break;
        }
        case TWO_STREAM_TEXTURES_WITH_DIFFERENT_SOURCE_ID_AND_SAME_FALLBACK_TEXTURE:
        {
            createQuadWithTextureConsumer(-2.0f, -1.0f, 2.0f, 2.0f, SamplerConsumerId1, *fallbackTexture1);
            createQuadWithTextureConsumer(0.0f, -1.0f, 2.0f, 2.0f, SamplerConsumerId2, *fallbackTexture1);
            break;
        }
        case TWO_STREAM_TEXTURES_WITH_DIFFERENT_SOURCE_ID_AND_SWIZZLED_FALLBACK_TEXTURES:
        {
            //use same png with different swizzle for fallbacks
            const ramses::TextureSwizzle leftTextureSwizzle { ramses::ETextureChannelColor::Green, ramses::ETextureChannelColor::One, ramses::ETextureChannelColor::Red, ramses::ETextureChannelColor::Alpha };
            const ramses::TextureSwizzle rightTextureSwizzle { ramses::ETextureChannelColor::Green, ramses::ETextureChannelColor::Blue, ramses::ETextureChannelColor::Red, ramses::ETextureChannelColor::Alpha };
            ramses::Texture2D* leftSceneFallbackTexture = ramses::RamsesUtils::CreateTextureResourceFromPng("res/ramses-test-client-embedded-compositing-1.png", m_scene, leftTextureSwizzle, "leftSceneFallbackTexture");
            ramses::Texture2D* rightSceneFallbackTexture = ramses::RamsesUtils::CreateTextureResourceFromPng("res/ramses-test-client-embedded-compositing-1.png", m_scene, rightTextureSwizzle, "leftSceneFallbackTexture");

            createQuadWithTextureConsumer(-2.0f, -1.0f, 2.0f, 2.0f, SamplerConsumerId1, *leftSceneFallbackTexture);
            createQuadWithTextureConsumer(0.0f, -1.0f, 2.0f, 2.0f, SamplerConsumerId2, *rightSceneFallbackTexture);
            break;
        }
        case SINGLE_STREAM_TEXTURE_ON_THE_LEFT:
        {
            ramses::Texture2D* leftSceneFallbackTexture = ramses::RamsesUtils::CreateTextureResourceFromPng("res/ramses-test-client-embedded-compositing-1.png", m_scene, {}, "leftSceneFallbackTexture");
            createQuadWithTextureConsumer(-2.0f, -1.0f, 2.0f, 2.0f, SamplerConsumerId1, *leftSceneFallbackTexture);
            break;
        }
        case SINGLE_STREAM_TEXTURE_ON_THE_RIGHT_WITH_FALLBACK_FROM_LEFT_SCENE:
        {
            ramses::Texture2D* leftSceneFallbackTexture = ramses::RamsesUtils::CreateTextureResourceFromPng("res/ramses-test-client-embedded-compositing-1.png", m_scene, {}, "leftSceneFallbackTexture");
            createQuadWithTextureConsumer(0.0f, -1.0f, 2.0f, 2.0f, SamplerConsumerId1, *leftSceneFallbackTexture);
            break;
        }
        case SINGLE_STREAM_TEXTURE_ON_THE_RIGHT:
        {
            createQuadWithTextureConsumer(0.0f, -1.0f, 2.0f, 2.0f, SamplerConsumerId2, *fallbackTexture2);
            break;
        }
        case SINGLE_STREAM_TEXTURE_ON_THE_RIGHT_WITH_SECOND_SOURCE_ID_AND_FALLBACK_FROM_LEFT_SCENE:
        {
            ramses::Texture2D* leftSceneFallbackTexture = ramses::RamsesUtils::CreateTextureResourceFromPng("res/ramses-test-client-embedded-compositing-1.png", m_scene, {}, "leftSceneFallbackTexture");
            createQuadWithTextureConsumer(0.0f, -1.0f, 2.0f, 2.0f, SamplerConsumerId2, *leftSceneFallbackTexture);
            break;
        }
        default:
            break;
        }
    }

    void EmbeddedCompositorScene::createQuad(float x, float y, float w, float h, ramses::Appearance& appearance)
    {
        const std::array<ramses::vec3f, 4u> vertexPositionsArray
        {
            ramses::vec3f{ x, y, -8.0f },
            ramses::vec3f{ x, y + h, -8.0f },
            ramses::vec3f{ x + w, y + h, -8.0f },
            ramses::vec3f{ x + w, y, -8.0f }
        };
        const ramses::ArrayResource* vertexPositions = m_scene.createArrayResource(4u, vertexPositionsArray.data());

        const uint16_t indicesArray[] =
        {
            0, 2, 1, //ABC
            0, 3, 2  //ACD
        };
        const ramses::ArrayResource* indices = m_scene.createArrayResource(6, indicesArray);

        // set vertex positions directly in geometry
        ramses::Geometry* geometry = m_scene.createGeometry(appearance.getEffect(), "triangle geometry");
        geometry->setIndices(*indices);
        geometry->setInputBuffer(*appearance.getEffect().findAttributeInput("a_position"), *vertexPositions);
        geometry->setInputBuffer(*appearance.getEffect().findAttributeInput("a_texcoord"), m_textureCoords);

        // create a mesh node to define the triangle with chosen appearance
        ramses::MeshNode* meshNode = m_scene.createMeshNode("textured triangle mesh node");
        meshNode->setAppearance(appearance);
        meshNode->setGeometry(*geometry);
        // mesh needs to be added to a render group that belongs to a render pass with camera in order to be rendered
        addMeshNodeToDefaultRenderGroup(*meshNode);
    }

    ramses::Appearance& EmbeddedCompositorScene::createAppearanceWithTextureConsumer(ramses::Scene& scene, ramses::dataConsumerId_t consumerId, const ramses::Texture2D& fallbackTexture)
    {
        const ramses::TextureSampler* sampler = scene.createTextureSampler(
                    ramses::ETextureAddressMode::Repeat,
                    ramses::ETextureAddressMode::Repeat,
                    ramses::ETextureSamplingMethod::Nearest,
                    ramses::ETextureSamplingMethod::Nearest,
                    fallbackTexture);
        ramses::Appearance* appearance = scene.createAppearance(m_effect, "triangle appearance");
        scene.createTextureConsumer(*sampler, consumerId);
        appearance->setInputTexture(*m_effect.findUniformInput("u_texture"), *sampler);

        return *appearance;
    }

    const ramses::Effect& EmbeddedCompositorScene::createTestEffect(uint32_t state)
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
        default:
            break;
        }

        effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic::ModelViewProjectionMatrix);

        const ramses::Effect* effect = m_scene.createEffect(effectDesc, "glsl shader");
        assert(nullptr != effect);

        return *effect;
    }

    const ramses::ArrayResource& EmbeddedCompositorScene::createTextureCoordinates(uint32_t state)
    {
        if (state == SINGLE_STREAM_TEXTURE_WITH_TEXCOORDS_OFFSET)
        {
            const std::array<ramses::vec2f, 4u> textureCoordsArray
            {
                ramses::vec2f{ 0.25f, 2.f },  //A   A-----D
                ramses::vec2f{ 0.25f, -1.f }, //B   |     |
                ramses::vec2f{ .75f,  -1.f }, //C   |     |
                ramses::vec2f{ .75f,  2.f }   //D   B-----C
            };
            return *m_scene.createArrayResource(4u, textureCoordsArray.data());
        }
        const std::array<ramses::vec2f, 4u> textureCoordsArray
        {
            ramses::vec2f{ 0.f, 1.f },  //A   A-----D
            ramses::vec2f{ 0.f, 0.f },  //B   |     |
            ramses::vec2f{ 1.f, 0.f },  //C   |     |
            ramses::vec2f{ 1.f, 1.f }   //D   B-----C
        };
        return *m_scene.createArrayResource(4u, textureCoordsArray.data());
    }

    void EmbeddedCompositorScene::createQuadWithTextureConsumer(float xPos, float yPos, float width, float height, ramses::dataConsumerId_t consumerId, const ramses::Texture2D& fallbackTexture)
    {
        ramses::Appearance& appearance = createAppearanceWithTextureConsumer(m_scene, consumerId, fallbackTexture);
        createQuad(xPos, yPos, width, height, appearance);
    }
}
