//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/MultiTextureConsumerScene.h"
#include "ramses-client-api/RamsesClient.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/AttributeInput.h"
#include "ramses-client-api/UniformInput.h"
#include "ramses-client-api/GeometryBinding.h"
#include "ramses-client-api/Effect.h"
#include "ramses-client-api/Appearance.h"

#include "Scene/ClientScene.h"
#include "TestScenes/TriangleStripQuad.h"
#include "ramses-utils.h"

namespace ramses_internal
{
    MultiTextureConsumerScene::MultiTextureConsumerScene(ramses::RamsesClient& ramsesClient, ramses::Scene& scene, UInt32 /*state*/, const Vector3& cameraPosition)
        : IntegrationScene(ramsesClient, scene, cameraPosition)
    {
        m_effectTex = getTestEffect("ramses-test-client-textured");

        ramses::TriangleStripQuad quad1(ramsesClient, scene, *m_effectTex, ramses::TriangleStripQuad::EColor_White);
        ramses::TriangleStripQuad quad2(ramsesClient, scene, *m_effectTex, ramses::TriangleStripQuad::EColor_White);
        ramses::TriangleStripQuad quad3(ramsesClient, scene, *m_effectTex, ramses::TriangleStripQuad::EColor_White);

        const ramses::Texture2D& texture1 = *ramses::RamsesUtils::CreateTextureResourceFromPng("res/ramses-test-client-embedded-compositing-1.png", m_client);
        const ramses::Texture2D& texture2 = *ramses::RamsesUtils::CreateTextureResourceFromPng("res/ramses-test-client-embedded-compositing-2.png", m_client);
        const ramses::Texture2D& texture3 = *ramses::RamsesUtils::CreateTextureResourceFromPng("res/ramses-test-client-embedded-compositing-3.png", m_client);

        const float textureCoordsArray[] = { 0.f, 0.f, 0.f, 1.f, 1.f, 0.f, 1.f, 1.f };
        const ramses::Vector2fArray& textureCoords = *m_client.createConstVector2fArray(4u, textureCoordsArray);

        createQuadWithTextureConsumer(quad1, 0, ramses::dataConsumerId_t(1u), texture1, textureCoords);
        createQuadWithTextureConsumer(quad2, 1, ramses::dataConsumerId_t(2u), texture2, textureCoords);
        createQuadWithTextureConsumer(quad3, 2, ramses::dataConsumerId_t(3u), texture3, textureCoords);
    }

    void MultiTextureConsumerScene::createQuadWithTextureConsumer(ramses::TriangleStripQuad& quad, uint32_t quadId, ramses::dataConsumerId_t textureConsumerId, const ramses::Texture2D& texture, const ramses::Vector2fArray& textureCoords)
    {
        ramses::MeshNode* mesh = m_scene.createMeshNode();
        ramses::Appearance& appearance = quad.GetAppearance();
        mesh->setAppearance(appearance);
        mesh->setGeometryBinding(quad.GetGeometry());
        ramses::Node* translate = m_scene.createNode();
        mesh->setParent(*translate);

        addMeshNodeToDefaultRenderGroup(*mesh);

        translate->setTranslation(-1.0f + 1.5f * quadId, 0.5f, -4.f);

        ramses::AttributeInput texCoordsInput;
        m_effectTex->findAttributeInput("a_texcoord", texCoordsInput);
        quad.GetGeometry().setInputBuffer(texCoordsInput, textureCoords);

        ramses::TextureSampler *sampler = m_scene.createTextureSampler(
            ramses::ETextureAddressMode_Repeat,
            ramses::ETextureAddressMode_Repeat,
            ramses::ETextureSamplingMethod_Linear,
            ramses::ETextureSamplingMethod_Linear,
            texture,
            1u);

        ramses::UniformInput texInput;
        appearance.getEffect().findUniformInput("u_texture", texInput);
        appearance.setInputTexture(texInput, *sampler);

        m_scene.createTextureConsumer(*sampler, textureConsumerId);
    }
}
