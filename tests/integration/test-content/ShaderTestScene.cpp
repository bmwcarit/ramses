//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/ShaderTestScene.h"
#include "ramses/client/ramses-utils.h"
#include "ramses/client/Scene.h"
#include "ramses/client/Appearance.h"
#include "ramses/client/MeshNode.h"
#include "ramses/client/Effect.h"
#include "ramses/client/UniformInput.h"
#include "ramses/client/AttributeInput.h"
#include "ramses/client/Geometry.h"
#include "ramses/client/Texture2D.h"
#include <cassert>

namespace ramses::internal
{
    ShaderTestScene::ShaderTestScene(ramses::Scene& scene, uint32_t state, const glm::vec3& cameraPosition)
        : IntegrationScene(scene, cameraPosition)
        , m_effect(*getTestEffect(GetEffectNameFromState(state)))
        , m_triangle(scene, m_effect, TriangleAppearance::EColor_Red)
    {
        ramses::MeshNode* meshNode = m_scene.createMeshNode("red triangle mesh node");
        addMeshNodeToDefaultRenderGroup(*meshNode);
        meshNode->setAppearance(m_triangle.GetAppearance());
        meshNode->setGeometry(m_triangle.GetGeometry());

        ramses::Node* transNode = m_scene.createNode();
        transNode->setTranslation({0.f, 0.f, -12.f});
        meshNode->setParent(*transNode);

        initInputs(state);
    }

    std::string ShaderTestScene::GetEffectNameFromState(uint32_t state)
    {
        switch (state)
        {
        case DISCARD:
            return "ramses-test-client-discard";
        case OPTIMIZED_INPUT:
            return "ramses-test-client-optimizedInput";
        case UNIFORM_WITH_SAME_NAME_IN_BOTH_STAGES:
            return "ramses-test-client-multiStageUniform";
        case STRUCT_UNIFORM:
            return "ramses-test-client-structUniform";
        case TEXTURE_SIZE:
            return "ramses-test-client-textureSize";
        case BOOL_UNIFORM:
            return "ramses-test-client-boolUniform";
        default:
            assert(false && "Unknown state!");
            return "";
        };

    }

    void ShaderTestScene::initInputs(uint32_t state)
    {
        ramses::Appearance& appearance = m_triangle.GetAppearance();
        std::optional<ramses::UniformInput> optInput;

        if (state == OPTIMIZED_INPUT)
        {
            optInput = m_effect.findUniformInput("zombieUniform");
            assert(optInput.has_value());
            if (optInput.has_value())
            {
                [[maybe_unused]] bool status = appearance.setInputValue(*optInput, 10.f);
                assert(status);
            }
        }
        else if (state == UNIFORM_WITH_SAME_NAME_IN_BOTH_STAGES)
        {
            optInput = m_effect.findUniformInput("multiStageUniform");
            assert(optInput.has_value());
            if (optInput.has_value())
            {
                [[maybe_unused]] bool status = appearance.setInputValue(*optInput, 1.f);
                assert(status);
            }
        }
        else if (state == STRUCT_UNIFORM)
        {
            // The idea behind this test is to show that setting the struct members individually works.
            // The shader itself is a combination of nested structs and arrays.

            // Set the specific values for rendering - this will result in a pink-ish color
            optInput = m_effect.findUniformInput("inputVar.data[0].red");
            assert(optInput.has_value());
            [[maybe_unused]] bool status = appearance.setInputValue(*optInput, 0.8f);
            assert(status);
            optInput = m_effect.findUniformInput("inputVar.data[0].green");
            assert(optInput.has_value());
            status = appearance.setInputValue(*optInput, 0.2f);
            assert(status);
            optInput = m_effect.findUniformInput("inputVar.data[1].blue");
            assert(optInput.has_value());
            status = appearance.setInputValue(*optInput, 0.75f);
            assert(status);
            optInput = m_effect.findUniformInput("inputVar.data[1].alpha");
            assert(optInput.has_value());
            status = appearance.setInputValue(*optInput, 0.9f);
            assert(status);
        }
        else if (state == TEXTURE_SIZE)
        {
            const ramses::Texture2D& texture = *ramses::RamsesUtils::CreateTextureResourceFromPng("res/ramses-test-client-file-loading-texture.png", m_scene);
            ramses::TextureSampler& texSampler = *m_scene.createTextureSampler(ramses::ETextureAddressMode::Repeat, ramses::ETextureAddressMode::Repeat, ramses::ETextureSamplingMethod::Nearest, ramses::ETextureSamplingMethod::Nearest, texture);

            const std::array<ramses::vec2f, 3u> textureCoordsArray{ ramses::vec2f{0.f, 1.f}, ramses::vec2f{1.f, 1.f}, ramses::vec2f{0.f, 0.f} };
            const ramses::ArrayResource* textureCoords = m_scene.createArrayResource(3u, textureCoordsArray.data());

            m_triangle.GetGeometry().setInputBuffer(*m_effect.findAttributeInput("a_texCoords"), *textureCoords);

            // Adjust the vertices, such that the triangle texture looks less skewed
            const std::array<ramses::vec3f, 3u> vertexPositionsData{ ramses::vec3f{-1.f, 0.f, -1.f}, ramses::vec3f{1.f, 0.f, -1.f}, ramses::vec3f{-1.f, 1.f, -1.f} };
            const ramses::ArrayResource* vertexPositions = m_scene.createArrayResource(3u, vertexPositionsData.data());

            m_triangle.GetGeometry().setInputBuffer(*m_effect.findAttributeInput("a_position"), *vertexPositions);
            appearance.setInputTexture(*appearance.getEffect().findUniformInput("u_texture"), texSampler);

            // Pass in the texture size as a uniform, which allows us to compare against the output of the "textureSize" method in the shader.
            appearance.setInputValue(*appearance.getEffect().findUniformInput("texSizeFromApplication"),
                                     ramses::vec2i{static_cast<int32_t>(texture.getWidth()), static_cast<int32_t>(texture.getHeight())});
        }
        else if (state == BOOL_UNIFORM)
        {
            // Tests that bool uniforms work
            optInput = m_effect.findUniformInput("makeSemiTransparent");
            assert(optInput.has_value());
            if (optInput.has_value())
            {
                [[maybe_unused]] bool status = appearance.setInputValue(*optInput, true);
                assert(status);
            }

            optInput = m_effect.findUniformInput("colorCh");
            assert(optInput.has_value());
            if (optInput.has_value())
            {
                std::array<bool, 3>               colorCh = {true, false, false};
                [[maybe_unused]] bool status  = appearance.setInputValue(*optInput, 3, colorCh.data());
                assert(status);
            }
        }
    }
}
