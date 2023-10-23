//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/ArrayBufferScene.h"
#include "TestScenes/Triangle.h"
#include "ramses/client/ramses-utils.h"

#include "ramses/client/Scene.h"
#include "ramses/client/Appearance.h"
#include "ramses/client/UniformInput.h"
#include "ramses/client/Effect.h"
#include "internal/SceneGraph/SceneAPI/EDataType.h"
#include "ramses/client/Geometry.h"
#include "ramses/client/ArrayBuffer.h"
#include "ramses/client/AttributeInput.h"

namespace ramses::internal
{

    ArrayBufferScene::ArrayBufferScene(ramses::Scene& scene, uint32_t state, const glm::vec3& cameraPosition, uint32_t vpWidth, uint32_t vpHeight)
        : IntegrationScene(scene, cameraPosition, vpWidth, vpHeight)
        , m_effect(*createEffect(state))
    {
        ramses::Appearance* appearance = m_scene.createAppearance(m_effect);

        m_meshNode = m_scene.createMeshNode("mesh");
        addMeshNodeToDefaultRenderGroup(*m_meshNode);

        m_meshNode->setAppearance(*appearance);

        createGeometry(m_effect, state);
    }

    void ArrayBufferScene::setState(uint32_t state)
    {
        switch (state)
        {
        case UPDATE_INDEX_DATA_BUFFER:
        {
            assert(nullptr != m_indexDataBufferUInt32);
            static const uint32_t updatedIndicesData[] =
            {
                3, 0
            };

            m_indexDataBufferUInt32->updateData(0u, 2u, updatedIndicesData);
            m_scene.flush();
            break;
        }
        case UPDATE_VERTEX_DATA_BUFFER:
        {
            assert(nullptr != m_vertexDataBufferVec4);

            static const std::array<ramses::vec4f, 3u> updatedVertices
            {
                ramses::vec4f{ 3.f,  0.f,  0.f, 1.0f },
                ramses::vec4f{ 0.f,  0.f,  0.f, 1.0f },
                ramses::vec4f{ 3.f, -3.f,  0.f, 1.0f },
            };
            m_vertexDataBufferVec4->updateData(0u, 3u, updatedVertices.data());
            m_scene.flush();
            break;
        }
        case UPDATE_INTERLEAVED_VERTEX_DATA_BUFFER:
        {
            assert(nullptr != m_vertexDataBufferInterleaved);

            //new vertices values
            //{
            //     3.f /*updated*/    , 0.f               , 0.f, 1.0f,
            //     0.f                , 0.f /*updated*/   , 0.f, 1.0f,
            //     3.f /*updated*/    , -3.f              , 0.f, 1.0f,
            //};
            const float val3p0 = 3.f;
            const float valZero = 0.f;
            m_vertexDataBufferInterleaved->updateData(0u                , sizeof(float), reinterpret_cast<const std::byte*>(&val3p0));
            m_vertexDataBufferInterleaved->updateData(5 * sizeof(float) , sizeof(float), reinterpret_cast<const std::byte*>(&valZero));
            m_vertexDataBufferInterleaved->updateData(8 * sizeof(float) , sizeof(float), reinterpret_cast<const std::byte*>(&val3p0));
            m_scene.flush();
            break;
        }
        case VERTEX_ARRAY_RESOURCE_VECTOR4F:
        {
            createVertexArrayResourceVector4F();
            m_scene.flush();
            break;
        }
        case VERTEX_DATA_BUFFER_VECTOR4F:
        {
            createVertexDataBufferVector4F();
            m_scene.flush();
            break;

        }
        default:
            assert(false);
        }
    }

    ramses::Effect* ArrayBufferScene::createEffect(uint32_t state)
    {
        switch (state)
        {
        case VERTEX_DATA_BUFFER_FLOAT:
            return getTestEffect("ramses-test-client-data-buffers-float");
        case VERTEX_DATA_BUFFER_VECTOR2F:
            return getTestEffect("ramses-test-client-data-buffers-vec2");
        case INDEX_DATA_BUFFER_UINT16:
        case VERTEX_DATA_BUFFER_VECTOR3F:
        case VERTEX_DATA_BUFFER_INTERLEAVED_SINGLE_ATTRIB:
            return getTestEffect("ramses-test-client-data-buffers-vec3");
        case INDEX_DATA_BUFFER_UINT32:
        case VERTEX_DATA_BUFFER_VECTOR4F:
        case VERTEX_ARRAY_RESOURCE_VECTOR4F:
            return getTestEffect("ramses-test-client-data-buffers-vec4");
        case VERTEX_DATA_BUFFER_INTERLEAVED:
        case VERTEX_DATA_BUFFER_INTERLEAVED_TWO_STRIDES:
        case VERTEX_DATA_BUFFER_INTERLEAVED_START_VERTEX:
            return getTestEffect("ramses-test-client-data-buffers-interleaved");
        default:
            assert(false);
        }

        return nullptr;
    }

    void ArrayBufferScene::createGeometry(ramses::Effect& effect, uint32_t state)
    {
        m_geometry = m_scene.createGeometry(effect);

        switch (state)
        {
        case INDEX_DATA_BUFFER_UINT16:
            createIndexDataBufferUInt16();
            createVertexDataBufferVector3F();
            break;
        case INDEX_DATA_BUFFER_UINT32:
            createIndexDataBufferUInt32();
            createVertexDataBufferVector4F();
            break;
        case VERTEX_DATA_BUFFER_FLOAT:
            createIndexDataBufferUInt32();
            createVertexDataBufferFloat();
            break;
        case VERTEX_DATA_BUFFER_VECTOR2F:
            createIndexDataBufferUInt32();
            createVertexDataBufferVector2F();
            break;
        case VERTEX_DATA_BUFFER_VECTOR3F:
            createIndexDataBufferUInt32();
            createVertexDataBufferVector3F();
            break;
        case VERTEX_DATA_BUFFER_VECTOR4F:
            createIndexDataBufferUInt32();
            createVertexDataBufferVector4F();
            break;
        case VERTEX_ARRAY_RESOURCE_VECTOR4F:
            createIndexDataBufferUInt32();
            createVertexArrayResourceVector4F();
            break;
        case VERTEX_DATA_BUFFER_INTERLEAVED:
            createIndexDataBufferUInt32();
            createVertexArrayBufferInterleaved();
            break;
        case VERTEX_DATA_BUFFER_INTERLEAVED_TWO_STRIDES:
            createIndexDataBufferUInt32();
            createVertexArrayBufferInterleavedTwoStrides();
            break;
        case VERTEX_DATA_BUFFER_INTERLEAVED_SINGLE_ATTRIB:
            createIndexDataBufferUInt32();
            createVertexArrayBufferInterleavedSingleAttrib();
            break;
        case VERTEX_DATA_BUFFER_INTERLEAVED_START_VERTEX:
            createIndexDataBufferUInt32();
            createVertexArrayBufferInterleavedStartVertex();
            m_meshNode->setStartVertex(1);
            break;
        default:
            assert(false);
        }


        m_meshNode->setGeometry(*m_geometry);

        m_meshNode->setIndexCount(3);
    }

    void ArrayBufferScene::createIndexDataBufferUInt16()
    {
        static const uint16_t indicesData[] =
        {
            0, 1, 2
        };

        ramses::ArrayBuffer* dataBuffer = m_scene.createArrayBuffer(ramses::EDataType::UInt16, 3u);
        dataBuffer->updateData(0u, 3u, indicesData);
        m_geometry->setIndices(*dataBuffer);
    }

    void ArrayBufferScene::createIndexDataBufferUInt32()
    {
        static const uint32_t indicesData[] =
        {
            0, 1, 2
        };

        ramses::ArrayBuffer* dataBuffer = m_scene.createArrayBuffer(ramses::EDataType::UInt32, 3u);
        dataBuffer->updateData(0u, 3u, indicesData);
        m_geometry->setIndices(*dataBuffer);

        m_indexDataBufferUInt32 = dataBuffer;
    }

    void ArrayBufferScene::createVertexDataBufferFloat()
    {
        static const float verticesX[] =
        {
            0.f,
            0.f,
            3.f,
            3.f
        };

        static const float verticesY[] =
        {
            0.f,
            -3.f,
            -3.f,
            0.f
        };

        ramses::ArrayBuffer* dataBufferX = m_scene.createArrayBuffer(ramses::EDataType::Float, 4u);
        dataBufferX->updateData(0u, 4u, verticesX);
        ramses::ArrayBuffer* dataBufferY = m_scene.createArrayBuffer(ramses::EDataType::Float, 4u);
        dataBufferY->updateData(0u, 4u, verticesY);

        m_geometry->setInputBuffer(*m_effect.findAttributeInput("a_positionX"), *dataBufferX);
        m_geometry->setInputBuffer(*m_effect.findAttributeInput("a_positionY"), *dataBufferY);
    }

    void ArrayBufferScene::createVertexDataBufferVector2F()
    {
        static const std::array<ramses::vec2f, 4u> vertices
        {
            ramses::vec2f{ 0.f,  0.f },
            ramses::vec2f{ 0.f, -3.f },
            ramses::vec2f{ 3.f, -3.f },
            ramses::vec2f{ 3.f,  0.f }
        };

        ramses::ArrayBuffer* dataBuffer = m_scene.createArrayBuffer(ramses::EDataType::Vector2F, 8u);
        dataBuffer->updateData(0u, 4u, vertices.data());
        m_geometry->setInputBuffer(*m_effect.findAttributeInput("a_position"), *dataBuffer);
    }

    void ArrayBufferScene::createVertexDataBufferVector3F()
    {
        static const std::array<ramses::vec3f, 4u> vertices
        {
            ramses::vec3f{ 0.f,  0.f,  0.f },
            ramses::vec3f{ 0.f, -3.f,  0.f },
            ramses::vec3f{ 3.f, -3.f,  0.f },
            ramses::vec3f{ 3.f,  0.f,  0.f }
        };

        ramses::ArrayBuffer* dataBuffer = m_scene.createArrayBuffer(ramses::EDataType::Vector3F, 12u);
        dataBuffer->updateData(0u, 4u, vertices.data());

        m_geometry->setInputBuffer(*m_effect.findAttributeInput("a_position"), *dataBuffer);
    }

    void ArrayBufferScene::createVertexDataBufferVector4F()
    {
        static const std::array<ramses::vec4f, 4u> vertices
        {
            ramses::vec4f{ 0.f,  0.f,  0.f, 1.0f },
            ramses::vec4f{ 0.f, -3.f,  0.f, 1.0f },
            ramses::vec4f{ 3.f, -3.f,  0.f, 1.0f },
            ramses::vec4f{ 3.f,  0.f,  0.f, 1.0f }
        };

        ramses::ArrayBuffer* dataBuffer = m_scene.createArrayBuffer(ramses::EDataType::Vector4F, 16u);
        dataBuffer->updateData(0u, 4u, vertices.data());
        m_geometry->setInputBuffer(*m_effect.findAttributeInput("a_position"), *dataBuffer);

        m_vertexDataBufferVec4 = dataBuffer;
    }

    void ArrayBufferScene::createVertexArrayResourceVector4F()
    {
        static const std::array<ramses::vec4f, 3u> vertices
        {
            ramses::vec4f{ 3.f,  0.f,  0.f, 1.0f },
            ramses::vec4f{ 0.f,  0.f,  0.f, 1.0f },
            ramses::vec4f{ 1.5f, -3.f,  0.f, 1.0f }
        };

        const ramses::ArrayResource* arrayBuffer = m_scene.createArrayResource(3u, vertices.data());
        m_geometry->setInputBuffer(*m_effect.findAttributeInput("a_position"), *arrayBuffer);
    }

    void ArrayBufferScene::createVertexArrayBufferInterleaved()
    {
        static const float vertices[] =
        {
            0.f,  0.f,  0.f, 1.0f,
            0.f, -3.f,  0.f, 1.0f,
            3.f, -3.f,  0.f, 1.0f,
        };

        ramses::ArrayBuffer* arrayBuffer = m_scene.createArrayBuffer(ramses::EDataType::ByteBlob, sizeof(vertices));
        arrayBuffer->updateData(0u, sizeof(vertices), reinterpret_cast<const std::byte*>(vertices));

        constexpr uint16_t stride = 4 * sizeof(float);
        m_geometry->setInputBuffer(*m_effect.findAttributeInput("a_positionX"), *arrayBuffer, 0u, stride);
        m_geometry->setInputBuffer(*m_effect.findAttributeInput("a_positionYZ"), *arrayBuffer, sizeof(float), stride);
        m_geometry->setInputBuffer(*m_effect.findAttributeInput("a_positionW"), *arrayBuffer, 3 * sizeof(float), stride);

        m_vertexDataBufferInterleaved = arrayBuffer;
    }

    static constexpr float UNUSEDVALUE = std::numeric_limits<float>::max();

    void ArrayBufferScene::createVertexArrayBufferInterleavedTwoStrides()
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

        ramses::ArrayBuffer* arrayBuffer = m_scene.createArrayBuffer(ramses::EDataType::ByteBlob, sizeof(vertices));
        arrayBuffer->updateData(0u, sizeof(vertices), reinterpret_cast<const std::byte*>(vertices));

        constexpr uint16_t strideXAndW = 8 * sizeof(float);
        constexpr uint16_t strideYZ = 4 * sizeof(float);
        m_geometry->setInputBuffer(*m_effect.findAttributeInput("a_positionX"), *arrayBuffer, 0u, strideXAndW);
        m_geometry->setInputBuffer(*m_effect.findAttributeInput("a_positionYZ"), *arrayBuffer, sizeof(float), strideYZ);
        m_geometry->setInputBuffer(*m_effect.findAttributeInput("a_positionW"), *arrayBuffer, 7 * sizeof(float), strideXAndW);
    }

    void ArrayBufferScene::createVertexArrayBufferInterleavedSingleAttrib()
    {
        static const float vertices[] =
        {
            UNUSEDVALUE, 0.f,  0.f,  0.f,
            UNUSEDVALUE, 0.f, -3.f,  0.f,
            UNUSEDVALUE, 3.f, -3.f,  0.f,
        };

        ramses::ArrayBuffer* arrayBuffer = m_scene.createArrayBuffer(ramses::EDataType::ByteBlob, sizeof(vertices));
        arrayBuffer->updateData(0u, sizeof(vertices), reinterpret_cast<const std::byte*>(vertices));

        constexpr uint16_t stride = 4 * sizeof(float);
        m_geometry->setInputBuffer(*m_effect.findAttributeInput("a_position"), *arrayBuffer, sizeof(float), stride);

        m_vertexDataBufferInterleaved = arrayBuffer;
    }

    void ArrayBufferScene::createVertexArrayBufferInterleavedStartVertex()
    {
        static const float vertices[] =
        {
            UNUSEDVALUE, UNUSEDVALUE, UNUSEDVALUE, UNUSEDVALUE,
            0.f,  0.f,  0.f, 1.0f,
            0.f, -3.f,  0.f, 1.0f,
            3.f, -3.f,  0.f, 1.0f,
        };

        ramses::ArrayBuffer* arrayBuffer = m_scene.createArrayBuffer(ramses::EDataType::ByteBlob, sizeof(vertices));
        arrayBuffer->updateData(0u, sizeof(vertices), reinterpret_cast<const std::byte*>(vertices));

        constexpr uint16_t stride = 4 * sizeof(float);
        m_geometry->setInputBuffer(*m_effect.findAttributeInput("a_positionX"), *arrayBuffer, 0u, stride);
        m_geometry->setInputBuffer(*m_effect.findAttributeInput("a_positionYZ"), *arrayBuffer, sizeof(float), stride);
        m_geometry->setInputBuffer(*m_effect.findAttributeInput("a_positionW"), *arrayBuffer, 3 * sizeof(float), stride);

        m_vertexDataBufferInterleaved = arrayBuffer;
    }
}
