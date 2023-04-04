//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/ArrayResourceScene.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/UniformInput.h"
#include "ramses-client-api/Effect.h"
#include "ramses-client-api/GeometryBinding.h"
#include "ramses-client-api/ArrayResource.h"
#include "ramses-client-api/AttributeInput.h"

namespace ramses_internal
{
    ArrayResourceScene::ArrayResourceScene(ramses::Scene& scene, UInt32 state, const Vector3& cameraPosition)
        : IntegrationScene(scene, cameraPosition)
        , m_effect(*createEffect(state))
    {
        ramses::Appearance* appearance = m_scene.createAppearance(m_effect);

        m_meshNode = m_scene.createMeshNode("mesh");
        addMeshNodeToDefaultRenderGroup(*m_meshNode);
        m_meshNode->setAppearance(*appearance);

        createGeometry(m_effect, state);
    }

    ramses::Effect* ArrayResourceScene::createEffect(UInt32 state)
    {
        switch (state)
        {
        case ARRAY_RESOURCE_INTERLEAVED_SINGLE_ATTRIB:
            return getTestEffect("ramses-test-client-data-buffers-vec3");
        case ARRAY_RESOURCE_INTERLEAVED:
        case ARRAY_RESOURCE_INTERLEAVED_TWO_STRIDES:
        case ARRAY_RESOURCE_INTERLEAVED_START_VERTEX:
            return getTestEffect("ramses-test-client-data-buffers-interleaved");
        default:
            assert(false);
        }

        return nullptr;
    }

    void ArrayResourceScene::createGeometry(ramses::Effect& effect, UInt32 state)
    {
        m_geometry = m_scene.createGeometryBinding(effect);

        static const UInt32 indicesData[] = { 0, 1, 2 };
        auto dataBuffer = m_scene.createArrayResource(3u, indicesData);
        m_geometry->setIndices(*dataBuffer);

        switch (state)
        {
        case ARRAY_RESOURCE_INTERLEAVED:
            createVertexArrayInterleaved();
            break;
        case ARRAY_RESOURCE_INTERLEAVED_TWO_STRIDES:
            createVertexArrayInterleavedTwoStrides();
            break;
        case ARRAY_RESOURCE_INTERLEAVED_SINGLE_ATTRIB:
            createVertexArrayInterleavedSingleAttrib();
            break;
        case ARRAY_RESOURCE_INTERLEAVED_START_VERTEX:
            createVertexArrayInterleavedStartVertex();
            m_meshNode->setStartVertex(1);
            break;
        default:
            assert(false);
        }

        m_meshNode->setGeometryBinding(*m_geometry);
        m_meshNode->setIndexCount(3);
    }

    void ArrayResourceScene::createVertexArrayInterleaved()
    {
        static const float vertices[] =
        {
            0.f,  0.f,  0.f, 1.0f,
            0.f, -3.f,  0.f, 1.0f,
            3.f, -3.f,  0.f, 1.0f,
        };

        auto arrayResource = m_scene.createArrayResource(sizeof(vertices), reinterpret_cast<const ramses::Byte*>(vertices));

        constexpr uint16_t stride = 4 * sizeof(float);
        ramses::AttributeInput inputX;
        m_effect.findAttributeInput("a_positionX", inputX);
        ramses::AttributeInput inputYZ;
        m_effect.findAttributeInput("a_positionYZ", inputYZ);
        ramses::AttributeInput inputW;
        m_effect.findAttributeInput("a_positionW", inputW);
        m_geometry->setInputBuffer(inputX, *arrayResource, 0u, stride);
        m_geometry->setInputBuffer(inputYZ, *arrayResource, sizeof(float), stride);
        m_geometry->setInputBuffer(inputW, *arrayResource, 3 * sizeof(float), stride);
    }

    static constexpr float UNUSEDVALUE = std::numeric_limits<float>::max();

    void ArrayResourceScene::createVertexArrayInterleavedTwoStrides()
    {
        //Create vertices for a triangle where the attributes for X and W components use different stride from
        //the attribute for YZ component. For clarify values which should be ignored will be filled with "UNUSEDVALUE"
        static const float vertices[] =
        {
            0.f         ,  0.f,  0.f                    , UNUSEDVALUE   , //x0, yz0 , _
            UNUSEDVALUE , -3.f,  0.f                    , 1.0f          , //_ , yz1 , w0
            0.f         , -3.f,  0.f                    , UNUSEDVALUE   , //x1, yz2 , _
            UNUSEDVALUE , UNUSEDVALUE   , UNUSEDVALUE   , 1.0f          , //_ , _ _ , w1
            3.f         , UNUSEDVALUE   , UNUSEDVALUE   , UNUSEDVALUE   , //x2, _ _ , _
            UNUSEDVALUE , UNUSEDVALUE   , UNUSEDVALUE   , 1.0f            //_ , _ _ , w2
        };

        auto arrayResource = m_scene.createArrayResource(sizeof(vertices), reinterpret_cast<const ramses::Byte*>(vertices));

        constexpr uint16_t strideXAndW = 8 * sizeof(float);
        constexpr uint16_t strideYZ = 4 * sizeof(float);

        ramses::AttributeInput inputX;
        m_effect.findAttributeInput("a_positionX", inputX);
        ramses::AttributeInput inputYZ;
        m_effect.findAttributeInput("a_positionYZ", inputYZ);
        ramses::AttributeInput inputW;
        m_effect.findAttributeInput("a_positionW", inputW);
        m_geometry->setInputBuffer(inputX, *arrayResource, 0u, strideXAndW);
        m_geometry->setInputBuffer(inputYZ, *arrayResource, sizeof(float), strideYZ);
        m_geometry->setInputBuffer(inputW, *arrayResource, 7 * sizeof(float), strideXAndW);
    }

    void ArrayResourceScene::createVertexArrayInterleavedSingleAttrib()
    {
        static const float vertices[] =
        {
            UNUSEDVALUE, 0.f,  0.f,  0.f,
            UNUSEDVALUE, 0.f, -3.f,  0.f,
            UNUSEDVALUE, 3.f, -3.f,  0.f,
        };

        auto arrayResource = m_scene.createArrayResource(sizeof(vertices), reinterpret_cast<const ramses::Byte*>(vertices));

        constexpr uint16_t stride = 4 * sizeof(float);
        ramses::AttributeInput input;
        m_effect.findAttributeInput("a_position", input);
        m_geometry->setInputBuffer(input, *arrayResource, sizeof(float), stride);
    }

    void ArrayResourceScene::createVertexArrayInterleavedStartVertex()
    {
        static const float vertices[] =
        {
            UNUSEDVALUE, UNUSEDVALUE, UNUSEDVALUE, UNUSEDVALUE,
            0.f,  0.f,  0.f, 1.0f,
            0.f, -3.f,  0.f, 1.0f,
            3.f, -3.f,  0.f, 1.0f,
        };

        auto arrayResource = m_scene.createArrayResource(sizeof(vertices), reinterpret_cast<const ramses::Byte*>(vertices));

        constexpr uint16_t stride = 4 * sizeof(float);
        ramses::AttributeInput inputX;
        m_effect.findAttributeInput("a_positionX", inputX);
        ramses::AttributeInput inputYZ;
        m_effect.findAttributeInput("a_positionYZ", inputYZ);
        ramses::AttributeInput inputW;
        m_effect.findAttributeInput("a_positionW", inputW);
        m_geometry->setInputBuffer(inputX, *arrayResource, 0u, stride);
        m_geometry->setInputBuffer(inputYZ, *arrayResource, sizeof(float), stride);
        m_geometry->setInputBuffer(inputW, *arrayResource, 3 * sizeof(float), stride);
    }
}
