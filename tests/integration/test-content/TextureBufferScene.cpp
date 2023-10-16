//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/TextureBufferScene.h"

#include "ramses/client/Scene.h"
#include "ramses/client/Appearance.h"
#include "ramses/client/Effect.h"
#include "ramses/client/Geometry.h"
#include "ramses/client/Texture2DBuffer.h"
#include "ramses/client/UniformInput.h"
#include "ramses/client/AttributeInput.h"
#include "ramses/client/MeshNode.h"
#include "ramses/client/OrthographicCamera.h"
#include "ramses/client/PerspectiveCamera.h"
#include <cassert>

namespace ramses::internal
{
    TextureBufferScene::TextureBufferScene(ramses::Scene& scene, uint32_t state, const glm::vec3& cameraPosition, uint32_t vpWidth, uint32_t vpHeight)
        : IntegrationScene(scene, cameraPosition, vpWidth, vpHeight)
        , m_quadMesh(*scene.createMeshNode())
        , m_effectSingleMip(*getTestEffect("ramses-test-client-texture-buffer"))
        , m_effectAllMips(*getTestEffect("ramses-test-client-texture-buffer-allmips"))
        , m_appearanceSingleMip(*scene.createAppearance(m_effectSingleMip))
        , m_appearanceAllMips(*scene.createAppearance(m_effectAllMips))
        , m_geometrySingleMip(*m_scene.createGeometry(m_effectSingleMip))
        , m_geometryAllMips(*m_scene.createGeometry(m_effectAllMips))
    {
        setOrthoCamera(cameraPosition);

        addMeshNodeToDefaultRenderGroup(m_quadMesh);

        m_appearanceSingleMip.setDrawMode(ramses::EDrawMode::TriangleStrip);
        m_appearanceAllMips.setDrawMode(ramses::EDrawMode::TriangleStrip);
        m_quadMesh.setIndexCount(4);

        const uint16_t indicesData[] = { 0, 1, 3, 2 };
        const std::array<ramses::vec3f, 4u> vertexPositionsArray{
            ramses::vec3f{ 0.0f, 0.0f, -1.0f },
            ramses::vec3f{ 1.0f, 0.0f, -1.0f },
            ramses::vec3f{ 1.0f, 1.0f, -1.0f },
            ramses::vec3f{ 0.0f, 1.0f, -1.0f }
        };
        const std::array<ramses::vec2f, 4u> textureCoordsArray{
            ramses::vec2f{ 0.0f, 0.0f },
            ramses::vec2f{ 1.0f, 0.0f },
            ramses::vec2f{ 1.0f, 1.0f },
            ramses::vec2f{ 0.0f, 1.0f }
        };

        const auto indices = m_scene.createArrayResource(4u, indicesData);
        const auto vertices = m_scene.createArrayResource(4u, vertexPositionsArray.data());
        const auto texcoords = m_scene.createArrayResource(4u, textureCoordsArray.data());

        m_geometrySingleMip.setIndices(*indices);
        m_geometryAllMips.setIndices(*indices);

        {
            m_geometrySingleMip.setInputBuffer(*m_effectSingleMip.findAttributeInput("a_position"), *vertices);
            m_geometrySingleMip.setInputBuffer(*m_effectSingleMip.findAttributeInput("a_texcoord"), *texcoords);
        }

        {
            m_geometryAllMips.setInputBuffer(*m_effectAllMips.findAttributeInput("a_position"), *vertices);
            m_geometryAllMips.setInputBuffer(*m_effectAllMips.findAttributeInput("a_texcoord"), *texcoords);
        }

        setState(state);
    }

    void TextureBufferScene::setOrthoCamera(const glm::vec3& cameraPosition)
    {
        ramses::Node* cameraTranslate = m_scene.createNode();
        cameraTranslate->setTranslation({cameraPosition.x, cameraPosition.y, cameraPosition.z});
        ramses::OrthographicCamera* orthoCamera(m_scene.createOrthographicCamera());
        orthoCamera->setFrustum(0.0f, 1.0f, 0.0f, 1.0f, 0.1f, 100.f);
        orthoCamera->setViewport(0, 0, getDefaultCamera().getViewportWidth(), getDefaultCamera().getViewportHeight());
        orthoCamera->setParent(*cameraTranslate);
        setCameraToDefaultRenderPass(orthoCamera);
    }

    void TextureBufferScene::setState(uint32_t state)
    {
        // Gradients of monochrome texels in different colors
        const uint8_t rgba_1x1_red[] = { 255, 0, 0, 255 };
        const uint8_t rgba_2x2_yellow[] =
        {
            64 , 64, 0, 255       , 128, 128, 0, 255,
            192, 192, 0, 255       , 255, 255, 0, 255
        };
        const uint8_t rgba_2x2_green[] =
        {
            0, 64 , 0, 255       , 0, 128, 0, 255,
            0, 192, 0, 255       , 0, 255, 0, 255
        };
        const uint8_t rgba_4x4_blue[] = {
            0, 0, 16 , 255      , 0, 0, 32 , 255        , 0, 0, 48 , 255        , 0, 0, 64 , 255,
            0, 0, 80 , 255      , 0, 0, 96 , 255        , 0, 0, 112, 255        , 0, 0, 128, 255,
            0, 0, 144, 255      , 0, 0, 160, 255        , 0, 0, 176, 255        , 0, 0, 192, 255,
            0, 0, 208, 255      , 0, 0, 224, 255        , 0, 0, 240, 255        , 0, 0, 255, 255 };
        const uint8_t rgba_4x4_red[] = {
            16 , 0, 0, 255      , 32 , 0, 0, 255        , 48 , 0, 0, 255        , 64 , 0, 0, 255,
            80 , 0, 0, 255      , 96 , 0, 0, 255        , 112, 0, 0, 255        , 128, 0, 0, 255,
            144, 0, 0, 255      , 160, 0, 0, 255        , 176, 0, 0, 255        , 192, 0, 0, 255,
            208, 0, 0, 255      , 224, 0, 0, 255        , 240, 0, 0, 255        , 255, 0, 0, 255 };

        // same, but in RG8 format
        const uint8_t rg_1x1_black[] = { 0, 0 };
        const uint8_t rg_2x2_green[] = {
            0, 64,      0, 128,
            0, 192,     0, 255 };
        const uint8_t rg_4x4_red[] = {
            16 , 0      , 32 , 0        ,48 , 0        , 64 , 0,
            80 , 0      , 96 , 0        ,112, 0        , 128, 0,
            144, 0      , 160, 0        ,176, 0        , 192, 0,
            208, 0      , 224, 0        ,240, 0        , 255, 0 };

        const ramses::TextureSampler* sampler = nullptr;
        int32_t mipToFetch = -1;

        switch (state)
        {
        case EState_RGBA8_OneMip:
        case EState_RGBA8_OneMip_ScaledDown:
        {
            assert(m_textureBuffer == nullptr);
            m_textureBuffer = m_scene.createTexture2DBuffer(ramses::ETextureFormat::RGBA8, 4u, 4u, 1);
            m_textureBuffer->updateData(0, 0, 0, 4, 4, rgba_4x4_blue);
            sampler = m_scene.createTextureSampler(ramses::ETextureAddressMode::Clamp, ramses::ETextureAddressMode::Clamp, ramses::ETextureSamplingMethod::Nearest_MipMapNearest, ramses::ETextureSamplingMethod::Nearest, *m_textureBuffer);
            mipToFetch = 0;
            break;
        }
        case EState_RGBA8_ThreeMips:
        {
            assert(m_textureBuffer == nullptr);
            m_textureBuffer = m_scene.createTexture2DBuffer(ramses::ETextureFormat::RGBA8, 4u, 4u, 3);
            m_textureBuffer->updateData(0, 0, 0, 4, 4, rgba_4x4_blue);
            m_textureBuffer->updateData(1, 0, 0, 2, 2, rgba_2x2_green);
            m_textureBuffer->updateData(2, 0, 0, 1, 1, rgba_1x1_red);
            sampler = m_scene.createTextureSampler(ramses::ETextureAddressMode::Clamp, ramses::ETextureAddressMode::Clamp, ramses::ETextureSamplingMethod::Nearest_MipMapNearest, ramses::ETextureSamplingMethod::Nearest, *m_textureBuffer);
            break;
        }
        case EState_PartialUpdate:
        {
            assert(m_textureBuffer == nullptr);
            m_textureBuffer = m_scene.createTexture2DBuffer(ramses::ETextureFormat::RGBA8, 4u, 4u, 1);
            m_textureBuffer->updateData(0, 0, 0, 4, 4, rgba_4x4_blue);
            sampler = m_scene.createTextureSampler(ramses::ETextureAddressMode::Clamp, ramses::ETextureAddressMode::Clamp, ramses::ETextureSamplingMethod::Nearest_MipMapNearest, ramses::ETextureSamplingMethod::Nearest, *m_textureBuffer);

            // update
            m_textureBuffer->updateData(0, 1, 1, 2, 2, rgba_2x2_green);

            mipToFetch = 0;
            break;
        }
        case EState_PartialUpdate1:
            assert(m_textureBuffer);
            m_textureBuffer->updateData(0, 1, 1, 2, 2, rgba_2x2_yellow);
            sampler = m_scene.createTextureSampler(ramses::ETextureAddressMode::Clamp, ramses::ETextureAddressMode::Clamp, ramses::ETextureSamplingMethod::Nearest_MipMapNearest, ramses::ETextureSamplingMethod::Nearest, *m_textureBuffer);
            mipToFetch = 0;
            break;
        case EState_PartialUpdate2:
            assert(m_textureBuffer);
            m_textureBuffer->updateData(0, 1, 1, 1, 1, rgba_1x1_red);
            m_textureBuffer->updateData(0, 1, 2, 1, 1, rgba_1x1_red);
            sampler = m_scene.createTextureSampler(ramses::ETextureAddressMode::Clamp, ramses::ETextureAddressMode::Clamp, ramses::ETextureSamplingMethod::Nearest_MipMapNearest, ramses::ETextureSamplingMethod::Nearest, *m_textureBuffer);
            mipToFetch = 0;
            break;
        case EState_PartialUpdateMipMap:
        {
            assert(m_textureBuffer == nullptr);
            m_textureBuffer = m_scene.createTexture2DBuffer(ramses::ETextureFormat::RGBA8, 4u, 4u, 3);
            m_textureBuffer->updateData(0, 0, 0, 4, 4, rgba_4x4_blue);
            m_textureBuffer->updateData(1, 0, 0, 2, 2, rgba_2x2_green);
            m_textureBuffer->updateData(2, 0, 0, 1, 1, rgba_1x1_red);
            sampler = m_scene.createTextureSampler(ramses::ETextureAddressMode::Clamp, ramses::ETextureAddressMode::Clamp, ramses::ETextureSamplingMethod::Nearest_MipMapNearest, ramses::ETextureSamplingMethod::Nearest, *m_textureBuffer);

            // update
            m_textureBuffer->updateData(1, 1, 1, 1, 1, rgba_1x1_red);

            mipToFetch = 1;
            break;
        }
        case EState_PartialUpdateMipMap_RG8:
        {
            assert(m_textureBuffer == nullptr);
            m_textureBuffer = m_scene.createTexture2DBuffer(ramses::ETextureFormat::RG8, 4u, 4u, 3);
            m_textureBuffer->updateData(0, 0, 0, 4, 4, rg_4x4_red);
            m_textureBuffer->updateData(1, 0, 0, 2, 2, rg_2x2_green);
            m_textureBuffer->updateData(2, 0, 0, 1, 1, rg_1x1_black);
            sampler = m_scene.createTextureSampler(ramses::ETextureAddressMode::Clamp, ramses::ETextureAddressMode::Clamp, ramses::ETextureSamplingMethod::Nearest_MipMapNearest, ramses::ETextureSamplingMethod::Nearest, *m_textureBuffer);

            // update
            m_textureBuffer->updateData(1, 1, 1, 1, 1, rg_1x1_black);

            mipToFetch = 1;
            break;
        }
        case EState_ClientTextureResource_RGBA8:
        {
            const ramses::MipLevelData mip2x2(sizeof(rgba_2x2_green), rgba_2x2_green);
            const ramses::MipLevelData mips[] = { mip2x2 };
            m_clientTexture = m_scene.createTexture2D(ramses::ETextureFormat::RGBA8, 2u, 2u, 1u, mips);
            sampler = m_scene.createTextureSampler(ramses::ETextureAddressMode::Clamp, ramses::ETextureAddressMode::Clamp, ramses::ETextureSamplingMethod::Nearest_MipMapNearest, ramses::ETextureSamplingMethod::Nearest, *m_clientTexture);
            mipToFetch = 0;
            break;
        }
        case EState_SwitchBackToClientTexture:
        {
            // don't create new resource, use existing one (to discover weird resource reuse bugs)
            assert(nullptr != m_clientTexture);
            sampler = m_scene.createTextureSampler(ramses::ETextureAddressMode::Clamp, ramses::ETextureAddressMode::Clamp, ramses::ETextureSamplingMethod::Nearest_MipMapNearest, ramses::ETextureSamplingMethod::Nearest, *m_clientTexture);
            mipToFetch = 0;
            break;
        }
        case EState_SwitchBackToExistingTextureBufferAndUpdate:
        {
            assert(nullptr != m_textureBuffer);
            sampler = m_scene.createTextureSampler(ramses::ETextureAddressMode::Clamp, ramses::ETextureAddressMode::Clamp, ramses::ETextureSamplingMethod::Nearest_MipMapNearest, ramses::ETextureSamplingMethod::Nearest, *m_textureBuffer);
            m_textureBuffer->updateData(0, 0, 0, 4, 4, rgba_4x4_red);
            mipToFetch = 0;
            break;
        }
        default:
            assert(false);
        }

        if (EState_RGBA8_OneMip_ScaledDown == state)
        {
            ramses::Node* scale = m_scene.createNode();
            scale->setScaling({0.2f, 0.2f, 0.2f});
            m_quadMesh.setParent(*scale);
        }

        m_quadMesh.removeAppearanceAndGeometry();
        if (mipToFetch < 0)
        {
            m_quadMesh.setAppearance(m_appearanceAllMips);
            m_quadMesh.setGeometry(m_geometryAllMips);
            m_appearanceAllMips.setInputTexture(*m_effectAllMips.findUniformInput("u_texture"), *sampler);
        }
        else
        {
            m_quadMesh.setAppearance(m_appearanceSingleMip);
            m_quadMesh.setGeometry(m_geometrySingleMip);

            m_appearanceSingleMip.setInputTexture(*m_effectSingleMip.findUniformInput("u_texture"), *sampler);
            m_appearanceSingleMip.setInputValue(*m_effectSingleMip.findUniformInput("u_mip"), mipToFetch);
        }

        m_scene.flush();
    }
}
