//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/TextureCubeAnisotropicTextureFilteringScene.h"
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
#include "PlatformAbstraction/PlatformMath.h"

// This test draws two textured quads, in the top half one with 16x anisotropic filtering, below the same with trilinear sampling without anisotropic filtering.
// The used texture in mip-level 0 has the following repeating 4x4 pattern (B = blue, G = green, . = black):
//
// BG..
// ..BG
// BG..
// ..BG
//
// Mip-level 1 is completely red, mip-levels above are all blue, but not used in rendering.
//
// One pixel maps to 2x1 texel of mip-level 0, thus we have minification. Without anisotropic filtering, mip-level 1 is choosen, so the bottom half is red.
// With anisotropic filtering mip-level 0 is used and the blue and green pixels are averaged by multiple samples. This gives the checkerboard pattern with
// colors (0,127,127) and (0,0,0).

namespace ramses_internal
{

    void TextureCubeAnisotropicTextureFilteringScene::FillMipLevelData(uint8_t* data, uint32_t resolution, uint32_t level)
    {
        switch (level)
        {
        case 0:
        {
            for (uint32_t y = 0; y < resolution / 2; y++)
            {
                for (uint32_t x = 0; x < resolution / 4; x++)
                {
                    *data++ = 0x00;
                    *data++ = 0x00;
                    *data++ = 0xff;

                    *data++ = 0x00;
                    *data++ = 0xff;
                    *data++ = 0x00;

                    *data++ = 0x00;
                    *data++ = 0x00;
                    *data++ = 0x00;

                    *data++ = 0x00;
                    *data++ = 0x00;
                    *data++ = 0x00;
                }
                for (uint32_t x = 0; x < resolution / 4; x++)
                {
                    *data++ = 0x00;
                    *data++ = 0x00;
                    *data++ = 0x00;

                    *data++ = 0x00;
                    *data++ = 0x00;
                    *data++ = 0x00;

                    *data++ = 0x00;
                    *data++ = 0x00;
                    *data++ = 0xff;

                    *data++ = 0x00;
                    *data++ = 0xff;
                    *data++ = 0x00;
                }
            }
            break;
        }

        case 1:
        {
            for (uint32_t y = 0; y < resolution; y++)
            {
                for (uint32_t x = 0; x < resolution; x++)
                {
                    *data++ = 0xff;
                    *data++ = 0x00;
                    *data++ = 0x00;
                }
            }
            break;
        }
        default:
        {
            for (uint32_t y = 0; y < resolution; y++)
            {
                for (uint32_t x = 0; x < resolution; x++)
                {
                    *data++ = 0x00;
                    *data++ = 0x00;
                    *data++ = 0xff;
                }
            }
            break;
        }
        }
    }

    TextureCubeAnisotropicTextureFilteringScene::TextureCubeAnisotropicTextureFilteringScene(ramses::Scene& scene, UInt32 /*state*/, const glm::vec3& cameraPosition)
        : IntegrationScene(scene, cameraPosition)
    {
        createOrthoCamera();
        /// Horizontally, 2 texel map to one pixel => IntegrationScene::DefaultViewportWidth * 2 texels needed.
        /// Vertically, 1 texel map to one pixel => IntegrationScene::DefaultViewportHeight texels needed.
        /// Cube map textures are of square size, so take larger value.
        const uint32_t numberOfMipLevels = GetNextLargerPowerOf2Exponent(std::max(IntegrationScene::DefaultViewportWidth * 2, uint32_t(IntegrationScene::DefaultViewportHeight))) + 1;
        ramses::CubeMipLevelData* mipLevelData = new ramses::CubeMipLevelData[numberOfMipLevels];

        const uint32_t textureResolution = 1u << (numberOfMipLevels - 1);

        for (uint32_t i = 0; i < numberOfMipLevels; i++)
        {
            const uint32_t resolution = textureResolution >> i;
            const uint32_t levelSize = resolution * resolution * 3;
            uint8_t* rgb8_data = new uint8_t[levelSize];
            FillMipLevelData(rgb8_data, resolution, i);

            mipLevelData[i].m_faceDataSize = levelSize;
            mipLevelData[i].m_dataNX = rgb8_data;
            mipLevelData[i].m_dataPX = rgb8_data;
            mipLevelData[i].m_dataNY = rgb8_data;
            mipLevelData[i].m_dataPY = rgb8_data;
            mipLevelData[i].m_dataNZ = rgb8_data;
            mipLevelData[i].m_dataPZ = rgb8_data;
        }

        ramses::Effect* effect(getTestEffect("ramses-test-client-textured-cube"));

        const uint16_t indicesArray[] = { 0, 1, 2, 0, 2, 3 };
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

        const float s = w / static_cast<float>(textureResolution) * 4.0f; // 2 texel maps to one pixel
        const float t = h / static_cast<float>(textureResolution) * 2.0f; // 1 texel maps to one pixel

        const std::array<ramses::vec3f, 4u> normalsArray{
            ramses::vec3f{ -1.0f, -1.0f, 1.0f },
            ramses::vec3f{ -1.0f + s, -1.0f, 1.0f },
            ramses::vec3f{ -1.0f + s, -1.0f + t, 1.0f },
            ramses::vec3f{ -1.0f, -1.0f + t, 1.0f }
        };
        const ramses::ArrayResource* normals = m_scene.createArrayResource(4u, normalsArray.data());

        ramses::TextureCube* texture = m_scene.createTextureCube(
            ramses::ETextureFormat::RGB8,
            textureResolution,
            numberOfMipLevels,
            mipLevelData,
            false);

        for (uint32_t i = 0; i < numberOfMipLevels; i++)
        {
            delete[] mipLevelData[i].m_dataNX;
        }
        delete[] mipLevelData;

        ramses::UniformInput textureInput;
        effect->findUniformInput("u_texture", textureInput);

        ramses::Appearance* appearance = createQuad(effect, vertexPositions, normals, indices, 0.0f, 0.0f);
        ramses::TextureSampler* sampler = m_scene.createTextureSampler(
            ramses::ETextureAddressMode_Repeat,
            ramses::ETextureAddressMode_Repeat,
            ramses::ETextureSamplingMethod_Linear_MipMapLinear,
            ramses::ETextureSamplingMethod_Linear,
            *texture,
            16u);
        appearance->setInputTexture(textureInput, *sampler);

        ramses::Appearance* appearance1 = createQuad(effect, vertexPositions, normals, indices, 0.0f, h);
        ramses::TextureSampler* sampler1 = m_scene.createTextureSampler(
            ramses::ETextureAddressMode_Repeat,
            ramses::ETextureAddressMode_Repeat,
            ramses::ETextureSamplingMethod_Linear_MipMapLinear,
            ramses::ETextureSamplingMethod_Linear,
            *texture);
        appearance1->setInputTexture(textureInput, *sampler1);
    }

    uint32_t TextureCubeAnisotropicTextureFilteringScene::GetNextLargerPowerOf2Exponent(uint32_t value)
    {
        uint32_t i = 0;
        while ((1u << i) < value)
        {
            i++;
        }
        return i;
    }

    void TextureCubeAnisotropicTextureFilteringScene::createOrthoCamera()
    {
        ramses::OrthographicCamera* orthoCamera(m_scene.createOrthographicCamera());
        orthoCamera->setFrustum(0.0f, static_cast<Float>(IntegrationScene::DefaultViewportWidth), 0.0f, static_cast<Float>(IntegrationScene::DefaultViewportHeight), 0.1f, 10.f);
        orthoCamera->setViewport(0, 0, IntegrationScene::DefaultViewportWidth, IntegrationScene::DefaultViewportHeight);
        setCameraToDefaultRenderPass(orthoCamera);
    }

    ramses::Appearance* TextureCubeAnisotropicTextureFilteringScene::createQuad(
        ramses::Effect* effect,
        const ramses::ArrayResource* vertexPositions,
        const ramses::ArrayResource* normals,
        const ramses::ArrayResource* indices,
        float x,
        float y)
    {
        ramses::Appearance* appearance = m_scene.createAppearance(*effect, "appearance");

        ramses::AttributeInput positionsInput;
        ramses::AttributeInput normalsInput;
        effect->findAttributeInput("a_position", positionsInput);
        effect->findAttributeInput("a_normal", normalsInput);

        ramses::GeometryBinding* geometry = m_scene.createGeometryBinding(*effect, "geometry");
        geometry->setIndices(*indices);
        geometry->setInputBuffer(positionsInput, *vertexPositions);
        geometry->setInputBuffer(normalsInput, *normals);

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
