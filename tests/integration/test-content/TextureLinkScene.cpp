//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/TextureLinkScene.h"
#include "ramses/client/Scene.h"
#include "ramses/client/MeshNode.h"
#include "ramses/client/AttributeInput.h"
#include "ramses/client/Geometry.h"
#include "ramses/client/TextureSampler.h"
#include "ramses/client/Effect.h"
#include "ramses/client/Appearance.h"
#include "ramses/client/RenderGroup.h"
#include "ramses/client/RenderPass.h"
#include "ramses/client/RenderTargetDescription.h"
#include "ramses/client/PerspectiveCamera.h"
#include "ramses/client/RenderTarget.h"

#include "internal/SceneGraph/Scene/ClientScene.h"
#include "TestScenes/Triangle.h"
#include <array>

namespace ramses::internal
{
    TextureLinkScene::TextureLinkScene(ramses::Scene& scene, uint32_t state, const glm::vec3& cameraPosition, uint32_t vpWidth, uint32_t vpHeight)
        : IntegrationScene(scene, cameraPosition, vpWidth, vpHeight)
    {
        ramses::Effect* effect = (state == DATA_CONSUMER_MS) ? getTestEffect("ramses-test-client-render-one-buffer-ms") : getTestEffect("ramses-test-client-textured");
        Triangle triangle1(scene, *effect, TriangleAppearance::EColor_Red);
        Triangle triangle2(scene, *effect, TriangleAppearance::EColor_Green);

        ramses::MeshNode* mesh1 = scene.createMeshNode();
        ramses::MeshNode* mesh2 = scene.createMeshNode();

        ramses::Appearance& appearance1 = triangle1.GetAppearance();
        ramses::Appearance& appearance2 = triangle2.GetAppearance();
        appearance1.setName("dataLinkAppearance1");
        appearance2.setName("dataLinkAppearance2");

        mesh1->setAppearance(appearance1);
        mesh2->setAppearance(appearance2);

        mesh1->setGeometry(triangle1.GetGeometry());
        mesh2->setGeometry(triangle2.GetGeometry());

        ramses::Node* translate1 = scene.createNode();
        ramses::Node* translate2 = scene.createNode();

        if (DATA_PROVIDER_LARGE == state || DATA_CONSUMER_AND_PROVIDER_LARGE == state)
        {
            ramses::Node* scale1 = scene.createNode();
            ramses::Node* scale2 = scene.createNode();

            scale1->setScaling({5.0f, 5.0f, 5.0f});
            scale2->setScaling({5.0f, 5.0f, 5.0f});

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

        translate1->setTranslation({-1.5f, 0.f, -15.f});
        translate2->setTranslation({1.5f, 0.f, -15.f});

        const std::array<ramses::vec2f, 3u> textureCoordsArray{ ramses::vec2f{0.f, 1.f}, ramses::vec2f{1.f, 1.f}, ramses::vec2f{0.f, 0.f} };
        const ramses::ArrayResource* textureCoords = m_scene.createArrayResource(3u, textureCoordsArray.data());

        const std::optional<ramses::AttributeInput> optTexCoordsInput = effect->findAttributeInput("a_texcoord");
        triangle1.GetGeometry().setInputBuffer(*optTexCoordsInput, *textureCoords);
        triangle2.GetGeometry().setInputBuffer(*optTexCoordsInput, *textureCoords);

        switch (state)
        {
        case DATA_PROVIDER_LARGE:
        case DATA_PROVIDER:
        {
            const std::array<uint8_t, 4> pxData{ {0xff, 0x0, 0x0, 0xff} };
            const std::vector<MipLevelData> mipLevelData{ MipLevelData(4, pxData.data()) };
            const ramses::Texture2D& texture = *m_scene.createTexture2D(ramses::ETextureFormat::RGBA8, 1u, 1u, mipLevelData, false, {}, "ProviderTexture");
            const ramses::TextureSampler& sampler = createSampler(texture);
            SetSampler(appearance1, sampler);
            SetSampler(appearance2, sampler);

            scene.createTextureProvider(texture, DataProviderId);
        }
            break;
        case DATA_CONSUMER:
        {
            const std::array<uint8_t, 4> pxData{ { 0x0, 0xff, 0x0, 0xff } };
            const std::vector<MipLevelData> mipLevelData{ MipLevelData(4, pxData.data()) };
            const ramses::Texture2D& texture = *m_scene.createTexture2D(ramses::ETextureFormat::RGBA8, 1u, 1u, mipLevelData, false, {}, "ConsumerTexture");
            const ramses::TextureSampler& sampler = createSampler(texture);
            SetSampler(appearance1, sampler);
            SetSampler(appearance2, sampler);

            scene.createTextureConsumer(sampler, DataConsumerId);
        }
            break;
        case DATA_CONSUMER_MS:
        {
            const ramses::RenderBuffer& fallBackBuffer = *m_scene.createRenderBuffer(16u, 16u, ramses::ERenderBufferFormat::RGB8, ramses::ERenderBufferAccessMode::ReadWrite, 4u, "ConsumerBuffer");
            // Create a RenderPass that clears the fallBackBuffer to a visible color, so it can be seen in case the fallBackBuffer is shown
            ramses::RenderPass& renderPassClear = *m_scene.createRenderPass();
            ramses::PerspectiveCamera& camera = *m_scene.createPerspectiveCamera();
            // dummy camera values here, because it is only a clear pass
            camera.setFrustum(1.0f, 1.0f, 1.0f, 100.0f);
            camera.setViewport(0u, 0u, 2u, 2u);
            renderPassClear.setCamera(camera);
            const ramses::RenderGroup& renderGroup = *m_scene.createRenderGroup();
            renderPassClear.addRenderGroup(renderGroup);
            ramses::RenderTargetDescription rtdesc{};
            rtdesc.addRenderBuffer(fallBackBuffer);
            ramses::RenderTarget* renderTarget = m_scene.createRenderTarget(rtdesc);
            renderPassClear.setRenderTarget(renderTarget);
            renderPassClear.setClearColor({1.0f, 1.0f, 0.f, 1.0f});

            const ramses::TextureSamplerMS& sampler = *m_scene.createTextureSamplerMS(fallBackBuffer, "samplerMSConsumer");

            appearance1.setInputTexture(*appearance1.getEffect().findUniformInput("textureSampler"), sampler);
            appearance2.setInputTexture(*appearance2.getEffect().findUniformInput("textureSampler"), sampler);

            appearance1.setInputValue(*appearance1.getEffect().findUniformInput("sampleCount"), 4);
            appearance2.setInputValue(*appearance2.getEffect().findUniformInput("sampleCount"), 4);

            scene.createTextureConsumer(sampler, DataConsumerId);
        }
            break;
        case DATA_CONSUMER_AND_PROVIDER_LARGE:
        case DATA_CONSUMER_AND_PROVIDER:
        {
            const std::array<uint8_t, 4> pxData{ { 0x0, 0x0, 0xff, 0xff } };
            const std::vector<MipLevelData> mipLevelData{ MipLevelData(4, pxData.data()) };
            const ramses::Texture2D& texture = *m_scene.createTexture2D(ramses::ETextureFormat::RGBA8, 1u, 1u, mipLevelData, false, {}, "ConsumerProviderTexture");
            const ramses::TextureSampler& sampler1 = createSampler(texture);
            const ramses::TextureSampler& sampler2 = createSampler(texture);
            SetSampler(appearance1, sampler1);
            SetSampler(appearance2, sampler2);

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
        return *m_scene.createTextureSampler(ramses::ETextureAddressMode::Repeat, ramses::ETextureAddressMode::Repeat, ramses::ETextureSamplingMethod::Nearest, ramses::ETextureSamplingMethod::Nearest, texture, 1u, "dataLinkSampler");
    }

    void TextureLinkScene::SetSampler(ramses::Appearance& appearance, const ramses::TextureSampler& sampler)
    {
        appearance.setInputTexture(*appearance.getEffect().findUniformInput("u_texture"), sampler);
    }
}
