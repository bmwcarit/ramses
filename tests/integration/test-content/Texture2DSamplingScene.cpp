//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/Texture2DSamplingScene.h"
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

// The magic of these tests is that they must show a significant difference in the output picture, when the wrong sampling mode is
// enabled. On the other side they must be stable across different OpenGL implementations in different platforms.
// Testing the minification modes, means that multiple texels must map to a single pixel, so the difference between right and wrong
// really lies in single pixels values. This tests are solely 2D, and are carefully choosen for stable output, although dealing in
// the sub-pixel ranges.
// The output picture of the tests are looking quite synthetic - stripes and dotted pixel patterns - so not trivial to see
// why it must look like that, that's why the expected result is explained in the createGeometry methods for the different modes.

namespace ramses::internal
{

    Texture2DSamplingScene::Texture2DSamplingScene(ramses::Scene& scene, uint32_t state, const glm::vec3& cameraPosition)
        : IntegrationScene(scene, cameraPosition)
    {
        createOrthoCamera();

        const auto enumState = static_cast<EState>(state);

        const uint8_t rgb8_level0[] = {
            0xff,0xff,0xff,  0xff,0xff,0xff, 0x00,0x00,0x00, 0x00,0x00,0x00,
            0xff,0xff,0xff,  0xff,0xff,0xff, 0x00,0x00,0x00, 0x00,0x00,0x00,
            0xff,0x00,0x00,  0xff,0x00,0x00, 0x00,0x00,0xff,  0x00,0x00,0xff,
            0xff,0x00,0x00,  0xff,0x00,0x00, 0x00,0x00,0xff,  0x00,0x00,0xff,
        };

        const uint8_t rgb8_level1[] = {
            0xff,0xff,0x00, 0x00,0x00,0xff,
            0xff,0xff,0x00, 0x00,0x00,0xff
        };

        const uint8_t rgb8_level2[] = {
            0x00,0xff,0x00
        };

        const ramses::MipLevelData mipLevelData[] = {
            ramses::MipLevelData(sizeof(rgb8_level0), rgb8_level0),
            ramses::MipLevelData(sizeof(rgb8_level1), rgb8_level1),
            ramses::MipLevelData(sizeof(rgb8_level2), rgb8_level2)
        };

        ramses::Texture2D* texture = m_scene.createTexture2D(
            ramses::ETextureFormat::RGB8,
            4, 4,
            3,
            mipLevelData,
            false);

        ramses::TextureSampler* sampler = m_scene.createTextureSampler(
            ramses::ETextureAddressMode::Repeat,
            ramses::ETextureAddressMode::Repeat,
            GetMinSamplingMethod(enumState),
            GetMagSamplingMethod(enumState),
            *texture);

        createMesh(*sampler, enumState);
    }

    void Texture2DSamplingScene::createOrthoCamera()
    {
        ramses::OrthographicCamera* orthoCamera(m_scene.createOrthographicCamera());
        orthoCamera->setFrustum(0.0f, static_cast<float>(IntegrationScene::DefaultViewportWidth), 0.0f, static_cast<float>(IntegrationScene::DefaultViewportHeight), 0.1f, 10.f);
        orthoCamera->setViewport(0, 0, IntegrationScene::DefaultViewportWidth, IntegrationScene::DefaultViewportHeight);
        setCameraToDefaultRenderPass(orthoCamera);
    }

    void Texture2DSamplingScene::createGeometry(EState state)
    {
        switch (state)
        {
        case EState_Nearest:
            createGeometryNearest();
            break;

        case EState_Bilinear:
            createGeometryBilinear();
            break;

        case EState_MinLinearMagNearest:
            createGeometryMinLinearMagNearest();
            break;

        case EState_MinNearestMagLinear:
            createGeometryMinNearestMagLinear();
            break;

        case EState_NearestWithMipMaps:
            createGeometryNearestWithMipMaps();
            break;

        case EState_BilinearWithMipMaps:
            createGeometryBilinearWithMipMaps();
            break;


        case EState_Trilinear:
            createGeometryTrilinear();
            break;
        }
    }

    void Texture2DSamplingScene::createMesh(const ramses::TextureSampler& sampler, EState state)
    {
        ramses::Effect* effect = getTestEffect("ramses-test-client-textured");
        assert(effect != nullptr);

        ramses::Appearance* appearance = m_scene.createAppearance(*effect);
        appearance->setInputTexture(*effect->findUniformInput("u_texture"), sampler);

        ramses::Geometry* geometry = m_scene.createGeometry(*effect);

        createGeometry(state);

        geometry->setIndices(*m_indexArray);
        geometry->setInputBuffer(*effect->findAttributeInput("a_position"), *m_vertexPositions);
        geometry->setInputBuffer(*effect->findAttributeInput("a_texcoord"), *m_textureCoords);

        ramses::MeshNode* mesh = m_scene.createMeshNode();
        addMeshNodeToDefaultRenderGroup(*mesh);
        mesh->setAppearance(*appearance);
        mesh->setGeometry(*geometry);
    }

    ramses::ETextureSamplingMethod Texture2DSamplingScene::GetMinSamplingMethod(EState state)
    {
        switch (state)
        {
        case EState_Nearest:
        case EState_MinNearestMagLinear:
            return ramses::ETextureSamplingMethod::Nearest;

        case EState_NearestWithMipMaps:
            return ramses::ETextureSamplingMethod::Nearest_MipMapNearest;

        case EState_Bilinear:
        case EState_MinLinearMagNearest:
            return ramses::ETextureSamplingMethod::Linear;

        case EState_BilinearWithMipMaps:
            return ramses::ETextureSamplingMethod::Linear_MipMapNearest;

        case EState_Trilinear:
            return ramses::ETextureSamplingMethod::Linear_MipMapLinear;
        default:
            assert(false);
            return ramses::ETextureSamplingMethod::Nearest;
        }
    }

    ramses::ETextureSamplingMethod Texture2DSamplingScene::GetMagSamplingMethod(EState state)
    {
        switch (state)
        {
        case EState_Nearest:
        case EState_NearestWithMipMaps:
        case EState_MinLinearMagNearest:
            return ramses::ETextureSamplingMethod::Nearest;

        case EState_Bilinear:
        case EState_BilinearWithMipMaps:
        case EState_Trilinear:
        case EState_MinNearestMagLinear:
            return ramses::ETextureSamplingMethod::Linear;

        default:
            assert(false);
            return ramses::ETextureSamplingMethod::Nearest;
        }
    }

    void Texture2DSamplingScene::createGeometryNearest()
    {
        // Draws two quads, for the left one, two texels (white/black or red/blue) map to one pixel,
        // quad is moved 0.45 pixel(equal to 0.9 texel), makes no difference for nearest filtering, but would be
        // looking different for linear filtering - without that movement MinLinearMagNearest would produce the same picture.
        // The right quad shows large magnification, LOD level-0 is taken, thus white/black/red/blue without gradient.

        createTwoQuads(0.45f);
    }

    void Texture2DSamplingScene::createGeometryBilinear()
    {
        // Draws two quads, for the left one, two texels (white/black or red/blue) map to one pixel,
        // quad is moved half a pixel, such that the pixel center falls exactly between two texels, thus alternative lines
        // with grey (127,127,127) and (127,0,127) are the result.
        // The right quad shows large magnification, LOD level-0 is taken and bilinear filtered, thus white/black/red/blue with gradient.

        createTwoQuads(0.5f);
    }

    void Texture2DSamplingScene::createGeometryMinLinearMagNearest()
    {
        // Draws two quads, for the left one, two texels (white/black or red/blue) map to one pixel (min filtering case),
        // quad is moved half a pixel, such that the pixel center falls exactly between two texels, thus alternative lines
        // with grey (127,127,127) and (127,0,127) are the result.
        // The right quad shows large magnification, LOD level-0 is taken, thus white/black/red/blue without gradient.

        createTwoQuads(0.5f);
    }

    void Texture2DSamplingScene::createGeometryMinNearestMagLinear()
    {
        // Draws two quads, for the left one, two texels (white/black or red/blue) map to one pixel,
        // quad is moved 0.45 pixel(equal to 0.9 texel), makes no difference for nearest filtering, but would be
        // looking different for linear filtering - without that movement Bilinear would produce the same picture.
        // The right quad shows large magnification, LOD level-0 is taken and bilinear filtered, thus white/black/red/blue with gradient.

        createTwoQuads(0.45f);
    }

    void Texture2DSamplingScene::createGeometryNearestWithMipMaps()
    {
        // Draws two quads, for the left one, two texels map to one pixel, LOD level-1 fits perfectly, thus yellow/blue stripes.
        // The right quad shows large magnification, LOD level-0 is taken, thus white/black/red/blue without gradient.

        createTwoQuads(0.0f);
    }

    void Texture2DSamplingScene::createGeometryBilinearWithMipMaps()
    {
        // Draws two quads, for the left one, two texels map to one pixel, LOD level-1 fits perfectly, thus yellow/blue stripes.
        // The right quad shows large magnification, LOD level-0 is taken and bilinear filtered, thus white/black/red/blue with gradient.

        createTwoQuads(0.0f);
    }

    void Texture2DSamplingScene::createTwoQuads(float x)
    {
        const float z = -1.0f;

        const uint16_t indicesArray[] = {
            0, 1, 2, 0, 2, 3,
            4, 5, 6, 4, 6, 7,
        };

        const float y = 0.0f;
        const float w = static_cast<float>(IntegrationScene::DefaultViewportWidth) / 2.0f;
        const auto h = static_cast<float>(IntegrationScene::DefaultViewportHeight);

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

        const float s = w / 2.0f;
        const float t = h / 2.0f;

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

    void Texture2DSamplingScene::createGeometryTrilinear()
    {
        // Draws three quads.
        // Left quad maps 2x2 texel to 1 pixel => LOD Level 1 maps pixel perfectly, yellow and blue vertical stripes in picture.
        // Right quad shows magnification of LOD Level 0 with filtering between the white, black, red, blue texels.
        // The middle quad maps 3x3 texel to 1 pixel => so λ = ld(3): mixture between level-1 and level-2 LOD
        //    - level-2 factor: frac(ld(3))
        //    - level-1 factor: 1 - frac(ld(3))
        //    - Bilinear filtering in level-1:
        //
        //      pixel sampling point: \/    \/    \/    \/    \/    \/    \/    \/
        //      level-1 texel:        YY  BB  YY  BB  YY  BB  YY  BB  YY  BB  YY  BB
        //
        //      First pixel matches center of first texel => Color(255,255,0)
        //      Second pixel lies in the middle between two texels => Mixture of yellow and blue => Color(127, 127, 127)
        //      Third pixel matches center of second texl => Color(0,0,255)
        //      Fourth pixel lies in the middle between two texels (same as second) => Color(127, 127, 127)
        //      ... Repeats with first, second, third, fourth pixel again.
        //
        //    - When the four different level-1 colors from bilinear filtering above are blended with level-2 color you got the final color:
        //        - (1 - frac(ld(3))) * Color(255,255,0) + frac(ld(3)) * Color(0,255,0)   = Color(106, 255, 0)
        //        - (1 - frac(ld(3))) * Color(127,127,127) + frac(ld(3)) * Color(0,255,0) = Color(53, 202, 53)
        //        - (1 - frac(ld(3))) * Color(0,0,255) + frac(ld(3)) * Color(0,255,0)     = Color(0, 149, 106)
        //        - (1 - frac(ld(3))) * Color(127,127,127) + frac(ld(3)) * Color(0,255,0) = Color(53, 202, 53)
        //    - These four colors are the colors of the vertical stripes.

        const float z = -1.0f;

        const uint16_t indicesArray[] = {
            0, 1, 2, 0, 2, 3,
            4, 5, 6, 4, 6, 7,
            8, 9, 10, 8, 10, 11
        };

        const float x = 0.0f;
        const float y = 0.0f;
        const float w = static_cast<float>(IntegrationScene::DefaultViewportWidth) * 0.35f;
        const auto h = static_cast<float>(IntegrationScene::DefaultViewportHeight);

        const float x2 = static_cast<float>(IntegrationScene::DefaultViewportWidth) - w;
        const float y2 = 0.0f;
        const float w2 = w;
        const float h2 = h;

        const float x3 = w + 1.0f/6.0f;
        // Move first sample in midpoint of first LOD level-1 texel
        // One pixel covers 3 level-0 texel, midpoint has to be moved half a texel left, which is 1/6 in geometry.

        const float y3 = 0.0f;
        const float w3 = static_cast<float>(IntegrationScene::DefaultViewportWidth) * 0.3f;
        const float h3 = h;

        const std::array<ramses::vec3f, 12u> vertexPositionsArray{
            ramses::vec3f{ x, y, z },
            ramses::vec3f{ x + w, y, z },
            ramses::vec3f{ x + w, y + h, z },
            ramses::vec3f{ x, y + h, z },

            ramses::vec3f{ x2, y2, z },
            ramses::vec3f{ x2 + w2, y2, z },
            ramses::vec3f{ x2 + w2, y2 + h2, z },
            ramses::vec3f{ x2, y2 + h2, z },

            ramses::vec3f{ x3, y3, z },
            ramses::vec3f{ x3 + w3, y3, z },
            ramses::vec3f{ x3 + w3, y3 + h3, z },
            ramses::vec3f{ x3, y3 + h3, z }
        };

        const float s = w / 2.0f;
        const float t = h / 2.0f;

        const float s2 = w2 / 32.0f;
        const float t2 = h2 / 32.0f;

        const float f = 3.0f;
        const float s3 = w3 * f / 4.0f;
        const float t3 = h3 * f / 4.0f;

        const std::array<ramses::vec2f, 12u> textureCoordsArray{
            ramses::vec2f{ 0.0f, 0.0f },
            ramses::vec2f{ s, 0.0f },
            ramses::vec2f{ s, t },
            ramses::vec2f{ 0.0f, t },

            ramses::vec2f{ 0.0f, 0.0f },
            ramses::vec2f{ s2, 0.0f },
            ramses::vec2f{ s2, t2 },
            ramses::vec2f{ 0.0f, t2 },

            ramses::vec2f{ 0.0f, 0.0f },
            ramses::vec2f{ s3, 0.0f },
            ramses::vec2f{ s3, t3 },
            ramses::vec2f{ 0.0f, t3 }
        };

        m_indexArray = m_scene.createArrayResource(18u, indicesArray);
        m_vertexPositions = m_scene.createArrayResource(12u, vertexPositionsArray.data());
        m_textureCoords = m_scene.createArrayResource(12u, textureCoordsArray.data());
    }
}
