//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/Texture2DAnisotropicTextureFilteringScene.h"
#include "ramses-utils.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/ArrayResource.h"
#include "ramses-client-api/GeometryBinding.h"
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/Effect.h"
#include "ramses-client-api/AttributeInput.h"
#include "PlatformAbstraction/PlatformMemory.h"
#include "ramses-client-api/OrthographicCamera.h"

// This test draws two textured quads, in the top half one with 16x anisotropic filtering, below the same with trilinear sampling without anisotropic filtering.
// The used texture in mip-level 0 has size 4x4 and looks as following (B = blue, G = green, . = black):
//
// BG..
// ..BG
// BG..
// ..BG
//
// Mip-level 1 is completely red, mip-level 2 is blue, but not used in rendering.
//
// One pixel maps to 2x1 texel of mip-level 0, thus we have minification. Without anisotropic filtering, mip-level 1 is choosen, so the bottom half is red.
// With anisotropic filtering mip-level 0 is used and the blue and green pixels are averaged by multiple samples. This gives the checkerboard pattern with
// colors (0,127,127) and (0,0,0).

namespace ramses_internal
{
    Texture2DAnisotropicTextureFilteringScene::Texture2DAnisotropicTextureFilteringScene(ramses::Scene& scene, UInt32 /*state*/, const glm::vec3& cameraPosition)
        : IntegrationScene(scene, cameraPosition)
    {
        createOrthoCamera();

        const UInt8 rgb8_level0[] = {
            0x00, 0x00, 0xff,  0x00, 0xff, 0x00,  0x00, 0x00, 0x00,  0x00, 0x00, 0x00,
            0x00, 0x00, 0x00,  0x00, 0x00, 0x00,  0x00, 0x00, 0xff,  0x00, 0xff, 0x00,
            0x00, 0x00, 0xff,  0x00, 0xff, 0x00,  0x00, 0x00, 0x00,  0x00, 0x00, 0x00,
            0x00, 0x00, 0x00,  0x00, 0x00, 0x00,  0x00, 0x00, 0xff,  0x00, 0xff, 0x00
        };

        const UInt8 rgb8_level1[] = {
            0xff, 0x00, 0x00, 0xff, 0x00, 0x00,
            0xff, 0x00, 0x00, 0xff, 0x00, 0x00
        };

        const UInt8 rgb8_level2[] = {
            0x00, 0x00, 0xff
        };

        const ramses::MipLevelData mipLevelData[] = {
            ramses::MipLevelData(sizeof(rgb8_level0), rgb8_level0),
            ramses::MipLevelData(sizeof(rgb8_level1), rgb8_level1),
            ramses::MipLevelData(sizeof(rgb8_level2), rgb8_level2)
        };

        ramses::Effect* effect(getTestEffect("ramses-test-client-textured"));

        uint16_t indicesArray[] = { 0, 1, 2, 0, 2, 3 };
        const ramses::ArrayResource* indices = m_scene.createArrayResource(6u, indicesArray);

        const float x = 0.0f;
        const float y = 0.0f;
        const float w = static_cast<float>(IntegrationScene::DefaultViewportWidth);
        const float h = static_cast<float>(IntegrationScene::DefaultViewportHeight) * 0.5f;
        const float z = -1.0f;

        const std::array<ramses::vec3f, 4u> vertexPositionsArray{
            ramses::vec3f{ x, y, z },
            ramses::vec3f{ x + w, y, z },
            ramses::vec3f{ x + w, y + h, z },
            ramses::vec3f{ x, y + h, z }
        };
        const ramses::ArrayResource* vertexPositions = m_scene.createArrayResource(4u, vertexPositionsArray.data());

        const float s = w / 4.0f * 2.0f;
        const float t = h / 4.0f;

        const std::array<ramses::vec2f, 4u> textureCoordsArray{
            ramses::vec2f{ 0.0f, 0.0f },
            ramses::vec2f{ s, 0.0f },
            ramses::vec2f{ s, t },
            ramses::vec2f{ 0.0f, t }
        };
        const ramses::ArrayResource* textureCoords = m_scene.createArrayResource(4u, textureCoordsArray.data());

        ramses::Texture2D* texture = m_scene.createTexture2D(
            ramses::ETextureFormat::RGB8,
            4, 4,
            3,
            mipLevelData,
            false);

        ramses::UniformInput textureInput;
        effect->findUniformInput("u_texture", textureInput);

        ramses::Appearance* appearance = createQuad(effect, vertexPositions, textureCoords, indices, 0.0f, 0.0f);
        ramses::TextureSampler* sampler = m_scene.createTextureSampler(
            ramses::ETextureAddressMode::Repeat,
            ramses::ETextureAddressMode::Repeat,
            ramses::ETextureSamplingMethod::Linear_MipMapLinear,
            ramses::ETextureSamplingMethod::Linear,
            *texture,
            16u);
        appearance->setInputTexture(textureInput, *sampler);

        ramses::Appearance* appearance1 = createQuad(effect, vertexPositions, textureCoords, indices, 0.0f, h);
        ramses::TextureSampler* sampler1 = m_scene.createTextureSampler(
            ramses::ETextureAddressMode::Repeat,
            ramses::ETextureAddressMode::Repeat,
            ramses::ETextureSamplingMethod::Linear_MipMapLinear,
            ramses::ETextureSamplingMethod::Linear,
            *texture);
        appearance1->setInputTexture(textureInput, *sampler1);
    }

    void Texture2DAnisotropicTextureFilteringScene::createOrthoCamera()
    {
        ramses::OrthographicCamera* orthoCamera(m_scene.createOrthographicCamera());
        orthoCamera->setFrustum(0.0f, static_cast<float>(IntegrationScene::DefaultViewportWidth), 0.0f, static_cast<float>(IntegrationScene::DefaultViewportHeight), 0.1f, 10.f);
        orthoCamera->setViewport(0, 0, IntegrationScene::DefaultViewportWidth, IntegrationScene::DefaultViewportHeight);
        setCameraToDefaultRenderPass(orthoCamera);
    }

    ramses::Appearance* Texture2DAnisotropicTextureFilteringScene::createQuad(
        ramses::Effect* effect,
        const ramses::ArrayResource* vertexPositions,
        const ramses::ArrayResource* textureCoords,
        const ramses::ArrayResource* indices,
        float x,
        float y)
    {
        ramses::Appearance* appearance = m_scene.createAppearance(*effect, "appearance");

        ramses::AttributeInput positionsInput;
        ramses::AttributeInput texCoordsInput;
        effect->findAttributeInput("a_position", positionsInput);
        effect->findAttributeInput("a_texcoord", texCoordsInput);

        ramses::GeometryBinding* geometry = m_scene.createGeometryBinding(*effect, "geometry");
        geometry->setIndices(*indices);
        geometry->setInputBuffer(positionsInput, *vertexPositions);
        geometry->setInputBuffer(texCoordsInput, *textureCoords);

        ramses::MeshNode* mesh = m_scene.createMeshNode("quad");
        addMeshNodeToDefaultRenderGroup(*mesh);
        mesh->setAppearance(*appearance);
        mesh->setGeometryBinding(*geometry);

        ramses::Node* translateNode = m_scene.createNode();
        translateNode->setTranslation({x, y, 0.0f});

        mesh->setParent(*translateNode);

        return appearance;
    }
}
