//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/CustomShaderTestScene.h"
#include "ramses/client/ramses-utils.h"
#include "ramses/client/Scene.h"
#include "ramses/client/Appearance.h"
#include "ramses/client/Geometry.h"
#include "ramses/client/MeshNode.h"
#include "ramses/client/Effect.h"
#include "ramses/client/UniformInput.h"
#include "ramses/client/AttributeInput.h"
#include <cassert>

namespace ramses::internal
{
    CustomShaderTestScene::CustomShaderTestScene(ramses::Scene& scene, uint32_t state, const glm::vec3& cameraPosition)
        : IntegrationScene(scene, cameraPosition)
        , m_effect(*getTestEffect(GetEffectNameFromState(state)))
        , m_appearance(*scene.createAppearance(m_effect))
        , m_geometryBinding(*scene.createGeometry(m_effect))
    {
        ramses::MeshNode* meshNode = m_scene.createMeshNode("red triangle mesh node");
        addMeshNodeToDefaultRenderGroup(*meshNode);
        meshNode->setAppearance(m_appearance);

        ramses::Node* transNode = m_scene.createNode();
        transNode->setTranslation({0.f, 0.f, -6.f});
        meshNode->setParent(*transNode);

        m_appearance.setDrawMode(ramses::EDrawMode::TriangleStrip);

        createGeometry();
        meshNode->setGeometry(m_geometryBinding);

        initInputs();
    }

    std::string CustomShaderTestScene::GetEffectNameFromState(uint32_t state)
    {
        switch (state)
        {
        case EXPLICIT_ATTRIBUTE_LOCATION:
            return "ramses-test-client-explicitLocation";
        case EXPLICIT_ATTRIBUTE_LOCATION_SWAPPED:
            return "ramses-test-client-explicitLocationSwapped";
        default:
            assert(false && "Unknown state!");
            return "";
        };
    }

    void CustomShaderTestScene::createGeometry()
    {
        const std::array<uint16_t, 4> indiceData_ccw = { 0, 1, 2, 3 };
        const ramses::ArrayResource& indices = *m_scene.createArrayResource(4u, indiceData_ccw.data());
        m_geometryBinding.setIndices(indices);

        const std::array<ramses::vec3f, 4u> vertexPositionsData
        {
            ramses::vec3f{ -1.f, 1.f, -1.f },
            ramses::vec3f{ -1.f, -1.f, -1.f },
            ramses::vec3f{ 1.f, 1.f, -1.f },
            ramses::vec3f{ 1.f, -1.f, -1.f }
        };
        const ramses::ArrayResource& vertexPositions = *m_scene.createArrayResource(4u, vertexPositionsData.data());
        m_geometryBinding.setInputBuffer(*m_effect.findAttributeInput("a_position"), vertexPositions);

        const std::array<ramses::vec2f, 4u> texCoordsData
        {
            ramses::vec2f{ 0.f, 0.f },
            ramses::vec2f{ 0.f, 1.f },
            ramses::vec2f{ 1.f, 0.f },
            ramses::vec2f{ 1.f, 1.f }
        };
        const ramses::ArrayResource& texCoords = *m_scene.createArrayResource(4u, texCoordsData.data());
        m_geometryBinding.setInputBuffer(*m_effect.findAttributeInput("a_texcoord"), texCoords);
    }

    void CustomShaderTestScene::initInputs()
    {
        ramses::Texture2D* texture = ramses::RamsesUtils::CreateTextureResourceFromPng("res/ramses-test-client-cube-px.png", m_scene);
        assert(texture != nullptr);

        ramses::TextureSampler& texSampler = *m_scene.createTextureSampler(ramses::ETextureAddressMode::Clamp, ramses::ETextureAddressMode::Clamp, ramses::ETextureSamplingMethod::Nearest, ramses::ETextureSamplingMethod::Nearest, *texture);
        m_appearance.setInputTexture(*m_effect.findUniformInput("u_texture"), texSampler);
    }
}
