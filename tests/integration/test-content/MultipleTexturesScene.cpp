//  -------------------------------------------------------------------------
//  Copyright (C) 2024 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/MultipleTexturesScene.h"
#include "TestScenes/Triangle.h"
#include "ramses/client/ramses-utils.h"

#include "ramses/client/Scene.h"
#include "ramses/client/Appearance.h"
#include "ramses/client/UniformInput.h"
#include "ramses/client/AttributeInput.h"
#include "ramses/client/Effect.h"
#include "ramses/client/Geometry.h"

namespace ramses::internal
{

    MultipleTexturesScene::MultipleTexturesScene(ramses::Scene& scene, uint32_t state, const glm::vec3& cameraPosition)
        : IntegrationScene(scene, cameraPosition)
    {
        auto* effect = createEffect(state);

        const auto* texture1 = ramses::RamsesUtils::CreateTextureResourceFromPng("res/ramses-test-client-file-loading-texture.png", m_scene);
        const auto* texture2 = ramses::RamsesUtils::CreateTextureResourceFromPng("res/ramses-test-client-logo-cropped.png", m_scene);
        const auto* texture3 = ramses::RamsesUtils::CreateTextureResourceFromPng("res/ramses-test-client-cube-px.png", m_scene);

        const auto* texSampler1 = m_scene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Nearest, ETextureSamplingMethod::Nearest, *texture1);
        const auto* texSampler2 = m_scene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Nearest, ETextureSamplingMethod::Nearest, *texture2);
        const auto* texSampler3 = m_scene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Nearest, ETextureSamplingMethod::Nearest, *texture3);

        const auto* texCoords = scene.createArrayResource(3u, std::array{ vec2f{0.f, 0.f}, vec2f{1.f, 0.f}, vec2f{1.f, 1.f} }.data());

        const std::array translation = {
            -1.0f,  0.0f, -12.0f,
            0.0f, -1.0f, -12.0f,
            1.0f,  0.0f, -12.0f };


        for (int i = 0; i < 3; ++i)
        {
            Triangle triangle(m_scene, *effect, TriangleAppearance::EColor::None);

            ramses::MeshNode* meshNode = m_scene.createMeshNode("triangle mesh node");
            addMeshNodeToDefaultRenderGroup(*meshNode);

            ramses::Node* trafoNode = m_scene.createNode("transformation node");
            trafoNode->setTranslation({translation[i * 3 + 0], translation[i * 3 + 1], translation[i * 3 + 2]});

            meshNode->setParent(*trafoNode);

            auto& appearance = triangle.GetAppearance();
            appearance.setInputValue(*effect->findUniformInput("u_multiplexer"), i + 1);
            appearance.setInputTexture(*effect->findUniformInput("u_texture1"), *texSampler1);
            appearance.setInputTexture(*effect->findUniformInput("u_texture2"), *texSampler2);
            appearance.setInputTexture(*effect->findUniformInput("u_texture3"), *texSampler3);
            meshNode->setAppearance(appearance);

            auto& geometry = triangle.GetGeometry();
            geometry.setInputBuffer(*effect->findAttributeInput("a_texcoord"), *texCoords);
            meshNode->setGeometry(geometry);
        }
    }

    ramses::Effect* MultipleTexturesScene::createEffect(uint32_t state)
    {
        if(state == THREE_MULTIPLEXED_TEXTURES)
            return getTestEffect("ramses-test-client-multiple-textures");

        if (state == THREE_MULTIPLEXED_TEXTURES_UBO)
            return getTestEffect("ramses-test-client-multiple-textures-ubo");

        return nullptr;
    }
}
