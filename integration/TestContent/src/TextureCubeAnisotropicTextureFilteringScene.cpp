//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/TextureCubeAnisotropicTextureFilteringScene.h"
#include "ramses-utils.h"
#include "ramses-client-api/RamsesClient.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/Vector2fArray.h"
#include "ramses-client-api/Vector3fArray.h"
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

    TextureCubeAnisotropicTextureFilteringScene::TextureCubeAnisotropicTextureFilteringScene(ramses::RamsesClient& ramsesClient, ramses::Scene& scene, UInt32 /*state*/, const Vector3& cameraPosition)
        : IntegrationScene(ramsesClient, scene, cameraPosition)
    {
        createOrthoCamera();
        /// Horizontally, 2 texel map to one pixel => DefaultDisplayWidth * 2 texels needed.
        /// Vertically, 1 texel map to one pixel => DefaultDisplayHeight texels needed.
        /// Cube map textures are of square size, so take larger value.
        const uint32_t numberOfMipLevels = GetNextLargerPowerOf2Exponent(max(DefaultDisplayWidth * 2, DefaultDisplayHeight)) + 1;
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

        uint16_t indicesArray[] = { 0, 1, 2, 0, 2, 3 };
        const ramses::UInt16Array* indices = m_client.createConstUInt16Array(6, indicesArray);

        const float x = 0.0f;
        const float y = 0.0f;
        const float w = static_cast<float>(DefaultDisplayWidth);
        const float h = static_cast<float>(DefaultDisplayHeight) * 0.5f;
        const float z = -1.0f;

        float vertexPositionsArray[] = {
            x, y, z,
            x + w, y, z,
            x + w, y + h, z,
            x, y + h, z,
        };
        const ramses::Vector3fArray* vertexPositions = m_client.createConstVector3fArray(4, vertexPositionsArray);

        const float s = w / static_cast<float>(textureResolution) * 4.0f; // 2 texel maps to one pixel
        const float t = h / static_cast<float>(textureResolution) * 2.0f; // 1 texel maps to one pixel

        float normalsArray[] = {
            -1.0f, -1.0f, 1.0f,
            -1.0f + s, -1.0f, 1.0f,
            -1.0f + s, -1.0f + t, 1.0f,
            -1.0f, -1.0f + t, 1.0f
        };
        const ramses::Vector3fArray* normals = m_client.createConstVector3fArray(4, normalsArray);

        ramses::TextureCube* texture = m_client.createTextureCube(
            textureResolution,
            ramses::ETextureFormat_RGB8,
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
        orthoCamera->setFrustum(0.0f, static_cast<Float>(DefaultDisplayWidth), 0.0f, static_cast<Float>(DefaultDisplayHeight), 0.1f, 10.f);
        orthoCamera->setViewport(0, 0, DefaultDisplayWidth, DefaultDisplayHeight);
        setCameraToDefaultRenderPass(orthoCamera);
    }

    ramses::Appearance* TextureCubeAnisotropicTextureFilteringScene::createQuad(
        ramses::Effect* effect,
        const ramses::Vector3fArray* vertexPositions,
        const ramses::Vector3fArray* normals,
        const ramses::UInt16Array* indices,
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
        translateNode->setTranslation(x, y, 0.0f);

        mesh->setParent(*translateNode);

        return appearance;
    }
}
