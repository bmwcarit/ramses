//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/Texture2DCompressedMipMapScene.h"

#include "ramses-utils.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/ArrayResource.h"
#include "ramses-client-api/GeometryBinding.h"
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/Effect.h"
#include "ramses-client-api/AttributeInput.h"
#include "ramses-client-api/OrthographicCamera.h"

#include "Collections/Vector.h"
#include <cassert>

// Renders one horizontal stripe for each mip-level.

namespace ramses_internal
{
    Texture2DCompressedMipMapScene::Texture2DCompressedMipMapScene(ramses::Scene&        scene,
                                                                   uint32_t              state,
                                                                   const glm::vec3&        cameraPosition)
        : IntegrationScene(scene, cameraPosition)
        , m_textureWidth(8)
        , m_textureHeight(8)
    {
        UNUSED(state);
        createOrthoCamera();

        const uint8_t dataLevel0[] = {0x7e, 0x80, 0x4, 0x7f, 0x0, 0x7, 0xe0, 0x0,0x81, 0x7e, 0x4, 0x2, 0xfe, 0x0, 0x1f, 0xc0, 0x80, 0x81, 0xfb, 0x82, 0x1, 0xf8, 0x0, 0x3f, 0xf8, 0xf8, 0xf8, 0x2, 0x0, 0x0, 0x0, 0x0};
        // 8x8: red, green, blue, white

        const uint8_t dataLevel1[] = {0x7e, 0x80, 0x4, 0x7f, 0x0, 0x7, 0xe0, 0x0};
        // 4x4: red

        const ramses::MipLevelData mipLevelData[NumMipMaps] = { { 32u, dataLevel0 }, { 8u, dataLevel1 } };
        const ramses::Texture2D* texture = m_scene.createTexture2D(
            ramses::ETextureFormat::ETC2RGB,
            m_textureWidth, m_textureHeight,
            NumMipMaps,
            mipLevelData,
            false);

        ramses::TextureSampler* sampler = m_scene.createTextureSampler(
            ramses::ETextureAddressMode_Repeat,
            ramses::ETextureAddressMode_Repeat,
            ramses::ETextureSamplingMethod_Nearest_MipMapNearest,
            ramses::ETextureSamplingMethod_Nearest,
            *texture);

        createMesh(*sampler);
    }

    void Texture2DCompressedMipMapScene::createOrthoCamera()
    {
        ramses::OrthographicCamera* orthoCamera(m_scene.createOrthographicCamera());
        orthoCamera->setFrustum(0.0f, static_cast<Float>(IntegrationScene::DefaultViewportWidth), 0.0f, static_cast<Float>(IntegrationScene::DefaultViewportHeight), 0.1f, 10.f);
        orthoCamera->setViewport(0, 0, IntegrationScene::DefaultViewportWidth, IntegrationScene::DefaultViewportHeight);
        setCameraToDefaultRenderPass(orthoCamera);
    }

    void Texture2DCompressedMipMapScene::createMesh(const ramses::TextureSampler& sampler)
    {
        ramses::Effect* effect = getTestEffect("ramses-test-client-textured");
        assert(effect != nullptr);

        ramses::AttributeInput positionsInput;
        ramses::AttributeInput texCoordsInput;
        ramses::UniformInput textureInput;
        effect->findAttributeInput("a_position", positionsInput);
        effect->findAttributeInput("a_texcoord", texCoordsInput);
        effect->findUniformInput("u_texture", textureInput);

        ramses::Appearance* appearance = m_scene.createAppearance(*effect);
        appearance->setInputTexture(textureInput, sampler);

        ramses::GeometryBinding* geometry = m_scene.createGeometryBinding(*effect);

        createGeometry();

        geometry->setIndices(*m_indexArray);
        geometry->setInputBuffer(positionsInput, *m_vertexPositions);
        geometry->setInputBuffer(texCoordsInput, *m_textureCoords);

        ramses::MeshNode* mesh = m_scene.createMeshNode();
        addMeshNodeToDefaultRenderGroup(*mesh);
        mesh->setAppearance(*appearance);
        mesh->setGeometryBinding(*geometry);
    }

    void Texture2DCompressedMipMapScene::createGeometry()
    {
        const uint32_t numberStripes = NumMipMaps;

        const float z = -1.0f;

        std::vector<uint16_t> indices;
        std::vector<ramses::vec3f> vertexPositions;
        std::vector<ramses::vec2f> textureCoords;

        const float x = 0.0f;
        const float w = static_cast<float>(IntegrationScene::DefaultViewportWidth);
        const float h = static_cast<float>(IntegrationScene::DefaultViewportHeight) / numberStripes;

        float s = w / m_textureWidth;
        float t = h / m_textureHeight;

        for (uint16_t i = 0; i < numberStripes; i++)
        {
            const float y = h * i;

            vertexPositions.push_back(ramses::vec3f{ x,   y,   z });
            vertexPositions.push_back(ramses::vec3f{ x+w, y,   z });
            vertexPositions.push_back(ramses::vec3f{ x+w, y+h, z });
            vertexPositions.push_back(ramses::vec3f{ x,   y+h, z });

            textureCoords.push_back(ramses::vec2f{ 0.f, 0.f });
            textureCoords.push_back(ramses::vec2f{ s,   0.f });
            textureCoords.push_back(ramses::vec2f{ s,   t });
            textureCoords.push_back(ramses::vec2f{ 0.f, t });

            indices.push_back(0 + i * 4);
            indices.push_back(1 + i * 4);
            indices.push_back(2 + i * 4);
            indices.push_back(0 + i * 4);
            indices.push_back(2 + i * 4);
            indices.push_back(3 + i * 4);

            s *= 2.0;
            t *= 2.0;
        }

        m_indexArray = m_scene.createArrayResource(static_cast<uint32_t>(indices.size()), indices.data());
        m_vertexPositions = m_scene.createArrayResource(static_cast<uint32_t>(vertexPositions.size()), vertexPositions.data());
        m_textureCoords = m_scene.createArrayResource(static_cast<uint32_t>(textureCoords.size()), textureCoords.data());
    }
}
