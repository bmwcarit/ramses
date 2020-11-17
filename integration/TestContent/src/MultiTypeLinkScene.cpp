//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/MultiTypeLinkScene.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/DataVector4f.h"
#include "ramses-client-api/AnimationSystem.h"
#include "ramses-client-api/SplineLinearFloat.h"
#include "ramses-client-api/AnimationSequence.h"
#include "ramses-client-api/AttributeInput.h"
#include "ramses-client-api/GeometryBinding.h"
#include "ramses-client-api/Effect.h"
#include "ramses-client-api/Appearance.h"

#include "Scene/ClientScene.h"
#include "TestScenes/Triangle.h"
#include <array>

namespace ramses_internal
{
    constexpr const ramses::dataProviderId_t MultiTypeLinkScene::TransformationProviderId;
    constexpr const ramses::dataConsumerId_t MultiTypeLinkScene::TransformationConsumerId;
    constexpr const ramses::dataProviderId_t MultiTypeLinkScene::DataProviderId;
    constexpr const ramses::dataConsumerId_t MultiTypeLinkScene::DataConsumerId;
    constexpr const ramses::dataProviderId_t MultiTypeLinkScene::TextureProviderId;
    constexpr const ramses::dataConsumerId_t MultiTypeLinkScene::TextureConsumerId;

    MultiTypeLinkScene::MultiTypeLinkScene(ramses::Scene& scene, UInt32 state, const Vector3& cameraPosition, uint32_t vpWidth, uint32_t vpHeight)
        : IntegrationScene(scene, cameraPosition, vpWidth, vpHeight)
    {
        ramses::Effect* effect = getTestEffect("ramses-test-client-basic");
        ramses::Effect* effectTex = getTestEffect("ramses-test-client-textured");
        ramses::Triangle triangle1(scene, *effect, ramses::TriangleAppearance::EColor_Blue);
        ramses::Triangle triangle2(scene, *effectTex, ramses::TriangleAppearance::EColor_Green);

        ramses::DataVector4f* colorData = scene.createDataVector4f();

        triangle1.bindColor(*colorData);

        ramses::MeshNode* mesh1 = scene.createMeshNode();
        ramses::MeshNode* mesh2 = scene.createMeshNode();

        ramses::Appearance& appearance1 = triangle1.GetAppearance();
        ramses::Appearance& appearance2 = triangle2.GetAppearance();

        mesh1->setAppearance(appearance1);
        mesh2->setAppearance(appearance2);

        mesh1->setGeometryBinding(triangle1.GetGeometry());
        mesh2->setGeometryBinding(triangle2.GetGeometry());

        ramses::Node* translate1 = scene.createNode();
        ramses::Node* translate2 = scene.createNode();

        mesh1->setParent(*translate1);
        mesh2->setParent(*translate2);

        addMeshNodeToDefaultRenderGroup(*mesh1);
        addMeshNodeToDefaultRenderGroup(*mesh2);

        ramses::Node* groupNode = scene.createNode();
        translate1->setParent(*groupNode);
        translate2->setParent(*groupNode);

        translate1->setTranslation(-1.5f, 0.f, -15.f);
        translate2->setTranslation(1.5f, 0.f, -15.f);

        const float textureCoordsArray[] = { 0.f, 1.f, 1.f, 1.f, 0.f, 0.f };
        const ramses::ArrayResource* textureCoords = m_scene.createArrayResource(ramses::EDataType::Vector2F, 3u, textureCoordsArray);

        ramses::AttributeInput texCoordsInput;
        effectTex->findAttributeInput("a_texcoord", texCoordsInput);
        triangle2.GetGeometry().setInputBuffer(texCoordsInput, *textureCoords);

        switch (state)
        {
        case TRANSFORMATION_CONSUMER_DATA_AND_TEXTURE_PROVIDER:
        {
            scene.createDataProvider(*colorData, DataProviderId);
            colorData->setValue(1.f, 0.f, 0.f, 1.f);
            animateProvidedContent(*colorData);

            scene.createTransformationDataConsumer(*groupNode, TransformationConsumerId);

            const std::array<uint8_t, 4> pxData{ {0xff, 0x0, 0x0, 0xff} };
            const ramses::MipLevelData mipLevelData(4, pxData.data());
            const ramses::Texture2D& texture = *m_scene.createTexture2D(ramses::ETextureFormat::RGBA8, 1u, 1u, 1, &mipLevelData, false, {}, ramses::ResourceCacheFlag_DoNotCache);
            const ramses::TextureSampler& sampler = createSampler(texture);
            setSampler(appearance2, sampler);

            scene.createTextureProvider(texture, TextureProviderId);
        }
            break;
        case TRANSFORMATION_PROVIDER_DATA_AND_TEXTURE_CONSUMER:
        {
            scene.createDataConsumer(*colorData, DataConsumerId);
            colorData->setValue(0.f, 1.f, 0.f, 1.f);

            ramses::Node* providerNode = scene.createNode();
            providerNode->setTranslation(1.5f, -2.f, 0.f);
            scene.createTransformationDataProvider(*providerNode, TransformationProviderId);
            animateProvidedContent(*providerNode);

            const std::array<uint8_t, 4> pxData{ { 0x0, 0xff, 0x0, 0xff } };
            const ramses::MipLevelData mipLevelData(4, pxData.data());
            const ramses::Texture2D& texture = *m_scene.createTexture2D(ramses::ETextureFormat::RGBA8, 1u, 1u, 1, &mipLevelData, false, {}, ramses::ResourceCacheFlag_DoNotCache);
            const ramses::TextureSampler& sampler = createSampler(texture);
            setSampler(appearance2, sampler);

            scene.createTextureConsumer(sampler, TextureConsumerId);
        }
            break;
        default:
            assert(false && "invalid scene state");
        }
    }

    void MultiTypeLinkScene::animateProvidedContent(ramses::DataObject& dataObject)
    {
        ramses::AnimationSystem* animSystem = m_scene.createAnimationSystem();
        ramses::AnimatedProperty* animProperty = animSystem->createAnimatedProperty(dataObject, ramses::EAnimatedPropertyComponent_Y);
        ramses::SplineLinearFloat* spline = animSystem->createSplineLinearFloat();
        spline->setKey(0u, 0.f);
        spline->setKey(1000u, 1.f);
        ramses::Animation* animation = animSystem->createAnimation(*animProperty, *spline);
        ramses::AnimationSequence* seq = animSystem->createAnimationSequence();
        seq->addAnimation(*animation);
        seq->startAt(0u);

        animSystem->setTime(0u);
        animSystem->setTime(500u);
        animSystem->setTime(1500u);
    }

    void MultiTypeLinkScene::animateProvidedContent(ramses::Node& translateNode)
    {
        ramses::AnimationSystem* animSystem = m_scene.createAnimationSystem();
        ramses::AnimatedProperty* animProperty = animSystem->createAnimatedProperty(translateNode, ramses::EAnimatedProperty_Translation, ramses::EAnimatedPropertyComponent_Z);
        ramses::SplineLinearFloat* spline = animSystem->createSplineLinearFloat();
        spline->setKey(0u, 0.f);
        spline->setKey(1000u, 10.f);
        ramses::Animation* animation = animSystem->createAnimation(*animProperty, *spline);
        ramses::AnimationSequence* seq = animSystem->createAnimationSequence();
        seq->addAnimation(*animation);

        animSystem->setTime(0u);
        seq->startAt(0u);
        animSystem->setTime(500u);
    }

    const ramses::TextureSampler& MultiTypeLinkScene::createSampler(const ramses::Texture2D& texture)
    {
        return *m_scene.createTextureSampler(ramses::ETextureAddressMode_Repeat, ramses::ETextureAddressMode_Repeat, ramses::ETextureSamplingMethod_Nearest, ramses::ETextureSamplingMethod_Nearest, texture, 1u, "dataLinkSampler");
    }

    void MultiTypeLinkScene::setSampler(ramses::Appearance& appearance, const ramses::TextureSampler& sampler)
    {
        ramses::UniformInput texInput;
        appearance.getEffect().findUniformInput("u_texture", texInput);
        appearance.setInputTexture(texInput, sampler);
    }
}
