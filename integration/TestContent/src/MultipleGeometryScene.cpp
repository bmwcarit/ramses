//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/MultipleGeometryScene.h"
#include "ramses-client-api/RamsesClient.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/TransformationNode.h"
#include "ramses-client-api/Vector3fArray.h"
#include "ramses-client-api/GeometryBinding.h"
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/Effect.h"
#include "ramses-client-api/AttributeInput.h"
#include "ramses-client-api/UniformInput.h"

namespace ramses_internal
{
    MultipleGeometryScene::MultipleGeometryScene(ramses::RamsesClient& ramsesClient, ramses::Scene& scene, UInt32 state, const Vector3& cameraPosition)
        : IntegrationScene(ramsesClient, scene, cameraPosition)
    {
        ramses::Effect* effect = getTestEffect("ramses-test-client-basic");
        ramses::Appearance* appearance = m_scene.createAppearance(*effect);
        ramses::UniformInput colorInput;
        effect->findUniformInput("color", colorInput);
        appearance->setInputValueVector4f(colorInput, 1.f, 0.f, 1.f, 1.f);
        if (MULTI_TRIANGLE_STRIP_GEOMETRY_WITHOUT_INDEX_ARRAY == state)
        {
            appearance->setDrawMode(ramses::EDrawMode_TriangleStrip);
        }

        static const float translation[] =
        {
            -2.0f,  1.0f, -18.0f,
            -0.5f, -1.0f, -18.0f,
            1.0f,   1.0f, -18.0f,
            -0.5f,  1.0f, -18.0f,
        };

        for (UInt32 i = 0; i < NumMeshes; ++i)
        {
            m_meshNode[i] = m_scene.createMeshNode("mesh");
            addMeshNodeToDefaultRenderGroup(*m_meshNode[i]);

            ramses::TransformationNode* trafoNode = m_scene.createTransformationNode("transformation node");
            trafoNode->setTranslation(translation[i * 3 + 0], translation[i * 3 + 1], translation[i * 3 + 2]);

            m_meshNode[i]->setParent(*trafoNode);
            m_meshNode[i]->setAppearance(*appearance);
        }

        createGeometries(*effect, state);
    }

    void MultipleGeometryScene::createGeometries(ramses::Effect& effect, UInt32 state)
    {
        const ramses::UInt16Array* indicesTri  = nullptr;
        const ramses::UInt16Array* indicesQuad = nullptr;
        const ramses::UInt16Array* indicesPoly = nullptr;

        if (MULTI_TRIANGLE_LIST_GEOMETRY_WITH_INDEX_ARRAY == state)
        {
            static const UInt16 indicesData[] =
            {
                0, 1, 2,
                0, 2, 3,
                0, 3, 4
            };

            indicesTri  = m_client.createConstUInt16Array(3, indicesData);
            indicesQuad = m_client.createConstUInt16Array(6, indicesData);
            indicesPoly = m_client.createConstUInt16Array(9, indicesData);
        }

        const ramses::Vector3fArray* verticesTri  = nullptr;
        const ramses::Vector3fArray* verticesQuad = nullptr;
        const ramses::Vector3fArray* verticesPoly = nullptr;

        switch (state)
        {
        case MULTI_TRIANGLE_LIST_GEOMETRY_WITH_INDEX_ARRAY:
        {
            static const float verticesData[] =
            {
                0.0f,  0.0f,  0.0f,
                0.0f, -1.0f,  0.0f,
                1.0f, -1.0f,  0.0f,
                1.0f,  0.0f,  0.0f,
                0.5f,  0.5f,  0.0f
            };
            verticesTri  = m_client.createConstVector3fArray(3, verticesData);
            verticesQuad = m_client.createConstVector3fArray(4, verticesData);
            verticesPoly = m_client.createConstVector3fArray(5, verticesData);
            break;
        }
        case MULTI_TRIANGLE_LIST_GEOMETRY_WITHOUT_INDEX_ARRAY:
        {
            static const float verticesData[] =
            {
                0.0f,  0.0f,  0.0f,
                0.0f, -1.0f,  0.0f,
                1.0f, -1.0f,  0.0f,
                0.0f,  0.0f,  0.0f,
                1.0f, -1.0f,  0.0f,
                1.0f,  0.0f,  0.0f,
                0.0f,  0.0f,  0.0f,
                1.0f,  0.0f,  0.0f,
                0.5f,  0.5f,  0.0f
            };
            verticesTri  = m_client.createConstVector3fArray(3, verticesData);
            verticesQuad = m_client.createConstVector3fArray(6, verticesData);
            verticesPoly = m_client.createConstVector3fArray(9, verticesData);
            break;
        }
        case MULTI_TRIANGLE_STRIP_GEOMETRY_WITHOUT_INDEX_ARRAY:
        {
            static const float verticesData[] =
            {
                    0.0f, -1.0f,  0.0f,
                    1.0f, -1.0f,  0.0f,
                    0.0f,  0.0f,  0.0f,
                    1.0f,  0.0f,  0.0f,
                    0.5f,  0.5f,  0.0f
            };
            verticesTri  = m_client.createConstVector3fArray(3, verticesData);
            verticesQuad = m_client.createConstVector3fArray(4, verticesData);
            verticesPoly = m_client.createConstVector3fArray(5, verticesData);
            break;
        }
        }

        ramses::AttributeInput positionsInput;
        effect.findAttributeInput("a_position", positionsInput);

        ramses::GeometryBinding* geometryTriangle = m_scene.createGeometryBinding(effect);
        ramses::GeometryBinding* geometryQuad     = m_scene.createGeometryBinding(effect);
        ramses::GeometryBinding* geometryPolygon  = m_scene.createGeometryBinding(effect);

        if (MULTI_TRIANGLE_LIST_GEOMETRY_WITH_INDEX_ARRAY == state)
        {
            geometryTriangle->setIndices(*indicesTri);
            geometryQuad->setIndices(*indicesQuad);
            geometryPolygon->setIndices(*indicesPoly);
        }

        geometryTriangle->setInputBuffer(positionsInput, *verticesTri);
        geometryQuad->setInputBuffer(positionsInput, *verticesQuad);
        geometryPolygon->setInputBuffer(positionsInput, *verticesPoly);

        m_meshNode[0]->setGeometryBinding(*geometryTriangle);
        m_meshNode[1]->setGeometryBinding(*geometryQuad);
        m_meshNode[2]->setGeometryBinding(*geometryPolygon);
        m_meshNode[3]->setGeometryBinding(*geometryPolygon);

        switch (state)
        {
        case MULTI_TRIANGLE_LIST_GEOMETRY_WITHOUT_INDEX_ARRAY:
        {
            m_meshNode[0]->setIndexCount(3);
            m_meshNode[1]->setIndexCount(6);
            m_meshNode[2]->setIndexCount(9);
            m_meshNode[3]->setIndexCount(9);
            break;
        }
        case MULTI_TRIANGLE_STRIP_GEOMETRY_WITHOUT_INDEX_ARRAY:
        {
            m_meshNode[0]->setIndexCount(3);
            m_meshNode[1]->setIndexCount(4);
            m_meshNode[2]->setIndexCount(5);
            m_meshNode[3]->setIndexCount(5);
            break;
        }
        }
    }
}
