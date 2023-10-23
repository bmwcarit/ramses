//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/IndexArray32BitScene.h"
#include "ramses/client/Scene.h"
#include "ramses/client/ArrayResource.h"
#include "ramses/client/Geometry.h"
#include "ramses/client/Appearance.h"
#include "ramses/client/Effect.h"
#include "ramses/client/AttributeInput.h"
#include "ramses/client/UniformInput.h"
#include "internal/PlatformAbstraction/PlatformMemory.h"

namespace ramses::internal
{
    IndexArray32BitScene::IndexArray32BitScene(ramses::Scene& scene, uint32_t state, const glm::vec3& cameraPosition)
        : IntegrationScene(scene, cameraPosition)
    {
        ramses::Effect* effect = getTestEffect("ramses-test-client-basic");
        ramses::Appearance* appearance = m_scene.createAppearance(*effect);
        appearance->setInputValue(*effect->findUniformInput("color"), ramses::vec4f{ 1.f, 0.f, 1.f, 1.f });

        const ramses::vec3f translation =
        {
            -2.0f,  1.0f, -18.0f
        };

        m_meshNode = m_scene.createMeshNode("mesh");
        addMeshNodeToDefaultRenderGroup(*m_meshNode);

        ramses::Node* trafoNode = m_scene.createNode("transformation node");
        trafoNode->setTranslation({translation});

        m_meshNode->setParent(*trafoNode);
        m_meshNode->setAppearance(*appearance);

        createGeometry(*effect,state);
    }

    void IndexArray32BitScene::create16BitIndexArray(ramses::Geometry* geometry)
    {
        const std::array<uint16_t, 7> indicesData =
        {
            10000, 10001, 10002, 10000, 10001, 10002, 10003
        };

        const ramses::ArrayResource* indicesTri  = m_scene.createArrayResource(7u, indicesData.data());

        geometry->setIndices(*indicesTri);
    }

    void IndexArray32BitScene::create32BitIndexArray(ramses::Geometry* geometry)
    {
        const std::array<uint32_t, 7> indicesData =
        {
            65536, 65537, 65538, 65536, 65537, 65538, 65539
        };

        const ramses::ArrayResource* indicesTri  = m_scene.createArrayResource(7u, indicesData.data());

        geometry->setIndices(*indicesTri);
    }

    void IndexArray32BitScene::createGeometry(ramses::Effect& effect, uint32_t state)
    {
        std::vector<ramses::vec3f> vertexPositionsArray;
        vertexPositionsArray.resize(65540u, ramses::vec3f{ 0.f, 0.f, 0.f });

        const std::array<ramses::vec3f, 4u> triangleVertices
        {
            ramses::vec3f{ 0.f,  0.f,  0.f },
            ramses::vec3f{ 0.f, -3.f,  0.f },
            ramses::vec3f{ 3.f, -3.f,  0.f },
            ramses::vec3f{ 3.f,  0.f,  0.f }
        };

        for(uint32_t i = 0u; i < 4u; ++i)
        {
            vertexPositionsArray[65536 + i] = triangleVertices[i];
            vertexPositionsArray[10000 + i] = triangleVertices[i];
        }

        const ramses::ArrayResource* verticesTri  = m_scene.createArrayResource(65540u, vertexPositionsArray.data());

        ramses::Geometry* geometryTriangle = m_scene.createGeometry(effect);

        switch(state)
        {
            case NO_OFFSET_32BIT_INDICES:
            case OFFSET_32BIT_INDICES:
                create32BitIndexArray(geometryTriangle);
                break;
            case NO_OFFSET_16BIT_INDICES:
            case OFFSET_16BIT_INDICES:
            default:
                create16BitIndexArray(geometryTriangle);
                break;
        }

        geometryTriangle->setInputBuffer(*effect.findAttributeInput("a_position"), *verticesTri);

        m_meshNode->setGeometry(*geometryTriangle);

        //with offset applied mirrored triangle is drawn
        if(state == OFFSET_32BIT_INDICES || state == OFFSET_16BIT_INDICES)
        {
            m_meshNode->setStartIndex(4);
        }
        else
        {
            m_meshNode->setStartIndex(0);
        }
        m_meshNode->setIndexCount(3);
    }
}
