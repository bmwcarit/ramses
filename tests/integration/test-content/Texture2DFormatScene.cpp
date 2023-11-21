//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/Texture2DFormatScene.h"
#include "ramses/client/ramses-utils.h"
#include "ramses/client/Scene.h"
#include "ramses/client/MeshNode.h"
#include "ramses/client/ArrayResource.h"
#include "ramses/client/Geometry.h"
#include "ramses/client/Appearance.h"
#include "ramses/client/Effect.h"
#include "ramses/client/AttributeInput.h"
#include "ramses/client/OrthographicCamera.h"
#include "ramses/client/TextureSwizzle.h"
#include "ramses/framework/TextureEnums.h"
#include <cassert>

/*
To test the texture data format and the sampling we use a simple 2x2 texture.
*/

const std::vector<uint16_t> rgba4Data =
{
    0xf00f,
    0x0f0f,
    0x00ff,
    0xfff7
};
const std::vector<ramses::MipLevelData> mipLevelData_rgba4{ ramses::internal::convertToBytes(rgba4Data) };

const std::vector<uint16_t> rgba5551Data =
{
    0xf801,
    0x07c1,
    0x003f,
    0xfffe
};
const std::vector<ramses::MipLevelData> mipLevelData_rgba5551{ ramses::internal::convertToBytes(rgba5551Data) };

const std::vector<std::byte> rgba8Data =
{
    std::byte{0xff}, std::byte{0x00}, std::byte{0x00}, std::byte{0xff},
    std::byte{0x00}, std::byte{0xff}, std::byte{0x00}, std::byte{0xff},
    std::byte{0x00}, std::byte{0x00}, std::byte{0xff}, std::byte{0xff},
    std::byte{0xff}, std::byte{0xff}, std::byte{0xff}, std::byte{0x7f}
};
const std::vector<ramses::MipLevelData> mipLevelData_rgba8{ rgba8Data };

const std::vector<uint16_t> rgba565Data =
{
    0xf800,
    0x07e0,
    0x001f,
    0xffff
};
const std::vector<ramses::MipLevelData> mipLevelData_rgba565{ ramses::internal::convertToBytes(rgba565Data) };

const std::vector<std::byte> rgb8Data =
{
    std::byte{0xff}, std::byte{0x00}, std::byte{0x00},
    std::byte{0x00}, std::byte{0xff}, std::byte{0x00},
    std::byte{0x00}, std::byte{0x00}, std::byte{0xff},
    std::byte{0xff}, std::byte{0xff}, std::byte{0xff}
};
const std::vector<ramses::MipLevelData> mipLevelData_rgb8{ rgb8Data };

const std::vector<std::byte> rg8Data =
{
    std::byte{0xff}, std::byte{0x3f},     // 1.00, 0.25
    std::byte{0xff}, std::byte{0x7f},     // 1.00, 0.50
    std::byte{0x7f}, std::byte{0xbf},     // 0.50, 0.75
    std::byte{0x7f}, std::byte{0x3f}
};   // 0.50, 0.25
const std::vector<ramses::MipLevelData> mipLevelData_rg8{ rg8Data };

const std::vector<std::byte> r8Data =
{
    std::byte{0xff},       // 1.00
    std::byte{0xbf},       // 0.75
    std::byte{0x7f},       // 0.50
    std::byte{0x3f}
};     // 0.25
const std::vector<ramses::MipLevelData> mipLevelData_r8{ r8Data };

const std::vector<std::byte> bgr8Data =
{
    std::byte{0x00}, std::byte{0x00}, std::byte{0xff},
    std::byte{0x00}, std::byte{0xff}, std::byte{0x00},
    std::byte{0xff}, std::byte{0x00}, std::byte{0x00},
    std::byte{0xff}, std::byte{0xff}, std::byte{0xff}
};
const std::vector<ramses::MipLevelData> mipLevelData_bgr8{ bgr8Data };

const std::vector<std::byte> bgra8Data =
{
    std::byte{0x00}, std::byte{0x00}, std::byte{0xff}, std::byte{0xff},
    std::byte{0x00}, std::byte{0xff}, std::byte{0x00}, std::byte{0xff},
    std::byte{0xff}, std::byte{0x00}, std::byte{0x00}, std::byte{0xff},
    std::byte{0xff}, std::byte{0xff}, std::byte{0xff}, std::byte{0x7f}
};
const std::vector<ramses::MipLevelData> mipLevelData_bgra8{ bgra8Data };

const std::vector<uint64_t> ETC2RGB =
{
    0xefee1f11fd7fbb7b
};
const std::vector<ramses::MipLevelData> mipLevelData_etc2rgb{ ramses::internal::convertToBytes(ETC2RGB) };

const std::vector<uint64_t> ETC2RGBA =
{
    0xefee1f11fd7fbb7b,
    0xefee0f00efcaf004
};
const std::vector<ramses::MipLevelData> mipLevelData_etc2rgba{ ramses::internal::convertToBytes(ETC2RGBA) };

const std::vector<std::byte> r16fData =   // Value (sign, exponent, fraction)
{
    std::byte{0x0}, std::byte{0x3C},               // 1.0   (0 01111 0000000000)
    std::byte{0x0}, std::byte{0x38},               // 0.50  (0 01110 0000000000)
    std::byte{0x0}, std::byte{0x34},               // 0.25  (0 01101 0000000000)
    std::byte{0x0}, std::byte{0x0}                 // 1.0   (0 00000 0000000000)
};
const std::vector<ramses::MipLevelData> mipLevelData_r16fData{ r16fData };

const std::vector<float> r32fData =
{
    1.0f,
    0.5f,
    0.25f,
    0.0f
};
const std::vector<ramses::MipLevelData> mipLevelData_r32fData{ ramses::internal::convertToBytes(r32fData) };

// 4x4 block with various colors encoded as ASTC 4x4 RGBA
const std::vector<std::byte> astcRGBA4x4Data =
{
    std::byte{0xDE}, std::byte{0x69}, std::byte{0x1C}, std::byte{0x10}, std::byte{0x05}, std::byte{0x42}, std::byte{0x0B}, std::byte{0x82}, std::byte{0x20}, std::byte{0x00}, std::byte{0x15}, std::byte{0x00}, std::byte{0x00}, std::byte{0x04}, std::byte{0x00}, std::byte{0x60}
};
const std::vector<ramses::MipLevelData> mipLevelData_astcRGBA4x4Data{ astcRGBA4x4Data };

// Similar as above, but now encoded as SRGBA
const std::vector<std::byte> astcSRGB_Alpha4x4Data =
{
    std::byte{0xDE}, std::byte{0x09}, std::byte{0x2C}, std::byte{0x50}, std::byte{0x00}, std::byte{0x02}, std::byte{0x0A}, std::byte{0x82}, std::byte{0x20}, std::byte{0x00}, std::byte{0x0B}, std::byte{0x00}, std::byte{0xA0}, std::byte{0xC7}, std::byte{0x02}, std::byte{0xF8}
};
const std::vector<ramses::MipLevelData> mipLevelData_astcSRGB_Alpha_4x4Data{ ramses::internal::convertToBytes(astcSRGB_Alpha4x4Data) };

const std::vector<std::byte> rg16fData =
{
    std::byte{0x0}, std::byte{0x3C}, std::byte{0x0}, std::byte{0x0},     // 1, 0
    std::byte{0x0}, std::byte{0x0},  std::byte{0x0}, std::byte{0x3C},    // 0, 1
    std::byte{0x0}, std::byte{0x0},  std::byte{0x0}, std::byte{0x0},     // 0, 0
    std::byte{0x0}, std::byte{0x3C}, std::byte{0x0}, std::byte{0x3C}     // 1, 1
};
const std::vector<ramses::MipLevelData> mipLevelData_rg16fData{ rg16fData };

const std::vector<float> rg32fData =
{
    1.0f, 0.0f,
    0.0f, 1.0f,
    0.0f, 0.0f,
    1.0f, 1.0f
};
const std::vector<ramses::MipLevelData> mipLevelData_rg32fData{ ramses::internal::convertToBytes(rg32fData) };

const std::vector<std::byte> rgb16fData =
{
    std::byte{0x0}, std::byte{0x3C}, std::byte{0x0}, std::byte{0x0}, std::byte{0x0}, std::byte{0x0},     // 1, 0, 0
    std::byte{0x0}, std::byte{0x0}, std::byte{0x0}, std::byte{0x3C}, std::byte{0x0}, std::byte{0x0},     // 0, 1, 0
    std::byte{0x0}, std::byte{0x0}, std::byte{0x0}, std::byte{0x0}, std::byte{0x0}, std::byte{0x3C},     // 0, 0, 1
    std::byte{0x0}, std::byte{0x3C}, std::byte{0x0}, std::byte{0x3C}, std::byte{0x0}, std::byte{0x3C}    // 1, 1, 1
};
const std::vector<ramses::MipLevelData> mipLevelData_rgb16fData{ ramses::internal::convertToBytes(rgb16fData) };

const std::vector<float> rgb32fData =
{
    1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 1.0f,
    1.0f, 1.0f, 1.0f
};
const std::vector<ramses::MipLevelData> mipLevelData_rgb32fData{ ramses::internal::convertToBytes(rgb32fData) };

const std::vector<std::byte> rgba16fData =
{
    std::byte{0x0}, std::byte{0x3C}, std::byte{0x0}, std::byte{0x0},  std::byte{0x0}, std::byte{0x0},  std::byte{0x0}, std::byte{0x3C},   // 1, 0, 0, 1
    std::byte{0x0}, std::byte{0x0},  std::byte{0x0}, std::byte{0x3C}, std::byte{0x0}, std::byte{0x0},  std::byte{0x0}, std::byte{0x3C},   // 0, 1, 0, 1
    std::byte{0x0}, std::byte{0x0},  std::byte{0x0}, std::byte{0x0},  std::byte{0x0}, std::byte{0x3C}, std::byte{0x0}, std::byte{0x3C},   // 0, 0, 1, 1
    std::byte{0x0}, std::byte{0x3C}, std::byte{0x0}, std::byte{0x3C}, std::byte{0x0}, std::byte{0x3C}, std::byte{0x0}, std::byte{0x3C}    // 1, 1, 1, 1
};
const std::vector<ramses::MipLevelData> mipLevelData_rgba16fData{ rgba16fData };

const std::vector<float> rgba32fData =
{
    1.0f, 0.0f, 0.0f, 1.0f,
    0.0f, 1.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f, 1.0f
};
const std::vector<ramses::MipLevelData> mipLevelData_rgba32fData{ ramses::internal::convertToBytes(rgba32fData) };

const std::vector<std::byte> srgb8Data =
{
    std::byte{0x88}, std::byte{0x00}, std::byte{0x00},
    std::byte{0x00}, std::byte{0x88}, std::byte{0x00},
    std::byte{0x00}, std::byte{0x00}, std::byte{0x88},
    std::byte{0x88}, std::byte{0x88}, std::byte{0x88}
};
const std::vector<ramses::MipLevelData> mipLevelData_srgb8Data{ srgb8Data };

const std::vector<std::byte> srgb8a8Data =
{
    std::byte{0x88}, std::byte{0x00}, std::byte{0x00}, std::byte{0x88},
    std::byte{0x00}, std::byte{0x88}, std::byte{0x00}, std::byte{0x88},
    std::byte{0x00}, std::byte{0x00}, std::byte{0x88}, std::byte{0x88},
    std::byte{0x88}, std::byte{0x88}, std::byte{0x88}, std::byte{0xff}
};
const std::vector<ramses::MipLevelData> mipLevelData_srgb8a8Data{ srgb8a8Data };

const std::vector<ramses::MipLevelData> mipLevelData_null;

namespace ramses::internal
{
    Texture2DFormatScene::Texture2DFormatScene(ramses::Scene& scene, uint32_t state, const glm::vec3& cameraPosition, uint32_t vpWidth, uint32_t vpHeight)
        : IntegrationScene(scene, cameraPosition, vpWidth, vpHeight)
    {
        createOrthoCamera();

        ramses::ETextureFormat format(ramses::ETextureFormat::R8);
        uint32_t width = 0u;
        uint32_t height = 0u;

        ramses::TextureSwizzle swizzle = {};
        const std::vector<MipLevelData>& mipLevelData = GetTextureFormatAndData(static_cast<EState>(state), format, width, height, swizzle);

        ramses::Texture2D* texture = m_scene.createTexture2D(
            format,
            width,
            height,
            mipLevelData,
            false,
            swizzle);

        ramses::TextureSampler* sampler = m_scene.createTextureSampler(
            ramses::ETextureAddressMode::Repeat,
            ramses::ETextureAddressMode::Repeat,
            ramses::ETextureSamplingMethod::Nearest,
            ramses::ETextureSamplingMethod::Nearest,
            *texture);

        createQuad(*sampler);
    }

    void Texture2DFormatScene::createOrthoCamera()
    {
        ramses::OrthographicCamera* orthoCamera(m_scene.createOrthographicCamera());
        orthoCamera->setFrustum(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 10.f);
        orthoCamera->setViewport(0, 0, IntegrationScene::DefaultViewportWidth, IntegrationScene::DefaultViewportHeight);
        setCameraToDefaultRenderPass(orthoCamera);
    }

    void Texture2DFormatScene::createQuad(const ramses::TextureSampler& sampler)
    {
        const uint16_t indicesArray[] = { 0, 1, 2, 2, 1, 3 };
        const ramses::ArrayResource* indices = m_scene.createArrayResource(6u, indicesArray);

        const std::array<ramses::vec3f, 4u> vertexPositionsArray{
            ramses::vec3f{ -1.0f, -1.0f, -1.0f },
            ramses::vec3f{ 1.0f, -1.0f, -1.0f },
            ramses::vec3f{ -1.0f, 1.0f, -1.0f },
            ramses::vec3f{ 1.0f, 1.0f, -1.0f }
        };
        const ramses::ArrayResource* vertexPositions = m_scene.createArrayResource(4u, vertexPositionsArray.data());

        const std::array<ramses::vec2f, 4u> textureCoordsArray{
            ramses::vec2f{ 0.0f, 1.0f },
            ramses::vec2f{ 1.0f, 1.0f },
            ramses::vec2f{ 0.0f, 0.0f },
            ramses::vec2f{ 1.0f, 0.0f }
        };
        const ramses::ArrayResource* textureCoords = m_scene.createArrayResource(4u, textureCoordsArray.data());

        ramses::Effect* effect = getTestEffect("ramses-test-client-textured");
        assert(effect != nullptr);

        ramses::Appearance* appearance = m_scene.createAppearance(*effect);
        appearance->setInputTexture(*effect->findUniformInput("u_texture"), sampler);
        appearance->setBlendingFactors(ramses::EBlendFactor::SrcAlpha, ramses::EBlendFactor::OneMinusSrcAlpha, ramses::EBlendFactor::One, ramses::EBlendFactor::OneMinusSrcAlpha);
        appearance->setBlendingOperations(ramses::EBlendOperation::Add, ramses::EBlendOperation::Add);

        ramses::Geometry* geometry = m_scene.createGeometry(*effect);
        geometry->setIndices(*indices);
        geometry->setInputBuffer(*effect->findAttributeInput("a_position"), *vertexPositions);
        geometry->setInputBuffer(*effect->findAttributeInput("a_texcoord"), *textureCoords);

        ramses::MeshNode* mesh = m_scene.createMeshNode();
        addMeshNodeToDefaultRenderGroup(*mesh);
        mesh->setAppearance(*appearance);
        mesh->setGeometry(*geometry);
    }

    const std::vector<ramses::MipLevelData>& Texture2DFormatScene::GetTextureFormatAndData(EState state, ramses::ETextureFormat& format, uint32_t& width, uint32_t& height, ramses::TextureSwizzle& swizzle)
    {
        width = 2u;
        height = 2u;

        switch (state)
        {
        case EState_R8:
            format = ramses::ETextureFormat::R8;
            return mipLevelData_r8;
        case EState_RG8:
            format = ramses::ETextureFormat::RG8;
            return mipLevelData_rg8;
        case EState_Swizzled_Luminance_Alpha:
            swizzle = { ramses::ETextureChannelColor::Red, ramses::ETextureChannelColor::Red, ramses::ETextureChannelColor::Red, ramses::ETextureChannelColor::Green };
            format = ramses::ETextureFormat::RG8;
            return mipLevelData_rg8;
        case EState_RGB8:
            format = ramses::ETextureFormat::RGB8;
            return mipLevelData_rgb8;
        case EState_RGB565:
            format = ramses::ETextureFormat::RGB565;
            return mipLevelData_rgba565;
        case EState_RGBA8:
            format = ramses::ETextureFormat::RGBA8;
            return mipLevelData_rgba8;
        case EState_RGBA4:
            format = ramses::ETextureFormat::RGBA4;
            return mipLevelData_rgba4;
        case EState_RGBA5551:
            format = ramses::ETextureFormat::RGBA5551;
            return mipLevelData_rgba5551;
        case EState_Swizzled_BGR8:
            swizzle = { ramses::ETextureChannelColor::Blue, ramses::ETextureChannelColor::Green, ramses::ETextureChannelColor::Red, ramses::ETextureChannelColor::Alpha };
            format = ramses::ETextureFormat::RGB8;
            return mipLevelData_bgr8;
        case EState_Swizzled_BGRA8:
            swizzle = { ramses::ETextureChannelColor::Blue, ramses::ETextureChannelColor::Green, ramses::ETextureChannelColor::Red, ramses::ETextureChannelColor::Alpha };
            format = ramses::ETextureFormat::RGBA8;
            return mipLevelData_bgra8;
        case EState_ETC2RGB:
            format = ramses::ETextureFormat::ETC2RGB;
            width = 4u;
            height = 4u;
            return mipLevelData_etc2rgb;
        case EState_ETC2RGBA:
            format = ramses::ETextureFormat::ETC2RGBA;
            width = 4u;
            height = 4u;
            return mipLevelData_etc2rgba;

        case EState_R16F:
            format = ramses::ETextureFormat::R16F;
            return mipLevelData_r16fData;
        case EState_R32F:
            format = ramses::ETextureFormat::R32F;
            return mipLevelData_r32fData;
        case EState_RG16F:
            format = ramses::ETextureFormat::RG16F;
            return mipLevelData_rg16fData;
        case EState_RG32F:
            format = ramses::ETextureFormat::RG32F;
            return mipLevelData_rg32fData;
        case EState_RGB16F:
            format = ramses::ETextureFormat::RGB16F;
            return mipLevelData_rgb16fData;
        case EState_RGB32F:
            format = ramses::ETextureFormat::RGB32F;
            return mipLevelData_rgb32fData;
        case EState_RGBA16F:
            format = ramses::ETextureFormat::RGBA16F;
            return mipLevelData_rgba16fData;
        case EState_RGBA32F:
            format = ramses::ETextureFormat::RGBA32F;
            return mipLevelData_rgba32fData;

        case EState_SRGB8:
            format = ramses::ETextureFormat::SRGB8;
            return mipLevelData_srgb8Data;
        case EState_SRGB8_ALPHA8:
            format = ramses::ETextureFormat::SRGB8_ALPHA8;
            return mipLevelData_srgb8a8Data;

        case EState_ASTC_RGBA_4x4:
            format = ramses::ETextureFormat::ASTC_RGBA_4x4;
            width = 4u;
            height = 4u;
            return mipLevelData_astcRGBA4x4Data;
        case EState_ASTC_SRGB_ALPHA_4x4:
            format = ramses::ETextureFormat::ASTC_SRGBA_4x4;
            width = 4u;
            height = 4u;
            return mipLevelData_astcSRGB_Alpha_4x4Data;

        default:
            assert(false && "Unknown texture data format.");
            return mipLevelData_null;
        };
    }
}
