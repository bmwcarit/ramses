//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "DynamicQuad_Base.h"

#include "ramses/client/Scene.h"
#include "ramses/client/Effect.h"
#include "ramses/client/EffectDescription.h"
#include "ramses/client/Appearance.h"
#include "ramses/client/Geometry.h"
#include "ramses/client/UniformInput.h"
#include "ramses/client/AttributeInput.h"
#include "ramses/client/ArrayResource.h"

#include "internal/PlatformAbstraction/PlatformMath.h"
#include "TestRandom.h"

namespace ramses::internal
{
    DynamicQuad_Base::DynamicQuad_Base(ramses::Scene& scene, const ScreenspaceQuad& screenspaceQuad)
        : m_scene(scene)
        , m_screenspaceQuad(screenspaceQuad)
        , m_renderGroup(*m_scene.createRenderGroup())
        , m_meshNode(*m_scene.createMeshNode())
        , m_effect(CreateTestEffect(m_scene))
        , m_appearance(*m_scene.createAppearance(m_effect))
        , m_geometryBinding(*m_scene.createGeometry(m_effect))
    {
    }

    const ramses::RenderGroup& DynamicQuad_Base::getRenderGroup() const
    {
        return m_renderGroup;
    }

    ramses::RenderGroup& DynamicQuad_Base::getRenderGroup()
    {
        return m_renderGroup;
    }

    const ramses::MeshNode& DynamicQuad_Base::getMeshNode() const
    {
        return m_meshNode;
    }

    QuadResources DynamicQuad_Base::createRandomizedQuadResources()
    {
        QuadResources resources;

        const std::array<uint16_t, 4> indiceData = { 0, 1, 3, 2 };
        resources.indices = m_scene.createArrayResource(4, indiceData.data());

        const std::array vertexPositionsData =
        {
            m_screenspaceQuad.getVertex(EScreenspaceQuadVertex::BottomLeft, 10u),
            m_screenspaceQuad.getVertex(EScreenspaceQuadVertex::BottomRight, 10u),
            m_screenspaceQuad.getVertex(EScreenspaceQuadVertex::TopRight, 10u),
            m_screenspaceQuad.getVertex(EScreenspaceQuadVertex::TopLeft, 10u)
        };

        std::array vertexTexcoordsData = {
            ramses::vec2f{0.f, 0.f},
            ramses::vec2f{1.f, 0.f},
            ramses::vec2f{1.f, 1.f},
            ramses::vec2f{0.f, 1.f}
        };

        for (auto& tc : vertexTexcoordsData)
        {
            tc[0] += 0.01f * static_cast<float>(TestRandom::Get(0, 10));
            tc[1] += 0.01f * static_cast<float>(TestRandom::Get(0, 10));
        }

        resources.texCoords = m_scene.createArrayResource(4, vertexTexcoordsData.data());
        resources.vertexPos = m_scene.createArrayResource(4, vertexPositionsData.data());

        return resources;
    }

    void DynamicQuad_Base::destroyQuadResources(const QuadResources& quadResources)
    {
        if (quadResources.indices)
            m_scene.destroy(*quadResources.indices);
        if (quadResources.vertexPos)
            m_scene.destroy(*quadResources.vertexPos);
        if (quadResources.texCoords)
            m_scene.destroy(*quadResources.texCoords);
    }

    void DynamicQuad_Base::setQuadResources(const QuadResources& quad, ramses::TextureSampler& textureSampler)
    {
        m_geometryBinding.setIndices(*quad.indices);
        m_geometryBinding.setInputBuffer(*m_appearance.getEffect().findAttributeInput("a_position"), *quad.vertexPos);
        m_geometryBinding.setInputBuffer(*m_appearance.getEffect().findAttributeInput("a_texcoord"), *quad.texCoords);

        m_appearance.setInputTexture(*m_appearance.getEffect().findUniformInput("u_texture"), textureSampler);
    }

    ramses::Effect& DynamicQuad_Base::CreateTestEffect(ramses::Scene& scene)
    {
        ramses::EffectDescription effectDesc;
        effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic::ModelViewProjectionMatrix);
        effectDesc.setVertexShaderFromFile("res/ramses-test-client-textured.vert");
        effectDesc.setFragmentShaderFromFile("res/ramses-test-client-textured.frag");
        ramses::Effect* effect = scene.createEffect(effectDesc);
        return *effect;
    }
}
