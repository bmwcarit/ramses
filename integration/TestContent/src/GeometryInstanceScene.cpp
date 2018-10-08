//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/GeometryInstanceScene.h"
#include "ramses-client-api/RamsesClient.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/TransformationNode.h"
#include "ramses-client-api/Vector3fArray.h"
#include "ramses-client-api/GeometryBinding.h"
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/Effect.h"
#include "ramses-client-api/AttributeInput.h"
#include "ramses-client-api/UniformInput.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/TranslateNode.h"

namespace ramses_internal
{
    GeometryInstanceScene::GeometryInstanceScene(ramses::RamsesClient& ramsesClient, ramses::Scene& scene, UInt32 state, const Vector3& cameraPosition)
        : IntegrationScene(ramsesClient, scene, cameraPosition)
    {
        ramses::Effect* effect = NULL;
        switch (state)
        {
        case GEOMETRY_INSTANCE_UNIFORM:
            effect = getTestEffect("ramses-test-instancing-uniform");
            break;
        case GEOMETRY_INSTANCE_VERTEX:
        case GEOMETRY_INSTANCE_AND_NOT_INSTANCE:
            effect = getTestEffect("ramses-test-instancing-vertex");
            break;
        default:
            assert(false);
        }

        ramses::Appearance* appearance = m_scene.createAppearance(*effect);

        {
            //create the instanced mesh
            ramses::MeshNode* meshNode = m_scene.createMeshNode("instanced_mesh");
            meshNode->setInstanceCount(NumInstances);
            addMeshNodeToDefaultRenderGroup(*meshNode, 0);
            meshNode->setAppearance(*appearance);

            ramses::GeometryBinding* geometry = createGeometry(*effect);
            if (GEOMETRY_INSTANCE_UNIFORM == state)
            {
                setInstancedUniforms(*appearance);
            }
            else
            {
                setInstancedAttributes(m_client, *effect, geometry, 1);
            }
            meshNode->setGeometryBinding(*geometry);
        }

        if (GEOMETRY_INSTANCE_AND_NOT_INSTANCE == state)
        {
            //create the not instanced mesh
            //Instancing is not enabled, so the translation and color buffers will advance per vertex.
            //This means, the vertices of the triangle will be one-one vertex of each individual triangle, with
            //the corresponding color. This will result in a weird looking triangle.
            ramses::MeshNode* notInstancedMeshNode = m_scene.createMeshNode("not_instanced_mesh");
            addMeshNodeToDefaultRenderGroup(*notInstancedMeshNode, 1);
            notInstancedMeshNode->setAppearance(*appearance);

            ramses::TranslateNode* translateNode = m_scene.createTranslateNode();
            translateNode->setTranslation(0.f, -1.5f, 0.f);
            notInstancedMeshNode->setParent(*translateNode);

            ramses::GeometryBinding* notInstancedGeometry = createGeometry(*effect);
            setInstancedAttributes(m_client, *effect, notInstancedGeometry, 0);
            notInstancedMeshNode->setGeometryBinding(*notInstancedGeometry);
        }
    }

    void GeometryInstanceScene::setInstancedUniforms(ramses::Appearance& appearance)
    {
        const ramses::Effect& effect = appearance.getEffect();

        ramses::UniformInput translationInput;
        effect.findUniformInput("translation", translationInput);

        ramses::UniformInput colorInput;
        effect.findUniformInput("color", colorInput);

        static const float translation[] =
        {
            -0.5f,  0.5f, 0.0f,
            0.0f,   0.0f, 1.0f,
            0.5f,  -0.5f, 2.0f
        };

        static const float color[] =
        {
            0.3f, 0.3f, 0.3f, 1.0f,
            1.0f, 0.0f, 0.0f, 1.0f,
            0.1f, 1.0f, 0.1f, 1.0f
        };

        appearance.setInputValueVector3f(translationInput, NumInstances, translation);
        appearance.setInputValueVector4f(colorInput, NumInstances, color);
    }

    void GeometryInstanceScene::setInstancedAttributes(ramses::RamsesClient& ramsesClient, const ramses::Effect& effect, ramses::GeometryBinding* geometry, UInt32 instancingDivisor)
    {
        ramses::AttributeInput translationInput;
        effect.findAttributeInput("translation", translationInput);

        ramses::AttributeInput colorInput;
        effect.findAttributeInput("color", colorInput);

        static const float translation[] =
        {
            -0.5f,  0.5f, 0.0f,
            0.0f,   0.0f, 1.0f,
            0.5f,  -0.5f, 2.0f
        };

        static const float color[] =
        {
            0.3f, 0.3f, 0.3f, 1.0f,
            1.0f, 0.0f, 0.0f, 1.0f,
            0.1f, 1.0f, 0.1f, 1.0f
        };

        const ramses::Vector3fArray* translationsArray = ramsesClient.createConstVector3fArray(3, translation);
        geometry->setInputBuffer(translationInput, *translationsArray, instancingDivisor);
        const ramses::Vector4fArray* colorArray = ramsesClient.createConstVector4fArray(3, color);
        geometry->setInputBuffer(colorInput, *colorArray, instancingDivisor);
    }

    ramses::GeometryBinding* GeometryInstanceScene::createGeometry(const ramses::Effect& effect)
    {
        static const float verticesData[] =
        {
            -1.0f,  0.f,  -15.0f,
            1.0f, 0.f,  -15.0f,
            0.0f, 1.f,  -15.0f
        };

        static const UInt16 indicesData[] =
        {
            0, 1, 2
        };

        const ramses::UInt16Array* indices  = m_client.createConstUInt16Array(NumInstances, indicesData);

        const ramses::Vector3fArray* vertices  = m_client.createConstVector3fArray(NumInstances, verticesData);

        ramses::AttributeInput positionsInput;
        effect.findAttributeInput("a_position", positionsInput);

        ramses::GeometryBinding* geometry = m_scene.createGeometryBinding(effect);

        geometry->setIndices(*indices);
        geometry->setInputBuffer(positionsInput, *vertices);

        return geometry;
    }
}
