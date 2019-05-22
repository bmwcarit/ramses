//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/Texture2DGenerateMipMapScene.h"
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
#include "ramses-client-api/OrthographicCamera.h"

// Uses mip-map generation to generate LOD level-1 from level-0 data. Draws two quads, for the left one, 2x2 texels
// map to one pixel, thus LOD level-1 is used, which is the average of the four level-0 texels (red, green, blue, white).
// The right quad shows large magnification, LOD level-0 is taken with red, green, blue, white texels.

namespace ramses_internal
{

    Texture2DGenerateMipMapScene::Texture2DGenerateMipMapScene(ramses::RamsesClient& ramsesClient, ramses::Scene& scene, uint32_t state, const Vector3& cameraPosition)
        : IntegrationScene(ramsesClient, scene, cameraPosition)
    {
        createOrthoCamera();

        switch (state)
        {
        case EState_GenerateMipMapSingle:
            createMesh(*createTexture2DSampler());
            break;
        case EState_GenerateMipMapMultiple:
            for (UInt8 i = 0; i < 8; i++)
            {
                // Note: here we need different texture content, otherwise ramses uses
                // the already known resource with the same content.
                // In that case, glGlenerateMipMaps would be called only once!
                // Here, transparency parameter is used to generate different texture content.
                ramses::TextureSampler* sampler = createTexture2DSampler(256u, 2048u, i * 20);
                createMesh(*sampler, i * DefaultDisplayWidth * 0.1f, 0.5f);
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
        orthoCamera->setFrustum(0.0f, static_cast<Float>(DefaultDisplayWidth), 0.0f, static_cast<Float>(DefaultDisplayHeight), 0.1f, 10.f);
        orthoCamera->setViewport(0, 0, DefaultDisplayWidth, DefaultDisplayHeight);
        setCameraToDefaultRenderPass(orthoCamera);
    }

    void Texture2DGenerateMipMapScene::createMesh(const ramses::TextureSampler& sampler, float translateXY, float scale)
    {
        ramses::Effect* effect = getTestEffect("ramses-test-client-textured");
        assert(effect != 0);

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

        ramses::Node* transform = m_scene.createNode();
        transform->translate(translateXY, translateXY, 0.0f);
        transform->scale(scale, scale, scale);
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
        const float w = static_cast<float>(DefaultDisplayWidth) * 0.5f;
        const float h = static_cast<float>(DefaultDisplayHeight);

        const float x2 = w;
        const float y2 = 0.0f;
        const float w2 = w;
        const float h2 = h;

        const float vertexPositionsArray[] = {
            x, y, z,
            x + w, y, z,
            x + w, y + h, z,
            x, y + h, z,

            x2, y2, z,
            x2 + w2, y2, z,
            x2 + w2, y2 + h2, z,
            x2, y2 + h2, z,
        };

        const float s = w;
        const float t = h;

        const float s2 = w2 / 32.0f;
        const float t2 = h2 / 32.0f;

        const float textureCoordsArray[] = {
            0.0f, 0.0f,
            s, 0.0f,
            s, t,
            0.0f, t,

            0.0f, 0.0f,
            s2, 0.0f,
            s2, t2,
            0.0f, t2
        };

        m_indexArray = m_client.createConstUInt16Array(12, indicesArray);
        m_vertexPositions = m_client.createConstVector3fArray(8, vertexPositionsArray);
        m_textureCoords = m_client.createConstVector2fArray(8, textureCoordsArray);
    }

    ramses::TextureSampler* Texture2DGenerateMipMapScene::createTexture2DSampler(UInt32 width, UInt32 height, UInt8 transparency)
    {
        const UInt32 pixelCount = width * height;

        const UInt32 idx_green = pixelCount;
        const UInt32 idx_blue = pixelCount * 2;
        const UInt32 idx_white = pixelCount * 3;

        UInt8* rgba8_level0 = new UInt8[pixelCount * 4];
        for (UInt32 pixel = 0u; pixel < pixelCount; pixel++)
        {
            const UInt32 idx = pixel * 4;
            if (idx < idx_green)
            {
                //red
                rgba8_level0[idx] = 0xff;
                rgba8_level0[idx + 1] = 0x00;
                rgba8_level0[idx + 2] = 0x00;
            }
            else if (idx < idx_blue)
            {
                //green
                rgba8_level0[idx] = 0x00;
                rgba8_level0[idx + 1] = 0xff;
                rgba8_level0[idx + 2] = 0x00;
            }
            else if (idx < idx_white)
            {
                //blue
                rgba8_level0[idx] = 0x00;
                rgba8_level0[idx + 1] = 0x00;
                rgba8_level0[idx + 2] = 0xff;
            }
            else
            {
                //white
                rgba8_level0[idx] = 0xff;
                rgba8_level0[idx + 1] = 0xff;
                rgba8_level0[idx + 2] = 0xff;
            }
            // opacity
            rgba8_level0[idx + 3] = 0xff - transparency;
        }

        const ramses::MipLevelData mipLevelData[] = {
            ramses::MipLevelData(width*height*4u*sizeof(UInt8), rgba8_level0)
        };

        ramses::Texture2D* texture = m_client.createTexture2D(
            width, height,
            ramses::ETextureFormat_RGBA8,
            1,
            mipLevelData,
            true);

        ramses::TextureSampler* sampler = m_scene.createTextureSampler(
            ramses::ETextureAddressMode_Repeat,
            ramses::ETextureAddressMode_Repeat,
            ramses::ETextureSamplingMethod_Nearest_MipMapNearest,
            ramses::ETextureSamplingMethod_Nearest,
            *texture);

        delete[] rgba8_level0;

        return sampler;
    }
}
