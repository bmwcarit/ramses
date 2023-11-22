//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/Texture2DGenerateMipMapScene.h"
#include "ramses/client/ramses-utils.h"
#include "ramses/client/Scene.h"
#include "ramses/client/MeshNode.h"
#include "ramses/client/ArrayResource.h"
#include "ramses/client/Geometry.h"
#include "ramses/client/Appearance.h"
#include "ramses/client/Effect.h"
#include "ramses/client/AttributeInput.h"
#include "ramses/client/OrthographicCamera.h"
#include <cassert>

// Uses mip-map generation to generate LOD level-1 from level-0 data. Draws two quads, for the left one, 2x2 texels
// map to one pixel, thus LOD level-1 is used, which is the average of the four level-0 texels (red, green, blue, white).
// The right quad shows large magnification, LOD level-0 is taken with red, green, blue, white texels.

namespace ramses::internal
{

    Texture2DGenerateMipMapScene::Texture2DGenerateMipMapScene(ramses::Scene& scene, uint32_t state, const glm::vec3& cameraPosition)
        : IntegrationScene(scene, cameraPosition)
    {
        createOrthoCamera();

        switch (state)
        {
        case EState_GenerateMipMapSingle:
            createMesh(*createTexture2DSampler());
            break;
        case EState_GenerateMipMapMultiple:
            for (uint8_t i = 0; i < 8; i++)
            {
                // Note: here we need different texture content, otherwise ramses uses
                // the already known resource with the same content.
                // In that case, glGlenerateMipMaps would be called only once!
                // Here, transparency parameter is used to generate different texture content.
                ramses::TextureSampler* sampler = createTexture2DSampler(256u, 2048u, i * 20);
                createMesh(*sampler, i * IntegrationScene::DefaultViewportWidth * 0.1f, 0.5f);
            }
            break;
        default:
            assert(false);
            return;
        }
    }

    void Texture2DGenerateMipMapScene::createOrthoCamera()
    {
        ramses::OrthographicCamera* orthoCamera(m_scene.createOrthographicCamera());
        orthoCamera->setFrustum(0.0f, static_cast<float>(IntegrationScene::DefaultViewportWidth), 0.0f, static_cast<float>(IntegrationScene::DefaultViewportHeight), 0.1f, 10.f);
        orthoCamera->setViewport(0, 0, IntegrationScene::DefaultViewportWidth, IntegrationScene::DefaultViewportHeight);
        setCameraToDefaultRenderPass(orthoCamera);
    }

    void Texture2DGenerateMipMapScene::createMesh(const ramses::TextureSampler& sampler, float translateXY, float scale)
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

        ramses::Node* transform = m_scene.createNode();
        transform->translate({translateXY, translateXY, 0.0f});
        transform->scale({scale, scale, scale});
        transform->addChild(*mesh);
    }

    void Texture2DGenerateMipMapScene::createGeometry()
    {
        const float z = -1.0f;

        const uint16_t indicesArray[] = {
            0, 1, 2, 0, 2, 3,
            4, 5, 6, 4, 6, 7,
        };

        const float x = 0.0f;
        const float y = 0.0f;
        const float w = float(IntegrationScene::DefaultViewportWidth) * 0.5f;
        const auto h = float(IntegrationScene::DefaultViewportHeight);

        const float x2 = w;
        const float y2 = 0.0f;
        const float w2 = w;
        const float h2 = h;

        const std::array<ramses::vec3f, 8u> vertexPositionsArray{
            ramses::vec3f{ x, y, z },
            ramses::vec3f{ x + w, y, z },
            ramses::vec3f{ x + w, y + h, z },
            ramses::vec3f{ x, y + h, z },

            ramses::vec3f{ x2, y2, z },
            ramses::vec3f{ x2 + w2, y2, z },
            ramses::vec3f{ x2 + w2, y2 + h2, z },
            ramses::vec3f{ x2, y2 + h2, z }
        };

        const float s = w;
        const float t = h;

        const float s2 = w2 / 32.0f;
        const float t2 = h2 / 32.0f;

        const std::array<ramses::vec2f, 8u> textureCoordsArray{
            ramses::vec2f{ 0.0f, 0.0f },
            ramses::vec2f{ s, 0.0f },
            ramses::vec2f{ s, t },
            ramses::vec2f{ 0.0f, t },

            ramses::vec2f{ 0.0f, 0.0f },
            ramses::vec2f{ s2, 0.0f },
            ramses::vec2f{ s2, t2 },
            ramses::vec2f{ 0.0f, t2 }
        };

        m_indexArray = m_scene.createArrayResource(12u, indicesArray);
        m_vertexPositions = m_scene.createArrayResource(8u, vertexPositionsArray.data());
        m_textureCoords = m_scene.createArrayResource(8u, textureCoordsArray.data());
    }

    ramses::TextureSampler* Texture2DGenerateMipMapScene::createTexture2DSampler(uint32_t width, uint32_t height, uint8_t transparency)
    {
        const uint32_t pixelCount = width * height;

        const uint32_t idx_green = pixelCount;
        const uint32_t idx_blue = pixelCount * 2;
        const uint32_t idx_white = pixelCount * 3;

        ramses::MipLevelData rgba8_level0(pixelCount * 4);
        for (uint32_t pixel = 0u; pixel < pixelCount; pixel++)
        {
            const uint32_t idx = pixel * 4;
            if (idx < idx_green)
            {
                //red
                rgba8_level0[idx]     = static_cast<std::byte>(0xff);
                rgba8_level0[idx + 1] = static_cast<std::byte>(0x00);
                rgba8_level0[idx + 2] = static_cast<std::byte>(0x00);
            }
            else if (idx < idx_blue)
            {
                //green
                rgba8_level0[idx]     = static_cast<std::byte>(0x00);
                rgba8_level0[idx + 1] = static_cast<std::byte>(0xff);
                rgba8_level0[idx + 2] = static_cast<std::byte>(0x00);
            }
            else if (idx < idx_white)
            {
                //blue
                rgba8_level0[idx]     = static_cast<std::byte>(0x00);
                rgba8_level0[idx + 1] = static_cast<std::byte>(0x00);
                rgba8_level0[idx + 2] = static_cast<std::byte>(0xff);
            }
            else
            {
                //white
                rgba8_level0[idx]     = static_cast<std::byte>(0xff);
                rgba8_level0[idx + 1] = static_cast<std::byte>(0xff);
                rgba8_level0[idx + 2] = static_cast<std::byte>(0xff);
            }
            // opacity
            rgba8_level0[idx + 3] = static_cast<std::byte>(0xffu - transparency);
        }

        const std::vector<ramses::MipLevelData> mipLevelData = { rgba8_level0 };

        ramses::Texture2D* texture = m_scene.createTexture2D(
            ramses::ETextureFormat::RGBA8,
            width, height,
            mipLevelData,
            true);

        ramses::TextureSampler* sampler = m_scene.createTextureSampler(
            ramses::ETextureAddressMode::Repeat,
            ramses::ETextureAddressMode::Repeat,
            ramses::ETextureSamplingMethod::Nearest_MipMapNearest,
            ramses::ETextureSamplingMethod::Nearest,
            *texture);

        return sampler;
    }
}
