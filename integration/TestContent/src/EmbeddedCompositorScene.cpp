//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/EmbeddedCompositorScene.h"
#include "ramses-client-api/RamsesClient.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/Vector3fArray.h"
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
    const StreamTextureSourceId EmbeddedCompositorScene::EmbeddedSurfaceStreamTextureSourceId = StreamTextureSourceId(10123u);

    EmbeddedCompositorScene::EmbeddedCompositorScene(ramses::RamsesClient& ramsesClient, ramses::Scene& scene, UInt32 state, const Vector3& cameraPosition)
        : IntegrationScene(ramsesClient, scene, cameraPosition)
        , m_effect(createTestEffect(state))
    {
        const ramses::streamSource_t streamSource1(GetStreamTextureSourceId().getValue());
        const ramses::streamSource_t streamSource2(GetSecondStreamTextureSourceId().getValue());
        const ramses::Texture2D* fallbackTexture1 = ramses::RamsesUtils::CreateTextureResourceFromPng("res/ramses-test-client-embedded-compositing-1.png", ramsesClient);
        const ramses::Texture2D* fallbackTexture2 = ramses::RamsesUtils::CreateTextureResourceFromPng("res/ramses-test-client-embedded-compositing-2.png", ramsesClient);

        switch (state)
        {
        case SINGLE_STREAM_TEXTURE:
        case SINGLE_STREAM_TEXTURE_WITH_TEXEL_FETCH:
        {
            createQuadWithStreamTexture(-1.0f, -1.0f, 2.0f, 2.0f, streamSource1, *fallbackTexture1);
            break;
        }
        case TWO_STREAM_TEXTURES_WITH_SAME_SOURCE_ID_AND_DIFFERENT_FALLBACK_TEXTURES:
        {
            createQuadWithStreamTexture(-2.0f, -1.0f, 2.0f, 2.0f, streamSource1, *fallbackTexture1);
            createQuadWithStreamTexture(0.0f, -1.0f, 2.0f, 2.0f, streamSource1, *fallbackTexture2);
            break;
        }
        case TWO_STREAM_TEXTURES_WITH_DIFFERENT_SOURCE_ID_AND_SAME_FALLBACK_TEXTURE:
        {
            createQuadWithStreamTexture(-2.0f, -1.0f, 2.0f, 2.0f, streamSource1, *fallbackTexture1);
            createQuadWithStreamTexture(0.0f, -1.0f, 2.0f, 2.0f, streamSource2, *fallbackTexture1);
            break;
        }
        case SINGLE_STREAM_TEXTURE_ON_THE_LEFT:
        {
            ramses::Texture2D* leftSceneFallbackTexture = ramses::RamsesUtils::CreateTextureResourceFromPng("res/ramses-test-client-embedded-compositing-1.png", ramsesClient, "leftSceneFallbackTexture");
            createQuadWithStreamTexture(-2.0f, -1.0f, 2.0f, 2.0f, streamSource1, *leftSceneFallbackTexture);
            break;
        }
        case SINGLE_STREAM_TEXTURE_ON_THE_RIGHT_WITH_FALLBACK_FROM_LEFT_SCENE:
        {
            const ramses::Texture2D& leftSceneFallbackTexture = ramses::RamsesObjectTypeUtils::ConvertTo<ramses::Texture2D>(*ramsesClient.findObjectByName("leftSceneFallbackTexture"));
            createQuadWithStreamTexture(0.0f, -1.0f, 2.0f, 2.0f, streamSource1, leftSceneFallbackTexture);
            break;
        }
        case SINGLE_STREAM_TEXTURE_ON_THE_RIGHT:
        {
            createQuadWithStreamTexture(0.0f, -1.0f, 2.0f, 2.0f, streamSource1, *fallbackTexture2);
            break;
        }
        case SINGLE_STREAM_TEXTURE_ON_THE_RIGHT_WITH_SECOND_SOURCE_ID_AND_FALLBACK_FROM_LEFT_SCENE:
        {
            ramses::Texture2D& leftSceneFallbackTexture = ramses::RamsesObjectTypeUtils::ConvertTo<ramses::Texture2D>(*ramsesClient.findObjectByName("leftSceneFallbackTexture"));
            createQuadWithStreamTexture(0.0f, -1.0f, 2.0f, 2.0f, streamSource2, leftSceneFallbackTexture);
            break;
        }
        default:
            assert(false);
        }
    }

    StreamTextureSourceId EmbeddedCompositorScene::GetStreamTextureSourceId()
    {
        return EmbeddedSurfaceStreamTextureSourceId;
    }

    StreamTextureSourceId EmbeddedCompositorScene::GetSecondStreamTextureSourceId()
    {
        return StreamTextureSourceId(EmbeddedSurfaceStreamTextureSourceId.getValue() + 1u);
    }

    StreamTextureSourceId EmbeddedCompositorScene::GetThirdStreamTextureSourceId()
    {
        return StreamTextureSourceId(EmbeddedSurfaceStreamTextureSourceId.getValue() + 2u);
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
        const ramses::Vector3fArray* vertexPositions = m_client.createConstVector3fArray(4, vertexPositionsArray);

        const float textureCoordsArray[] =
        {
            0.f, 1.f, //A   A-----D
            0.f, 0.f, //B   |     |
            1.f, 0.f, //C   |     |
            1.f, 1.f  //D   B-----C
        };
        const ramses::Vector2fArray* textureCoords = m_client.createConstVector2fArray(4, textureCoordsArray);

        const uint16_t indicesArray[] =
        {
            0, 2, 1, //ABC
            0, 3, 2  //ACD
        };
        const ramses::UInt16Array* indices = m_client.createConstUInt16Array(6, indicesArray);

        // set vertex positions directly in geometry
        ramses::GeometryBinding* geometry = m_scene.createGeometryBinding(appearance.getEffect(), "triangle geometry");
        geometry->setIndices(*indices);
        ramses::AttributeInput positionsInput;
        ramses::AttributeInput texcoordsInput;
        appearance.getEffect().findAttributeInput("a_position", positionsInput);
        appearance.getEffect().findAttributeInput("a_texcoord", texcoordsInput);
        geometry->setInputBuffer(positionsInput, *vertexPositions);
        geometry->setInputBuffer(texcoordsInput, *textureCoords);

        // create a mesh node to define the triangle with chosen appearance
        ramses::MeshNode* meshNode = m_scene.createMeshNode("textured triangle mesh node");
        meshNode->setAppearance(appearance);
        meshNode->setGeometryBinding(*geometry);
        // mesh needs to be added to a render group that belongs to a render pass with camera in order to be rendered
        addMeshNodeToDefaultRenderGroup(*meshNode);
    }

    ramses::Appearance& EmbeddedCompositorScene::createAppearanceWithStreamTexture(ramses::Scene& scene, ramses::streamSource_t sourceId, const ramses::Texture2D& fallbackTexture)
    {
        ramses::StreamTexture* streamTexture = scene.createStreamTexture(fallbackTexture, sourceId);
        const ramses::TextureSampler* sampler = scene.createTextureSampler(
                    ramses::ETextureAddressMode_Repeat,
                    ramses::ETextureAddressMode_Repeat,
                    ramses::ETextureSamplingMethod_Nearest,
                    ramses::ETextureSamplingMethod_Nearest,
                    *streamTexture);
        ramses::Appearance* appearance = scene.createAppearance(m_effect, "triangle appearance");

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
        case TWO_STREAM_TEXTURES_WITH_SAME_SOURCE_ID_AND_DIFFERENT_FALLBACK_TEXTURES:
        case TWO_STREAM_TEXTURES_WITH_DIFFERENT_SOURCE_ID_AND_SAME_FALLBACK_TEXTURE:
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
            assert(false);
        }

        effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic_ModelViewProjectionMatrix);

        const ramses::Effect* effect = m_client.createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "glsl shader");
        assert(NULL != effect);

        return *effect;
    }

    void EmbeddedCompositorScene::createQuadWithStreamTexture(float xPos, float yPos, float width, float height, ramses::streamSource_t sourceId, const ramses::Texture2D& fallbackTexture)
    {
        ramses::Appearance& appearance = createAppearanceWithStreamTexture(m_scene, sourceId, fallbackTexture);
        createQuad(xPos, yPos, width, height, appearance);
    }
}
