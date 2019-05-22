//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/Texture2DFormatScene.h"
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


/*
To test the texture data format and the sampling we use a simple 2x2 texture.
*/

const uint16_t rgba4Data[] =
{
    0xf00f,
    0x0f0f,
    0x00ff,
    0xfff7
};
const ramses::MipLevelData mipLevelData_rgba4(sizeof(rgba4Data), reinterpret_cast<const uint8_t*>(rgba4Data));

const uint16_t rgba5551Data[] =
{
    0xf801,
    0x07c1,
    0x003f,
    0xfffe
};
const ramses::MipLevelData mipLevelData_rgba5551(sizeof(rgba5551Data), reinterpret_cast<const uint8_t*>(rgba5551Data));

const uint8_t rgba8Data[] =
{
    0xff, 0x00, 0x00, 0xff,
    0x00, 0xff, 0x00, 0xff,
    0x00, 0x00, 0xff, 0xff,
    0xff, 0xff, 0xff, 0x7f
};
const ramses::MipLevelData mipLevelData_rgba8(sizeof(rgba8Data), reinterpret_cast<const uint8_t*>(rgba8Data));

const uint16_t rgba565Data[] =
{
    0xf800,
    0x07e0,
    0x001f,
    0xffff
};
const ramses::MipLevelData mipLevelData_rgba565(sizeof(rgba565Data), reinterpret_cast<const uint8_t*>(rgba565Data));

const uint8_t rgb8Data[] =
{
    0xff, 0x00, 0x00,
    0x00, 0xff, 0x00,
    0x00, 0x00, 0xff,
    0xff, 0xff, 0xff
};
const ramses::MipLevelData mipLevelData_rgb8(sizeof(rgb8Data), reinterpret_cast<const uint8_t*>(rgb8Data));

const uint8_t rg8Data[] =
{
    0xff, 0x3f,     // 1.00, 0.25
    0xff, 0x7f,     // 1.00, 0.50
    0x7f, 0xbf,     // 0.50, 0.75
    0x7f, 0x3f
};   // 0.50, 0.25
const ramses::MipLevelData mipLevelData_rg8(sizeof(rg8Data), reinterpret_cast<const uint8_t*>(rg8Data));

const uint8_t r8Data[] =
{
    0xff,       // 1.00
    0xbf,       // 0.75
    0x7f,       // 0.50
    0x3f
};     // 0.25
const ramses::MipLevelData mipLevelData_r8(sizeof(r8Data), reinterpret_cast<const uint8_t*>(r8Data));

const uint8_t bgr8Data[] =
{
    0x00, 0x00, 0xff,
    0x00, 0xff, 0x00,
    0xff, 0x00, 0x00,
    0xff, 0xff, 0xff
};
const ramses::MipLevelData mipLevelData_bgr8(sizeof(bgr8Data), reinterpret_cast<const uint8_t*>(bgr8Data));

const uint8_t bgra8Data[] =
{
    0x00, 0x00, 0xff, 0xff,
    0x00, 0xff, 0x00, 0xff,
    0xff, 0x00, 0x00, 0xff,
    0xff, 0xff, 0xff, 0x7f
};
const ramses::MipLevelData mipLevelData_bgra8(sizeof(bgra8Data), reinterpret_cast<const uint8_t*>(bgra8Data));

const uint64_t ETC2RGB[] =
{
    0xefee0f00efcaf004
};
const ramses::MipLevelData mipLevelData_etc2rgb(sizeof(ETC2RGB), reinterpret_cast<const uint8_t*>(ETC2RGB));

const uint64_t ETC2RGBA[] =
{
    0xefee1f11fd7fbb7b,
    0xefee0f00efcaf004
};
const ramses::MipLevelData mipLevelData_etc2rgba(ramses::MipLevelData(sizeof(ETC2RGBA), reinterpret_cast<const uint8_t*>(ETC2RGBA)));

const uint8_t r16fData[] =   // Value (sign, exponent, fraction)
{
    0x0, 0x3C,               // 1.0   (0 01111 0000000000)
    0x0, 0x38,               // 0.50  (0 01110 0000000000)
    0x0, 0x34,               // 0.25  (0 01101 0000000000)
    0x0, 0x0                 // 1.0   (0 00000 0000000000)
};
const ramses::MipLevelData mipLevelData_r16fData(sizeof(r16fData), reinterpret_cast<const uint8_t*>(r16fData));

const float r32fData[] =
{
    1.0f,
    0.5f,
    0.25f,
    0.0f
};
const ramses::MipLevelData mipLevelData_r32fData(sizeof(r32fData), reinterpret_cast<const uint8_t*>(r32fData));

// 4x4 block with various colors encoded as ASTC 4x4 RGBA
const uint8_t astcRGBA4x4Data[] =
{
    0xDE, 0x69, 0x1C, 0x10, 0x05, 0x42, 0x0B, 0x82, 0x20, 0x00, 0x15, 0x00, 0x00, 0x04, 0x00, 0x60
};
const ramses::MipLevelData mipLevelData_astcRGBA4x4Data(sizeof(astcRGBA4x4Data), reinterpret_cast<const uint8_t*>(astcRGBA4x4Data));

// Similar as above, but now encoded as SRGBA
const uint8_t astcSRGB_Alpha4x4Data[] =
{
    0xDE, 0x09, 0x2C, 0x50, 0x00, 0x02, 0x0A, 0x82, 0x20, 0x00, 0x0B, 0x00, 0xA0, 0xC7, 0x02, 0xF8
};
const ramses::MipLevelData mipLevelData_astcSRGB_Alpha_4x4Data(sizeof(astcSRGB_Alpha4x4Data), reinterpret_cast<const uint8_t*>(astcSRGB_Alpha4x4Data));

const uint8_t rg16fData[] =
{
    0x0, 0x3C, 0x0, 0x0,     // 1, 0
    0x0, 0x0, 0x0, 0x3C,     // 0, 1
    0x0, 0x0, 0x0, 0x0,      // 0, 0
    0x0, 0x3C, 0x0, 0x3C     // 1, 1
};
const ramses::MipLevelData mipLevelData_rg16fData(sizeof(rg16fData), reinterpret_cast<const uint8_t*>(rg16fData));

const float rg32fData[] =
{
    1.0f, 0.0f,
    0.0f, 1.0f,
    0.0f, 0.0f,
    1.0f, 1.0f
};
const ramses::MipLevelData mipLevelData_rg32fData(sizeof(rg32fData), reinterpret_cast<const uint8_t*>(rg32fData));

const uint8_t rgb16fData[] =
{
    0x0, 0x3C, 0x0, 0x0, 0x0, 0x0,     // 1, 0, 0
    0x0, 0x0, 0x0, 0x3C, 0x0, 0x0,     // 0, 1, 0
    0x0, 0x0, 0x0, 0x0, 0x0, 0x3C,     // 0, 0, 1
    0x0, 0x3C, 0x0, 0x3C, 0x0, 0x3C    // 1, 1, 1
};
const ramses::MipLevelData mipLevelData_rgb16fData(sizeof(rgb16fData), reinterpret_cast<const uint8_t*>(rgb16fData));

const float rgb32fData[] =
{
    1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 1.0f,
    1.0f, 1.0f, 1.0f
};
const ramses::MipLevelData mipLevelData_rgb32fData(sizeof(rgb32fData), reinterpret_cast<const uint8_t*>(rgb32fData));

const uint8_t rgba16fData[] =
{
    0x0, 0x3C, 0x0, 0x0, 0x0, 0x0, 0x0, 0x3C,     // 1, 0, 0, 1
    0x0, 0x0, 0x0, 0x3C, 0x0, 0x0, 0x0, 0x3C,     // 0, 1, 0, 1
    0x0, 0x0, 0x0, 0x0, 0x0, 0x3C, 0x0, 0x3C,     // 0, 0, 1, 1
    0x0, 0x3C, 0x0, 0x3C, 0x0, 0x3C, 0x0, 0x3C    // 1, 1, 1, 1
};
const ramses::MipLevelData mipLevelData_rgba16fData(sizeof(rgba16fData), reinterpret_cast<const uint8_t*>(rgba16fData));

const float rgba32fData[] =
{
    1.0f, 0.0f, 0.0f, 1.0f,
    0.0f, 1.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f, 1.0f
};
const ramses::MipLevelData mipLevelData_rgba32fData(sizeof(rgba32fData), reinterpret_cast<const uint8_t*>(rgba32fData));

const uint8_t srgb8Data[] =
{
    0x88, 0x00, 0x00,
    0x00, 0x88, 0x00,
    0x00, 0x00, 0x88,
    0x88, 0x88, 0x88
};
const ramses::MipLevelData mipLevelData_srgb8Data(sizeof(srgb8Data), reinterpret_cast<const uint8_t*>(srgb8Data));

const uint8_t srgb8a8Data[] =
{
    0x88, 0x00, 0x00, 0x88,
    0x00, 0x88, 0x00, 0x88,
    0x00, 0x00, 0x88, 0x88,
    0x88, 0x88, 0x88, 0xff
};
const ramses::MipLevelData mipLevelData_srgb8a8Data(sizeof(srgb8a8Data), reinterpret_cast<const uint8_t*>(srgb8a8Data));

const ramses::MipLevelData mipLevelData_null;

namespace ramses_internal
{
    Texture2DFormatScene::Texture2DFormatScene(ramses::RamsesClient& ramsesClient, ramses::Scene& scene, uint32_t state, const Vector3& cameraPosition)
        : IntegrationScene(ramsesClient, scene, cameraPosition)
    {
        createOrthoCamera();

        ramses::ETextureFormat format(ramses::ETextureFormat_R8);
        const ramses::MipLevelData& mipLevelData = getTextureFormatAndData(static_cast<EState>(state), format);

        ramses::Texture2D* texture = m_client.createTexture2D(
            2, 2,
            format,
            1,
            &mipLevelData,
            false);

        ramses::TextureSampler* sampler = m_scene.createTextureSampler(
            ramses::ETextureAddressMode_Repeat,
            ramses::ETextureAddressMode_Repeat,
            ramses::ETextureSamplingMethod_Nearest,
            ramses::ETextureSamplingMethod_Nearest,
            *texture);

        createQuad(*sampler);
    }

    void Texture2DFormatScene::createOrthoCamera()
    {
        ramses::OrthographicCamera* orthoCamera(m_scene.createOrthographicCamera());
        orthoCamera->setFrustum(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 10.f);
        orthoCamera->setViewport(0, 0, DefaultDisplayWidth, DefaultDisplayHeight);
        setCameraToDefaultRenderPass(orthoCamera);
    }

    void Texture2DFormatScene::createQuad(const ramses::TextureSampler& sampler)
    {
        uint16_t indicesArray[] = { 0, 1, 2, 2, 1, 3 };
        const ramses::UInt16Array* indices = m_client.createConstUInt16Array(6, indicesArray);

        float vertexPositionsArray[] = {
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, 1.0f, -1.0f,
            1.0f, 1.0f, -1.0f
        };

        const ramses::Vector3fArray* vertexPositions = m_client.createConstVector3fArray(4, vertexPositionsArray);

        float textureCoordsArray[] = { 0.0f, 1.0f,
            1.0f, 1.0f,
            0.0f, 0.0f,
            1.0f, 0.0f
        };

        const ramses::Vector2fArray* textureCoords = m_client.createConstVector2fArray(4, textureCoordsArray);

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
        appearance->setBlendingFactors(ramses::EBlendFactor_SrcAlpha, ramses::EBlendFactor_OneMinusSrcAlpha, ramses::EBlendFactor_One, ramses::EBlendFactor_OneMinusSrcAlpha);
        appearance->setBlendingOperations(ramses::EBlendOperation_Add, ramses::EBlendOperation_Add);

        ramses::GeometryBinding* geometry = m_scene.createGeometryBinding(*effect);
        geometry->setIndices(*indices);
        geometry->setInputBuffer(positionsInput, *vertexPositions);
        geometry->setInputBuffer(texCoordsInput, *textureCoords);

        ramses::MeshNode* mesh = m_scene.createMeshNode();
        addMeshNodeToDefaultRenderGroup(*mesh);
        mesh->setAppearance(*appearance);
        mesh->setGeometryBinding(*geometry);
    }

    const ramses::MipLevelData& Texture2DFormatScene::getTextureFormatAndData(EState state, ramses::ETextureFormat &format) const
    {
        switch (state)
        {
        case EState_R8:
            format = ramses::ETextureFormat_R8;
            return mipLevelData_r8;
        case EState_RG8:
            format = ramses::ETextureFormat_RG8;
            return mipLevelData_rg8;
        case EState_RGB8:
            format = ramses::ETextureFormat_RGB8;
            return mipLevelData_rgb8;
        case EState_RGB565:
            format = ramses::ETextureFormat_RGB565;
            return mipLevelData_rgba565;
        case EState_RGBA8:
            format = ramses::ETextureFormat_RGBA8;
            return mipLevelData_rgba8;
        case EState_RGBA4:
            format = ramses::ETextureFormat_RGBA4;
            return mipLevelData_rgba4;
        case EState_RGBA5551:
            format = ramses::ETextureFormat_RGBA5551;
            return mipLevelData_rgba5551;
        case EState_BGR8:
            format = ramses::ETextureFormat_BGR8;
            return mipLevelData_bgr8;
        case EState_BGRA8:
            format = ramses::ETextureFormat_BGRA8;
            return mipLevelData_bgra8;
        case EState_ETC2RGB:
            format = ramses::ETextureFormat_ETC2RGB;
            return mipLevelData_etc2rgb;
        case EState_ETC2RGBA:
            format = ramses::ETextureFormat_ETC2RGBA;
            return mipLevelData_etc2rgba;

        case EState_R16F:
            format = ramses::ETextureFormat_R16F;
            return mipLevelData_r16fData;
        case EState_R32F:
            format = ramses::ETextureFormat_R32F;
            return mipLevelData_r32fData;
        case EState_RG16F:
            format = ramses::ETextureFormat_RG16F;
            return mipLevelData_rg16fData;
        case EState_RG32F:
            format = ramses::ETextureFormat_RG32F;
            return mipLevelData_rg32fData;
        case EState_RGB16F:
            format = ramses::ETextureFormat_RGB16F;
            return mipLevelData_rgb16fData;
        case EState_RGB32F:
            format = ramses::ETextureFormat_RGB32F;
            return mipLevelData_rgb32fData;
        case EState_RGBA16F:
            format = ramses::ETextureFormat_RGBA16F;
            return mipLevelData_rgba16fData;
        case EState_RGBA32F:
            format = ramses::ETextureFormat_RGBA32F;
            return mipLevelData_rgba32fData;

        case EState_SRGB8:
            format = ramses::ETextureFormat_SRGB8;
            return mipLevelData_srgb8Data;
        case EState_SRGB8_ALPHA8:
            format = ramses::ETextureFormat_SRGB8_ALPHA8;
            return mipLevelData_srgb8a8Data;

        case EState_ASTC_RGBA_4x4:
            format = ramses::ETextureFormat_ASTC_RGBA_4x4;
            return mipLevelData_astcRGBA4x4Data;
        case EState_ASTC_SRGB_ALPHA_4x4:
            format = ramses::ETextureFormat_ASTC_SRGBA_4x4;
            return mipLevelData_astcSRGB_Alpha_4x4Data;

        default:
            assert(false && "Unknown texture data format.");
            return mipLevelData_null;
        };
    }
}
