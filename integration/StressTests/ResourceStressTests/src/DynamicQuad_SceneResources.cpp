//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "DynamicQuad_SceneResources.h"

#include "ramses-client-api/Scene.h"
#include "ramses-client-api/Effect.h"
#include "ramses-client-api/RenderPass.h"
#include "ramses-client-api/RenderGroup.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/GeometryBinding.h"
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/OrthographicCamera.h"
#include "ramses-client-api/Texture2DBuffer.h"
#include "ramses-client-api/ArrayBuffer.h"
#include "ramses-client-api/TextureSampler.h"
#include "ramses-client-api/UniformInput.h"
#include "ramses-client-api/AttributeInput.h"

#include "TestRandom.h"
#include <memory>

namespace ramses_internal
{
    DynamicQuad_SceneResources::DynamicQuad_SceneResources(ramses::Scene& scene, const ScreenspaceQuad& screenspaceQuad)
        : DynamicQuad_Base(scene, screenspaceQuad)
        , m_textureBuffer   (m_scene.createTexture2DBuffer(ramses::ETextureFormat::RGB8, DynamicTextureWidth, DynamicTextureHeight, 1u))
        , m_textureSampler  (m_scene.createTextureSampler(ramses::ETextureAddressMode_Repeat, ramses::ETextureAddressMode_Repeat, ramses::ETextureSamplingMethod_Linear_MipMapLinear, ramses::ETextureSamplingMethod_Linear, *m_textureBuffer))
        , m_indices         (m_scene.createArrayBuffer( ramses::EDataType::UInt16, 4))
        , m_texCoords       (m_scene.createArrayBuffer(ramses::EDataType::Vector2F, 8))
        , m_vertexPos       (m_scene.createArrayBuffer( ramses::EDataType::Vector3F, 12))
    {
        m_renderGroup.addMeshNode(m_meshNode);

        ramses::AttributeInput positionsInput;
        ramses::AttributeInput texcoordInput;
        ramses::UniformInput textureInput;
        m_appearance.getEffect().findUniformInput("u_texture", textureInput);
        m_appearance.getEffect().findAttributeInput("a_position", positionsInput);
        m_appearance.getEffect().findAttributeInput("a_texcoord", texcoordInput);

        m_geometryBinding.setIndices(*m_indices);
        m_geometryBinding.setInputBuffer(positionsInput, *m_vertexPos);
        m_geometryBinding.setInputBuffer(texcoordInput, *m_texCoords);

        m_appearance.setInputTexture(textureInput, *m_textureSampler);

        recreate();

        m_meshNode.setAppearance(m_appearance);
        m_meshNode.setIndexCount(4);
        m_appearance.setDrawMode(ramses::EDrawMode_TriangleStrip);
        m_meshNode.setGeometryBinding(m_geometryBinding);
    }

    DynamicQuad_SceneResources::~DynamicQuad_SceneResources()
    {
        if (nullptr != m_textureSampler)
        {
            m_scene.destroy(*m_textureSampler);
        }
        if (nullptr != m_indices)
        {
            m_scene.destroy(*m_indices);
        }
        if (nullptr != m_texCoords)
        {
            m_scene.destroy(*m_texCoords);
        }
        if (nullptr != m_vertexPos)
        {
            m_scene.destroy(*m_vertexPos);
        }
    }

    void DynamicQuad_SceneResources::markSceneObjectsDestroyed()
    {
        m_textureSampler = nullptr;
        m_indices = nullptr;
        m_texCoords = nullptr;
        m_vertexPos = nullptr;
    }

    void DynamicQuad_SceneResources::recreate()
    {
        static const uint16_t indicesData[] = { 0, 1, 3, 2 };

        // Vertex positions in normalized screen space, i.e. fraction of the screen (0.0f == bottom/left, 1.0f == top/right)
        const std::array<ramses::vec3f, 4u> vertexPositionsData
        {
            m_screenspaceQuad.getVertex(EScreenspaceQuadVertex::BottomLeft, 10u).getAsVec3(),
            m_screenspaceQuad.getVertex(EScreenspaceQuadVertex::BottomRight, 10u).getAsVec3(),
            m_screenspaceQuad.getVertex(EScreenspaceQuadVertex::TopRight, 10u).getAsVec3(),
            m_screenspaceQuad.getVertex(EScreenspaceQuadVertex::TopLeft, 10u).getAsVec3()
        };

        std::array<ramses::vec2f, 4u> vertexTexcoordsData
        {
            ramses::vec2f{ 0.f, 0.f },
            ramses::vec2f{ 1.f, 0.f },
            ramses::vec2f{ 1.f, 1.f },
            ramses::vec2f{ 0.f, 1.f }
        };

        for (auto& tc : vertexTexcoordsData)
        {
            tc[0] += 0.01f * static_cast<float>(TestRandom::Get(0, 10));
            tc[1] += 0.01f * static_cast<float>(TestRandom::Get(0, 10));
        }

        m_indices->updateData(0u, 4, indicesData);
        m_vertexPos->updateData(0u, 4, vertexPositionsData.data());
        m_texCoords->updateData(0u, 4, vertexTexcoordsData.data());

        std::unique_ptr<uint8_t[]> rawData(new uint8_t[DynamicTextureWidth * DynamicTextureHeight * 3]);

        for (uint32_t x = 0; x < DynamicTextureWidth; ++x)
        {
            for (uint32_t y = 0; y < DynamicTextureHeight; ++y)
            {
                rawData[3 * (y * DynamicTextureWidth + x) + 0] = static_cast<uint8_t>(TestRandom::Get(128, 255));
                rawData[3 * (y * DynamicTextureWidth + x) + 1] = 0u;
                rawData[3 * (y * DynamicTextureWidth + x) + 2] = static_cast<uint8_t>(TestRandom::Get(0, 128));
            }
        }

        m_textureBuffer->updateData(0, 0, 0, DynamicTextureWidth, DynamicTextureHeight, rawData.get());
    }
}
