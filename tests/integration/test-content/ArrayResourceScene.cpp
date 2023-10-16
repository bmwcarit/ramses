//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/ArrayResourceScene.h"
#include "ramses/client/Scene.h"
#include "ramses/client/Appearance.h"
#include "ramses/client/UniformInput.h"
#include "ramses/client/Effect.h"
#include "ramses/client/Geometry.h"
#include "ramses/client/ArrayResource.h"
#include "ramses/client/AttributeInput.h"

namespace ramses::internal
{
    ArrayResourceScene::ArrayResourceScene(ramses::Scene& scene, uint32_t state, const glm::vec3& cameraPosition)
        : IntegrationScene(scene, cameraPosition)
        , m_effect(*createEffect(state))
    {
        ramses::Appearance* appearance = m_scene.createAppearance(m_effect);

        m_meshNode = m_scene.createMeshNode("mesh");
        addMeshNodeToDefaultRenderGroup(*m_meshNode);
        m_meshNode->setAppearance(*appearance);

        createGeometry(m_effect, state);
    }

    ramses::Effect* ArrayResourceScene::createEffect(uint32_t state)
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

    void ArrayResourceScene::createGeometry(ramses::Effect& effect, uint32_t state)
    {
        m_geometry = m_scene.createGeometry(effect);

        static const uint32_t indicesData[] = { 0, 1, 2 };
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

        m_meshNode->setGeometry(*m_geometry);
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

        auto arrayResource = m_scene.createArrayResource(sizeof(vertices), reinterpret_cast<const std::byte*>(vertices));

        constexpr uint16_t stride = 4 * sizeof(float);
        m_geometry->setInputBuffer(*m_effect.findAttributeInput("a_positionX"), *arrayResource, 0u, stride);
        m_geometry->setInputBuffer(*m_effect.findAttributeInput("a_positionYZ"), *arrayResource, sizeof(float), stride);
        m_geometry->setInputBuffer(*m_effect.findAttributeInput("a_positionW"), *arrayResource, 3 * sizeof(float), stride);
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

        auto arrayResource = m_scene.createArrayResource(sizeof(vertices), reinterpret_cast<const std::byte*>(vertices));

        constexpr uint16_t strideXAndW = 8 * sizeof(float);
        constexpr uint16_t strideYZ = 4 * sizeof(float);
        m_geometry->setInputBuffer(*m_effect.findAttributeInput("a_positionX"), *arrayResource, 0u, strideXAndW);
        m_geometry->setInputBuffer(*m_effect.findAttributeInput("a_positionYZ"), *arrayResource, sizeof(float), strideYZ);
        m_geometry->setInputBuffer(*m_effect.findAttributeInput("a_positionW"), *arrayResource, 7 * sizeof(float), strideXAndW);
    }

    void ArrayResourceScene::createVertexArrayInterleavedSingleAttrib()
    {
        static const float vertices[] =
        {
            UNUSEDVALUE, 0.f,  0.f,  0.f,
            UNUSEDVALUE, 0.f, -3.f,  0.f,
            UNUSEDVALUE, 3.f, -3.f,  0.f,
        };

        auto arrayResource = m_scene.createArrayResource(sizeof(vertices), reinterpret_cast<const std::byte*>(vertices));

        constexpr uint16_t stride = 4 * sizeof(float);
        m_geometry->setInputBuffer(*m_effect.findAttributeInput("a_position"), *arrayResource, sizeof(float), stride);
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

        auto arrayResource = m_scene.createArrayResource(sizeof(vertices), reinterpret_cast<const std::byte*>(vertices));

        constexpr uint16_t stride = 4 * sizeof(float);
        m_geometry->setInputBuffer(*m_effect.findAttributeInput("a_positionX"), *arrayResource, 0u, stride);
        m_geometry->setInputBuffer(*m_effect.findAttributeInput("a_positionYZ"), *arrayResource, sizeof(float), stride);
        m_geometry->setInputBuffer(*m_effect.findAttributeInput("a_positionW"), *arrayResource, 3 * sizeof(float), stride);
    }
}
