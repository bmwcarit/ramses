//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/MultipleGeometryScene.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/ArrayResource.h"
#include "ramses-client-api/GeometryBinding.h"
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/Effect.h"
#include "ramses-client-api/AttributeInput.h"
#include "ramses-client-api/UniformInput.h"
#include <cassert>

namespace ramses_internal
{
    MultipleGeometryScene::MultipleGeometryScene(ramses::Scene& scene, UInt32 state, const Vector3& cameraPosition)
        : IntegrationScene(scene, cameraPosition)
    {
        ramses::Effect* effect = getTestEffect("ramses-test-client-basic");
        ramses::Appearance* appearance = m_scene.createAppearance(*effect);
        ramses::UniformInput colorInput;
        effect->findUniformInput("color", colorInput);
        appearance->setInputValue(colorInput, ramses::vec4f{ 1.f, 0.f, 1.f, 1.f });
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

            ramses::Node* trafoNode = m_scene.createNode("transformation node");
            trafoNode->setTranslation(translation[i * 3 + 0], translation[i * 3 + 1], translation[i * 3 + 2]);

            m_meshNode[i]->setParent(*trafoNode);
            m_meshNode[i]->setAppearance(*appearance);
        }

        createGeometries(*effect, state);
    }

    void MultipleGeometryScene::createGeometries(ramses::Effect& effect, UInt32 state)
    {
        const ramses::ArrayResource* indicesTri  = nullptr;
        const ramses::ArrayResource* indicesQuad = nullptr;
        const ramses::ArrayResource* indicesPoly = nullptr;

        if (MULTI_TRIANGLE_LIST_GEOMETRY_WITH_INDEX_ARRAY == state)
        {
            static const UInt16 indicesData[] =
            {
                0, 1, 2,
                0, 2, 3,
                0, 3, 4
            };

            indicesTri  = m_scene.createArrayResource(3, indicesData);
            indicesQuad = m_scene.createArrayResource(6, indicesData);
            indicesPoly = m_scene.createArrayResource(9, indicesData);
        }

        const ramses::ArrayResource* verticesTri  = nullptr;
        const ramses::ArrayResource* verticesQuad = nullptr;
        const ramses::ArrayResource* verticesPoly = nullptr;

        switch (state)
        {
        case MULTI_TRIANGLE_LIST_GEOMETRY_WITH_INDEX_ARRAY:
        {
            const std::array<ramses::vec3f, 5u> verticesData
            {
                ramses::vec3f{ 0.0f,  0.0f,  0.0f },
                ramses::vec3f{ 0.0f, -1.0f,  0.0f },
                ramses::vec3f{ 1.0f, -1.0f,  0.0f },
                ramses::vec3f{ 1.0f,  0.0f,  0.0f },
                ramses::vec3f{ 0.5f,  0.5f,  0.0f }
            };
            verticesTri = m_scene.createArrayResource(3u, verticesData.data());
            verticesQuad = m_scene.createArrayResource(4u, verticesData.data());
            verticesPoly = m_scene.createArrayResource(5u, verticesData.data());
            break;
        }
        case MULTI_TRIANGLE_LIST_GEOMETRY_WITHOUT_INDEX_ARRAY:
        {
            const std::array<ramses::vec3f, 9u> verticesData
            {
                ramses::vec3f{ 0.0f,  0.0f,  0.0f },
                ramses::vec3f{ 0.0f, -1.0f,  0.0f },
                ramses::vec3f{ 1.0f, -1.0f,  0.0f },
                ramses::vec3f{ 0.0f,  0.0f,  0.0f },
                ramses::vec3f{ 1.0f, -1.0f,  0.0f },
                ramses::vec3f{ 1.0f,  0.0f,  0.0f },
                ramses::vec3f{ 0.0f,  0.0f,  0.0f },
                ramses::vec3f{ 1.0f,  0.0f,  0.0f },
                ramses::vec3f{ 0.5f,  0.5f,  0.0f }
            };
            verticesTri  = m_scene.createArrayResource(3u, verticesData.data());
            verticesQuad = m_scene.createArrayResource(6u, verticesData.data());
            verticesPoly = m_scene.createArrayResource(9u, verticesData.data());
            break;
        }
        case MULTI_TRIANGLE_STRIP_GEOMETRY_WITHOUT_INDEX_ARRAY:
        {
            const std::array<ramses::vec3f, 5u> verticesData
            {
                ramses::vec3f{ 0.0f, -1.0f,  0.0f},
                ramses::vec3f{ 1.0f, -1.0f,  0.0f},
                ramses::vec3f{ 0.0f,  0.0f,  0.0f},
                ramses::vec3f{ 1.0f,  0.0f,  0.0f},
                ramses::vec3f{ 0.5f,  0.5f,  0.0f}
            };
            verticesTri  = m_scene.createArrayResource(3u, verticesData.data());
            verticesQuad = m_scene.createArrayResource(4u, verticesData.data());
            verticesPoly = m_scene.createArrayResource(5u, verticesData.data());
            break;
        }
        case VERTEX_ARRAYS_WITH_OFFSET:
        {
            const std::array<ramses::vec3f, 18u> verticesData
            {
                //triangle
                ramses::vec3f{ 0.0f, -1.0f,  0.0f },
                ramses::vec3f{ 1.0f, -1.0f,  0.0f },
                ramses::vec3f{ 0.0f,  0.0f,  0.0f },

                //quad
                ramses::vec3f{ 0.0f,  0.0f,  0.0f },
                ramses::vec3f{ 0.0f, -1.0f,  0.0f },
                ramses::vec3f{ 1.0f, -1.0f,  0.0f },
                ramses::vec3f{ 0.0f,  0.0f,  0.0f },
                ramses::vec3f{ 1.0f, -1.0f,  0.0f },
                ramses::vec3f{ 1.0f,  0.0f,  0.0f },

                //polygon
                ramses::vec3f{ 0.0f,  0.0f,  0.0f },
                ramses::vec3f{ 0.0f, -1.0f,  0.0f },
                ramses::vec3f{ 1.0f, -1.0f,  0.0f },
                ramses::vec3f{ 0.0f,  0.0f,  0.0f },
                ramses::vec3f{ 1.0f, -1.0f,  0.0f },
                ramses::vec3f{ 1.0f,  0.0f,  0.0f },
                ramses::vec3f{ 0.0f,  0.0f,  0.0f },
                ramses::vec3f{ 1.0f,  0.0f,  0.0f },
                ramses::vec3f{ 0.5f,  0.5f,  0.0f }
            };
            verticesTri = m_scene.createArrayResource(3 + 6 + 9, verticesData.data());
            verticesQuad = verticesTri;
            verticesPoly = verticesTri;
            break;
        }
        default:
            assert(false && "Invalid state");
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
        case VERTEX_ARRAYS_WITH_OFFSET:
        {
            m_meshNode[0]->setIndexCount(3);
            m_meshNode[1]->setIndexCount(6);
            m_meshNode[2]->setIndexCount(9);
            m_meshNode[3]->setIndexCount(9);

            m_meshNode[0]->setStartVertex(0);
            m_meshNode[1]->setStartVertex(3);
            m_meshNode[2]->setStartVertex(9);
            m_meshNode[3]->setStartVertex(9);
            break;
        }
        }
    }
}
