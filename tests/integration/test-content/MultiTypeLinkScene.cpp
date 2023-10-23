//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/MultiTypeLinkScene.h"
#include "ramses/client/Scene.h"
#include "ramses/client/MeshNode.h"
#include "ramses/client/DataObject.h"
#include "ramses/client/AttributeInput.h"
#include "ramses/client/Geometry.h"
#include "ramses/client/Effect.h"
#include "ramses/client/Appearance.h"

#include "internal/SceneGraph/Scene/ClientScene.h"
#include "TestScenes/Triangle.h"
#include <array>

namespace ramses::internal
{
    MultiTypeLinkScene::MultiTypeLinkScene(ramses::Scene& scene, uint32_t state, const glm::vec3& cameraPosition, uint32_t vpWidth, uint32_t vpHeight)
        : IntegrationScene(scene, cameraPosition, vpWidth, vpHeight)
    {
        ramses::Effect* effect = getTestEffect("ramses-test-client-basic");
        ramses::Effect* effectTex = getTestEffect("ramses-test-client-textured");
        Triangle triangle1(scene, *effect, TriangleAppearance::EColor_Blue);
        Triangle triangle2(scene, *effectTex, TriangleAppearance::EColor_Green);

        ramses::DataObject* colorData = scene.createDataObject(ramses::EDataType::Vector4F);

        triangle1.bindColor(*colorData);

        ramses::MeshNode* mesh1 = scene.createMeshNode();
        ramses::MeshNode* mesh2 = scene.createMeshNode();

        ramses::Appearance& appearance1 = triangle1.GetAppearance();
        ramses::Appearance& appearance2 = triangle2.GetAppearance();

        mesh1->setAppearance(appearance1);
        mesh2->setAppearance(appearance2);

        mesh1->setGeometry(triangle1.GetGeometry());
        mesh2->setGeometry(triangle2.GetGeometry());

        ramses::Node* translate1 = scene.createNode();
        ramses::Node* translate2 = scene.createNode();

        mesh1->setParent(*translate1);
        mesh2->setParent(*translate2);

        addMeshNodeToDefaultRenderGroup(*mesh1);
        addMeshNodeToDefaultRenderGroup(*mesh2);

        ramses::Node* groupNode = scene.createNode();
        translate1->setParent(*groupNode);
        translate2->setParent(*groupNode);

        translate1->setTranslation({-1.5f, 0.f, -15.f});
        translate2->setTranslation({1.5f, 0.f, -15.f});

        const std::array<ramses::vec2f, 3u> textureCoordsArray{ ramses::vec2f{0.f, 1.f}, ramses::vec2f{1.f, 1.f}, ramses::vec2f{0.f, 0.f} };
        const ramses::ArrayResource* textureCoords = m_scene.createArrayResource(3u, textureCoordsArray.data());

        triangle2.GetGeometry().setInputBuffer(*effectTex->findAttributeInput("a_texcoord"), *textureCoords);

        switch (state)
        {
        case TRANSFORMATION_CONSUMER_DATA_AND_TEXTURE_PROVIDER:
        {
            scene.createDataProvider(*colorData, DataProviderId);
            colorData->setValue(ramses::vec4f{ 1.f, 1.f, 0.f, 1.f });

            scene.createTransformationDataConsumer(*groupNode, TransformationConsumerId);

            const std::array<uint8_t, 4> pxData{ {0xff, 0x0, 0x0, 0xff} };
            std::vector<MipLevelData> mipLevelData{ MipLevelData(4, pxData.data()) };
            const ramses::Texture2D& texture = *m_scene.createTexture2D(ramses::ETextureFormat::RGBA8, 1u, 1u, mipLevelData, false, {});
            const ramses::TextureSampler& sampler = createSampler(texture);
            SetSampler(appearance2, sampler);

            scene.createTextureProvider(texture, TextureProviderId);
        }
            break;
        case TRANSFORMATION_PROVIDER_DATA_AND_TEXTURE_CONSUMER:
        {
            scene.createDataConsumer(*colorData, DataConsumerId);
            colorData->setValue(ramses::vec4f{ 0.f, 1.f, 0.f, 1.f });

            ramses::Node* providerNode = scene.createNode();
            providerNode->setTranslation({ 1.5f, -2.f, 5.f });
            scene.createTransformationDataProvider(*providerNode, TransformationProviderId);

            const std::array<uint8_t, 4> pxData{ { 0x0, 0xff, 0x0, 0xff } };
            const std::vector<MipLevelData> mipLevelData{ MipLevelData(4, pxData.data()) };
            const ramses::Texture2D& texture = *m_scene.createTexture2D(ramses::ETextureFormat::RGBA8, 1u, 1u, mipLevelData, false, {});
            const ramses::TextureSampler& sampler = createSampler(texture);
            SetSampler(appearance2, sampler);

            scene.createTextureConsumer(sampler, TextureConsumerId);
        }
            break;
        default:
            assert(false && "invalid scene state");
        }
    }

    const ramses::TextureSampler& MultiTypeLinkScene::createSampler(const ramses::Texture2D& texture)
    {
        return *m_scene.createTextureSampler(ramses::ETextureAddressMode::Repeat, ramses::ETextureAddressMode::Repeat, ramses::ETextureSamplingMethod::Nearest, ramses::ETextureSamplingMethod::Nearest, texture, 1u, "dataLinkSampler");
    }

    void MultiTypeLinkScene::SetSampler(ramses::Appearance& appearance, const ramses::TextureSampler& sampler)
    {
        appearance.setInputTexture(*appearance.getEffect().findUniformInput("u_texture"), sampler);
    }
}
