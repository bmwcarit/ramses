//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/ShaderTestScene.h"
#include "ramses-utils.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/Effect.h"
#include "ramses-client-api/UniformInput.h"
#include "ramses-client-api/AttributeInput.h"
#include "ramses-client-api/GeometryBinding.h"
#include "ramses-client-api/RamsesClient.h"
#include "ramses-client-api/Texture2D.h"
#include "TextureUtils.h"

namespace ramses_internal
{
    ShaderTestScene::ShaderTestScene(ramses::RamsesClient& ramsesClient, ramses::Scene& scene, UInt32 state, const Vector3& cameraPosition)
        : IntegrationScene(ramsesClient, scene, cameraPosition)
        , m_effect(*getTestEffect(getEffectNameFromState(state)))
        , m_triangle(ramsesClient, scene, m_effect, ramses::TriangleAppearance::EColor_Red)
    {
        ramses::MeshNode* meshNode = m_scene.createMeshNode("red triangle mesh node");
        addMeshNodeToDefaultRenderGroup(*meshNode);
        meshNode->setAppearance(m_triangle.GetAppearance());
        meshNode->setGeometryBinding(m_triangle.GetGeometry());

        ramses::Node* transNode = m_scene.createNode();
        transNode->setTranslation(0.f, 0.f, -12.f);
        meshNode->setParent(*transNode);

        initInputs(state);
    }

    String ShaderTestScene::getEffectNameFromState(UInt32 state) const
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
        default:
            assert(false && "Unknown state!");
            return "";
        };

    }

    void ShaderTestScene::initInputs(UInt32 state)
    {
        ramses::Appearance& appearance = m_triangle.GetAppearance();
        ramses::UniformInput input;

        if (state == OPTIMIZED_INPUT)
        {
            ramses::status_t status = m_effect.findUniformInput("zombieUniform", input);
            assert(status == ramses::StatusOK);
            if (status == ramses::StatusOK)
            {
                status = appearance.setInputValueFloat(input, 10.f);
                assert(status == ramses::StatusOK);
            }
        }
        else if (state == UNIFORM_WITH_SAME_NAME_IN_BOTH_STAGES)
        {
            ramses::status_t status = m_effect.findUniformInput("multiStageUniform", input);
            assert(status == ramses::StatusOK);
            if (status == ramses::StatusOK)
            {
                status = appearance.setInputValueFloat(input, 1.f);
                assert(status == ramses::StatusOK);
            }
        }
        else if (state == STRUCT_UNIFORM)
        {
            // The idea behind this test is to show that setting the struct members individually works.
            // The shader itself is a combination of nested structs and arrays.

            // Set the specific values for rendering - this will result in a pink-ish color
            ramses::status_t status = m_effect.findUniformInput("inputVar.data[0].red", input);
            UNUSED(status); // Needed by release build
            assert(ramses::StatusOK == status);
            status = appearance.setInputValueFloat(input, 0.8f);
            assert(ramses::StatusOK == status);
            status = m_effect.findUniformInput("inputVar.data[0].green", input);
            assert(ramses::StatusOK == status);
            status = appearance.setInputValueFloat(input, 0.2f);
            assert(ramses::StatusOK == status);
            status = m_effect.findUniformInput("inputVar.data[1].blue", input);
            assert(ramses::StatusOK == status);
            status = appearance.setInputValueFloat(input, 0.75f);
            assert(ramses::StatusOK == status);
            status = m_effect.findUniformInput("inputVar.data[1].alpha", input);
            assert(ramses::StatusOK == status);
            status = appearance.setInputValueFloat(input, 0.9f);
            assert(ramses::StatusOK == status);
        }
        else if (state == TEXTURE_SIZE)
        {
            const ramses::Texture2D& texture = *ramses::TextureUtils::CreateTextureResourceFromPng("res/ramses-test-client-file-loading-texture.png", m_client);
            ramses::TextureSampler& texSampler = *m_scene.createTextureSampler(ramses::ETextureAddressMode_Repeat, ramses::ETextureAddressMode_Repeat, ramses::ETextureSamplingMethod_Nearest, ramses::ETextureSamplingMethod_Nearest, texture);

            const Float textureCoordsArray[] = { 0.f, 1.f, 1.f, 1.f, 0.f, 0.f };
            const ramses::Vector2fArray* textureCoords = m_client.createConstVector2fArray(3u, textureCoordsArray);

            ramses::AttributeInput texCoordsInput;
            m_effect.findAttributeInput("a_texCoords", texCoordsInput);
            m_triangle.GetGeometry().setInputBuffer(texCoordsInput, *textureCoords);

            // Adjust the vertices, such that the triangle texture looks less skewed
            const Float vertexPositionsData[] = { -1.f, 0.f, -1.f, 1.f, 0.f, -1.f, -1.f, 1.f, -1.f };
            const ramses::Vector3fArray* vertexPositions = m_client.createConstVector3fArray(3, vertexPositionsData);

            ramses::AttributeInput positionsInput;
            m_effect.findAttributeInput("a_position", positionsInput);
            m_triangle.GetGeometry().setInputBuffer(positionsInput, *vertexPositions);

            ramses::UniformInput texInput;
            appearance.getEffect().findUniformInput("u_texture", texInput);
            appearance.setInputTexture(texInput, texSampler);

            // Pass in the texture size as a uniform, which allows us to compare against the output of the "textureSize" method in the shader.
            appearance.getEffect().findUniformInput("texSizeFromApplication", texInput);
            appearance.setInputValueVector2i(texInput, texture.getWidth(), texture.getHeight());
        }
    }
}
