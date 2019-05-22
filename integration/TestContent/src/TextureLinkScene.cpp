//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/TextureLinkScene.h"
#include "ramses-client-api/RamsesClient.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/AttributeInput.h"
#include "ramses-client-api/GeometryBinding.h"
#include "ramses-client-api/TextureSampler.h"
#include "ramses-client-api/Effect.h"
#include "ramses-client-api/Appearance.h"

#include "Scene/ClientScene.h"
#include "TestScenes/Triangle.h"
#include <array>

namespace ramses_internal
{
    TextureLinkScene::TextureLinkScene(ramses::RamsesClient& ramsesClient, ramses::Scene& scene, UInt32 state, const Vector3& cameraPosition)
        : IntegrationScene(ramsesClient, scene, cameraPosition)
    {
        ramses::Effect* effect = getTestEffect("ramses-test-client-textured");
        ramses::Triangle triangle1(ramsesClient, scene, *effect, ramses::TriangleAppearance::EColor_Red);
        ramses::Triangle triangle2(ramsesClient, scene, *effect, ramses::TriangleAppearance::EColor_Green);

        ramses::MeshNode* mesh1 = scene.createMeshNode();
        ramses::MeshNode* mesh2 = scene.createMeshNode();

        ramses::Appearance& appearance1 = triangle1.GetAppearance();
        ramses::Appearance& appearance2 = triangle2.GetAppearance();
        appearance1.setName("dataLinkAppearance1");
        appearance2.setName("dataLinkAppearance2");

        mesh1->setAppearance(appearance1);
        mesh2->setAppearance(appearance2);

        mesh1->setGeometryBinding(triangle1.GetGeometry());
        mesh2->setGeometryBinding(triangle2.GetGeometry());

        ramses::Node* translate1 = scene.createNode();
        ramses::Node* translate2 = scene.createNode();

        if (DATA_PROVIDER_LARGE == state || DATA_CONSUMER_AND_PROVIDER_LARGE == state)
        {
            ramses::Node* scale1 = scene.createNode();
            ramses::Node* scale2 = scene.createNode();

            scale1->setScaling(5.0f, 5.0f, 5.0f);
            scale2->setScaling(5.0f, 5.0f, 5.0f);

            scale1->setParent(*translate1);
            scale2->setParent(*translate2);

            mesh1->setParent(*scale1);
            mesh2->setParent(*scale2);
        }
        else
        {
            mesh1->setParent(*translate1);
            mesh2->setParent(*translate2);
        }

        addMeshNodeToDefaultRenderGroup(*mesh1);
        addMeshNodeToDefaultRenderGroup(*mesh2);

        translate1->setTranslation(-1.5f, 0.f, -15.f);
        translate2->setTranslation(1.5f, 0.f, -15.f);

        const float textureCoordsArray[] = { 0.f, 1.f, 1.f, 1.f, 0.f, 0.f };
        const ramses::Vector2fArray* textureCoords = ramsesClient.createConstVector2fArray(3u, textureCoordsArray);

        ramses::AttributeInput texCoordsInput;
        effect->findAttributeInput("a_texcoord", texCoordsInput);
        triangle1.GetGeometry().setInputBuffer(texCoordsInput, *textureCoords);
        triangle2.GetGeometry().setInputBuffer(texCoordsInput, *textureCoords);

        switch (state)
        {
        case DATA_PROVIDER_LARGE:
        case DATA_PROVIDER:
        {
            const std::array<uint8_t, 4> pxData{ {0xff, 0x0, 0x0, 0xff} };
            const ramses::MipLevelData mipLevelData(4, pxData.data());
            const ramses::Texture2D& texture = *m_client.createTexture2D(1u, 1u, ramses::ETextureFormat_RGBA8, 1, &mipLevelData, false, ramses::ResourceCacheFlag_DoNotCache, "ProviderTexture");
            const ramses::TextureSampler& sampler = createSampler(texture);
            setSampler(appearance1, sampler);
            setSampler(appearance2, sampler);

            scene.createTextureProvider(texture, DataProviderId);
        }
            break;
        case DATA_CONSUMER:
        {
            const std::array<uint8_t, 4> pxData{ { 0x0, 0xff, 0x0, 0xff } };
            const ramses::MipLevelData mipLevelData(4, pxData.data());
            const ramses::Texture2D& texture = *m_client.createTexture2D(1u, 1u, ramses::ETextureFormat_RGBA8, 1, &mipLevelData, false, ramses::ResourceCacheFlag_DoNotCache, "ConsumerTexture");
            const ramses::TextureSampler& sampler = createSampler(texture);
            setSampler(appearance1, sampler);
            setSampler(appearance2, sampler);

            scene.createTextureConsumer(sampler, DataConsumerId);
        }
            break;
        case DATA_CONSUMER_AND_PROVIDER_LARGE:
        case DATA_CONSUMER_AND_PROVIDER:
        {
            const std::array<uint8_t, 4> pxData{ { 0x0, 0x0, 0xff, 0xff } };
            const ramses::MipLevelData mipLevelData(4, pxData.data());
            const ramses::Texture2D& texture = *m_client.createTexture2D(1u, 1u, ramses::ETextureFormat_RGBA8, 1, &mipLevelData, false, ramses::ResourceCacheFlag_DoNotCache, "ConsumerProviderTexture");
            const ramses::TextureSampler& sampler1 = createSampler(texture);
            const ramses::TextureSampler& sampler2 = createSampler(texture);
            setSampler(appearance1, sampler1);
            setSampler(appearance2, sampler2);

            scene.createTextureConsumer(sampler1, DataConsumerId);
            scene.createTextureProvider(texture, DataProviderId);
        }
            break;
        default:
            assert(false && "invalid scene state");
        }
    }

    const ramses::TextureSampler& TextureLinkScene::createSampler(const ramses::Texture2D& texture)
    {
        return *m_scene.createTextureSampler(ramses::ETextureAddressMode_Repeat, ramses::ETextureAddressMode_Repeat, ramses::ETextureSamplingMethod_Nearest, ramses::ETextureSamplingMethod_Nearest, texture, 1u, "dataLinkSampler");
    }

    void TextureLinkScene::setSampler(ramses::Appearance& appearance, const ramses::TextureSampler& sampler)
    {
        ramses::UniformInput texInput;
        appearance.getEffect().findUniformInput("u_texture", texInput);
        appearance.setInputTexture(texInput, sampler);
    }
}
