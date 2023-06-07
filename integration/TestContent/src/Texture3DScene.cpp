//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/Texture3DScene.h"
#include "ramses-utils.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/ArrayResource.h"
#include "ramses-client-api/GeometryBinding.h"
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/AttributeInput.h"
#include "ramses-client-api/UniformInput.h"
#include "ramses-client-api/Effect.h"

namespace ramses_internal
{
    Texture3DScene::Texture3DScene(ramses::Scene& scene, uint32_t /*state*/, const glm::vec3& cameraPosition)
        : IntegrationScene(scene, cameraPosition)
        , m_effect(getTestEffect("ramses-test-client-3d-textured"))
    {

        const uint16_t indicesArray[] = { 0, 1, 2, 2, 1, 3 };
        m_indices = m_scene.createArrayResource(6u, indicesArray);

        m_groupNode = m_scene.createNode();

        const std::array<ramses::vec3f, 4u> vertexPositionsArray{
            ramses::vec3f{ -1.0f, -1.f, 0.0f },
            ramses::vec3f{ 1.0f, -1.0f, 0.0f },
            ramses::vec3f{ -1.0f, 1.0f, 0.0f },
            ramses::vec3f{ 1.0f, 1.0f, 0.0f }};
        m_vertexPositions = m_scene.createArrayResource(4u, vertexPositionsArray.data());

        const uint8_t rgb8Data_0[] = {
            0xff, 0x00, 0x00,
            0xff, 0x00, 0x00,
            0xff, 0x00, 0x00,
            0xff, 0x00, 0x00,
            0x00, 0xff, 0x00,
            0x00, 0xff, 0x00,
            0x00, 0xff, 0x00,
            0x00, 0xff, 0x00,
            0x00, 0x00, 0xff,
            0x00, 0x00, 0xff,
            0x00, 0x00, 0xff,
            0x00, 0x00, 0xff,
            0x00, 0xff, 0xff,
            0x00, 0xff, 0xff,
            0x00, 0xff, 0xff,
            0x00, 0xff, 0xff };

        const uint8_t rgb8Data_1[] = {
            0xff, 0x00, 0x00,
            0x00, 0xff, 0x00,
        };

        const uint8_t rgb8Data_2[] = {
            0xff, 0xff, 0x00,
        };

        const ramses::MipLevelData mipLevelData[] = {
            ramses::MipLevelData(sizeof(rgb8Data_0), rgb8Data_0),
            ramses::MipLevelData(sizeof(rgb8Data_1), rgb8Data_1),
            ramses::MipLevelData(sizeof(rgb8Data_2), rgb8Data_2)
        };

        m_texture = m_scene.createTexture3D(
            ramses::ETextureFormat::RGB8,
            2, 2, 4,
            3,
            mipLevelData,
            false);

        createQuad(-1.05f, -1.05f, 0.125f);
        createQuad(-1.05f, 1.05f, 0.375f);
        createQuad(1.05f, -1.05f, 0.625f, 100.f);
        createQuad(1.05f, 1.05f, 0.875f);
    }

    void Texture3DScene::createQuad(float x, float y, float depth, float texCoordMagnifier)
    {
        ramses::Appearance* appearance = m_scene.createAppearance(*m_effect, "appearance");

        const std::array<ramses::vec3f, 4u> textureCoordsArray{
            ramses::vec3f{ 0.f * texCoordMagnifier, 0.f * texCoordMagnifier, depth * texCoordMagnifier },
            ramses::vec3f{ 2.f * texCoordMagnifier, 0.f * texCoordMagnifier, depth * texCoordMagnifier },
            ramses::vec3f{ 0.f * texCoordMagnifier, 2.f * texCoordMagnifier, depth * texCoordMagnifier },
            ramses::vec3f{ 2.f * texCoordMagnifier, 2.f * texCoordMagnifier, depth * texCoordMagnifier } };
        const ramses::ArrayResource* textureCoords = m_scene.createArrayResource(4u, textureCoordsArray.data());

        ramses::AttributeInput positionsInput;
        ramses::AttributeInput texCoordsInput;
        m_effect->findAttributeInput("a_position", positionsInput);
        m_effect->findAttributeInput("a_texcoord", texCoordsInput);

        // set vertex positions directly in geometry
        ramses::GeometryBinding* geometry = m_scene.createGeometryBinding(*m_effect, "quad geometry");
        geometry->setIndices(*m_indices);
        geometry->setInputBuffer(positionsInput, *m_vertexPositions);
        geometry->setInputBuffer(texCoordsInput, *textureCoords);

        ramses::TextureSampler* sampler = m_scene.createTextureSampler(
            ramses::ETextureAddressMode::Repeat,
            ramses::ETextureAddressMode::Repeat,
            ramses::ETextureAddressMode::Repeat,
            ramses::ETextureSamplingMethod::Nearest_MipMapNearest,
            ramses::ETextureSamplingMethod::Nearest,
            *m_texture);

        ramses::UniformInput textureInput;
        m_effect->findUniformInput("u_texture", textureInput);
        appearance->setInputTexture(textureInput, *sampler);

        ramses::MeshNode* mesh = m_scene.createMeshNode("quad");
        addMeshNodeToDefaultRenderGroup(*mesh);
        mesh->setAppearance(*appearance);
        mesh->setGeometryBinding(*geometry);

        ramses::Node* transNode = m_scene.createNode();
        transNode->setTranslation({x, y, -12.5f});

        mesh->setParent(*transNode);
        transNode->setParent(*m_groupNode);
    }
}
