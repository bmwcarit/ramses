//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/TextureBufferScene.h"

#include "ramses-client-api/RamsesClient.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/Effect.h"
#include "ramses-client-api/GeometryBinding.h"
#include "ramses-client-api/Texture2DBuffer.h"
#include "ramses-client-api/UniformInput.h"
#include "ramses-client-api/AttributeInput.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/OrthographicCamera.h"

namespace ramses_internal
{
    TextureBufferScene::TextureBufferScene(ramses::RamsesClient& ramsesClient, ramses::Scene& scene, UInt32 state, const Vector3& cameraPosition)
        : IntegrationScene(ramsesClient, scene, cameraPosition)
        , m_quadMesh(*scene.createMeshNode())
        , m_effectSingleMip(*getTestEffect("ramses-test-client-texture-buffer"))
        , m_effectAllMips(*getTestEffect("ramses-test-client-texture-buffer-allmips"))
        , m_appearanceSingleMip(*scene.createAppearance(m_effectSingleMip))
        , m_appearanceAllMips(*scene.createAppearance(m_effectAllMips))
        , m_geometrySingleMip(*m_scene.createGeometryBinding(m_effectSingleMip))
        , m_geometryAllMips(*m_scene.createGeometryBinding(m_effectAllMips))
    {
        setOrthoCamera(cameraPosition);

        addMeshNodeToDefaultRenderGroup(m_quadMesh);

        m_appearanceSingleMip.setDrawMode(ramses::EDrawMode_TriangleStrip);
        m_appearanceAllMips.setDrawMode(ramses::EDrawMode_TriangleStrip);
        m_quadMesh.setIndexCount(4);

        const uint16_t indicesData[] = { 0, 1, 3, 2 };

        const float verticesData[] = {
            0.0f, 0.0f, -1.0f,
            1.0f, 0.0f, -1.0f,
            1.0f, 1.0f, -1.0f,
            0.0f, 1.0f, -1.0f
        };

        const float textureCoordData[] = {
            0.0f, 0.0f,
            1.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f
        };

        const auto indices = ramsesClient.createConstUInt16Array(4u, indicesData);
        const auto vertices = ramsesClient.createConstVector3fArray(4u, verticesData);
        const auto texcoords = ramsesClient.createConstVector2fArray(4u, textureCoordData);

        m_geometrySingleMip.setIndices(*indices);
        m_geometryAllMips.setIndices(*indices);

        {
            ramses::AttributeInput verticesInput;
            ramses::AttributeInput texcoordInput;
            m_effectSingleMip.findAttributeInput("a_position", verticesInput);
            m_effectSingleMip.findAttributeInput("a_texcoord", texcoordInput);

            m_geometrySingleMip.setInputBuffer(verticesInput, *vertices);
            m_geometrySingleMip.setInputBuffer(texcoordInput, *texcoords);
        }

        {
            ramses::AttributeInput verticesInput;
            ramses::AttributeInput texcoordInput;
            m_effectAllMips.findAttributeInput("a_position", verticesInput);
            m_effectAllMips.findAttributeInput("a_texcoord", texcoordInput);

            m_geometryAllMips.setInputBuffer(verticesInput, *vertices);
            m_geometryAllMips.setInputBuffer(texcoordInput, *texcoords);
        }

        setState(state);
    }

    void TextureBufferScene::setOrthoCamera(const Vector3& cameraPosition)
    {
        ramses::Node* cameraTranslate = m_scene.createNode();
        cameraTranslate->setTranslation(cameraPosition.x, cameraPosition.y, cameraPosition.z);
        ramses::OrthographicCamera* orthoCamera(m_scene.createOrthographicCamera());
        orthoCamera->setFrustum(0.0f, 1.0f, 0.0f, 1.0f, 0.1f, 100.f);
        orthoCamera->setViewport(0, 0, DefaultDisplayWidth, DefaultDisplayHeight);
        orthoCamera->setParent(*cameraTranslate);
        setCameraToDefaultRenderPass(orthoCamera);
    }

    void TextureBufferScene::setState(UInt32 state)
    {
        // Gradients of monochrome texels in different colors
        const UInt8 rgba_1x1_red[] = { 255, 0, 0, 255 };
        const UInt8 rgba_2x2_green[] =
        {
            0, 64 , 0, 255       , 0, 128, 0, 255,
            0, 192, 0, 255       , 0, 255, 0, 255
        };
        const UInt8 rgba_4x4_blue[] = {
            0, 0, 16 , 255      , 0, 0, 32 , 255        , 0, 0, 48 , 255        , 0, 0, 64 , 255,
            0, 0, 80 , 255      , 0, 0, 96 , 255        , 0, 0, 112, 255        , 0, 0, 128, 255,
            0, 0, 144, 255      , 0, 0, 160, 255        , 0, 0, 176, 255        , 0, 0, 192, 255,
            0, 0, 208, 255      , 0, 0, 224, 255        , 0, 0, 240, 255        , 0, 0, 255, 255 };
        const UInt8 rgba_4x4_red[] = {
            16 , 0, 0, 255      , 32 , 0, 0, 255        , 48 , 0, 0, 255        , 64 , 0, 0, 255,
            80 , 0, 0, 255      , 96 , 0, 0, 255        , 112, 0, 0, 255        , 128, 0, 0, 255,
            144, 0, 0, 255      , 160, 0, 0, 255        , 176, 0, 0, 255        , 192, 0, 0, 255,
            208, 0, 0, 255      , 224, 0, 0, 255        , 240, 0, 0, 255        , 255, 0, 0, 255 };

        // same, but in RG8 format
        const UInt8 rg_1x1_black[] = { 0, 0 };
        const UInt8 rg_2x2_green[] = {
            0, 64,      0, 128,
            0, 192,     0, 255 };
        const UInt8 rg_4x4_red[] = {
            16 , 0      , 32 , 0        ,48 , 0        , 64 , 0,
            80 , 0      , 96 , 0        ,112, 0        , 128, 0,
            144, 0      , 160, 0        ,176, 0        , 192, 0,
            208, 0      , 224, 0        ,240, 0        , 255, 0 };

        const ramses::TextureSampler* sampler = nullptr;
        Int32 mipToFetch = -1;

        switch (state)
        {
        case EState_RGBA8_OneMip:
        case EState_RGBA8_OneMip_ScaledDown:
        {
            assert(m_textureBuffer == nullptr);
            m_textureBuffer = m_scene.createTexture2DBuffer(1, 4u, 4u, ramses::ETextureFormat_RGBA8);
            m_textureBuffer->setData(reinterpret_cast<const char*>(rgba_4x4_blue), 0, 0, 0, 4, 4);
            sampler = m_scene.createTextureSampler(ramses::ETextureAddressMode_Clamp, ramses::ETextureAddressMode_Clamp, ramses::ETextureSamplingMethod_Nearest_MipMapNearest, ramses::ETextureSamplingMethod_Nearest, *m_textureBuffer);
            mipToFetch = 0;
            break;
        }
        case EState_RGBA8_ThreeMips:
        {
            assert(m_textureBuffer == nullptr);
            m_textureBuffer = m_scene.createTexture2DBuffer(3, 4u, 4u, ramses::ETextureFormat_RGBA8);
            m_textureBuffer->setData(reinterpret_cast<const char*>(rgba_4x4_blue) , 0, 0, 0, 4, 4);
            m_textureBuffer->setData(reinterpret_cast<const char*>(rgba_2x2_green), 1, 0, 0, 2, 2);
            m_textureBuffer->setData(reinterpret_cast<const char*>(rgba_1x1_red)  , 2, 0, 0, 1, 1);
            sampler = m_scene.createTextureSampler(ramses::ETextureAddressMode_Clamp, ramses::ETextureAddressMode_Clamp, ramses::ETextureSamplingMethod_Nearest_MipMapNearest, ramses::ETextureSamplingMethod_Nearest, *m_textureBuffer);
            break;
        }
        case EState_PartialUpdate:
        {
            assert(m_textureBuffer == nullptr);
            m_textureBuffer = m_scene.createTexture2DBuffer(1, 4u, 4u, ramses::ETextureFormat_RGBA8);
            m_textureBuffer->setData(reinterpret_cast<const char*>(rgba_4x4_blue), 0, 0, 0, 4, 4);
            sampler = m_scene.createTextureSampler(ramses::ETextureAddressMode_Clamp, ramses::ETextureAddressMode_Clamp, ramses::ETextureSamplingMethod_Nearest_MipMapNearest, ramses::ETextureSamplingMethod_Nearest, *m_textureBuffer);

            // update
            m_textureBuffer->setData(reinterpret_cast<const char*>(rgba_2x2_green), 0, 1, 1, 2, 2);

            mipToFetch = 0;
            break;
        }
        case EState_PartialUpdateMipMap:
        {
            assert(m_textureBuffer == nullptr);
            m_textureBuffer = m_scene.createTexture2DBuffer(3, 4u, 4u, ramses::ETextureFormat_RGBA8);
            m_textureBuffer->setData(reinterpret_cast<const char*>(rgba_4x4_blue), 0, 0, 0, 4, 4);
            m_textureBuffer->setData(reinterpret_cast<const char*>(rgba_2x2_green), 1, 0, 0, 2, 2);
            m_textureBuffer->setData(reinterpret_cast<const char*>(rgba_1x1_red), 2, 0, 0, 1, 1);
            sampler = m_scene.createTextureSampler(ramses::ETextureAddressMode_Clamp, ramses::ETextureAddressMode_Clamp, ramses::ETextureSamplingMethod_Nearest_MipMapNearest, ramses::ETextureSamplingMethod_Nearest, *m_textureBuffer);

            // update
            m_textureBuffer->setData(reinterpret_cast<const char*>(rgba_1x1_red), 1, 1, 1, 1, 1);

            mipToFetch = 1;
            break;
        }
        case EState_PartialUpdateMipMap_RG8:
        {
            assert(m_textureBuffer == nullptr);
            m_textureBuffer = m_scene.createTexture2DBuffer(3, 4u, 4u, ramses::ETextureFormat_RG8);
            m_textureBuffer->setData(reinterpret_cast<const char*>(rg_4x4_red)  , 0, 0, 0, 4, 4);
            m_textureBuffer->setData(reinterpret_cast<const char*>(rg_2x2_green), 1, 0, 0, 2, 2);
            m_textureBuffer->setData(reinterpret_cast<const char*>(rg_1x1_black), 2, 0, 0, 1, 1);
            sampler = m_scene.createTextureSampler(ramses::ETextureAddressMode_Clamp, ramses::ETextureAddressMode_Clamp, ramses::ETextureSamplingMethod_Nearest_MipMapNearest, ramses::ETextureSamplingMethod_Nearest, *m_textureBuffer);

            // update
            m_textureBuffer->setData(reinterpret_cast<const char*>(rg_1x1_black), 1, 1, 1, 1, 1);

            mipToFetch = 1;
            break;
        }
        case EState_ClientTextureResource_RGBA8:
        {
            const ramses::MipLevelData mip2x2(sizeof(rgba_2x2_green), rgba_2x2_green);
            const ramses::MipLevelData mips[] = { mip2x2 };
            m_clientTexture = m_client.createTexture2D(2u, 2u, ramses::ETextureFormat_RGBA8, 1u, mips);
            sampler = m_scene.createTextureSampler(ramses::ETextureAddressMode_Clamp, ramses::ETextureAddressMode_Clamp, ramses::ETextureSamplingMethod_Nearest_MipMapNearest, ramses::ETextureSamplingMethod_Nearest, *m_clientTexture);
            mipToFetch = 0;
            break;
        }
        case EState_SwitchBackToClientTexture:
        {
            // don't create new resource, use existing one (to discover weird resource reuse bugs)
            assert(nullptr != m_clientTexture);
            sampler = m_scene.createTextureSampler(ramses::ETextureAddressMode_Clamp, ramses::ETextureAddressMode_Clamp, ramses::ETextureSamplingMethod_Nearest_MipMapNearest, ramses::ETextureSamplingMethod_Nearest, *m_clientTexture);
            mipToFetch = 0;
            break;
        }
        case EState_SwitchBackToExistingTextureBufferAndUpdate:
        {
            assert(nullptr != m_textureBuffer);
            sampler = m_scene.createTextureSampler(ramses::ETextureAddressMode_Clamp, ramses::ETextureAddressMode_Clamp, ramses::ETextureSamplingMethod_Nearest_MipMapNearest, ramses::ETextureSamplingMethod_Nearest, *m_textureBuffer);
            m_textureBuffer->setData(reinterpret_cast<const char*>(rgba_4x4_red), 0, 0, 0, 4, 4);
            mipToFetch = 0;
            break;
        }
        default:
            assert(false);
        }

        if (EState_RGBA8_OneMip_ScaledDown == state)
        {
            ramses::Node* scale = m_scene.createNode();
            scale->setScaling(0.2f, 0.2f, 0.2f);
            m_quadMesh.setParent(*scale);
        }

        m_quadMesh.removeAppearanceAndGeometry();
        if (mipToFetch < 0)
        {
            m_quadMesh.setAppearance(m_appearanceAllMips);
            m_quadMesh.setGeometryBinding(m_geometryAllMips);

            ramses::UniformInput samplerInput;
            m_effectAllMips.findUniformInput("u_texture", samplerInput);
            m_appearanceAllMips.setInputTexture(samplerInput, *sampler);
        }
        else
        {
            m_quadMesh.setAppearance(m_appearanceSingleMip);
            m_quadMesh.setGeometryBinding(m_geometrySingleMip);

            ramses::UniformInput samplerInput;
            m_effectSingleMip.findUniformInput("u_texture", samplerInput);
            m_appearanceSingleMip.setInputTexture(samplerInput, *sampler);

            ramses::UniformInput mipInput;
            m_effectSingleMip.findUniformInput("u_mip", mipInput);
            m_appearanceSingleMip.setInputValueInt32(mipInput, mipToFetch);
        }

        m_scene.flush();
    }
}
