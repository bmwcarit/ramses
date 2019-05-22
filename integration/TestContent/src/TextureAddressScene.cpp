//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/TextureAddressScene.h"
#include "ramses-utils.h"
#include "ramses-client-api/RamsesClient.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/Vector2fArray.h"
#include "ramses-client-api/Vector3fArray.h"
#include "ramses-client-api/GeometryBinding.h"
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/AttributeInput.h"
#include "ramses-client-api/UniformInput.h"
#include "ramses-client-api/Effect.h"

namespace ramses_internal
{
    TextureAddressScene::TextureAddressScene(ramses::RamsesClient& ramsesClient, ramses::Scene& scene, UInt32 /*state*/, const Vector3& cameraPosition)
        : IntegrationScene(ramsesClient, scene, cameraPosition)
        , m_groupNode()
        , m_indices(0)
        , m_effect(getTestEffect("ramses-test-client-textured"))
    {
        uint16_t indicesArray[] = { 0, 1, 2, 2, 1, 3 };
        m_indices = m_client.createConstUInt16Array(6, indicesArray);

        m_groupNode = m_scene.createNode();

        float vertexPositionsArray[] = {
            -1.0f, -1.f, 0.0f,
            1.0f, -1.0f, 0.0f,
            -1.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 0.0f };
        m_vertexPositions = m_client.createConstVector3fArray(4, vertexPositionsArray);

        float textureCoordsArray[] = { 0.f, 1.f, 2.f, 1.f, 0.f, -1.f, 2.f, -1.f };
        m_textureCoords = m_client.createConstVector2fArray(4, textureCoordsArray);

        m_texture = ramses::RamsesUtils::CreateTextureResourceFromPng("res/ramses-test-client-logo-cropped.png", m_client);

        createQuad(-1.05f, -1.05f, ramses::ETextureAddressMode_Repeat, ramses::ETextureAddressMode_Repeat);
        createQuad(-1.05f, 1.05f, ramses::ETextureAddressMode_Clamp, ramses::ETextureAddressMode_Clamp);
        createQuad(1.05f, -1.05f, ramses::ETextureAddressMode_Mirror, ramses::ETextureAddressMode_Mirror);
        createQuad(1.05f, 1.05f, ramses::ETextureAddressMode_Repeat, ramses::ETextureAddressMode_Clamp);
    }

    void TextureAddressScene::createQuad(
        Float x,
        Float y,
        ramses::ETextureAddressMode addressMethodU,
        ramses::ETextureAddressMode addressMethodV)
    {
        ramses::Appearance* appearance = m_scene.createAppearance(*m_effect, "appearance");

        ramses::AttributeInput positionsInput;
        ramses::AttributeInput texCoordsInput;
        m_effect->findAttributeInput("a_position", positionsInput);
        m_effect->findAttributeInput("a_texcoord", texCoordsInput);

        // set vertex positions directly in geometry
        ramses::GeometryBinding* geometry = m_scene.createGeometryBinding(*m_effect, "quad geometry");
        geometry->setIndices(*m_indices);
        geometry->setInputBuffer(positionsInput, *m_vertexPositions);
        geometry->setInputBuffer(texCoordsInput, *m_textureCoords);

        ramses::MeshNode* mesh = m_scene.createMeshNode("quad");
        addMeshNodeToDefaultRenderGroup(*mesh);
        mesh->setAppearance(*appearance);
        mesh->setGeometryBinding(*geometry);

        ramses::Node* transNode = m_scene.createNode();
        transNode->setTranslation(x, y, -12.5f);

        mesh->setParent(*transNode);
        transNode->setParent(*m_groupNode);

        ramses::TextureSampler* sampler = m_scene.createTextureSampler(
            addressMethodU,
            addressMethodV,
            ramses::ETextureSamplingMethod_Linear,
            ramses::ETextureSamplingMethod_Linear,
            *m_texture);

        ramses::UniformInput textureInput;
        m_effect->findUniformInput("u_texture", textureInput);
        appearance->setInputTexture(textureInput, *sampler);
    }
}
