//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "DynamicQuad_Base.h"

#include "ramses-client-api/Scene.h"
#include "ramses-client-api/Effect.h"
#include "ramses-client-api/EffectDescription.h"
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/GeometryBinding.h"
#include "ramses-client-api/UniformInput.h"
#include "ramses-client-api/AttributeInput.h"
#include "ramses-client-api/UInt16Array.h"
#include "ramses-client-api/Vector2fArray.h"
#include "ramses-client-api/Vector3fArray.h"

#include "PlatformAbstraction/PlatformMath.h"
#include "TestRandom.h"

namespace ramses_internal
{
    DynamicQuad_Base::DynamicQuad_Base(ramses::RamsesClient& client, ramses::Scene& scene, const ScreenspaceQuad& screenspaceQuad)
        : m_client(client)
        , m_scene(scene)
        , m_screenspaceQuad(screenspaceQuad)
        , m_renderGroup(*m_scene.createRenderGroup())
        , m_meshNode(*m_scene.createMeshNode())
        , m_effect(CreateTestEffect(m_client))
        , m_appearance(*m_scene.createAppearance(m_effect))
        , m_geometryBinding(*m_scene.createGeometryBinding(m_effect))
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

        static const uint16_t indiceData[] = { 0, 1, 3, 2 };
        resources.indices = m_client.createConstUInt16Array(4, indiceData);

        Vector3 vertexPositionsData[] =
        {
            m_screenspaceQuad.getVertex(EScreenspaceQuadVertex::BottomLeft, 10u),
            m_screenspaceQuad.getVertex(EScreenspaceQuadVertex::BottomRight, 10u),
            m_screenspaceQuad.getVertex(EScreenspaceQuadVertex::TopRight, 10u),
            m_screenspaceQuad.getVertex(EScreenspaceQuadVertex::TopLeft, 10u)
        };

        float vertexTexcoordsData[] = {
            0.f, 0.f,
            1.f, 0.f,
            1.f, 1.f,
            0.f, 1.f
        };

        for (uint32_t t = 0; t < sizeof(vertexTexcoordsData) / sizeof(float); ++t)
        {
            vertexTexcoordsData[t] += 0.01f * static_cast<float>(TestRandom::Get(0, 10));
        }

        resources.texCoords = m_client.createConstVector2fArray(4, vertexTexcoordsData);
        resources.vertexPos = m_client.createConstVector3fArray(4, &vertexPositionsData[0].x);

        return resources;
    }

    void DynamicQuad_Base::destroyQuadResources(const QuadResources& quadResources)
    {
        m_client.destroy(*quadResources.indices);
        m_client.destroy(*quadResources.vertexPos);
        m_client.destroy(*quadResources.texCoords);
    }

    void DynamicQuad_Base::setQuadResources(const QuadResources& quad, ramses::TextureSampler& textureSampler)
    {
        ramses::AttributeInput positionsInput;
        ramses::AttributeInput texcoordInput;
        ramses::UniformInput textureInput;
        m_appearance.getEffect().findUniformInput("u_texture", textureInput);
        m_appearance.getEffect().findAttributeInput("a_position", positionsInput);
        m_appearance.getEffect().findAttributeInput("a_texcoord", texcoordInput);

        m_geometryBinding.setIndices(*quad.indices);
        m_geometryBinding.setInputBuffer(positionsInput, *quad.vertexPos);
        m_geometryBinding.setInputBuffer(texcoordInput, *quad.texCoords);

        m_appearance.setInputTexture(textureInput, textureSampler);
    }

    ramses::Effect& DynamicQuad_Base::CreateTestEffect(ramses::RamsesClient& client)
    {
        ramses::EffectDescription effectDesc;
        effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic_ModelViewProjectionMatrix);
        effectDesc.setVertexShaderFromFile("res/ramses-test-client-textured.vert");
        effectDesc.setFragmentShaderFromFile("res/ramses-test-client-textured.frag");
        ramses::Effect* effect = client.createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache);
        return *effect;
    }
}
