//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/TextureAddressScene.h"
#include "ramses/client/ramses-utils.h"
#include "ramses/client/Scene.h"
#include "ramses/client/MeshNode.h"
#include "ramses/client/ArrayResource.h"
#include "ramses/client/Geometry.h"
#include "ramses/client/Appearance.h"
#include "ramses/client/AttributeInput.h"
#include "ramses/client/UniformInput.h"
#include "ramses/client/Effect.h"

namespace ramses::internal
{
    TextureAddressScene::TextureAddressScene(ramses::Scene& scene, uint32_t /*state*/, const glm::vec3& cameraPosition)
        : IntegrationScene(scene, cameraPosition)
        , m_groupNode()
        , m_indices(nullptr)
        , m_effect(getTestEffect("ramses-test-client-textured"))
    {
        const std::array<uint16_t, 6> indicesArray = { 0, 1, 2, 2, 1, 3 };
        m_indices = m_scene.createArrayResource(6u, indicesArray.data());

        m_groupNode = m_scene.createNode();

        const std::array<ramses::vec3f, 4u> vertexPositionsArray{
            ramses::vec3f{ -1.0f, -1.f, 0.0f },
            ramses::vec3f{ 1.0f, -1.0f, 0.0f },
            ramses::vec3f{ -1.0f, 1.0f, 0.0f },
            ramses::vec3f{ 1.0f, 1.0f, 0.0f }};
        m_vertexPositions = m_scene.createArrayResource(4u, vertexPositionsArray.data());

        const std::array<ramses::vec2f, 4u> textureCoordsArray{ ramses::vec2f{0.f, 1.f}, ramses::vec2f{2.f, 1.f}, ramses::vec2f{0.f, -1.f}, ramses::vec2f{2.f, -1.f} };
        m_textureCoords = m_scene.createArrayResource(4u, textureCoordsArray.data());

        m_texture = ramses::RamsesUtils::CreateTextureResourceFromPng("res/ramses-test-client-logo-cropped.png", m_scene);

        createQuad(-1.05f, -1.05f, ramses::ETextureAddressMode::Repeat, ramses::ETextureAddressMode::Repeat);
        createQuad(-1.05f, 1.05f, ramses::ETextureAddressMode::Clamp, ramses::ETextureAddressMode::Clamp);
        createQuad(1.05f, -1.05f, ramses::ETextureAddressMode::Mirror, ramses::ETextureAddressMode::Mirror);
        createQuad(1.05f, 1.05f, ramses::ETextureAddressMode::Repeat, ramses::ETextureAddressMode::Clamp);
    }

    void TextureAddressScene::createQuad(
        float x,
        float y,
        ramses::ETextureAddressMode addressMethodU,
        ramses::ETextureAddressMode addressMethodV)
    {
        ramses::Appearance* appearance = m_scene.createAppearance(*m_effect, "appearance");

        // set vertex positions directly in geometry
        ramses::Geometry* geometry = m_scene.createGeometry(*m_effect, "quad geometry");
        geometry->setIndices(*m_indices);
        geometry->setInputBuffer(*m_effect->findAttributeInput("a_position"), *m_vertexPositions);
        geometry->setInputBuffer(*m_effect->findAttributeInput("a_texcoord"), *m_textureCoords);

        ramses::MeshNode* mesh = m_scene.createMeshNode("quad");
        addMeshNodeToDefaultRenderGroup(*mesh);
        mesh->setAppearance(*appearance);
        mesh->setGeometry(*geometry);

        ramses::Node* transNode = m_scene.createNode();
        transNode->setTranslation({x, y, -12.5f});

        mesh->setParent(*transNode);
        transNode->setParent(*m_groupNode);

        ramses::TextureSampler* sampler = m_scene.createTextureSampler(
            addressMethodU,
            addressMethodV,
            ramses::ETextureSamplingMethod::Linear,
            ramses::ETextureSamplingMethod::Linear,
            *m_texture);

        appearance->setInputTexture(*m_effect->findUniformInput("u_texture"), *sampler);
    }
}
