//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/Texture2DCompressedMipMapScene.h"

#include "ramses/client/ramses-utils.h"
#include "ramses/client/Scene.h"
#include "ramses/client/MeshNode.h"
#include "ramses/client/ArrayResource.h"
#include "ramses/client/Geometry.h"
#include "ramses/client/Appearance.h"
#include "ramses/client/Effect.h"
#include "ramses/client/AttributeInput.h"
#include "ramses/client/OrthographicCamera.h"

#include "internal/PlatformAbstraction/Collections/Vector.h"
#include <cassert>

// Renders one horizontal stripe for each mip-level.

namespace ramses::internal
{
    Texture2DCompressedMipMapScene::Texture2DCompressedMipMapScene(ramses::Scene& scene, [[maybe_unused]] uint32_t state, const glm::vec3& cameraPosition)
        : IntegrationScene(scene, cameraPosition)
    {
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
            ramses::ETextureAddressMode::Repeat,
            ramses::ETextureAddressMode::Repeat,
            ramses::ETextureSamplingMethod::Nearest_MipMapNearest,
            ramses::ETextureSamplingMethod::Nearest,
            *texture);

        createMesh(*sampler);
    }

    void Texture2DCompressedMipMapScene::createOrthoCamera()
    {
        ramses::OrthographicCamera* orthoCamera(m_scene.createOrthographicCamera());
        orthoCamera->setFrustum(0.0f, static_cast<float>(IntegrationScene::DefaultViewportWidth), 0.0f, static_cast<float>(IntegrationScene::DefaultViewportHeight), 0.1f, 10.f);
        orthoCamera->setViewport(0, 0, IntegrationScene::DefaultViewportWidth, IntegrationScene::DefaultViewportHeight);
        setCameraToDefaultRenderPass(orthoCamera);
    }

    void Texture2DCompressedMipMapScene::createMesh(const ramses::TextureSampler& sampler)
    {
        ramses::Effect* effect = getTestEffect("ramses-test-client-textured");
        assert(effect != nullptr);

        ramses::Appearance* appearance = m_scene.createAppearance(*effect);
        appearance->setInputTexture(*effect->findUniformInput("u_texture"), sampler);

        ramses::Geometry* geometry = m_scene.createGeometry(*effect);

        createGeometry();

        geometry->setIndices(*m_indexArray);
        geometry->setInputBuffer(*effect->findAttributeInput("a_position"), *m_vertexPositions);
        geometry->setInputBuffer(*effect->findAttributeInput("a_texcoord"), *m_textureCoords);

        ramses::MeshNode* mesh = m_scene.createMeshNode();
        addMeshNodeToDefaultRenderGroup(*mesh);
        mesh->setAppearance(*appearance);
        mesh->setGeometry(*geometry);
    }

    void Texture2DCompressedMipMapScene::createGeometry()
    {
        const uint32_t numberStripes = NumMipMaps;

        const float z = -1.0f;

        std::vector<uint16_t> indices;
        std::vector<ramses::vec3f> vertexPositions;
        std::vector<ramses::vec2f> textureCoords;

        const float x = 0.0f;
        const auto w = static_cast<float>(IntegrationScene::DefaultViewportWidth);
        const float h = static_cast<float>(IntegrationScene::DefaultViewportHeight) / numberStripes;

        float s = w / m_textureWidth;
        float t = h / m_textureHeight;

        for (uint16_t i = 0; i < numberStripes; i++)
        {
            const float y = h * i;

            vertexPositions.emplace_back(x,   y,   z);
            vertexPositions.emplace_back(x+w, y,   z);
            vertexPositions.emplace_back(x+w, y+h, z);
            vertexPositions.emplace_back(x,   y+h, z);

            textureCoords.emplace_back(0.f, 0.f);
            textureCoords.emplace_back(s,   0.f);
            textureCoords.emplace_back(s,   t);
            textureCoords.emplace_back(0.f, t);

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
