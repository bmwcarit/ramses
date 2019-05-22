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
#include "ramses-client-api/IndexDataBuffer.h"
#include "ramses-client-api/VertexDataBuffer.h"
#include "ramses-client-api/TextureSampler.h"
#include "ramses-client-api/UniformInput.h"
#include "ramses-client-api/AttributeInput.h"

#include "TestRandom.h"
#include <memory>

namespace ramses_internal
{
    DynamicQuad_SceneResources::DynamicQuad_SceneResources(ramses::RamsesClient& client, ramses::Scene& scene, const ScreenspaceQuad& screenspaceQuad)
        : DynamicQuad_Base(client, scene, screenspaceQuad)
        , m_textureBuffer   (m_scene.createTexture2DBuffer(1u, DynamicTextureWidth, DynamicTextureHeight, ramses::ETextureFormat_BGR8))
        , m_textureSampler  (m_scene.createTextureSampler(ramses::ETextureAddressMode_Repeat, ramses::ETextureAddressMode_Repeat, ramses::ETextureSamplingMethod_Linear_MipMapLinear, ramses::ETextureSamplingMethod_Linear, *m_textureBuffer))
        , m_indices         (m_scene.createIndexDataBuffer(sizeof(uint16_t) * 4, ramses::EDataType_UInt16))
        , m_texCoords       (m_scene.createVertexDataBuffer(sizeof(float) * 8, ramses::EDataType_Vector2F))
        , m_vertexPos       (m_scene.createVertexDataBuffer(sizeof(float) * 12, ramses::EDataType_Vector3F))
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

        m_indices->setData(reinterpret_cast<const char*>(indicesData), sizeof(indicesData), 0);
        m_vertexPos->setData(reinterpret_cast<const char*>(&vertexPositionsData[0].x), sizeof(vertexPositionsData), 0);
        m_texCoords->setData(reinterpret_cast<const char*>(vertexTexcoordsData), sizeof(vertexTexcoordsData), 0);

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

        m_textureBuffer->setData(reinterpret_cast<const char*>(rawData.get()), 0, 0, 0, DynamicTextureWidth, DynamicTextureHeight);
    }
}
